// BigMp4.cpp

#include "ppbox/peer/Common.h"
#include "ppbox/peer/BigMp4.h"
#include "ppbox/peer/Error.h"
#include "ppbox/demux/DemuxerError.h"

#include <ppbox/ppbox/Common.h>

#include <util/protocol/http/HttpRequest.h>
#include <util/archive/ArchiveBuffer.h>
#include <util/stream/StreamTransfer.h>
#include <util/protocol/pptv/TimeKey.h>
#include <util/protocol/pptv/Url.h>

#include <framework/system/BytesOrder.h>
#include <framework/logger/LoggerStreamRecord.h>
using namespace framework::logger;
using namespace framework::string;

#include <boost/asio/read.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/thread/detail/thread.hpp>

#include <fstream>

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("BigMp4", 0);



namespace ppbox
{
    namespace peer
    {

        size_t mp4_head_size(std::istream & ios)
        {
            size_t tellg_sie = ios.tellg();
            ios.seekg(0, std::ios_base::end);
            size_t total_len = ios.tellg();
            size_t head_len = 0;

            const char* g_data = "mdat";

            char mdata[4] = {0};
            boost::uint32_t size1 = 0;

            while(true)
            {
                if(total_len < 8)
                {
                    head_len = 8;
                    break;
                }

                ios.seekg(head_len, std::ios_base::beg);


                ios.read((char*)&size1,4);
                size1 = framework::system::BytesOrder::host_to_net_long(size1);

                ios.read(mdata,4);
                if(0 == strncmp(g_data,mdata,4))
                {
                    head_len += 8;
                    break;
                }

                head_len += size1;

                if(total_len < head_len+4)
                {
                    head_len += 4;
                    break;
                }

            }

            ios.seekg(tellg_sie, std::ios::beg);
            return head_len;
        }

        BigMp4::BigMp4(
            boost::asio::io_service & io_svc)
            :io_svc_(io_svc)
            ,vod_(util::daemon::use_module<ppbox::peer::Peer>(global_daemon()))
            ,http_client_(io_svc)
            ,recv_size_(0)
            ,down_load_size_(0)
            ,head_size_(0)
            ,cur_size_(0)
            ,body_size_(0)
            ,tail_size_(0)
            ,stream_out_(NULL)
            ,stream_down_(NULL)
            ,stream_tmp_(NULL)
            ,stlostream_(NULL)
            ,open_step_(StepType::closed)
            ,index_(0)
            ,mode_(FetchMode::small_head)
            ,handle_(NULL)
            ,http_callback_(true)
            ,on_jumped_(false)
            ,on_draged_(false)
        {

        }

        BigMp4::~BigMp4()
        {
            close();
            if(NULL != stlostream_)
            {
                delete stlostream_;
                stlostream_ = NULL;
            }
        }

        void BigMp4::async_open(
            std::string const & playlink
            ,FetchMode::Mode mode
            ,std::iostream &  stream
            ,response_type const & resp)
        {
            LOG_S(framework::logger::Logger::kLevelError,"[async_open] mode:"<<mode
                <<" playlink:"<<playlink);

            open_step_ = StepType::closed;

            boost::system::error_code ec;
            mode_ = mode;
            stream_tmp_ = &stream;
            local_time_ = time(NULL);

            resp_ = resp;

            std::string link = playlink;
            const char *str = playlink.c_str();
            if (std::string::npos == link.find("://")) {
                if ('/' == *str) {
                    link = "ppvod://" + link;
                } else {
                    link = "ppvod:///" + link;
                }
            } else {
                if (std::string::npos == link.find(":///")) {
                    link = "ppvod:///" + link.substr(8);
                }
            }
            url_.from_string(link);
            std::string oldUrl = parse_url(url_.path().substr(1), ec);
            url_.path(oldUrl);

            handle_async_open(ec);
        }

        void BigMp4::async_tranfer(
            boost::uint32_t iSize         //位置
            ,std::ostream &stream
            ,response_type const &resp)
        {
            LOG_S(framework::logger::Logger::kLevelError,"[async_tranfer] iSize:"<<iSize<<" head_size:"<<head_size_);
            open_step_ = StepType::fetch_head;

            resp_ = resp;
            stream_out_ = &stream;

            boost::int32_t beg = 0;

            if(iSize < head_size_) {
                char szBuf[1024] = {0};
                beg = 0;
                boost::uint32_t iCount = 0;
                boost::uint32_t iTotal = 0;
                //先发头部
                if (mode_ == FetchMode::small_head) {
                    iTotal = head_size_ - iSize;
                    stream_tmp_->seekg(iSize, std::ios::beg);
                    while(iTotal-iCount > 1024) {
                        stream_tmp_->read(szBuf,1024);
                        stream_out_->write(szBuf,1024);
                        iCount+=1024;
                    }
                    stream_tmp_->read(szBuf,iTotal-iCount);
                    stream_out_->write(szBuf,iTotal-iCount);
                    //(*stream_out_)<<stream_tmp_->str();
                } else {
                    open_step_ = StepType::download_bighead_header;
                    stream_tmp_->seekg(0, std::ios::end);
                    boost::uint32_t begin_part_size = 0;
                    stream_tmp_->seekg(0, std::ios::beg);
                    iTotal = begin_part_size - iSize;
                    if (iSize < begin_part_size) {
                        stream_tmp_->seekg(iSize, std::ios::beg);
                        while(iTotal-iCount > 1024) {
                            stream_tmp_->read(szBuf,1024);
                            stream_out_->write(szBuf,1024);
                            iCount+=1024;
                        }
                        stream_tmp_->read(szBuf,iTotal-iCount);
                        stream_out_->write(szBuf,iTotal-iCount);
                        async_tranfer_bighead(begin_part_size, head_size_, stream_out_);
                    } else {
                        async_tranfer_bighead(iSize, head_size_, stream_out_);
                    }
                    return;
                }
            } else if (iSize < (head_size_ + body_size_)) {
                beg = iSize - head_size_;
            } else {
                assert(mode_ == FetchMode::big_head && tail_size_ > 0);
                open_step_ = StepType::download_bighead_tail;
                async_tranfer_bighead(iSize, head_size_+body_size_+tail_size_, stream_out_);
                return;
            }
            //下后面的
            async_tranfer_body(beg,stream_out_);

        }

        void BigMp4::cancel()
        {
            boost::system::error_code ec;
            vod_.cancel(handle_);
            http_client_.cancel(ec);
        }

        void BigMp4::close()
        {
            //StepType::Enum open_step_;
            LOG_S(framework::logger::Logger::kLevelError,"[close] StepType:"<<open_step_);

            switch(open_step_)
            {
            case StepType::closed:
                break;
            case StepType::jumping:
            case StepType::draging:
                {
                    if(NULL != handle_)
                    {
                        vod_.cancel(handle_);
                        handle_ = NULL;
                    }
                }
                break;
            case StepType::fetch_head:
                break;
            default:
                open_step_ = StepType::closed;
                if (http_callback_) {
                    http_client_.close();
                } else {
                    boost::system::error_code ec;
                    http_client_.cancel(ec);
                }

                break;
            }
        }

        boost::system::error_code BigMp4::get_valid_size(boost::uint32_t& size)
        {
            size = cur_size_;
            return boost::system::error_code();
        }

        boost::system::error_code BigMp4::get_total_size(boost::uint32_t& size)
        {
            size = (body_size_+head_size_+tail_size_);
            return boost::system::error_code();
        }

        void BigMp4::handle_async_open(boost::system::error_code const & ec)
        {
            LOG_S(framework::logger::Logger::kLevelError,"[handle_async_open] StepType:"<<open_step_
                <<" ec:"<<ec.message());

            boost::system::error_code ec1;

            if (ec)
            {
                if(open_step_ == StepType::jumping 
                    || open_step_ == StepType::draging)
                {
                    close();
                }
                open_step_ = StepType::closed;
                response(ec);
                return;
            }

            switch (open_step_)
            {
            case StepType::closed:
                {
                    begin_jump();
                }
                break;
            case StepType::jumping:
                {
                    begin_drag();
                }
                break;
            case StepType::draging:
                {
                    begin_fetch_head();
                }
                break;
            case StepType::fetch_head:
                {
                    assert(0);
                }
                break;
            case StepType::download_mid_header:
                {
                    assert(0);
                }
                break;
            case StepType::download_end_header:
                {
                    if(mode_ == FetchMode::small_head)
                    {
                        if (!head_size_) {
                            mp4_merge.meger_small_head(*stream_tmp_, *stream_tmp_, segment_infos_,head_size_, ec1);
                            cur_size_= head_size_;
                        }
                    }
                    else
                    {
                        //去重获取下体的大小
                        stream_tmp_->seekg(0,std::ios::beg);
                        //mp4_merge.vod_valid_segment_info(*stream_tmp_,segment_infos_,ec1);
                        head_size_ = drag_info_.segments[0].offset;
                        LOG_S(framework::logger::Logger::kLevelDebug,"[handle_async_open], head_size: " << head_size_);
                    }
                    if (0 == body_size_) {
                        for(boost::uint32_t i = 0; i < segment_infos_.size(); ++i) 
                        {
                            //计算实际要下载体部大小
                            body_size_ += segment_infos_[i].file_length - segment_infos_[i].head_length;
                        }
                    }
                    tail_size_ = 0;
                    if (head_size_ < 1024) {
                        tail_size_ = drag_info_.video.filesize - head_size_ - body_size_;
                    }
                    open("xxx", head_size_, tail_size_, body_size_); // 目前统计只适合大头模式
                    response(ec1);
                }
                break;
            case StepType::download_bighead_header:
                {
                    async_tranfer_body(0, stream_out_);
                }
                break;
            case StepType::download_bighead_tail:
                {
                    response(ec);
                }
                break;
            case StepType::download_mid_body:
                {
                    assert(0);
                }
                break;
            case StepType::download_end_body:
                {
                    if (tail_size_ > 0) {
                        begin_fetch_tail();
                    } else {
                        response(ec);
                    }
                }
                break;
            default:
                break;
            }
        }

        void BigMp4::async_tranfer_samallhead(
            boost::int32_t iSize          //起始位置
            , std::ostream* stream)
        {

            boost::system::error_code ec;

            assert(open_step_ >= StepType::fetch_head);

            stream_down_ = stream;
            open_step_ = StepType::download_mid_header;

            recv_size_ = iSize;

            boost::uint64_t body_size = 0;
            for(size_t ii = 0; ii <  segment_infos_.size(); ++ii)
            {
                boost::uint64_t segment_lengh = segment_infos_[ii].head_length;
                if((body_size + segment_lengh) > recv_size_)
                {//Range 下载
                    index_ = ii; //从第几部分开始下
                    down_load_segment_header(recv_size_-body_size);
                    return;
                }
                else
                {
                    body_size += segment_lengh;
                }
            }
            if(recv_size_ != body_size)
            {
                //== 时为正好相等  .head超过大小了
                ec = ppbox::peer::error::bad_file_format;
            }
            else
            {
                open_step_ = StepType::download_end_header;
            }

            handle_async_open(ec);
        }

        void BigMp4::async_tranfer_bighead( 
            boost::uint64_t begin, 
            boost::uint64_t end, 
            std::ostream* stream)
        {
            http_callback_ = false;
            boost::system::error_code ec;

            stream_down_ = stream;

            framework::string::Url url_t("http://localhost/");
            framework::network::NetName host_tmp = jump_info_.server_host;

            host_tmp.port(81);
            url_t.host(host_tmp.host());
            url_t.svc(host_tmp.svc());
            url_t.path("/" + url_.path() );

            util::protocol::HttpRequest request;
            request.head().method = util::protocol::HttpRequestHead::get;
            request.head().path = url_t.path_all();
            request.head().host.reset(url_t.host_svc());
            request.head().range.reset(util::protocol::http_field::Range(begin, end));

            request.head().get_content(std::cout);
            LOG_S(framework::logger::Logger::kLevelDebug,"[async_tranfer_bighead] Range from:"<<begin
                <<" to:"<<end
                <<" url:"<<url_t.to_string());

            // http_client_.close();
            recv_size_ = 0;
            down_load_size_ = end - begin;
            down_load_begin_offset_ = begin;

            http_client_.close();
            http_client_.async_open(request
                ,boost::bind(&BigMp4::open_bighead_callback, this, _1));
        }

        struct BigHeadFinish
        {
            BigHeadFinish(
                std::istream & is)
                : is_(is)
                , min_head_size_(1)
                ,init_stream_size_(0)
            {
            }

            bool operator()(
                bool is_read, 
                boost::system::error_code const & ec, 
                std::pair<size_t, size_t> const & bytes_transferred)
            {
                return (ec || 
                    (!is_read && func(bytes_transferred.second)));
            }

            bool func(
                size_t bytes_transferred)
            {
                if (min_head_size_ <= bytes_transferred) {
                    min_head_size_ = mp4_head_size(is_);
                }
                return min_head_size_ <= bytes_transferred;
            }

        private:
            std::istream & is_;
            size_t min_head_size_;
            size_t init_stream_size_;
        };

         void BigMp4::open_bighead_callback(boost::system::error_code const & ec)
        {
            http_callback_ = true ;
            if(ec)
            {
                LOG_S(framework::logger::Logger::kLevelError,"[open_bighead_callback] ec:"<<ec.message());
                handle_async_open(ec);
            }
            else 
            {
                /*if(NULL != stlostream_)
                {
                    delete stlostream_;
                }
                stlostream_ = new util::stream::StlOStream(io_svc_,*stream_tmp_);

                util::stream::async_transfer(
                    http_client_, 
                    *stlostream_, 
                    boost::asio::buffer(buf_,10000), 
                    BigHeadFinish(*stream_tmp_), 
                    boost::bind(&BigMp4::down_bighead_callback, this, _1));*/
                // 不下载bighead数据
                http_client_.response_head().get_content(std::cout);
                boost::asio::async_read(http_client_,
                    boost::asio::buffer(buf_, (down_load_size_-recv_size_>1024)?1024:(down_load_size_-recv_size_)),
                    boost::asio::transfer_all(),
                    boost::bind(&BigMp4::download_big_mp4_head_handler, this, _1, _2));
            }

        }

        void BigMp4::down_bighead_callback(boost::system::error_code const & ec)
        {
            boost::system::error_code ec1 = ec;
            if(!ec1)
            {
                open_step_ = StepType::download_end_header;
                head_size_ = mp4_head_size(*stream_tmp_);
                cur_size_ = head_size_;
            }
            handle_async_open(ec1);
        }


        void BigMp4::down_async_open(boost::system::error_code const & ec)
        {
            http_callback_ = true;
            if(ec)
            {
                LOG_S(framework::logger::Logger::kLevelError,"[down_async_open] ec:"<<ec.message());
                handle_async_open(ec);
            }
            else 
            {
                //http_client_.response_head().get_content(std::cout);

                boost::asio::async_read(http_client_,
                    boost::asio::buffer(buf_, (down_load_size_-recv_size_>1024)?1024:(down_load_size_-recv_size_)),
                    boost::asio::transfer_all(),
                    boost::bind(&BigMp4::download_handler, this, _1, _2));
            }
        }

        void BigMp4::download_handler(
            boost::system::error_code const & ec
            ,std::size_t bytes_transferred)
        {
            if(ec)
            {
                LOG_S(framework::logger::Logger::kLevelError,"[download_handler] ec:"<<ec.message());
                handle_async_open(ec);

            }
            else
            {
                if(bytes_transferred > 0)
                {
                    down_load_begin_offset_ += bytes_transferred;
                    increase(bytes_transferred, down_load_begin_offset_);
                    recv_size_ += bytes_transferred;
                    boost::system::error_code ec1;
                    stream_down_->write(buf_.data(), bytes_transferred);
                    if(!(*stream_down_))
                    {
                        ec1 = ppbox::peer::error::bad_file_format;
                        handle_async_open(ec1);
                        return;
                    }
                    cur_size_ += bytes_transferred;

                    if(ec1 || (recv_size_ >= down_load_size_))
                    {
                        LOG_S(framework::logger::Logger::kLevelError,"[download_handler] Finish");
                        if(ec1)
                        {
                            handle_async_open(ec1);
                            return;
                        }

                        if(open_step_ ==  StepType::download_mid_header)
                        {
                            down_load_segment_header();
                        }
                        else if(open_step_ ==  StepType::download_end_header)
                        {
                            handle_async_open(ec1);
                        }
                        else if(open_step_ == StepType::download_end_body)
                        {
                            handle_async_open(ec1);
                        }
                        else if(open_step_ ==  StepType::download_mid_body )
                        {
                            down_load_segment_body();
                        }
                        else
                        {
                            assert(0);
                        }
                        return;
                    }
                    boost::asio::async_read(http_client_,
                        boost::asio::buffer(buf_, (down_load_size_-recv_size_>1024)?1024:(down_load_size_-recv_size_)),
                        boost::asio::transfer_all(),
                        boost::bind(&BigMp4::download_handler, this, _1, _2));

                }
                else
                {
                    LOG_S(framework::logger::Logger::kLevelError,"[download_handler] download size < 1");
                    handle_async_open(ec);
                }
            }
        }

        void BigMp4::download_big_mp4_head_handler(
            boost::system::error_code const & ec, 
            std::size_t bytes_transferred)
        {
            if(ec) {
                LOG_S(framework::logger::Logger::kLevelError,"[download_big_mp4_head_handler] ec:"<<ec.message());
                handle_async_open(ec);
            } else {
                if(bytes_transferred > 0) {
                    down_load_begin_offset_ += bytes_transferred;
                    increase(bytes_transferred, down_load_begin_offset_);
                    recv_size_ += bytes_transferred;
                    stream_down_->write(buf_.data(), bytes_transferred);
                    if(!(*stream_down_))
                    {
                        boost::system::error_code ec1;
                        ec1 = ppbox::peer::error::bad_file_format;
                        handle_async_open(ec1);
                        return;
                    }
                    if (down_load_size_ == 0) {
                        handle_async_open(ec);
                    } else {
                        boost::asio::async_read(http_client_,
                            boost::asio::buffer(buf_, (down_load_size_-recv_size_>1024)?1024:(down_load_size_-recv_size_)),
                            boost::asio::transfer_all(),
                            boost::bind(&BigMp4::download_big_mp4_head_handler, this, _1, _2));
                    }
                } else {
                    LOG_S(framework::logger::Logger::kLevelError,"[download_big_mp4_head_handler] download size < 1");
                    handle_async_open(ec);
                }
            }
        }

        void BigMp4::async_tranfer_body(
            boost::int32_t iSize
            , std::ostream* stream)
        {
            boost::system::error_code ec;

            assert(open_step_ >= StepType::fetch_head);
            open_step_ = StepType::download_mid_body;

            stream_down_ = stream;
            recv_size_ = iSize; //body的位置 是属于第几个分段
            down_load_begin_offset_ = recv_size_ + head_size_;

            boost::uint64_t body_size = 0;
            for(size_t ii = 0; ii <  segment_infos_.size(); ++ii)
            {
                boost::uint64_t segment_lengh = segment_infos_[ii].file_length-segment_infos_[ii].head_length;
                if((body_size + segment_lengh) > recv_size_)
                {//Range 下载
                    index_ = ii; //从第几部分开始下
                    down_load_segment_body(recv_size_-body_size);
                    return;
                }
                else
                {
                    body_size += segment_lengh;
                }
            }

            if(body_size != recv_size_)
            {
                ec = ppbox::peer::error::bad_file_format;
            }
            else
            {
                open_step_ = StepType::download_end_body;
            }
            handle_async_open(ec);
            //返回错误
        }

        void BigMp4::down_load_segment_header(boost::uint64_t recv_size )
        {
            http_callback_ = false;
            boost::system::error_code ec;

            if(index_ == segment_infos_.size()-1)
            {
                open_step_ = StepType::download_end_header;
            }

            recv_size_ = recv_size ; 

            down_load_size_ = segment_infos_[index_].head_length;// - head_size_;

            util::protocol::HttpRequest request;
            get_request(index_,jump_info_.server_host,request,ec);
            //get_cdn_request(index_,jump_info_.server_host,request,ec);
            request.head().get_content(std::cout);
            http_client_.close();

            http_client_.async_open(request
                ,boost::bind(&BigMp4::down_async_open, this, _1));

            index_++;
        }

        void BigMp4::down_load_segment_body(boost::uint64_t recv_size)
        {
            http_callback_ = false;

            boost::system::error_code ec;

            if(index_ == segment_infos_.size()-1)
            {
                open_step_ = StepType::download_end_body;
            }

            recv_size_ = recv_size + segment_infos_[index_].head_length; 

            down_load_size_ = segment_infos_[index_].file_length;// - head_size_;

            util::protocol::HttpRequest request;
            get_request(index_,jump_info_.server_host,request,ec);
            request.head().get_content(std::cout);
            http_client_.close();

            http_client_.async_open(request
                ,boost::bind(&BigMp4::down_async_open, this, _1));

            index_++;

        }


        void BigMp4::begin_fetch_head()
        {
            open_step_ = StepType::download_end_header;
            segment_infos_.clear();
            for(boost::uint32_t i = 0; i < drag_info_.segments.size(); ++i) 
            {
                ppbox::avformat::SegmentInfo seg;
                seg.duration = drag_info_.segments[i].duration;
                seg.file_length = drag_info_.segments[i].file_length;
                seg.head_length = drag_info_.segments[i].head_length;
                if (mode_ == FetchMode::big_head
                    && i < (drag_info_.segments.size()-1)) {
                        assert(seg.file_length >= drag_info_.segments[i+1].offset - drag_info_.segments[i].offset);
                        seg.file_length = 
                            drag_info_.segments[i+1].offset - drag_info_.segments[i].offset + seg.head_length;
                }
                segment_infos_.push_back(seg);
            }

            boost::uint32_t size = 0;
            bool full_head = false;

            check_head(full_head,size);
            if (full_head)
            {
                cur_size_ = size;
            }
            else
            {
                if(FetchMode::small_head == mode_)
                {
                    //下小头
                    async_tranfer_samallhead(size,stream_tmp_);
                }
                else
                {
                    assert(0 == size);
                    // stream_tmp_->seekg(0, std::ios::beg);
                    handle_async_open(boost::system::error_code());
                }
                return;
            }
            handle_async_open(boost::system::error_code());
        }

        void BigMp4::begin_fetch_tail()
        {
            open_step_ = StepType::download_bighead_tail;
            async_tranfer_bighead(
                head_size_+body_size_, 
                head_size_+body_size_+tail_size_,
                stream_out_);
        }

        void BigMp4::check_head(bool& full_head,boost::uint32_t& size)
        {
            if (stream_tmp_->rdbuf()->in_avail() < 1)
            {
                full_head = false;
                size = 0;
                return;
            }
            stream_tmp_->seekg(0,std::ios::end);
            boost::uint32_t total_size = stream_tmp_->tellg();
            boost::uint32_t head_size = mp4_head_size(*stream_tmp_);

            if(FetchMode::big_head == mode_)
            {
                head_size_ = drag_info_.segments[0].offset;
                if (total_size < 1024) {
                    full_head = false;
                    size = 0;
                } else {
                    full_head = true;
                    size = 1024;
                }
            }
            else
            {
                if (head_size > total_size) //没一个完整的head
                {
                    full_head = false;
                    size = 0;
                    return;
                }
                //要考虑到只有一个分段的
                if(head_size ==segment_infos_[0].head_length
                    && segment_infos_.size() > 1)
                {
                    //断点下小头
                    full_head = false;
                }
                else
                {
                    //大头已经合并完成
                    full_head = true;
                    head_size_ = head_size; 
                }
                size = total_size;
            }
            return;
        }

        void BigMp4::begin_jump()
        {
            LOG_S(framework::logger::Logger::kLevelDebug,"[begin_jump]");

            open_step_ = StepType::jumping;

            assert(NULL == handle_);
            if (!on_jumped_) {
                handle_ = vod_.async_fetch_jump(
                    url_.path()
                    ,"ppbox"
                    ,boost::bind(&BigMp4::jump_callback,this,_1,_2)
                    );
            } else {
                begin_drag();
            }
        }

        void BigMp4::jump_callback(
            boost::system::error_code const & ec
            ,ppbox::peer::VodJumpInfo const & jump_info)
        {
            boost::system::error_code ec1 = ec;
            if(NULL != handle_)
            {   
                vod_.close(handle_); 
                handle_ = NULL;
            }
            if(!ec1)
            {
                on_jumped_ = true;
                jump_info_ = jump_info;
            }
            handle_async_open(ec1);
        }

        void BigMp4::begin_drag()
        {
            LOG_S(framework::logger::Logger::kLevelDebug,"[begin_drag]");

            open_step_ = StepType::draging;

            assert(NULL == handle_);
            if (!on_draged_) {
                handle_ = vod_.async_fetch_drag(
                    url_.path()
                    ,"ppbox"
                    ,jump_info_
                    ,boost::bind(&BigMp4::drag_callback,this,_1,_2)
                    );
            } else {
                begin_fetch_head();
            }
        }

        void BigMp4::drag_callback(
            boost::system::error_code const & ec
            ,ppbox::peer::VodDragInfoNew const & drag_info)
        {
            boost::system::error_code ec1 = ec;
            if(NULL != handle_)
            {   
                vod_.close(handle_); 
                handle_ = NULL;
            }
            if(!ec1)
            {
                on_draged_ = true;
                drag_info_ = drag_info;
            }
            handle_async_open(ec1);
        }




        std::string BigMp4::get_key() const
        {
            return util::protocol::pptv::gen_key_from_time(
                jump_info_.server_time.to_time_t()
                + (time(NULL) - local_time_));
        }

        boost::system::error_code BigMp4::get_cdn_request(
            size_t segment,
            framework::network::NetName & addr,
            util::protocol::HttpRequest &  request ,
            boost::system::error_code & ec)
        {
            framework::string::Url real_url("http://localhost/");
            real_url.host(jump_info_.server_host.host());
            real_url.svc(jump_info_.server_host.svc());
            real_url.path("/" + format(segment) + "/" + url_.path());
            real_url.param("key", get_key());
            real_url.param("type", "ppbox");

            request.head().path = real_url.path_all();
            request.head().method = util::protocol::HttpRequestHead::get;
            request.head().host.reset(jump_info_.server_host.host_svc());
            request.head().range.reset(
            util::protocol::http_field::Range((boost::int64_t)(recv_size_)
            ,down_load_size_));

            LOG_S(framework::logger::Logger::kLevelDebug,"[get_cdn_request] Range from:"<<recv_size_
                <<" to:"<<down_load_size_
                <<" url:"<<real_url.to_string());
            return ec;
        }

        boost::system::error_code BigMp4::get_request(
            size_t segment,
            framework::network::NetName & addr,
            util::protocol::HttpRequest &  request ,
            boost::system::error_code & ec)
        {

            framework::network::NetName peer_worker("127.0.0.1:9000");

            framework::string::Url real_url(url_.to_string());
            real_url.protocol("http");
            real_url.host(jump_info_.server_host.host());
            real_url.svc(jump_info_.server_host.svc());
            real_url.path("/" + format(segment) + "/" + url_.path());
            real_url.param("key", get_key());
            if ("" == url_.param("type")) {
                real_url.param("type","ppbox");
            }
            std::string tmp =  real_url.to_string();
            tmp = framework::string::Url::encode(tmp);

            Url url("http://localhost/ppvaplaybyopen?" + drag_info_.segments[segment].va_rid);
            char const * headonly = down_load_size_ <= drag_info_.segments[segment].head_length ? "1" : "0";
            url.param("headonly", headonly);
            url.param("url", tmp);
            url.param("rid", drag_info_.segments[segment].va_rid);
            url.param("blocksize", format(drag_info_.segments[segment].block_size));
            url.param("filelength", format(drag_info_.segments[segment].file_length));
            url.param("headlength", format(drag_info_.segments[segment].head_length));

            url.param("autoclose", "false");
            url.param("drag", "1");
            url.param("BWType", "" != real_url.param("bwtype") ? real_url.param("bwtype") : format(jump_info_.BWType));
            url.param("blocknum", format(drag_info_.segments[segment].block_num));

            request.head().path = url.path_all();
            request.head().method = util::protocol::HttpRequestHead::get;
            request.head().host.reset(peer_worker.host_svc());
            request.head().range.reset(
                util::protocol::http_field::Range((boost::int64_t)(recv_size_)
                ,down_load_size_));

            LOG_S(framework::logger::Logger::kLevelDebug,"[get_request] Range from:"<<recv_size_
                <<" to:"<<down_load_size_
                <<" cdn url:"<<real_url.to_string());

            LOG_S(framework::logger::Logger::kLevelDebug,"[get_request] full url:"<<url.to_string());
            return ec;
        }

        void BigMp4::response(boost::system::error_code const & ec)
        {
            response_type resp;
            resp.swap(resp_);
            resp(ec);
        }


        std::string BigMp4::parse_url(
            std::string const &url,
            boost::system::error_code& ec)
        {
            std::string  newUrl ;
            if (url.size() > 4 && url.substr(url.size() - 4) == ".mp4") {
                if (url.find('%') == std::string::npos) {
                    newUrl = Url::encode(url, ".");
                    return newUrl;
                }
                return url;
            } else {
                std::string key = "kioe257ds";
                newUrl = util::protocol::pptv::url_decode(url, key);
                framework::string::StringToken st(newUrl, "||");
                if (!st.next_token(ec)) {
                    newUrl = st.remain();
                }
            }
            return newUrl;
        }

    } // namespace BigMp4
} // namespace ppbox

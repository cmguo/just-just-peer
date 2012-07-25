// BigMp4.h

#ifndef _PPBOX_PEER_BIGMP4_H_
#define _PPBOX_PEER_BIGMP4_H_

#include "ppbox/peer/Mp4Merge.h"
#include "ppbox/peer/Peer.h"

#include <ppbox/mux/tool/Sink.h>

#include <framework/string/Url.h>

#include <util/protocol/http/HttpClient.h>
#include <util/stream/StlStream.h>

#include <ostream>

namespace ppbox
{
    namespace peer
    {

        class BigMp4
        {
        public:

            struct FetchMode
            {
                enum Mode
                {
                    small_head,
                    big_head
                };
            };

            typedef boost::function<void (
                boost::system::error_code const &)
            >  response_type;

            BigMp4(boost::asio::io_service & io_svc);
            virtual ~BigMp4();

            void async_open(
                std::string const & 
                ,FetchMode::Mode 
                ,std::iostream &  
                ,response_type const &);

            void async_tranfer(
                boost::uint32_t          //位置
                , std::ostream &
                ,response_type const &);

            void cancel();

            void close();

            //获取下载有效位置
            boost::system::error_code get_valid_size(boost::uint32_t& size);//已下载多少
            boost::system::error_code get_total_size(boost::uint32_t& size); //需要下载的总大小



        private:

            void async_tranfer_samallhead(
                boost::int32_t          //起始位置
                , std::ostream*);

            void async_tranfer_body(
                boost::int32_t
                , std::ostream*);

            void async_tranfer_bighead(
                 std::ostream*);


        private:

            void begin_fetch_head();

            // bool 是否有完整头  , 下载的起始位置
            void check_head(bool& full_head,boost::uint32_t& size);

            //状态机制控制函数
            void handle_async_open(boost::system::error_code const & ec);



            //下分段体
            void down_load_segment_body(boost::uint64_t recv_size = 0);

            //下分段头
            void down_load_segment_header(boost::uint64_t recv_size = 0);



            //下Big head回调
            void open_bighead_callback(boost::system::error_code const & ec);
            void down_bighead_callback(boost::system::error_code const & ec);


            //下分段头与分段体的回调
            void down_async_open(boost::system::error_code const & ec);
            void download_handler(
                boost::system::error_code const & ec
                ,std::size_t bytes_transferred);


// jump、 drag------
            void begin_jump();

            void jump_callback(
                boost::system::error_code const & ec
                 ,ppbox::peer::VodJumpInfo const & jump_info);

             void begin_drag();

            void drag_callback(
                boost::system::error_code const & ec
               ,ppbox::peer::VodDragInfoNew const & drag_info);

// tool ----------


            std::string get_key() const;

            boost::system::error_code get_request(
                size_t segment,
                framework::network::NetName & addr,
                util::protocol::HttpRequest & request,
                boost::system::error_code & ec);

            void response(boost::system::error_code const & ec);

            std::string parse_url(
                std::string const &url,
                boost::system::error_code& ec);
        
		private:
            struct StepType
            {
                enum Enum
                {
                    closed, 
                    jumping, 
                    draging,  //分段的drag  xml 
                    fetch_head,
                    download_mid_header,
                    download_end_header,
                    download_mid_body,
                    download_end_body,
                };
            };


        private:
            boost::asio::io_service& io_svc_;
            Peer& vod_;
            
            ppbox::peer::VodJumpInfo jump_info_;
            ppbox::peer::VodDragInfoNew drag_info_;

            util::protocol::HttpClient http_client_;
            boost::array<char, 1024> buf_;

            boost::uint64_t recv_size_; //已下载多少
            boost::uint64_t down_load_size_; //需要下载多少

            //当前下载的mp4头大小
            boost::uint32_t head_size_;
            //有效下载位置
            boost::uint32_t cur_size_;
            //数据部份总大小
            boost::uint32_t body_size_;


            std::ostream * stream_out_; //aync_transfer时保存的流
            std::ostream* stream_down_; //下载时用到的流
            std::iostream* stream_tmp_; //aync_open时保存的流
            util::stream::StlOStream *stlostream_; //只用于下载大头时
            
            
            response_type resp_;
            StepType::Enum open_step_;
            framework::string::Url url_;
            time_t local_time_;

            //当前下载到第几个分段
            boost::uint32_t index_;

            //下载模块
            FetchMode::Mode mode_;
            //jump/drag的handle_
            ppbox::common::FetchHandle handle_;

            std::vector<ppbox::avformat::SegmentInfo> segment_infos_;
            Mp4Merge mp4_merge;
        };
    } // namespace peer
} // namespace ppbox

#endif // _PPBOX_peer_VOD_H_

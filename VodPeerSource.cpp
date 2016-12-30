// VodPeerSource.cpp

#include "just/peer/Common.h"
#include "just/peer/VodPeerSource.h"

#include <just/cdn/pptv/PptvVod.h>

#include <util/protocol/http/HttpSocket.hpp>

#include <framework/string/Format.h>
using namespace framework::string;

#include <framework/logger/StreamRecord.h>

namespace just
{
    namespace peer
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("just.peer.VodPeerSource", framework::logger::Debug);

        VodPeerSource::VodPeerSource(
            boost::asio::io_service & io_svc)
            : PeerSource(io_svc)
        {
        }

        VodPeerSource::~VodPeerSource()
        {
        }

        std::size_t VodPeerSource::private_read_some(
            buffers_t const & buffers,
            boost::system::error_code & ec)
        {
            assert(http_.is_open(ec));
            return http_.read_some(buffers, ec);
        }

        void VodPeerSource::private_async_read_some(
            buffers_t const & buffers,
            util::stream::StreamHandler const & handler)
        {
            boost::system::error_code ec;
            (void)ec;
            assert(http_.is_open(ec));
            http_.async_read_some(buffers, handler);
        }

        bool VodPeerSource::prepare(
            framework::string::Url & url,
            boost::uint64_t & beg,
            boost::uint64_t & end,
            boost::system::error_code & ec)
        {
            just::cdn::PptvVod const & vod = (just::cdn::PptvVod const &)(*pptv_media());

            char const * str_no = url.path().c_str() + 1;
            size_t no = 0;
            for (; *str_no >= '0' && *str_no <= '9'; ++str_no) {
                no = no * 10 + (*str_no - '0');
            }

            // 格式不对的都直接通过CDN服务器下载
            if (*str_no != '/' || !use_peer()) {
                ec.clear();
                return true;
            }

            just::data::SegmentInfo info;
            vod.segment_info(no, info, ec);

            std::string temp_rid = url.param("rid");
            url.param_del("rid");

            PeerSource::prepare(url, beg, end, ec);

            if (!ec) {
                url.path("/ppvaplaybyopen");
                url.param("filelength", format(info.size));
                url.param("headlength", format(info.head_size));
                url.param("drag", status_->buffer_time() < 15000 ? "1" : "0");
                url.param("headonly", end <= info.head_size ? "1" : "0");
                url.param("rid", temp_rid);
            }

            return !ec;
        }

    } // namespace peer
} // namespace just

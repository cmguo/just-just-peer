// LivePeerSource.cpp

#include "just/peer/Common.h"
#include "just/peer/LivePeerSource.h"

#include <just/cdn/pptv/PptvLive.h>

#include <framework/logger/StreamRecord.h>
#include <framework/string/Format.h>
#include <framework/string/Slice.h>
using namespace framework::string;

#include <boost/bind.hpp>

namespace just
{
    namespace peer
    {

        //FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("just.peer.LivePeerSource", Debug);

        LivePeerSource::LivePeerSource(
            boost::asio::io_service & io_svc)
            : PeerSource(io_svc)
            , seq_(0)
        {
        }

        LivePeerSource::~LivePeerSource()
        {
        }

        boost::uint64_t LivePeerSource::total(
            boost::system::error_code & ec)
        {
            if (!use_peer()) {
                return PeerSource::total(ec) - 1400;
            } else {
                ec.clear();
                return boost::uint64_t(0x8000000000000000ULL); // 一个非常大的数值，假设永远下载不完
            }
        }

        bool LivePeerSource::prepare(
            framework::string::Url & url, 
            boost::uint64_t & beg, 
            boost::uint64_t & end, 
            boost::system::error_code & ec)
        {
            just::cdn::PptvLive const & live = (just::cdn::PptvLive const &)(*pptv_media());

            if (!use_peer()) {
                beg += 1400;
                ec.clear();
                return true;
            } else if (beg > 0) {
                // 不能断点续传
                ec = framework::system::logic_error::not_supported;
                return false;
            }

             // "/live/<stream_id>/<file_time>"
            std::vector<std::string> vec;
            std::vector<std::string> vec1;
            slice<std::string>(url.path(), std::back_inserter(vec), "/");
            slice<std::string>(vec[3], std::back_inserter(vec1), ".");

            url.path("/live/");
            PeerSource::prepare(url, beg, end, ec);

            if (!ec) {
                url.path("/playlive.flv");
                url.param("channelid", vec[2]);
                url.param("rid", vec[2]);
                if(pptv_media()!=NULL)
                {
                    url.param("datarate", format(live.video().bitrate / 1000)); // kbps
                    url.param("interval", format(live.segment().interval));
                }else
                {
                    url.param("interval", format(5));
                }
                url.param("replay", "1");
                url.param("start", vec1[0]);
                url.param("source", "0");
                url.param("uniqueid", format(++seq_));
            }

            if (status_ != NULL)
            {
                status_->set_current_url(url.to_string());
            }

            return !ec;
        }

    } // namespace peer
} // namespace just

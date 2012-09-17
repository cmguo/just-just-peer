// LivePeerSource.cpp

#include "ppbox/peer/Common.h"
#include "ppbox/peer/LivePeerSource.h"

#include <ppbox/cdn/PptvLive.h>

#include <framework/logger/StreamRecord.h>
#include <framework/string/Format.h>
#include <framework/string/Slice.h>
using namespace framework::string;

namespace ppbox
{
    namespace peer
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("LivePeerSource", 0);

        LivePeerSource::LivePeerSource(
            boost::asio::io_service & io_svc)
            : PeerSource(io_svc)
            , seq_(0)
        {
        }

        LivePeerSource::~LivePeerSource()
        {
        }

        boost::system::error_code LivePeerSource::make_url(
            framework::string::Url const & cdn_url, 
            framework::string::Url & url)
        {
            ppbox::cdn::PptvLive * live = (ppbox::cdn::PptvLive *)media_;

            boost::system::error_code ec = PeerSource::make_url(cdn_url, url);

            // "/live/<stream_id>/<file_time>"
            std::vector<std::string> vec;
            std::vector<std::string> vec1;
            slice<std::string>(cdn_url.path(), std::back_inserter(vec), "/");
            slice<std::string>(vec[2], std::back_inserter(vec1), ".");

            if (!ec) {
                url.path("/playlive.flv");
                url.param("channelid", live->video().rid);
                url.param("rid", live->video().rid);
                url.param("datarate", format(live->video().bitrate));
                url.param("replay", "1");
                url.param("start", vec1[0]);
                url.param("interval", format(live->segment().interval));
                url.param("source", "0");
                url.param("uniqueid", format(++seq_));
            }

            return ec;
        }

    }//peer
}//ppbox
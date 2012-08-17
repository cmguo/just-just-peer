//LivePeerSource.cpp
#include "ppbox/peer/Common.h"
#include "ppbox/peer/LivePeerSource.h"

#include <framework/logger/LoggerStreamRecord.h>
#include <framework/string/Format.h>

using namespace framework::string;

namespace ppbox
{
    namespace peer
    {
        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("LivePeerSource", 0);


        LivePeerSource::LivePeerSource(
            boost::asio::io_service & ios_svc)
            : PeerSource(ios_svc)
        {
        }

        framework::string::Url LivePeerSource::get_peer_url(
            framework::string::Url const & url )
        {
            std::string url_str = url.to_string();
            std::string::size_type pos = url_str.find("block");
            assert(pos != std::string::npos);
            std::string cdn_str = url_str.substr(0, pos + 4);
            cdn_str = framework::string::Url::encode(cdn_str);
            std::string add_str = url_str.substr(pos + 5);

            std::string peer_str("http://127.0.0.1:9000/playlive.flv?url=");
            peer_str += cdn_str;
            peer_str += add_str;

            framework::string::Url peer_url(peer_str);
            peer_url.svc(format(get_port()));
            addr_ = url.host_svc();

            return peer_url;
        }


        boost::system::error_code LivePeerSource::open(
            framework::string::Url const & url,
            boost::uint64_t beg, 
            boost::uint64_t end, 
            boost::system::error_code & ec)
        {
            flag_ = true;

            framework::string::Url peer_url = get_peer_url(url);
            util::protocol::HttpRequestHead & head = request_.head();
            head.path = peer_url.path_all();
            if (beg != 0 || end != (boost::uint64_t)-1) {
                head.range.reset(util::protocol::http_field::Range((boost::int64_t)beg, (boost::int64_t)end));
            } else {
                head.range.reset();
            }

            return PeerSource::open(url, beg, end, ec);
        }

        void LivePeerSource::async_open(
            framework::string::Url const & url,
            boost::uint64_t beg, 
            boost::uint64_t end, 
            SourceBase::response_type const & resp)
        {
            flag_ = true;

            framework::string::Url peer_url = get_peer_url(url);
            boost::system::error_code ec;
            util::protocol::HttpRequestHead & head = request_.head();
            head.path = peer_url.path_all();
            if (beg != 0 || end != (boost::uint64_t)-1) {
                head.range.reset(util::protocol::http_field::Range((boost::int64_t)beg, (boost::int64_t)end));
            } else {
                head.range.reset();
            }

            PeerSource::async_open(url, beg, end, resp);
        }

    }//peer
}//ppbox
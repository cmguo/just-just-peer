//LivePeerSource.cpp
#include "ppbox/peer/Common.h"
#include "ppbox/peer/LivePeerSource.h"

#include <framework/logger/LoggerStreamRecord.h>

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

        boost::system::error_code LivePeerSource::open(
            framework::string::Url const & url,
            boost::uint64_t beg, 
            boost::uint64_t end, 
            boost::system::error_code & ec)
        {
            flag_ = true;
            std::string url_str = url.to_string();
            std::string::size_type pos = url_str.find("block");
            assert(pos != std::string::npos);
            std::string cdn_str = url_str.substr(0, pos + 4);
            cdn_str = framework::string::Url::encode(cdn_str);
            std::string add_str = url_str.substr(pos + 5);

            std::string peer_str("http://127.0.0.1:9000/playlive.flv?url=");
            peer_str += cdn_str;
            peer_str += add_str;

            framework::string::Url peer_url;
            peer_url.from_string(peer_str);
            LOG_S(framework::logger::Logger::kLevelDebug,"[get_request] peer_work url:"<<peer_str
                <<" from:"<<beg
                <<" to:"<<end);

            addr_ = url.host_svc();
            util::protocol::HttpRequestHead & head = request_.head();
            head.path = peer_url.path_all();
            if (beg != 0 || end != (boost::uint64_t)-1) {
                head.range.reset(util::protocol::http_field::Range((boost::int64_t)beg, (boost::int64_t)end));
            } else {
                head.range.reset();
            }
            std::ostringstream oss;
            head.get_content(oss);
            LOG_STR(framework::logger::Logger::kLevelDebug2, oss.str().c_str());
            http_.bind_host(addr_, ec);
            http_.open(request_, ec);
            http_.request().head().get_content(std::cout);

            return ec;
        }

        void LivePeerSource::async_open(
            framework::string::Url const & url,
            boost::uint64_t beg, 
            boost::uint64_t end, 
            SourceBase::response_type const & resp)
        {
            flag_ = true;
            std::string url_str = url.to_string();
            std::string::size_type pos = url_str.find("block");
            assert(pos != std::string::npos);
            std::string cdn_str = url_str.substr(0, pos + 4);
            cdn_str = framework::string::Url::encode(cdn_str);
            std::string add_str = url_str.substr(pos + 5);

            std::string peer_str("http://127.0.0.1:9000/playlive.flv?url=");
            peer_str += cdn_str;
            peer_str += add_str;

            framework::string::Url peer_url;
            peer_url.from_string(peer_str);
            LOG_S(framework::logger::Logger::kLevelDebug,"[get_request] peer_work url:"<<peer_str
                <<" from:"<<beg
                <<" to:"<<end);

            addr_ = url.host_svc();
            boost::system::error_code ec;
            util::protocol::HttpRequestHead & head = request_.head();
            head.path = peer_url.path_all();
            if (beg != 0 || end != (boost::uint64_t)-1) {
                head.range.reset(util::protocol::http_field::Range((boost::int64_t)beg, (boost::int64_t)end));
            } else {
                head.range.reset();
            }

            std::ostringstream oss;
            head.get_content(oss);
            LOG_STR(framework::logger::Logger::kLevelDebug2, oss.str().c_str());
            http_.bind_host(addr_, ec);
            http_.async_open(request_, resp);
        }

    }//peer
}//ppbox
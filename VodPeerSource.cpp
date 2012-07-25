//VodPeerSource.cpp

#include "ppbox/peer/Common.h"
#include "ppbox/peer/VodPeerSource.h"

#include <framework/logger/LoggerStreamRecord.h>

namespace ppbox
{
    namespace peer
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("VodPeerSource", 0);



        VodPeerSource::VodPeerSource(
            boost::asio::io_service & ios_svc)
            : PeerSource(ios_svc)
        {
        }

        VodPeerSource::~VodPeerSource()
        {
        }

        boost::system::error_code VodPeerSource::open(
            framework::string::Url const & url,
            boost::uint64_t beg, 
            boost::uint64_t end, 
            boost::system::error_code & ec)
        {
            framework::string::Url cdn_url(url.to_string());
            framework::string::Url peer_url("http://127.0.0.1:9000/ppvaplaybyopen?" + url.param("rid"));
            peer_url.param("blocksize", url.param("blocksize"));
            peer_url.param("filelength", url.param("filelength"));
            peer_url.param("headlength", url.param("headlength"));
            peer_url.param("rid", url.param("rid"));
            cdn_url.param("blocksize","");
            cdn_url.param("filelength","");
            cdn_url.param("headlength","");
            cdn_url.param("rid","");
            std::string cdn_str = framework::string::Url::encode(cdn_url.to_string());
            peer_url.param("url", cdn_str);

            LOG_S(framework::logger::Logger::kLevelDebug,"[get_request] peer_work url:"<<peer_url.to_string()
                <<" from:"<<beg
                <<" to:"<<end);

            addr_ = cdn_url.host_svc();
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

        void VodPeerSource::async_open(
            framework::string::Url const & url,
            boost::uint64_t beg, 
            boost::uint64_t end, 
            response_type const & resp)
        {
            flag_ = true;
            framework::string::Url cdn_url(url.to_string());
            framework::string::Url peer_url("http://127.0.0.1:9000/ppvaplaybyopen?" + url.param("rid"));
            peer_url.param("blocksize", url.param("blocksize"));
            peer_url.param("filelength", url.param("filelength"));
            peer_url.param("headlength", url.param("headlength"));
            peer_url.param("rid", url.param("rid"));
            cdn_url.param("blocksize","");
            cdn_url.param("filelength","");
            cdn_url.param("headlength","");
            cdn_url.param("rid","");
            std::string cdn_str = framework::string::Url::encode(cdn_url.to_string());
            peer_url.param("url", cdn_str);
            LOG_S(framework::logger::Logger::kLevelDebug,"[get_request] peer_work url:"<<peer_url.to_string()
                <<" from:"<<beg
                <<" to:"<<end);

            addr_ = cdn_url.host_svc();
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
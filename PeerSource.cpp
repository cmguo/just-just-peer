//PeerSource.cpp

#include "ppbox/peer/Common.h"
#include "ppbox/peer/PeerSource.h"

#include <ppbox/ppbox/Common.h>

#include <util/protocol/http/HttpError.h>
#include <util/daemon/detail/Module.h>

#include <framework/string/Url.h>
#include <framework/logger/LoggerStreamRecord.h>

#include <boost/asio/read.hpp>
#include <boost/asio/buffer.hpp>

using namespace framework::logger;

namespace ppbox
{
    namespace peer
    {
        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("PeerSource", 0);

        PeerSource::PeerSource(
            boost::asio::io_service & ios_svc)
            : HttpSource(ios_svc)
            , Peer_(util::daemon::use_module<ppbox::peer::Peer>(ppbox::global_daemon()))
        {
        }

        PeerSource::~PeerSource()
        {
        }

        boost::uint16_t PeerSource::get_port() const
        {
            return Peer_.port();
        }

        boost::system::error_code PeerSource::open(
            framework::string::Url const & url,
            boost::uint64_t beg, 
            boost::uint64_t end, 
            boost::system::error_code & ec)
        {
            LOG_S(framework::logger::Logger::kLevelDebug,"[get_request] peer_work url:"<<url.to_string()
                <<" from:"<<beg
                <<" to:"<<end);

            std::ostringstream oss;
            LOG_STR(framework::logger::Logger::kLevelDebug2, oss.str().c_str());
            http_.request().head().get_content(std::cout);

            http_.bind_host(addr_, ec);
            http_.open(request_, ec);
            return ec;
        }

        void PeerSource::async_open(
            framework::string::Url const & url,
            boost::uint64_t beg, 
            boost::uint64_t end, 
            response_type const & resp)
        {
            LOG_S(framework::logger::Logger::kLevelDebug,"[get_request] peer_work url:"<<url.to_string()
                <<" from:"<<beg
                <<" to:"<<end);

            std::ostringstream oss;
            LOG_STR(framework::logger::Logger::kLevelDebug2, oss.str().c_str());
            http_.request().head().get_content(std::cout);

            boost::system::error_code ec;
            http_.bind_host(addr_, ec);
            http_.async_open(request_, resp);
        }

    }//peer
}//ppbox


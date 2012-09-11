//PeerSource.cpp

#include "ppbox/peer/Common.h"
#include "ppbox/peer/PeerSource.h"

#include <ppbox/cdn/PptvMedia.h>

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>
#include <framework/string/Format.h>
using namespace framework::string;
using namespace framework::logger;

namespace ppbox
{
    namespace peer
    {
        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("PeerSource", 0);

        PeerSource::PeerSource(
            boost::asio::io_service & io_svc)
            : HttpSource(io_svc)
            , module_(util::daemon::use_module<ppbox::peer::PeerModule>(io_svc))
            // , media_((PptvMedia const *)util::daemon::use_module<ppbox::data::DataModule>(io_svc).get_media_from_source(this))
            , status_(module_.alloc_status())
        {
            parse_param(media_->p2p_params());
        }

        PeerSource::~PeerSource()
        {
            module_.free_status(status_);
        }

        boost::system::error_code PeerSource::open(
            framework::string::Url const & url,
            boost::uint64_t beg, 
            boost::uint64_t end, 
            boost::system::error_code & ec)
        {
            LOG_DEBUG("[open] url:"<<url.to_string()
                <<" range: "<< beg << " - " << end);

            if (module_.port() > 0) {
                framework::string::Url peer_url;
                make_url(url, peer_url);
                return HttpSource::open(peer_url, beg, end, ec);
            } else {
                return HttpSource::open(url, beg, end, ec);
            }
        }

        void PeerSource::async_open(
            framework::string::Url const & url,
            boost::uint64_t beg, 
            boost::uint64_t end, 
            response_type const & resp)
        {
            LOG_DEBUG("[async_open] url:"<<url.to_string()
                <<" range: "<< beg << " - " << end);

            if (module_.port() > 0) {
                framework::string::Url peer_url;
                make_url(url, peer_url);
                HttpSource::async_open(peer_url, beg, end, resp);
            } else {
                HttpSource::async_open(url, beg, end, resp);
            }
        }

        void PeerSource::parse_param(
            std::string const & params)
        {
            size_t adv_time = 0;
            map_find(params, "advtime", adv_time, "&");
            if (adv_time > 0)
                status_->set_adv_duration(adv_time);
        }

        boost::system::error_code PeerSource::make_url(
            framework::string::Url const & cdn_url,
            framework::string::Url & url)
        {
            url.protocol("http");
            url.host("127.0.0.1");
            url.svc(format(module_.port()));
            url.param("url", cdn_url.to_string());
            url.param("BWType", format(media_->jump().bw_type));
            url.param("autoclose", "false");

            status_->set_current_url(url.to_string());

            return boost::system::error_code();
        }

    }//peer
}//ppbox


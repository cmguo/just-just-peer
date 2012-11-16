// PeerSource.cpp

#include "ppbox/peer/Common.h"
#include "ppbox/peer/PeerSource.h"

#include <ppbox/cdn/PptvMedia.h>

#include <ppbox/demux/base/DemuxEvent.h>
#include <ppbox/demux/segment/SegmentDemuxer.h>

#include <ppbox/merge/MergerBase.h>

#include <ppbox/data/segment/SegmentSource.h>

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>
#include <framework/string/Format.h>
using namespace framework::string;

namespace ppbox
{
    namespace peer
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("ppbox.peer.PeerSource", framework::logger::Debug);

        PeerSource::PeerSource(
            boost::asio::io_service & io_svc)
            : HttpSource(io_svc)
            , module_(util::daemon::use_module<ppbox::peer::PeerModule>(io_svc))
            , status_(NULL)
            , peer_fail_(false)
        {
        }

        PeerSource::~PeerSource()
        {
            if (status_)
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

            if (use_peer()) {
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

            if (use_peer()) {
                framework::string::Url peer_url;
                make_url(url, peer_url);
                HttpSource::async_open(peer_url, beg, end, resp);
            } else {
                HttpSource::async_open(url, beg, end, resp);
            }
        }

        ppbox::cdn::HttpStatistics const & PeerSource::http_stat() const
        {
            const_cast<PeerSource *>(this)->open_log(true);
            return http_stat_;
        }

        void PeerSource::pptv_media(
            ppbox::cdn::PptvMedia const & media)
        {
            pptv_media_ = &media;

            status_ = module_.alloc_status();
            parse_param(pptv_media().p2p_params());
            switch (pptv_media().owner_type()) {
                case ppbox::cdn::PptvMedia::ot_demuxer:
                    pptv_media().demuxer().on<ppbox::demux::BufferingEvent>(boost::bind(&PeerSource::on_event, this, _1));
                    seg_source_ = &pptv_media().demuxer().source();
                    break;
                case ppbox::cdn::PptvMedia::ot_merger:
                     //pptv_media().merger().on<ppbox::demux::BufferingEvent>(boost::bind(&PeerSource::on_event, this, _1));
                    seg_source_ = &pptv_media().merger().source();
                    break;
                default:
                    assert(0);
                    break;
            }
        }

        void PeerSource::on_event(
            util::event::Event const & e)
        {
            ppbox::demux::BufferingEvent const & event = *e.as<ppbox::demux::BufferingEvent>();
            status_->update_buffer_time((boost::uint32_t)event.stat.buf_time());
            module_.update_status(status_);
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
            LOG_DEBUG("Use vod worker, BWType: " << pptv_media().jump().bw_type);

            url.protocol("http");
            url.host("127.0.0.1");
            url.svc(format(module_.port()));
            url.param("url", cdn_url.to_string());
            url.param("BWType", format(pptv_media().jump().bw_type));
            url.param("autoclose", "false");

            open_log(false);

            status_->set_current_url(url.to_string());

            return boost::system::error_code();
        }

        void PeerSource::open_log(
            bool end)
        {
            if (http_stat_.try_times > 0) {
                http_stat_.end_try(http_.stat());
                if (http_stat_.try_times == 1)
                    http_stat_.response_data_time = http_stat_.total_elapse;
            }
            if (!end) {
                http_stat_.begin_try();
            } else {
                http_stat_.total_elapse = http_stat_.elapse();
            }
        }

        bool PeerSource::use_peer()
        {
            if (!peer_fail_ && seg_source_->num_try() >= 3)
                peer_fail_ = true;
            return module_.port() > 0 && !peer_fail_;
        }

    } // namespace peer
} // namespace ppbox

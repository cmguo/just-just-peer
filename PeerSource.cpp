// PeerSource.cpp

#include "just/peer/Common.h"
#include "just/peer/PeerSource.h"

#include <just/cdn/pptv/PptvMedia.h>

#include <just/demux/base/DemuxEvent.h>
#include <just/demux/segment/SegmentDemuxer.h>

#include <just/merge/MergerBase.h>

#include <just/data/segment/SegmentSource.h>

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>
#include <framework/string/Format.h>
using namespace framework::string;

namespace just
{
    namespace peer
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("just.peer.PeerSource", framework::logger::Debug);

        PeerSource::PeerSource(
            boost::asio::io_service & io_svc)
            : just::cdn::PptvP2pSource(io_svc)
            , module_(util::daemon::use_module<just::peer::PeerModule>(io_svc))
            , status_(NULL)
            , peer_fail_(false)
        {
        }

        PeerSource::~PeerSource()
        {
            if (status_)
                module_.free_status(status_);
        }

        void PeerSource::on_stream_status(
            just::avbase::StreamStatus const & stat)
        {
            status_->update_buffer_time((boost::uint32_t)stat.buf_time());
            module_.update_status(status_);
        }

        void PeerSource::parse_param(
            std::string const & params)
        {
            if (use_peer()) {
                const_cast<just::data::SegmentSource &>(seg_source()).set_time_out(0);
            }
            status_ = module_.alloc_status();
            size_t adv_time = 0;
            map_find(params, "advtime", adv_time, "&");
            if (adv_time > 0)
                status_->set_adv_duration(adv_time);
        }

        bool PeerSource::prepare(
            framework::string::Url & url, 
            boost::uint64_t & beg, 
            boost::uint64_t & end, 
            boost::system::error_code & ec)
        {
            LOG_DEBUG("Use peer worker, BWType: " << pptv_media().jump().bw_type);

            std::string cdn_url = url.to_string();
            url = framework::string::Url();
            url.protocol("http");
            url.host("127.0.0.1");
            url.svc(format(module_.port()));
            url.param("url", cdn_url);
            url.param("BWType", format(pptv_media().jump().bw_type));
            url.param("autoclose", "false");

            status_->set_current_url(cdn_url);
            
            ec.clear();
            return true;
        }

        bool PeerSource::use_peer()
        {
            if (!peer_fail_ && seg_source().num_try() > 3)
                peer_fail_ = true;
            return module_.port() > 0 && !peer_fail_ && pptv_media().jump().bw_type != 100;
        }

    } // namespace peer
} // namespace just

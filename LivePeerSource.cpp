// LivePeerSource.cpp

#include "ppbox/peer/Common.h"
#include "ppbox/peer/LivePeerSource.h"

#include <ppbox/cdn/PptvLive.h>

#include <framework/logger/StreamRecord.h>
#include <framework/string/Format.h>
#include <framework/string/Slice.h>
using namespace framework::string;

#include <boost/bind.hpp>

namespace ppbox
{
    namespace peer
    {

        //FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("ppbox.peer.LivePeerSource", Debug);

        LivePeerSource::LivePeerSource(
            boost::asio::io_service & io_svc)
            : PeerSource(io_svc)
            , seq_(0)
        {
        }

        LivePeerSource::~LivePeerSource()
        {
        }

        boost::system::error_code LivePeerSource::open(
            framework::string::Url const & url,
            boost::uint64_t beg, 
            boost::uint64_t end, 
            boost::system::error_code & ec)
        {
            if (!use_peer()) {
                beg += 1400;
            } else if (beg > 0) {
                // 不能断点续传
                return ec = framework::system::logic_error::not_supported;
            }
            return PeerSource::open(url, beg, end, ec);
        }

        void LivePeerSource::async_open(
            framework::string::Url const & url,
            boost::uint64_t beg, 
            boost::uint64_t end, 
            response_type const & resp)
        {
            if (!use_peer()) {
                beg += 1400;
            } else if (beg > 0) {
                get_io_service().post(boost::bind(resp, framework::system::logic_error::not_supported));
                return;
            }
            return PeerSource::async_open(url, beg, end, resp);
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

        boost::system::error_code LivePeerSource::make_url(
            framework::string::Url const & cdn_url, 
            framework::string::Url & url)
        {
            ppbox::cdn::PptvLive const & live = (ppbox::cdn::PptvLive const &)pptv_media();

            framework::string::Url cdn_url2 = cdn_url;
            cdn_url2.path("/live/");
            boost::system::error_code ec = PeerSource::make_url(cdn_url2, url);

            // "/live/<stream_id>/<file_time>"
            std::vector<std::string> vec;
            std::vector<std::string> vec1;
            slice<std::string>(cdn_url.path(), std::back_inserter(vec), "/");
            slice<std::string>(vec[3], std::back_inserter(vec1), ".");

            if (!ec) {
                url.path("/playlive.flv");
                url.param("channelid", live.video().rid);
                url.param("rid", live.video().rid);
                url.param("datarate", format(live.video().bitrate));
                url.param("replay", "1");
                url.param("start", vec1[0]);
                url.param("interval", format(live.segment().interval));
                url.param("source", "0");
                url.param("uniqueid", format(++seq_));
            }

            status_->set_current_url(url.to_string());

            return ec;
        }

    } // namespace peer
} // namespace ppbox

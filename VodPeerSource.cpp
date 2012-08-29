//VodPeerSource.cpp

#include "ppbox/peer/Common.h"
#include "ppbox/peer/VodPeerSource.h"

#include <framework/string/Parse.h>
#include <framework/string/Format.h>
using namespace framework::string;

#include <framework/logger/StreamRecord.h>

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

        framework::string::Url VodPeerSource::get_peer_url(
            framework::string::Url const & url)
        {
            framework::string::Url cdn_url = url;
            framework::string::Url peer_url("http://127.0.0.1:0000/ppvaplaybyopen?");
            peer_url.svc(format(get_port()));
            if ("" != cdn_url.param("cdn.drag.segment")) {
                framework::string::Url drag_url(cdn_url.param("cdn.drag.segment"));
                peer_url.param("blocksize", drag_url.param("blocksize"));
                peer_url.param("filelength", drag_url.param("filelength"));
                peer_url.param("headlength", drag_url.param("headlength"));
                peer_url.param("rid", drag_url.param("rid"));
                cdn_url.param("cdn.drag.segment","");
            }
            framework::string::Url jump_url(cdn_url.param("cdn.jump"));
            peer_url.param("BWType", jump_url.param("bwtype"));
            cdn_url.param("cdn.jump", "");
            std::string cdn_str = framework::string::Url::encode(cdn_url.to_string());
            peer_url.param("url", cdn_str);
            addr_ = cdn_url.host_svc();

            return peer_url;
        }

        boost::system::error_code VodPeerSource::open(
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

            return PeerSource::open(peer_url, beg, end, ec);
        }

        void VodPeerSource::async_open(
            framework::string::Url const & url,
            boost::uint64_t beg, 
            boost::uint64_t end, 
            response_type const & resp)
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

            PeerSource::async_open(peer_url, beg, end, resp);
        }

    }//peer
}//ppbox
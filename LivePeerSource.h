//LivePeerSource.h

#ifndef PPBOX_PEER_LIVE_PEER_SOURCE_H_
#define PPBOX_PEER_LIVE_PEER_SOURCE_H_

#include "ppbox/peer/PeerSource.h"

#include <ppbox/common/SourceBase.h>

namespace ppbox
{
    namespace peer
    {
        class LivePeerSource
            : public PeerSource
        {
        public:
            LivePeerSource(
                boost::asio::io_service & ios_svc);
            ~LivePeerSource();

            boost::system::error_code open(
                framework::string::Url const & url,
                boost::uint64_t beg, 
                boost::uint64_t end, 
                boost::system::error_code & ec);

            void async_open(
                framework::string::Url const & url,
                boost::uint64_t beg, 
                boost::uint64_t end, 
                ppbox::common::SourceBase::response_type const & resp);

        private:
            framework::string::Url get_peer_url(
                framework::string::Url const & url );
        };

    }//peer
}//ppbox

#endif

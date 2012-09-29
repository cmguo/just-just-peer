// LivePeerSource.h

#ifndef PPBOX_PEER_LIVE_PEER_SOURCE_H_
#define PPBOX_PEER_LIVE_PEER_SOURCE_H_

#include "ppbox/peer/PeerSource.h"

namespace ppbox
{
    namespace peer
    {
        class LivePeerSource
            : public PeerSource
        {
        public:
            LivePeerSource(
                boost::asio::io_service & io_svc);

            ~LivePeerSource();

        private:
            virtual boost::system::error_code make_url(
                framework::string::Url const & cdn_url, 
                framework::string::Url & url);

        private:
            size_t seq_;
        };

        PPBOX_REGISTER_SOURCE(pplive2, LivePeerSource);
        PPBOX_REGISTER_SOURCE(pplive3, LivePeerSource);

    } // namespace peer
} // namespace ppbox

#endif // PPBOX_PEER_LIVE_PEER_SOURCE_H_

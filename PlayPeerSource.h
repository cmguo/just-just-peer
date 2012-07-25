//PlayPeerSource.h

#ifndef PPBOX_PEER_PLAY_PEER_SOURCE_H_
#define PPBOX_PEER_PLAY_PEER_SOURCE_H_

#include "ppbox/peer/PeerSource.h"

namespace ppbox
{
    namespace peer
    {
        class PlayPeerSource
            : public PeerSource
        {
        public:
            PlayPeerSource(
                boost::asio::io_service & ios_svc);

            ~PlayPeerSource();

        };

    }//peer
}//ppbox

#endif

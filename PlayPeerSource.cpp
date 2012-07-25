//PlayPeerSource.cpp

#include "ppbox/peer/Common.h"
#include "ppbox/peer/PlayPeerSource.h"

namespace ppbox
{
    namespace peer
    {
        PlayPeerSource::PlayPeerSource(
            boost::asio::io_service & ios_svc)
            : PeerSource(ios_svc)
        {
        }

        PlayPeerSource::~PlayPeerSource()
        {
        }

    }//peer
}//ppbox
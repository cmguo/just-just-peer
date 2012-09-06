//VodPeerSource.h

#ifndef PPBOX_PEER_JD_PEER_SOURCE_H_
#define PPBOX_PEER_JD_PEER_SOURCE_H_

#include "ppbox/peer/PeerSource.h"

namespace ppbox
{
    namespace peer
    {
        class VodPeerSource
            : public PeerSource
        {
        public:
            VodPeerSource(
                boost::asio::io_service & io_svc);

            ~VodPeerSource();

        private:
            virtual boost::system::error_code make_url(
                framework::string::Url const & cdn_url, 
                framework::string::Url & url);
        };

    }//peer
}//ppbox

#endif

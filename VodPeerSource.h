// VodPeerSource.h

#ifndef PPBOX_PEER_VOD_PEER_SOURCE_H_
#define PPBOX_PEER_VOD_PEER_SOURCE_H_

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
                boost::uint64_t beg, 
                boost::uint64_t end, 
                framework::string::Url & url);
        };

        PPBOX_REGISTER_URL_SOURCE("ppvod", VodPeerSource);
        PPBOX_REGISTER_URL_SOURCE("ppvod2", VodPeerSource);

    } // namespace peer
} // namespace ppbox

#endif // PPBOX_PEER_VOD_PEER_SOURCE_H_

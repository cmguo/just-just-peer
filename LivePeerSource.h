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

        public:
            virtual boost::uint64_t total(
                boost::system::error_code & ec);

        private:
            virtual bool prepare(
                framework::string::Url & url, 
                boost::uint64_t & beg, 
                boost::uint64_t & end, 
                boost::system::error_code & ec);

        private:
            size_t seq_;
        };

        UTIL_REGISTER_URL_SOURCE("pplive2", LivePeerSource);
        UTIL_REGISTER_URL_SOURCE("pplive3", LivePeerSource);

    } // namespace peer
} // namespace ppbox

#endif // PPBOX_PEER_LIVE_PEER_SOURCE_H_

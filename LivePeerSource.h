// LivePeerSource.h

#ifndef JUST_PEER_LIVE_PEER_SOURCE_H_
#define JUST_PEER_LIVE_PEER_SOURCE_H_

#include "just/peer/PeerSource.h"

namespace just
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
} // namespace just

#endif // JUST_PEER_LIVE_PEER_SOURCE_H_

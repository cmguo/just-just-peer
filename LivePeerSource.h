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
            virtual boost::system::error_code open(
                framework::string::Url const & url,
                boost::uint64_t beg, 
                boost::uint64_t end, 
                boost::system::error_code & ec);

            virtual void async_open(
                framework::string::Url const & url,
                boost::uint64_t beg, 
                boost::uint64_t end, 
                response_type const & resp);

            virtual boost::uint64_t total(
                boost::system::error_code & ec);

        private:
            virtual boost::system::error_code make_url(
                framework::string::Url const & cdn_url, 
                boost::uint64_t beg, 
                boost::uint64_t end, 
                framework::string::Url & url);

        private:
            size_t seq_;
        };

        PPBOX_REGISTER_URL_SOURCE("pplive2", LivePeerSource);
        PPBOX_REGISTER_URL_SOURCE("pplive3", LivePeerSource);

    } // namespace peer
} // namespace ppbox

#endif // PPBOX_PEER_LIVE_PEER_SOURCE_H_

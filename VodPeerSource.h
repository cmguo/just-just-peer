// VodPeerSource.h

#ifndef JUST_PEER_VOD_PEER_SOURCE_H_
#define JUST_PEER_VOD_PEER_SOURCE_H_

#include "just/peer/PeerSource.h"

namespace just
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
            // not use encodings
            virtual std::size_t private_read_some(
                buffers_t const & buffers,
                boost::system::error_code & ec);

            virtual void private_async_read_some(
                buffers_t const & buffers, 
                handler_t const & handler);

        private:
            virtual bool prepare(
                framework::string::Url & url, 
                boost::uint64_t & beg, 
                boost::uint64_t & end, 
                boost::system::error_code & ec);
        };

        UTIL_REGISTER_URL_SOURCE("ppvod", VodPeerSource);
        UTIL_REGISTER_URL_SOURCE("ppvod2", VodPeerSource);

    } // namespace peer
} // namespace just

#endif // JUST_PEER_VOD_PEER_SOURCE_H_

// PeerSource.h

#ifndef JUST_PEER_PEER_SOURCE_H_
#define JUST_PEER_PEER_SOURCE_H_

#include "just/peer/PeerModule.h"

#include <just/cdn/pptv/PptvP2pSource.h>

namespace just
{
    namespace peer_worker
    {
        class ClientStatus;
    }

    namespace peer
    {

        class PeerSource
            : public just::cdn::PptvP2pSource
        {
        public:
            PeerSource(
                boost::asio::io_service & io_svc);

            virtual ~PeerSource();

        private:
            virtual void on_stream_status(
                just::avbase::StreamStatus const & stat);

        protected:
            virtual void parse_param(
                std::string const & params);

            virtual bool prepare(
                framework::string::Url & url, 
                boost::uint64_t & beg, 
                boost::uint64_t & end, 
                boost::system::error_code & ec);

            bool use_peer();

        protected:
            PeerModule & module_;
            just::peer_worker::ClientStatus * status_;

        private:
            bool peer_fail_;
        };

    } // namespace peer
} // namespace just

#endif // JUST_PEER_PEER_SOURCE_H_

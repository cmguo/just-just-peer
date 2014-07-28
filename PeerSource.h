// PeerSource.h

#ifndef PPBOX_PEER_PEER_SOURCE_H_
#define PPBOX_PEER_PEER_SOURCE_H_

#include "ppbox/peer/PeerModule.h"

#include <ppbox/cdn/P2pSource.h>

namespace ppbox
{
    namespace peer_worker
    {
        class ClientStatus;
    }

    namespace peer
    {

        class PeerSource
            : public ppbox::cdn::P2pSource
        {
        public:
            PeerSource(
                boost::asio::io_service & io_svc);

            virtual ~PeerSource();

        private:
            virtual void on_stream_status(
                ppbox::data::StreamStatus const & stat);

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
            ppbox::peer_worker::ClientStatus * status_;

        private:
            bool peer_fail_;
        };

    } // namespace peer
} // namespace ppbox

#endif // PPBOX_PEER_PEER_SOURCE_H_

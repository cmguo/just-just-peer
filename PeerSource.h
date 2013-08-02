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

            virtual boost::system::error_code close(
                boost::system::error_code & ec);

        private:
            virtual void on_demux_stat(
                ppbox::demux::DemuxStatistic const & stat);

        protected:
            virtual void parse_param(
                std::string const & params);

            virtual boost::system::error_code make_url(
                framework::string::Url const & cdn_url, 
                boost::uint64_t beg, 
                boost::uint64_t end, 
                framework::string::Url & url);

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

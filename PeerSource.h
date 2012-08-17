//PeerSource.h

#ifndef PPBOX_PEER_PEER_SOURCE_H_
#define PPBOX_PEER_PEER_SOURCE_H_

#include "ppbox/peer/Peer.h"

#include <ppbox/common/HttpSource.h>

#include <framework/string/Url.h>

namespace ppbox
{
    namespace peer
    {

        class PeerSource
            : public  ppbox::common::HttpSource
        {
        public:
            PeerSource(
                boost::asio::io_service & ios_svc);
            ~PeerSource();

            void set_demux_statu(){};

        public:
            boost::system::error_code open(
                framework::string::Url const & url,
                boost::uint64_t beg, 
                boost::uint64_t end, 
                boost::system::error_code & ec);

            void async_open(
                framework::string::Url const & url,
                boost::uint64_t beg, 
                boost::uint64_t end, 
                response_type const & resp);

        protected:
            boost::uint16_t get_port() const;

        private:
            ppbox::peer::Peer & Peer_;

        };
    }//peer
}//ppbox

#endif

//PPBOX_PEER_PEER_SOURCE_H_
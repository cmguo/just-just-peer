//PeerSource.h

#ifndef PPBOX_PEER_PEER_SOURCE_H_
#define PPBOX_PEER_PEER_SOURCE_H_

#include "ppbox/peer/PeerModule.h"

#include <ppbox/data/HttpSource.h>

#include <framework/string/Url.h>

namespace ppbox
{
    namespace cdn
    {
        class PptvMedia;
    }

    namespace peer
    {

        class PeerSource
            : public  ppbox::data::HttpSource
        {
        public:
            PeerSource(
                boost::asio::io_service & io_svc);

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
            virtual boost::system::error_code make_url(
                framework::string::Url const & cdn_url, 
                framework::string::Url & url);

        protected:
            PeerModule & module_;
            ppbox::cdn::PptvMedia const * media_;
        };
    }//peer
}//ppbox

#endif

//PPBOX_PEER_PEER_SOURCE_H_
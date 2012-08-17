//VodPeerSource.h

#ifndef PPBOX_PEER_JD_PEER_SOURCE_H_
#define PPBOX_PEER_JD_PEER_SOURCE_H_

#include "ppbox/peer/PeerSource.h"

#include <ppbox/common/HttpSource.h>


namespace ppbox
{
    namespace peer
    {
        class VodPeerSource
            : public PeerSource
        {
        public:
            VodPeerSource(
                boost::asio::io_service & ios_svc);

            ~VodPeerSource();

            boost::system::error_code open(
                framework::string::Url const & url,
                boost::uint64_t beg, 
                boost::uint64_t end, 
                boost::system::error_code & ec);

            void async_open(
                framework::string::Url const & url,
                boost::uint64_t beg, 
                boost::uint64_t end, 
                SourceBase::response_type const & resp);

            void set_demux_statu();

        private:
            framework::string::Url get_peer_url(
                framework::string::Url const & url);
        };

    }//peer
}//ppbox

#endif

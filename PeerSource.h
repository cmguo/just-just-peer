//PeerSource.h

#ifndef PPBOX_PEER_PEER_SOURCE_H_
#define PPBOX_PEER_PEER_SOURCE_H_

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

        };
    }//peer
}//ppbox

#endif

//PPBOX_PEER_PEER_SOURCE_H_
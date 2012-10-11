// PeerSource.h

#ifndef PPBOX_PEER_PEER_SOURCE_H_
#define PPBOX_PEER_PEER_SOURCE_H_

#include "ppbox/peer/PeerModule.h"

#include <ppbox/cdn/HttpStatistics.h>

#include <ppbox/data/source/HttpSource.h>

#include <util/event/Event.h>

#include <framework/string/Url.h>

namespace ppbox
{
    namespace cdn
    {
        class PptvMedia;
    }

    namespace peer_worker
    {
        class ClientStatus;
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

        public:
            ppbox::cdn::HttpStatistics const & http_stat() const;

            void pptv_media(
                ppbox::cdn::PptvMedia const & media);

            ppbox::cdn::PptvMedia const & pptv_media()
            {
                return *pptv_media_;
            }

        private:
            void on_event(
                util::event::Event const & e);

        protected:
            virtual void parse_param(
                std::string const & params);

            virtual boost::system::error_code make_url(
                framework::string::Url const & cdn_url, 
                framework::string::Url & url);

            void open_log(
                bool end);

        protected:
            PeerModule & module_;
            ppbox::cdn::PptvMedia const * pptv_media_;
            ppbox::peer_worker::ClientStatus * status_;
            ppbox::cdn::HttpStatistics http_stat_;
        };

    } // namespace peer
} // namespace ppbox

#endif // PPBOX_PEER_PEER_SOURCE_H_
// Peer.h

#ifndef _PPBOX_PEER_PEER_H_
#define _PPBOX_PEER_PEER_H_

#include "ppbox/peer/VodInfo.h"

#include <ppbox/common/HttpFetchManager.h>
#include <ppbox/common/PortManager.h>

#ifndef PPBOX_DISABLE_DAC
#include <ppbox/dac/Dac.h>
#endif

#ifndef PPBOX_CONTAIN_PEER_WORKER
#include <framework/process/NamedMutex.h>
using namespace framework::process;

namespace framework
{
    namespace process { class Process; }
    namespace timer { class Timer; }
}
#endif

namespace ppbox
{

    namespace peer
    {

        class Peer
            : public ppbox::common::HttpFetchManager
        {
        public:

            typedef boost::function<void (
                boost::system::error_code const &, 
                ppbox::peer::VodJumpInfo const & jump_info)
            > jump_response_type;

            typedef boost::function<void (
                boost::system::error_code const &, 
                ppbox::peer::VodDragInfoNew const& drag_info)
            > drag_response_type;

            Peer(
                util::daemon::Daemon & daemon);

            ~Peer();

        public:
            virtual boost::system::error_code startup();

            virtual void shutdown();

        public:

        ppbox::common::FetchHandle async_fetch_jump(
            std::string const & playlink, 
            std::string const & client_type,
            jump_response_type const & resq);

        ppbox::common::FetchHandle async_fetch_drag(
            std::string const & playlink, 
            std::string const & client_type, 
            ppbox::peer::VodJumpInfo const & jump_info,
            drag_response_type const & resq);

        private:
            void jump_callback(
                jump_response_type const & resq
                ,boost::system::error_code const & ec
                ,boost::asio::streambuf & buf);

            void drag_callback(
                drag_response_type const & resq
                ,boost::system::error_code const & ec
                ,boost::asio::streambuf & buf);

        public:
            boost::uint16_t port() const
            {
                return port_;
            }

            bool is_alive();

#ifndef PPBOX_CONTAIN_PEER_WORKER
            framework::process::Process const & process() const
            {
                return *process_;
            }
#endif

        public:
            static std::string version();

            static std::string name();

        private:
            void check();

            bool is_lock();

        private:
#ifndef PPBOX_DISABLE_DAC
            ppbox::dac::Dac& dac_;
#endif
            ppbox::common::PortManager &portMgr_;

            boost::uint16_t port_;

#ifndef PPBOX_CONTAIN_PEER_WORKER
        private:
            framework::process::Process * process_;
            framework::timer::Timer * timer_;

            NamedMutex mutex_;

            bool is_locked_;
#endif
        };

    } // namespace peer
} // namespace ppbox

#endif // _PPBOX_PEER_PEER_H_

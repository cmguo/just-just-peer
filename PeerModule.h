// PeerModule.h

#ifndef _PPBOX_PEER_PEER_MODULE_H_
#define _PPBOX_PEER_PEER_MODULE_H_

#include <ppbox/common/PortManager.h>
#include <ppbox/peer_worker/ClientStatus.h>

#ifndef PPBOX_DISABLE_DAC
#include <ppbox/dac/Dac.h>
#endif

#ifndef PPBOX_CONTAIN_PEER_WORKER
#include <framework/process/NamedMutex.h>

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

        class PeerModule
            : public ppbox::common::CommonModuleBase<PeerModule>
        {
        public:

            PeerModule(
                util::daemon::Daemon & daemon);

            ~PeerModule();

        public:
            virtual boost::system::error_code startup();

            virtual void shutdown();

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
            ppbox::peer_worker::ClientStatus * alloc_status();

            void free_status(
                ppbox::peer_worker::ClientStatus * status);

            void update_status(
                ppbox::peer_worker::ClientStatus * status);

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

            framework::container::List<ppbox::peer_worker::ClientStatus> * stats_;
#ifndef PPBOX_CONTAIN_PEER_WORKER
        private:
            framework::process::Process * process_;
            framework::timer::Timer * timer_;

            framework::process::NamedMutex mutex_;

            bool is_locked_;
#endif
        };

    } // namespace peer
} // namespace ppbox

#endif // _PPBOX_PEER_PEER_MODULE_H_

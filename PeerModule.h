// PeerModule.h

#ifndef _JUST_PEER_PEER_MODULE_H_
#define _JUST_PEER_PEER_MODULE_H_

#include <just/common/PortManager.h>
#include <just/peer_worker/ClientStatus.h>

#ifndef JUST_DISABLE_DAC
#include <just/dac/DacModule.h>
#endif

#ifndef JUST_CONTAIN_PEER_WORKER
#include <framework/process/NamedMutex.h>



namespace framework
{
    namespace process { class Process; }
    namespace timer { class Timer; }
}
#endif

namespace just
{
    namespace peer
    {

        class PeerModule
            : public just::common::CommonModuleBase<PeerModule>
        {
        public:

            PeerModule(
                util::daemon::Daemon & daemon);

            ~PeerModule();

        public:
            virtual bool startup(
                boost::system::error_code & ec);

            virtual bool shutdown(
                boost::system::error_code & ec);

        public:
            boost::uint16_t port() const
            {
                return port_;
            }

            bool is_alive();

#ifndef JUST_CONTAIN_PEER_WORKER
            framework::process::Process const & process() const
            {
                return *process_;
            }
#endif

        public:
            just::peer_worker::ClientStatus * alloc_status();

            void free_status(
                just::peer_worker::ClientStatus * status);

            void update_status(
                just::peer_worker::ClientStatus * status);

        public:
            static std::string version();

            static std::string name();

        private:
            void check();

            bool is_lock();

        private:
#ifndef JUST_DISABLE_DAC
            just::dac::DacModule& dac_;
#endif
            just::common::PortManager &portMgr_;

            boost::uint16_t port_;

            framework::container::List<just::peer_worker::ClientStatus> * stats_;
#ifndef JUST_CONTAIN_PEER_WORKER
        private:
            framework::process::Process * process_;
            framework::timer::Timer * timer_;

            framework::process::NamedMutex mutex_;

            bool is_locked_;
#endif
        };

    } // namespace peer
} // namespace just

#endif // _JUST_PEER_PEER_MODULE_H_

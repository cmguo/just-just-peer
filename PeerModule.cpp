// PeerModule.cpp

#include "just/peer/Common.h"
#include "just/peer/PeerModule.h"
#include "just/peer/Error.h"
#include "just/peer/ClassRegister.h"

#include <just/peer_worker/Name.h>

#ifndef JUST_DISABLE_DAC
#include <just/dac/DacModule.h>
#include <just/dac/DacInfoWorker.h>
using namespace just::dac;
#endif

#ifdef JUST_CONTAIN_PEER_WORKER
namespace just { namespace peer_worker {
    void register_module(util::daemon::Daemon & daemon);
}}
#else
#include <framework/process/Process.h>
#include <framework/timer/Timer.h>
using namespace framework::timer;
using namespace framework::process;
#endif
#include <framework/memory/MemoryReference.h>
#include <framework/system/ErrorCode.h>
#include <framework/system/LogicError.h>
#include <framework/string/Format.h>
#include <framework/logger/StreamRecord.h>
using namespace framework::system;
using namespace framework::string;

#include <boost/bind.hpp>
using namespace boost::system;

namespace just
{
    namespace peer
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("just.peer.PeerModule", framework::logger::Debug)

        PeerModule::PeerModule(
            util::daemon::Daemon & daemon)
            : just::common::CommonModuleBase<PeerModule>(daemon, "PeerModule")
#ifndef JUST_DISABLE_DAC
            , dac_(util::daemon::use_module<just::dac::DacModule>(daemon))
#endif
            , portMgr_(util::daemon::use_module<just::common::PortManager>(daemon))
            , port_(9000)
#ifndef JUST_CONTAIN_PEER_WORKER
            , mutex_(9000)
            , is_locked_(false)
#endif
        {
#ifdef JUST_CONTAIN_PEER_WORKER
            just::peer_worker::register_module(daemon);
#else
            process_ = new Process;
            timer_ = new Timer(timer_queue(),
                    10, // 5 seconds
                boost::bind(&PeerModule::check, this));
#endif

            just::peer_worker::ClientStatus::set_pool(framework::memory::BigFixedPool(
                framework::memory::MemoryReference<framework::memory::SharedMemory>(shared_memory())));

            stats_ = (framework::container::List<just::peer_worker::ClientStatus> *)shared_memory()
                .alloc_with_id(SHARED_OBJECT_ID_DEMUX, sizeof(framework::container::List<just::peer_worker::ClientStatus>));
            if (!stats_)
                stats_ = (framework::container::List<just::peer_worker::ClientStatus> *)shared_memory()
                .get_by_id(SHARED_OBJECT_ID_DEMUX);
            new (stats_) framework::container::List<just::peer_worker::ClientStatus>;
        }

        PeerModule::~PeerModule()
        {
            just::peer_worker::ClientStatus::set_pool(framework::memory::BigFixedPool(
                framework::memory::PrivateMemory()));

#ifndef JUST_CONTAIN_PEER_WORKER

            if (is_lock()) {
                mutex_.unlock();
                is_locked_ = false;
            }
            if (process_) {
                delete process_;
                process_ = NULL;
            }
            if (timer_) {
                delete timer_;
                timer_ = NULL;
            }
#endif
        }



        bool PeerModule::startup(
            error_code & ec)
        {
            LOG_INFO("[startup]");
#ifndef JUST_DISABLE_DAC
            dac_.set_vod_version(version());
            dac_.set_vod_name(name());
#endif
#ifndef JUST_CONTAIN_PEER_WORKER
            timer_->start();

            if (is_lock()) {
                LOG_INFO("[startup] try_lock");
#ifdef __APPLE__
                boost::filesystem::path cmd_file(MAC_PEER_WORKER);
#else
                boost::filesystem::path cmd_file(just::peer_worker::name_string());
#endif
                Process::CreateParamter param;
                param.wait = true;
                process_->open(cmd_file, param, ec);
                if (!ec) {
                    portMgr_.get_port(just::common::vod,port_);
                    LOG_INFO("[startup] ok port:"<<port_);
                } else {
                    LOG_WARN("[startup] ec = " << ec.message());
                    port_ = 0;
                    if (ec == boost::system::errc::no_such_file_or_directory) {
                        ec.clear();
                    }

                    timer_->stop();
                }
            }
#else
            portMgr_.get_port(just::common::vod,port_);
            LOG_INFO("[startup] ok port:"<<port_);
#endif
            return !ec;
        }

        bool PeerModule::shutdown(
            error_code & ec)
        {
#ifndef JUST_CONTAIN_PEER_WORKER
            if (process_) {
                process_->signal(Signal::sig_int, ec);
                process_->timed_join(1000, ec);
                if (!ec) {
                    LOG_INFO("[shutdown] ok");
                } else {
                    LOG_WARN("[shutdown] ec = " << ec.message());
                }
                process_->kill(ec);
                process_->close(ec);
            }
            if (timer_) {
                timer_->stop();
            }
            if (is_locked_) {
                mutex_.unlock();
                is_locked_ = false;
            }
#endif
            return !ec;
        }

        void PeerModule::check()
        {
#ifndef JUST_CONTAIN_PEER_WORKER
            error_code ec;
            if (is_lock()) {
                if (process_ && !process_->is_alive(ec)) {
                    LOG_ERROR("[check] worker is dead: " << ec.message());

#ifndef JUST_DISABLE_DAC
                    util::daemon::use_module<just::dac::DacModule>(get_daemon())
                        .submit(DacRestartInfo(DacRestartInfo::vod));
#endif
                    process_->close(ec);
#ifdef __APPLE__
                    boost::filesystem::path cmd_file(MAC_PEER_WORKER);
#else
                    boost::filesystem::path cmd_file(just::peer_worker::name_string());
#endif
                    Process::CreateParamter param;
                    param.wait = true;
                    process_->open(cmd_file, param, ec);
                    if (!ec) {
                        portMgr_.get_port(just::common::vod,port_);
                        LOG_INFO("[check] ok port:"<<port_);
                    } else {
                        LOG_WARN("[check] ec = " << ec.message());
                        port_ = 0;

                        timer_->stop();
                    }
                }
            }
#endif
        }

        bool PeerModule::is_alive()
        {
            error_code ec;
#ifdef JUST_CONTAIN_PEER_WORKER
            return true;
#else

            if (is_locked_) {
                return process_ && process_->is_alive(ec);
            } else {
                framework::process::Process process;
#ifdef __APPLE__
                boost::filesystem::path cmd_file(MAC_PEER_WORKER);
#else
                boost::filesystem::path cmd_file(just::peer_worker::name_string());
#endif
                process.open(cmd_file, ec);
                return !ec;
            }
#endif
        }

        just::peer_worker::ClientStatus * PeerModule::alloc_status()
        {
            LOG_DEBUG("alloc_status");
            just::peer_worker::ClientStatus * stat = new just::peer_worker::ClientStatus;
            stats_->insert(stat);
            return stat;
        }

        void PeerModule::free_status(
            just::peer_worker::ClientStatus * status)
        {
            // auto detach from list
            delete status;
        }



        void PeerModule::update_status(
            just::peer_worker::ClientStatus * status)
        {

        }

        std::string PeerModule::version()
        {
            //return just::peer_worker::version_string();
            return "1.0.0.1";
        }

        std::string PeerModule::name()
        {
            return just::peer_worker::name_string();
        }

#ifndef JUST_CONTAIN_PEER_WORKER
        bool PeerModule::is_lock()
        {
            if (!is_locked_) {
                is_locked_ = mutex_.try_lock();
            }

            return is_locked_;
        }
#endif

    } // namespace peer
} // namespace just

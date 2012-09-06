// PeerModule.cpp

#include "ppbox/peer/Common.h"
#include "ppbox/peer/PeerModule.h"
#include "ppbox/peer/Error.h"
#include "ppbox/peer/VodPeerSource.h"
#include "ppbox/peer/LivePeerSource.h"

#include <ppbox/data/SourceBase.h>

#include <ppbox/peer_worker/Name.h>

#ifndef PPBOX_DISABLE_DAC
#include <ppbox/dac/Dac.h>
#endif

#ifdef PPBOX_CONTAIN_PEER_WORKER
#include <ppbox/peer_worker/StatusProxy.h>
#include <ppbox/peer_worker/WorkerModule.h>
using namespace ppbox::peer_worker;
#else
#include <framework/process/Process.h>
#include <framework/timer/Timer.h>
using namespace framework::timer;
using namespace framework::process;
#endif

#include <util/buffers/BufferCopy.h>

#include <framework/system/ErrorCode.h>
#include <framework/system/LogicError.h>
#include <framework/string/Format.h>
#include <framework/logger/StreamRecord.h>
using namespace framework::system;
using namespace framework::string;
using namespace framework::logger;

#include <boost/bind.hpp>
using namespace boost::system;

namespace ppbox
{
    namespace peer
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("PeerModule", 0)

        PPBOX_REGISTER_SOURCE(ppvod, VodPeerSource);
        PPBOX_REGISTER_SOURCE(pplive2, LivePeerSource);

#ifdef PPBOX_CONTAIN_PEER_WORKER
        PeerModule::PeerModule(
            util::daemon::Daemon & daemon)
            : ppbox::common::CommonModuleBase<PeerModule>(daemon, "PeerModule")
#ifndef PPBOX_DISABLE_DAC
            , dac_(util::daemon::use_module<ppbox::dac::Dac>(daemon))
#endif
            , portMgr_(util::daemon::use_module<ppbox::common::PortManager>(daemon))
            , port_(9000)
        {

            util::daemon::use_module<ppbox::peer_worker::WorkerModule>(daemon);
            util::daemon::use_module<ppbox::peer_worker::StatusProxy>(daemon);
        }
#else
        PeerModule::PeerModule(
            util::daemon::Daemon & daemon)
            : ppbox::common::CommonModuleBase<PeerModule>(daemon, "PeerModule")
#ifndef PPBOX_DISABLE_DAC
            , dac_(util::daemon::use_module<ppbox::dac::Dac>(daemon))
#endif
            , portMgr_(util::daemon::use_module<ppbox::common::PortManager>(daemon))
            , port_(9000)
            , mutex_(9000)
            , is_locked_(false)
        {

            process_ = new Process;
            timer_ = new Timer(timer_queue(), 
                10, // 5 seconds
                boost::bind(&PeerModule::check, this));
        }
#endif

        PeerModule::~PeerModule()
        {
#ifndef PPBOX_CONTAIN_PEER_WORKER

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


        error_code PeerModule::startup()
        {
            error_code ec;
            LOG_INFO("[startup]");
#ifndef PPBOX_DISABLE_DAC
            dac_.set_vod_version(version());
            dac_.set_vod_name(name());
#endif
#ifndef PPBOX_CONTAIN_PEER_WORKER
            timer_->start();

            if (is_lock()) {
                LOG_INFO("[startup] try_lock");
#ifdef __APPLE__
                boost::filesystem::path cmd_file(MAC_PEER_WORKER);
#else
                boost::filesystem::path cmd_file(ppbox::peer_worker::name_string());
#endif
                Process::CreateParamter param;
                param.wait = true;
                process_->open(cmd_file, param, ec);
                if (!ec) {
                    portMgr_.get_port(ppbox::common::vod,port_);
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
			portMgr_.get_port(ppbox::common::vod,port_);
			LOG_INFO("[startup] ok port:"<<port_);
#endif
            return ec;
        }

        void PeerModule::check()
        {
#ifndef PPBOX_CONTAIN_PEER_WORKER
            error_code ec;
            if (is_lock()) {
                if (process_ && !process_->is_alive(ec)) {
                    LOG_ERROR("[check] worker is dead: " << ec.message());

#ifndef PPBOX_DISABLE_DAC
                    util::daemon::use_module<ppbox::dac::Dac>(get_daemon())
                        .run_info(ppbox::dac::CoreType::vod);
#endif
                    process_->close(ec);
#ifdef __APPLE__
                    boost::filesystem::path cmd_file(MAC_PEER_WORKER);
#else
                    boost::filesystem::path cmd_file(ppbox::peer_worker::name_string());
#endif
                    Process::CreateParamter param;
                    param.wait = true;
                    process_->open(cmd_file, param, ec);
                    if (!ec) {
                        portMgr_.get_port(ppbox::common::vod,port_);
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
#ifdef PPBOX_CONTAIN_PEER_WORKER
            return true;
#else

            if (is_locked_) {
                return process_ && process_->is_alive(ec);
            } else {
                framework::process::Process process;
#ifdef __APPLE__
                boost::filesystem::path cmd_file(MAC_PEER_WORKER);
#else
                boost::filesystem::path cmd_file(ppbox::peer_worker::name_string());
#endif
                process.open(cmd_file, ec);
                return !ec;
            }
#endif
        }

        void PeerModule::shutdown()
        {
#ifndef PPBOX_CONTAIN_PEER_WORKER
            if (process_) {
                error_code ec;
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
        }

        std::string PeerModule::version()
        {
            //return ppbox::peer_worker::version_string();
            return "1.0.0.1";
        }

        std::string PeerModule::name()
        {
            return ppbox::peer_worker::name_string();
        }

#ifndef PPBOX_CONTAIN_PEER_WORKER
        bool PeerModule::is_lock()
        {
            if (!is_locked_) {
                is_locked_ = mutex_.try_lock();
            }

            return is_locked_;
        }
#endif

    } // namespace PeerModule
} // namespace ppbox

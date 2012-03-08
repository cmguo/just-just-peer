// Peer.cpp

#include "ppbox/peer/Common.h"
#include "ppbox/peer/Peer.h"
#include "ppbox/peer/Error.h"

#include <ppbox/peer_worker/Name.h>

#ifndef PPBOX_DISABLE_DAC
#include <ppbox/dac/Dac.h>
using namespace ppbox::dac;
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
#include <framework/logger/LoggerStreamRecord.h>
using namespace framework::system;
using namespace framework::string;
using namespace framework::logger;

#include <boost/bind.hpp>
using namespace boost::system;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("Peer", 0)


#ifndef PPBOX_DNS_VOD_JUMP
#define PPBOX_DNS_VOD_JUMP "(tcp)(v4)jump.150hi.com:80"
#endif

#ifndef PPBOX_DNS_VOD_DRAG
#define PPBOX_DNS_VOD_DRAG "(tcp)(v4)drag.150hi.com:80"
#endif

namespace ppbox
{
    namespace peer
    {

        static const framework::network::NetName dns_vod_jump_server(PPBOX_DNS_VOD_JUMP);
        static const framework::network::NetName dns_vod_drag_server(PPBOX_DNS_VOD_DRAG);

#ifdef PPBOX_CONTAIN_PEER_WORKER
        Peer::Peer(
            util::daemon::Daemon & daemon)
            : HttpFetchManager(daemon)
            , port_(9000)
        {
        
			util::daemon::use_module<ppbox::peer_worker::WorkerModule>(daemon);															
			util::daemon::use_module<ppbox::peer_worker::StatusProxy>(daemon);
        }
#else
        Peer::Peer(
            util::daemon::Daemon & daemon)
            : HttpFetchManager(daemon)
            , port_(9000)
            , mutex_(9000)
            , is_locked_(false)
        {
 
			process_ = new Process;
            timer_ = new Timer(timer_queue(), 
                10, // 5 seconds
                boost::bind(&Peer::check, this));
        }
#endif

        Peer::~Peer()
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


        error_code Peer::startup()
        {
            error_code ec;
            LOG_S(Logger::kLevelEvent, "[startup]");
#ifndef PPBOX_CONTAIN_PEER_WORKER
            timer_->start();

            if (is_lock()) {
                LOG_S(Logger::kLevelEvent, "[startup] try_lock");

                boost::filesystem::path cmd_file(ppbox::peer_worker::name_string());
                Process::CreateParamter param;
                param.wait = true;
                process_->open(cmd_file, param, ec);
                if (!ec) {
                    LOG_S(Logger::kLevelEvent, "[startup] ok");
                } else {
                    LOG_S(Logger::kLevelAlarm, "[startup] ec = " << ec.message());
                    port_ = 0;
                    if (ec == boost::system::errc::no_such_file_or_directory) {
                        ec.clear();
                    }

                    timer_->stop();
                }
            }
#endif			
            return ec;
        }

        void Peer::check()
        {
#ifndef PPBOX_CONTAIN_PEER_WORKER
            error_code ec;
            if (is_lock()) {
                if (process_ && !process_->is_alive(ec)) {
                    LOG_S(Logger::kLevelError, "[check] worker is dead: " << ec.message());

#ifndef PPBOX_DISABLE_DAC
                    util::daemon::use_module<ppbox::dac::Dac>(get_daemon())
                        .run_info(CoreType::vod);
#endif
                    process_->close(ec);
                    boost::filesystem::path cmd_file(ppbox::peer_worker::name_string());
                    Process::CreateParamter param;
                    param.wait = true;
                    process_->open(cmd_file, param, ec);
                    if (!ec) {
                        LOG_S(Logger::kLevelEvent, "[check] ok");
                    } else {
                        LOG_S(Logger::kLevelAlarm, "[check] ec = " << ec.message());
                        port_ = 0;

                        timer_->stop();
                    }
                }
            }
#endif			
        }

        bool Peer::is_alive()
        {
            error_code ec;
#ifdef PPBOX_CONTAIN_PEER_WORKER
 			return true;
 #else

			if (is_locked_) {
                return process_ && process_->is_alive(ec);
            } else {
                framework::process::Process process;
                boost::filesystem::path cmd_file(ppbox::peer_worker::name_string());
                process.open(cmd_file, ec);
                return !ec;
            }
 #endif 			
        }

        void Peer::shutdown()
        {
#ifndef PPBOX_CONTAIN_PEER_WORKER
            if (process_) {
                error_code ec;
                process_->signal(Signal::sig_int, ec);
                process_->timed_join(1000, ec);
                if (!ec) {
                    LOG_S(Logger::kLevelEvent, "[shutdown] ok");
                } else {
                    LOG_S(Logger::kLevelAlarm, "[shutdown] ec = " << ec.message());
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

        std::string Peer::version()
        {
            //return ppbox::peer_worker::version_string();
            return "1.0.0.1";
        }

        std::string Peer::name()
        {
            return ppbox::peer_worker::name_string();
        }

#ifndef PPBOX_CONTAIN_PEER_WORKER
        bool Peer::is_lock()
        {
            if (!is_locked_) {
                is_locked_ = mutex_.try_lock();
            }

            return is_locked_;
        }
#endif

        ppbox::common::FetchHandle Peer::async_fetch_jump(
            std::string const & playlink, 
            std::string const & client_type,
            jump_response_type const & resq)
        {
            LOG_S(framework::logger::Logger::kLevelDebug,"[async_fetch_jump]");

            framework::string::Url url("http://localhost/");
            url.host(dns_vod_jump_server.host());
            url.svc(dns_vod_jump_server.svc());
            url.path("/" + playlink + "dt");
            url.param("type", client_type);
            url.param("t", format(rand()));

            return  async_fetch(url
                ,dns_vod_jump_server
                ,boost::bind(&Peer::jump_callback,this,resq,_1,_2)
                );
        }

        void Peer::jump_callback(
            jump_response_type const & resq
            ,boost::system::error_code const & ec
            ,boost::asio::streambuf & buf)
        {
            boost::system::error_code ec1 = ec;
            ppbox::peer::VodJumpInfo jump_info;
            if(!ec1)
            {
                std::string buffer = boost::asio::buffer_cast<char const *>(buf.data());
                LOG_S(Logger::kLevelDebug2, "[drag_callback] jump buffer: " << buffer);

                boost::asio::streambuf buf2;
                util::buffers::buffer_copy(
                    buf2.prepare(buf.size()), 
                    buf.data());
                buf2.commit(buf.size());

                util::archive::XmlIArchive<> ia(buf2);
                ia >> (ppbox::peer::VodJumpInfo &)jump_info;
                if (!ia) 
                {
                    util::archive::XmlIArchive<> ia2(buf);
                    ia2 >> jump_info;
                    if (!ia2) 
                    {
                        ec1 = ppbox::peer::error::bad_xml_format;
                    }
                }
            }

            resq(ec1,jump_info);
        }

        

        ppbox::common::FetchHandle Peer::async_fetch_drag(
            std::string const & playlink, 
            std::string const & client_type, 
            ppbox::peer::VodJumpInfo const & jump_info,
            drag_response_type const & resq)
        {
            LOG_S(framework::logger::Logger::kLevelDebug,"[async_fetch_drag]");


            framework::string::Url url("http://localhost/");
            url.host(dns_vod_drag_server.host());
            url.svc(dns_vod_drag_server.svc());
            url.path("/" + playlink + "0drag");
            url.param("type", client_type);

            return  async_fetch(url
                ,jump_info.server_host
                ,boost::bind(&Peer::drag_callback,this,resq,_1,_2)
                );
        }
        
        void Peer::drag_callback(
            drag_response_type const & resq
            ,boost::system::error_code const & ec
            ,boost::asio::streambuf & buf)
        {
            boost::system::error_code ec1 = ec;
            ppbox::peer::VodDragInfoNew drag_info;
            if(!ec1)
            {
                std::string buffer = boost::asio::buffer_cast<char const *>(buf.data());
                LOG_S(Logger::kLevelDebug2, "[drag_callback] jump buffer: " << buffer);

                boost::asio::streambuf buf2;
                util::buffers::buffer_copy(buf2.prepare(buf.size()), buf.data());
                buf2.commit(buf.size());

                util::archive::XmlIArchive<> ia(buf2);
                ia >> (ppbox::peer::VodDragInfoNew &)drag_info;
                if (!ia) 
                {
                    util::archive::XmlIArchive<> ia2(buf);
                    ppbox::peer::VodDragInfo drag_info_old;
                    ia2 >> drag_info_old;
                    if (!ia2) {
                        ec1 = ppbox::peer::error::bad_xml_format;
                    } else {
                        drag_info = drag_info_old;
                    }
                }
            }
            resq(ec1,drag_info);
        }


    } // namespace Peer
} // namespace ppbox

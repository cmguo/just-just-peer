//PeerSource.cpp

#include "ppbox/peer/Common.h"
#include "ppbox/peer/PeerSource.h"

#include <util/protocol/http/HttpError.h>

#include <framework/string/Url.h>
#include <framework/logger/LoggerStreamRecord.h>

#include <boost/asio/read.hpp>
#include <boost/asio/buffer.hpp>

using namespace framework::logger;

namespace ppbox
{
    namespace peer
    {
        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("PeerSource", 0);

        PeerSource::PeerSource(
            boost::asio::io_service & ios_svc)
            : HttpSource(ios_svc)
        {
        }

        PeerSource::~PeerSource()
        {
        }

    }//peer
}//ppbox


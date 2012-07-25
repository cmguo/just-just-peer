// BigMp4Statistic.cpp

#include "ppbox/peer/Common.h"
#include "ppbox/peer/BigMp4Statistic.h"

namespace ppbox
{
    namespace peer
    {
        BigMp4Statistic::BigMp4Statistic()
            : increase_bytes_(0)
        {
            tick_counter_.start();
        }

        BigMp4Statistic::~BigMp4Statistic()
        {
        }

        void BigMp4Statistic::open(
            std::string name,
            boost::uint32_t head_size,
            boost::uint32_t tail_size,
            boost::uint32_t body_size)
        {
            name_ = name;
            info_.head_size = head_size;
            info_.tail_size = tail_size;
            info_.body_size = body_size;
            tick_counter_.reset();
        }

        void BigMp4Statistic::increase(
            boost::uint32_t bytes,
            boost::uint64_t offset)
        {
            info_.cur_offset = offset;
            boost::uint32_t time_elapsed = tick_counter_.elapsed();
            increase_bytes_ += bytes;
            if (time_elapsed > 500) {
                float percent = 0;
                if (info_.tail_size > 0) {
                    if (offset <= info_.head_size 
                        || offset >= (info_.head_size + info_.body_size)) {
                            percent = (float)(offset > info_.head_size ? (offset - info_.body_size ) : offset) / (info_.head_size + info_.tail_size);
                            info_.area = 0;
                    } else {
                        percent = (float)(offset - info_.head_size) / info_.body_size;
                        info_.area = 1;
                    }
                } else {
                    if (offset <= info_.head_size) {
                        info_.area = 0;
                        percent = (float)offset / info_.head_size;
                    } else {
                        info_.area = 1;
                        percent = (float)(offset - info_.head_size) / info_.body_size;
                    }
                }

                info_.percent = boost::uint32_t(percent * 100);
                // out info
                //if (info_.area == 0) {
                //    std::cout << "head: " << std::endl;
                //} else if (info_.area == 1) {
                //    std::cout << "body: " << std::endl;
                //} else {
                //    std::cout << "nodata: " << std::endl;
                //}
                //std::cout << "percent: " << info_.percent << std::endl;
                //std::cout << "speed: " << info_.speed << std::endl;
                // end out info
                info_.speed = (increase_bytes_ * 1000) / time_elapsed;
                tick_counter_.reset();
                increase_bytes_ = 0;
            }
        }

        boost::system::error_code BigMp4Statistic::get_info_statictis(BigMp4Info & info)
        {
            info = info_;
            return boost::system::error_code();
        }

        void BigMp4Statistic::close(void)
        {
        }

    }
}

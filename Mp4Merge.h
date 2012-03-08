// Vod.h

#ifndef _PPBOX_PEER_MP4_MERGE_H_
#define _PPBOX_PEER_MP4_MERGE_H_

#include <ppbox/avformat/mp4/Ap4HeadMerge.h>

namespace ppbox
{
    namespace peer
    {
        class Mp4Merge
        {
        public:
            Mp4Merge();
            virtual ~Mp4Merge();

            boost::system::error_code meger_small_head(
                std::iostream & ios,
                std::ostream & os, 
                std::vector<ppbox::avformat::SegmentInfo> & segment_infos,
                boost::uint32_t& head_size,
                boost::system::error_code & ec);

            boost::system::error_code vod_valid_segment_info(
                std::istream & is,
                std::vector<ppbox::avformat::SegmentInfo> & segment_infos,
                boost::system::error_code & ec);
        };
    } // namespace peer
} // namespace ppbox

#endif // _PPBOX_VOD_VOD_H_

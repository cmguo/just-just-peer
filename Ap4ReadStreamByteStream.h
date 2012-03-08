// AP4_ReadStreamByteStream.h
#ifndef _PPBOX_PEER_AP4_READ_STREAMBYTESTREAM_H_
#define _PPBOX_PEER_AP4_READ_STREAMBYTESTREAM_H_

//#include <ppbox/download/PptvMp4Head.h>
#include "bento4/Core/Ap4ByteStream.h"
#include <istream>

namespace ppbox
{
    namespace peer
    {
        class AP4_ReadStreamByteStream
            : public AP4_ByteStream
        {
        public:
            static AP4_Result Create(
                std::istream& ios
                ,AP4_Size offset
                ,AP4_Size size
                ,AP4_ByteStream* &stream);


            AP4_ReadStreamByteStream(
                std::istream& ios
                ,AP4_Size offset //当前偏移位置
                ,AP4_Size size);//总大小

            virtual ~AP4_ReadStreamByteStream();

            AP4_Result ReadPartial(void*     buffer, 
                AP4_Size  bytes_to_read, 
                AP4_Size& bytes_read);

            AP4_Result WritePartial(const void* buffer, 
                AP4_Size    bytes_to_write, 
                AP4_Size&   bytes_written) ;;

            AP4_Result Seek(AP4_Position position);
            AP4_Result Tell(AP4_Position& position);
            AP4_Result GetSize(AP4_LargeSize& size);

            void AddReference();
            void Release();
            AP4_Result Flush();
        private:
            std::istream& ios_;
            AP4_Size ref_;
            AP4_Size offset_;
            AP4_Size size_;
            AP4_Size position_;
        };
    }
}

#endif /* _PPBOX_AP4_READ_STREAMBYTESTREAM_H_ */

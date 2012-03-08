// AP4_WriteStreamByteStream.h
#ifndef _PPBOX_PEER_AP4_WRITE_STREAMBYTESTREAM_H_
#define _PPBOX_PEER_AP4_WRITE_STREAMBYTESTREAM_H_

//#include <ppbox/download/PptvMp4Head.h>
#include "bento4/Core/Ap4ByteStream.h"
#include <ostream>

namespace ppbox
{
    namespace peer
    {
        class AP4_WriteStreamByteStream
            : public AP4_ByteStream
        {
        public:


            static AP4_Result Create(
                std::ostream& ios
                ,AP4_Size offset
                ,AP4_Size size
                ,AP4_ByteStream* &stream);


            AP4_WriteStreamByteStream(
                std::ostream& ios
                ,AP4_Size offset //��ǰƫ��λ��
                ,AP4_Size size);//�ܴ�С

            virtual ~AP4_WriteStreamByteStream();

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
            std::ostream& ios_;
            AP4_Size ref_;
            AP4_Size offset_;
            AP4_Size size_;
            AP4_Size position_;
        };
    }
}
#endif /* _PPBOX_AP4_WRITE_STREAMBYTESTREAM_H_ */

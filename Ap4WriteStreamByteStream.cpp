#include "ppbox/peer/Common.h"
#include "ppbox/peer/Ap4WriteStreamByteStream.h"

namespace ppbox
{
    namespace peer
    {
        //AP4_WriteStreamByteStream
        AP4_WriteStreamByteStream::AP4_WriteStreamByteStream(
            std::ostream& ios
            ,AP4_Size offset
            ,AP4_Size size)
            :ios_(ios)
            ,ref_(1)
            ,offset_(offset)
            ,size_(size)
            ,position_(0)
        {
            ios_.seekp(offset_,std::ios::beg);
        }

        AP4_Result AP4_WriteStreamByteStream::Create(
            std::ostream& ios
            ,AP4_Size offset
            ,AP4_Size size
            ,AP4_ByteStream* &stream)
        {
            stream = new AP4_WriteStreamByteStream(ios,offset,size);
            return AP4_SUCCESS;
        }

        AP4_WriteStreamByteStream::~AP4_WriteStreamByteStream()
        {
        }


        // methods
        AP4_Result AP4_WriteStreamByteStream::ReadPartial(void*     buffer, 
            AP4_Size  bytes_to_read, 
            AP4_Size& bytes_read)
        {
            return AP4_SUCCESS;
        }

        AP4_Result AP4_WriteStreamByteStream::WritePartial(const void* buffer, 
            AP4_Size    bytes_to_write, 
            AP4_Size&   bytes_written) 
        {
            ios_.write((char*)buffer,bytes_to_write);
            position_ += bytes_to_write;
            bytes_written = bytes_to_write;
            return AP4_SUCCESS;
        }

        AP4_Result AP4_WriteStreamByteStream::Seek(AP4_Position position)
        {
            if(position_ != position)
            {
                ios_.seekp(offset_+position,std::ios::beg);
            }
            position_ = position;

            return AP4_SUCCESS;
        }
        AP4_Result AP4_WriteStreamByteStream::Tell(AP4_Position& position)
        {
            position = position_;
            return AP4_SUCCESS;
        }
        AP4_Result AP4_WriteStreamByteStream::GetSize(AP4_LargeSize& size)
        {
            size = size_;
            return AP4_SUCCESS;
        }

        void AP4_WriteStreamByteStream::AddReference()
        {
            ++ref_;
        }
        void AP4_WriteStreamByteStream::Release()
        {
            if(0 == (--ref_))
            {
                delete this;
            }
        }
        AP4_Result AP4_WriteStreamByteStream::Flush()
        {
            ios_.flush();
            return AP4_SUCCESS;
        }
    }
}

#include "ppbox/peer/Common.h"
#include "ppbox/peer/Ap4ReadStreamByteStream.h"

namespace ppbox
{
    namespace peer
    {
        //AP4_ReadStreamByteStream
        AP4_ReadStreamByteStream::AP4_ReadStreamByteStream(
            std::istream& ios
            ,AP4_Size offset
            ,AP4_Size size)
            :ios_(ios)
            ,ref_(1)
            ,offset_(offset)
            ,size_(size)
            ,position_(0)
        {
            ios_.seekg(offset_,std::ios::beg);
        }

        AP4_Result AP4_ReadStreamByteStream::Create(
            std::istream& ios
            ,AP4_Size offset
            ,AP4_Size size
            ,AP4_ByteStream* &stream)
        {
            stream = new AP4_ReadStreamByteStream(ios,offset,size);
            return AP4_SUCCESS;
        }

        AP4_ReadStreamByteStream::~AP4_ReadStreamByteStream()
        {
        }


        // methods
        AP4_Result AP4_ReadStreamByteStream::ReadPartial(void*     buffer, 
            AP4_Size  bytes_to_read, 
            AP4_Size& bytes_read)
        {
            assert(bytes_to_read+position_ <= size_);
            ios_.read((char*)buffer,bytes_to_read);
            position_ += bytes_to_read;
            bytes_read = bytes_to_read;
            return AP4_SUCCESS;
        }

        AP4_Result AP4_ReadStreamByteStream::WritePartial(const void* buffer, 
            AP4_Size    bytes_to_write, 
            AP4_Size&   bytes_written) 
        {
            return AP4_SUCCESS;
        }

        AP4_Result AP4_ReadStreamByteStream::Seek(AP4_Position position)
        {
            if(position_ != position)
            {
                ios_.seekg(offset_+position,std::ios::beg);
            }
            position_ = position;

            return AP4_SUCCESS;
        }
        AP4_Result AP4_ReadStreamByteStream::Tell(AP4_Position& position)
        {
            position = position_;
            return AP4_SUCCESS;
        }
        AP4_Result AP4_ReadStreamByteStream::GetSize(AP4_LargeSize& size)
        {
            size = size_;
            return AP4_SUCCESS;
        }

        void AP4_ReadStreamByteStream::AddReference()
        {
            ++ref_;
        }
        void AP4_ReadStreamByteStream::Release()
        {
            if(0 == (--ref_))
            {
                delete this;
            }
        }
        AP4_Result AP4_ReadStreamByteStream::Flush()
        {
            return AP4_SUCCESS;
        }
    }
}

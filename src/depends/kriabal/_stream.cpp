// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "_stream.h"

namespace kriabal::stream
{
    FileStream::FileStream(std::wstring path, bool read_only, bool create_if_not_exists)
    {
        stream_ = std::make_unique<CFileStream>(path.c_str(), read_only, create_if_not_exists);
        if (!stream_->IsValid())
            throw RuntimeError();
    }

    int64_t FileStream::GetPosition()
    {
        return stream_->GetPos();
    }

    int64_t FileStream::GetFileSizeInBytes()
    {
        return stream_->GetSize();
    }

    void FileStream::Seek(int64_t position)
    {
        if (!stream_->Seek(position, STREAM_BEGIN))
            throw ReadError();
    }

    void FileStream::Skip(int64_t num_of_bytes)
    {
        if (!stream_->Skip(num_of_bytes))
            throw ReadError();
    }

    void FileStream::ReadBytes(std::string& buffer, size_t num_of_bytes)
    {
        if (buffer.size() < num_of_bytes)
            throw ReadError();

        if (!stream_->ReadBuffer(buffer.data(), num_of_bytes))
            throw ReadError();
    }

    void FileStream::WriteBytes(const std::string& buffer, size_t num_of_bytes)
    {
        if (buffer.size() < num_of_bytes)
            throw WriteError();

        if (!stream_->WriteBuffer(buffer.c_str(), num_of_bytes))
            throw WriteError();
    }

    int32_t FileStream::ReadSignedPositiveInt32FromBytes()
    {
        int32_t result = ReadSignedPositiveOrZeroInt32FromBytes();

        if (result == 0)
            throw NumberReadNotAPositiveNumberError();

        return result;
    }

    int32_t FileStream::ReadSignedPositiveOrZeroInt32FromBytes()
    {
        unsigned char buffer[4];
        if (!stream_->ReadBuffer(&buffer, 4))
            throw ReadError();

        int32_t result = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
        if (result < 0)
            throw NumberReadNotAPositiveOrZeroNumberError();

        return result;
    }

    int64_t FileStream::ReadSignedPositiveInt64FromHexString()
    {
        std::string buffer(sizeof(int64_t) * 2, '\0');
        ReadBytes(buffer, buffer.size());

        char* dummy;
        int64_t result = std::strtoll(buffer.c_str(), &dummy, 16);
        if (errno == ERANGE)
        {
            if (result == LLONG_MIN)
                throw NumberReadOverflowError();
            if (result == LLONG_MAX)
                throw NumberReadUnderflowError();
        }

        if (result == 0)
            throw NumberReadNotANumberError();

        if (result < 0)
            throw NumberReadNotAPositiveNumberError();

        return result;
    }
}


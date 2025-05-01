// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "stdafx.h"

#include "streams.h"

#include <cerrno>
#include <climits>
#include <cstdint>
#include <cstdlib>

#include <memory>
#include <string>

#include "modulecrt/Streams.h"

namespace kriabal::stream
{
    int64_t AbstractStream::GetPosition()
    {
        return stream_->GetPos();
    }

    int64_t AbstractStream::GetSizeInBytes()
    {
        return stream_->GetSize();
    }

    void AbstractStream::Seek(int64_t position)
    {
        if (!stream_->Seek(position, STREAM_BEGIN))
            throw ReadError();
    }

    void AbstractStream::Skip(int64_t num_of_bytes)
    {
        if (!stream_->Skip(num_of_bytes))
            throw ReadError();
    }

    void AbstractStream::ReadBytes(std::string& buffer, size_t num_of_bytes)
    {
        if (buffer.size() < num_of_bytes)
            throw ReadError();

        if (!stream_->ReadBuffer(buffer.data(), num_of_bytes))
            throw ReadError();
    }

    void AbstractStream::WriteBytes(const std::string& buffer, size_t num_of_bytes)
    {
        if (buffer.size() < num_of_bytes)
            throw WriteError();

        if (!stream_->WriteBuffer(buffer.c_str(), num_of_bytes))
            throw WriteError();
    }

    int32_t AdvancedStreamReader::ReadSignedPositiveInt32FromBytes()
    {
        const int32_t result = ReadSignedPositiveOrZeroInt32FromBytes();

        if (result == 0)
            throw NumberReadNotAPositiveNumberError();

        return result;
    }

    int32_t AdvancedStreamReader::ReadSignedPositiveOrZeroInt32FromBytes()
    {
        std::string buffer(4, '\0');
        stream_.ReadBytes(buffer, buffer.size());

        const int32_t result = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
        if (result < 0)
            throw NumberReadNotAPositiveOrZeroNumberError();

        return result;
    }

    int64_t AdvancedStreamReader::ReadSignedPositiveInt64FromHexString()
    {
        std::string buffer(sizeof(int64_t) * 2, '\0');
        stream_.ReadBytes(buffer, buffer.size());

        char* dummy;
        const int64_t result = std::strtoll(buffer.c_str(), &dummy, 16);
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

    MemoryStream::MemoryStream(size_t initial_capacity)
    {
        auto stream = std::make_unique<CMemoryStream>(initial_capacity);
        if (!stream->IsValid())
            throw RuntimeError();
        stream_ = std::move(stream);
    }

    FileStream::FileStream(std::wstring path, bool read_only, bool create_if_not_exists)
    {
        auto stream = std::make_unique<CFileStream>(path.c_str(), read_only, create_if_not_exists);
        if (!stream->IsValid())
            throw RuntimeError();
        stream_ = std::move(stream);
    }
}


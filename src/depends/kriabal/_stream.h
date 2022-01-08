#ifndef _stream_h__
#define _stream_h__

#include <cstdint>
#include <string>

#include "_windef.h"
#include "modulecrt/Streams.h"
#include "_common.h"

namespace kriabal::stream
{
    class RuntimeError : public kriabal::RuntimeError {};

    class FileOpenError : public RuntimeError {};
    class ReadError : public RuntimeError {};
    class WriteError : public RuntimeError {};

    class NumberReadError : public RuntimeError {};
    class NumberReadNotANumberError : public RuntimeError {};
    class NumberReadNotAPositiveNumberError : public RuntimeError {};
    class NumberReadOverflowError : public RuntimeError {};
    class NumberReadUnderflowError : public RuntimeError {};

    class FileStream
    {
    public:
        FileStream(std::wstring path, bool read_only, bool create_if_not_exists);
        int64_t GetFileSizeInBytes();
        void Seek(int64_t position);
        void Skip(int64_t num_of_bytes);
        void ReadBytes(std::string& buffer, size_t num_of_bytes);
        void WriteBytes(const std::string& buffer, size_t num_of_bytes);
        int64_t ReadSignedPositiveInt64FromHexString();
    private:
        std::unique_ptr<CFileStream> stream_;
    };
}

#endif

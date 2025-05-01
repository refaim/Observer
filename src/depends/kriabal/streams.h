#ifndef KRIABAL_STREAM_H_
#define KRIABAL_STREAM_H_

#include <cstdint>

#include <memory>
#include <string>

#include "ModuleWin.h"
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
    class NumberReadNotAPositiveOrZeroNumberError : public RuntimeError {};
    class NumberReadOverflowError : public RuntimeError {};
    class NumberReadUnderflowError : public RuntimeError {};

    class AbstractStream
    {
    public:
        int64_t GetPosition();
        int64_t GetSizeInBytes();
        void Seek(int64_t position);
        void Skip(int64_t num_of_bytes);
        void ReadBytes(std::string& buffer, size_t num_of_bytes);
        void WriteBytes(const std::string& buffer, size_t num_of_bytes);
    protected:
        // TODO check that CHILD destructor is called !!!
        std::unique_ptr<AStream> stream_;
    };

    class MemoryStream : public AbstractStream
    {
    public:
        MemoryStream(size_t initial_capacity);
    };

    class FileStream : public AbstractStream
    {
    public:
        FileStream(std::wstring path, bool read_only, bool create_if_not_exists);
    };

    class AdvancedStreamReader
    {
    public:
        AdvancedStreamReader(AbstractStream& stream) : stream_(stream) {}
        int32_t ReadSignedPositiveInt32FromBytes();
        int32_t ReadSignedPositiveOrZeroInt32FromBytes();
        int64_t ReadSignedPositiveInt64FromHexString();
    private:
        AbstractStream& stream_;
    };
}

#endif // KRIABAL_STREAM_H_

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "kriabal.h"

#include <algorithm>
#include <boost/numeric/conversion/cast.hpp>

#include "ModuleCRT.h"

namespace kriabal
{
    void CopyString(std::wstring source, wchar_t* destination, int32_t max_len)
    {
        if (wcscpy_s(destination, max_len, source.c_str()) != 0)
            throw RuntimeError();
    }

    void Tome::Open(StorageOpenParams params, StorageGeneralInfo* info)
    {
        if (!SignatureMatchOrNull(params.Data, params.DataSize, signature_.data()))
            throw RuntimeError();

        stream_ = std::make_unique<stream::FileStream>(params.FilePath, true, false);

        std::memset(info, 0, sizeof(StorageGeneralInfo));
        CopyString(format_, info->Format, STORAGE_FORMAT_NAME_MAX_LEN);
        CopyString(L"-", info->Compression, STORAGE_PARAM_MAX_LEN);
        CopyString(L"-", info->Comment, STORAGE_PARAM_MAX_LEN);
    }

    const Item& Tome::GetItem(int64_t index) const
    {
        if (index < 0) throw RuntimeError();
        if (boost::numeric_cast<size_t>(index) >= items_.size()) throw ItemIndexTooLargeError();
        return *items_.at(index).get();
    }

    void Tome::FillItemInfo(const Item& item, StorageItemInfo* output)
    {
        std::memset(output, 0, sizeof(StorageItemInfo));
        output->Attributes = FILE_ATTRIBUTE_NORMAL;
        output->Size = item.size_in_bytes;
        output->PackedSize = output->Size;
        Assert(MultiByteToWideChar(CP_UTF8, 0, item.path.c_str(), -1, output->Path, STRBUF_SIZE(output->Path)));
    }

    void Tome::ExtractItem(const Item& item, const ExtractOperationParams& params)
    {
        auto output_stream = std::make_unique<stream::FileStream>(params.DestPath, false, true);

        uint64_t bytes_left = item.size_in_bytes;

        int64_t header_size_in_bytes = ExtractItemHeader(item, *output_stream);
        if (header_size_in_bytes > 0)
        {
            bytes_left -= header_size_in_bytes;
            ReportExtractionProgress(params.Callbacks, header_size_in_bytes);
        }

        auto buffer = std::make_unique<std::string>(32 * 1024, '\0');

        stream_->Seek(item.offset);
        while (bytes_left > 0)
        {
            int64_t chunk_length = min(bytes_left, buffer->size());
            stream_->ReadBytes(*buffer.get(), chunk_length);
            output_stream->WriteBytes(*buffer.get(), chunk_length);

            bytes_left -= chunk_length;
            ReportExtractionProgress(params.Callbacks, chunk_length);
        }
    }

    size_t Tome::ExtractItemHeader(const Item& item, stream::FileStream& output_stream)
    {
        return 0;
    }

    void Tome::ReportExtractionProgress(const ExtractProcessCallbacks& callbacks, int64_t num_of_bytes)
    {
        if (!callbacks.FileProgress(callbacks.signalContext, num_of_bytes))
            throw UserInterrupt();
    }

    void Tome::SkipSignature()
    {
        stream_->Seek(signature_.size());
    }

    void Tome::ReserveItems(int64_t count)
    {
        items_.reserve(count);
    }

    void Tome::PushItem(std::unique_ptr<Item> item)
    {
        std::replace(item->path.begin(), item->path.end(), '/', '\\');
        items_.push_back(std::move(item));
    }

    void Tome::Assert(bool condition)
    {
        if (!condition)
            throw RuntimeError();
    }
}

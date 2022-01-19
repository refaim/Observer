// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "stdafx.h"

#include <cstdint>

#include <memory>
#include <string>
#include <vector>
#include <utility>

#include "archive.h"

#include "ModuleDef.h"

namespace zanzarah
{
    void Archive::Open(StorageOpenParams params)
    {
        Tome::Open(params);

        SkipSignature();

        num_of_files_ = stream_->ReadSignedPositiveInt32FromBytes();
    }

    void Archive::PrepareItems()
    {
        const std::string kRelativePathPrefix = "..\\";
        const int32_t data_block_header = 0x101;
        const int32_t data_block_footer = 0x202;

        auto path_buffer = std::string(1024, '\0');
        auto items = std::make_unique<std::vector<std::unique_ptr<ArchiveItem>>>();
        for (auto i = 0; i < num_of_files_; ++i)
        {
            const auto path_size_in_bytes = stream_->ReadSignedPositiveInt32FromBytes();
            path_buffer.reserve(path_size_in_bytes);
            stream_->ReadBytes(path_buffer, path_size_in_bytes);

            size_t path_bytes_to_skip = 0;
            if (path_buffer.rfind(kRelativePathPrefix, 0) == 0) path_bytes_to_skip = kRelativePathPrefix.length();

            const auto data_block_relative_offset = stream_->ReadSignedPositiveOrZeroInt32FromBytes();
            const auto data_block_size_in_bytes = stream_->ReadSignedPositiveInt32FromBytes();

            auto item = std::make_unique<ArchiveItem>();
            item->path.assign(path_buffer.c_str() + path_bytes_to_skip, path_size_in_bytes - path_bytes_to_skip);
            item->offset = data_block_relative_offset + sizeof(data_block_header);
            // [0x101, data, 0x202], [0x101, data, 0x202], ...
            // No idea why I should subtract next data block header, but apparently that's the right way
            item->size_in_bytes = data_block_size_in_bytes - sizeof(data_block_header) - sizeof(data_block_footer) - sizeof(data_block_header);

            items->push_back(std::move(item));
        }

        const auto index_end_offset = stream_->GetPosition();
        for (auto& item : (*items))
        {
            item->offset += index_end_offset;
            PushItem(std::move(item));
        }
    }
}

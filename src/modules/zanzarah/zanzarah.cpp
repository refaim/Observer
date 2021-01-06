// zanzarah.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ModuleDef.h"
#include "ModuleCRT.h"
#include "modulecrt\Helpers.h"
#include "modulecrt\Streams.h"

#define ENSURE_SUCCESS(SUCCESS_CONDITION) if (!(SUCCESS_CONDITION)) { success = false; goto cleanup; }
#define ENSURE_SUCCESS_EX(SUCCESS_CONDITION, ERROR) if (!(SUCCESS_CONDITION)) { status = ERROR; goto cleanup; }

struct ZPakIndexEntry
{
    std::string path;
    int64_t offset;
    int64_t size;
};

struct ZPakArchive
{
    CFileStream* inputStream;
    int32_t fileCount;
    std::vector<ZPakIndexEntry*> index;
};

int MODULE_EXPORT OpenStorage(StorageOpenParams params, HANDLE *storage, StorageGeneralInfo* info)
{
    char signature[] = { 0, 0, 0, 0 };
    auto signatureSize = sizeof(signature);

    const char* pCharData = (const char*)params.Data;
    if (pCharData != nullptr && (params.DataSize < signatureSize + 1 || pCharData[signatureSize + 1] == '\0' || memcmp(pCharData, signature, signatureSize) != 0))
        return SOR_INVALID_FILE;

    bool success = true;
    ZPakArchive* archive = nullptr;

    auto stream = CFileStream::Open(params.FilePath, true, false);
    if (stream == nullptr) return SOR_INVALID_FILE;

    ENSURE_SUCCESS(stream->Seek(signatureSize, STREAM_BEGIN));
    int32_t fileCount = ReadSignedPositiveInt32FromBytes(stream);
    ENSURE_SUCCESS(fileCount > 0);

    archive = new ZPakArchive();
    archive->inputStream = stream;
    archive->fileCount = fileCount;
    *storage = archive;

    memset(info, 0, sizeof(StorageGeneralInfo));
    ENSURE_SUCCESS(wcscpy_s(info->Format, STORAGE_FORMAT_NAME_MAX_LEN, L"Zanzarah PAK") == 0);
    ENSURE_SUCCESS(wcscpy_s(info->Compression, STORAGE_PARAM_MAX_LEN, L"-") == 0);
    ENSURE_SUCCESS(wcscpy_s(info->Comment, STORAGE_PARAM_MAX_LEN, L"-") == 0);

cleanup:
    if (!success)
    {
        if (archive != nullptr)
        {
            archive->inputStream = nullptr;
            delete archive;
            archive = nullptr;
        }
        *storage = nullptr;

        stream->Close();
        delete stream;

        return SOR_INVALID_FILE;
    }
    return SOR_SUCCESS;
}

void MODULE_EXPORT CloseStorage(HANDLE storage)
{
    // TODO implement !!!
    return;
}

int MODULE_EXPORT PrepareFiles(HANDLE storage)
{
    auto archive = (ZPakArchive*)storage;
    if (archive == nullptr) return FALSE;
    auto stream = archive->inputStream;

    bool success = true;

    char pathBuffer[1024];
    auto pathBufferSize = sizeof(pathBuffer);
    char pathPrefixToRemove[] = "..\\";
    char pathPrefixToRemoveSize = sizeof(pathPrefixToRemove) - 1;
    for (auto i = 0; i < archive->fileCount; ++i)
    {
        auto pathSize = ReadSignedPositiveInt32FromBytes(stream);
        ENSURE_SUCCESS(pathSize > 0 && pathSize < pathBufferSize);
        memset(pathBuffer, 0, pathBufferSize);
        ENSURE_SUCCESS(stream->ReadBuffer(pathBuffer, pathSize));

        char* filePath = pathBuffer;
        if (memcmp(pathBuffer, pathPrefixToRemove, pathPrefixToRemoveSize) == 0)
            filePath = pathBuffer + pathPrefixToRemoveSize;

        auto fileOffset = ReadSignedNonNegativeInt32FromBytes(stream);
        ENSURE_SUCCESS(fileOffset >= 0);

        // TODO handle 4 or 12 bytes at the end of file (file attributes)
        auto fileSize = ReadSignedPositiveInt32FromBytes(stream);
        ENSURE_SUCCESS(fileSize > 0);

        auto entry = new ZPakIndexEntry();
        entry->path.assign(filePath);
        entry->offset = fileOffset;
        entry->size = fileSize;

        archive->index.push_back(entry);
    }

    auto indexSize = stream->GetPos();
    ENSURE_SUCCESS(indexSize > 0);
    for (auto entry = archive->index.begin(); entry != archive->index.end(); ++entry)
        (*entry)->offset += indexSize;

cleanup:

    // TODO cleanup entries? archive? need to test that CloseStorage will be called if returning FALSE (and maybe fix RenPy accordingly)

    return success ? TRUE : FALSE;
}

int MODULE_EXPORT GetStorageItem(HANDLE storage, int item_index, StorageItemInfo* item_info)
{
    auto archive = (ZPakArchive*)storage;
    if (archive == nullptr || item_index < 0)
        return GET_ITEM_ERROR;
    if (item_index >= (int)archive->index.size())
        return GET_ITEM_NOMOREITEMS;

    auto indexEntry = archive->index.at((size_t)item_index);

    memset(item_info, 0, sizeof(StorageItemInfo));
    item_info->Attributes = FILE_ATTRIBUTE_NORMAL;
    item_info->Size = indexEntry->size;
    item_info->PackedSize = indexEntry->size;
    if (MultiByteToWideChar(CP_UTF8, 0, indexEntry->path.c_str(), -1, item_info->Path, STRBUF_SIZE(item_info->Path)) == 0)
        return GET_ITEM_ERROR;

    return GET_ITEM_OK;
}

int MODULE_EXPORT ExtractItem(HANDLE storage, ExtractOperationParams params)
{
    int status = SER_SUCCESS;
    CFileStream* outputStream = nullptr;

    auto archive = (ZPakArchive*)storage;
    if (archive == nullptr) return SER_ERROR_SYSTEM;

    if (params.ItemIndex < 0 || params.ItemIndex >= (int)archive->index.size())
        return SER_ERROR_SYSTEM;

    auto indexEntry = archive->index.at((size_t)params.ItemIndex);
    if (!archive->inputStream->Seek(indexEntry->offset, STREAM_BEGIN))
        return SER_ERROR_READ;

    outputStream = CFileStream::Open(params.DestPath, false, true);
    if (outputStream == nullptr) return SER_ERROR_WRITE;

    uint64_t bytesLeft = indexEntry->size;
    char buffer[32 * 1024];
    while (bytesLeft > 0)
    {
        uint64_t chunkSize = min(bytesLeft, sizeof(buffer));
        bytesLeft -= chunkSize;

        ENSURE_SUCCESS_EX(archive->inputStream->ReadBuffer(buffer, chunkSize), SER_ERROR_READ);
        ENSURE_SUCCESS_EX(outputStream->WriteBuffer(buffer, chunkSize), SER_ERROR_WRITE);
        ENSURE_SUCCESS_EX(params.Callbacks.FileProgress(params.Callbacks.signalContext, chunkSize), SER_USERABORT);
    }

cleanup:
    outputStream->Close();
    delete outputStream;

    return status;
}

//////////////////////////////////////////////////////////////////////////
// Exported Functions
//////////////////////////////////////////////////////////////////////////

// {86E7E4C3-BC44-4E8E-90AF-BDBD1CB61A83}
static const GUID MODULE_GUID = { 0x86e7e4c3, 0xbc44, 0x4e8e, { 0x90, 0xaf, 0xbd, 0xbd, 0x1c, 0xb6, 0x1a, 0x83 } };

int MODULE_EXPORT LoadSubModule(ModuleLoadParameters* LoadParams)
{
    LoadParams->ModuleId = MODULE_GUID;
    LoadParams->ModuleVersion = MAKEMODULEVERSION(1, 0);
    LoadParams->ApiVersion = ACTUAL_API_VERSION;
    LoadParams->ApiFuncs.OpenStorage = OpenStorage;
    LoadParams->ApiFuncs.CloseStorage = CloseStorage;
    LoadParams->ApiFuncs.GetItem = GetStorageItem;
    LoadParams->ApiFuncs.ExtractItem = ExtractItem;
    LoadParams->ApiFuncs.PrepareFiles = PrepareFiles;

    return TRUE;
}

void MODULE_EXPORT UnloadSubModule()
{
}

// rpa.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ModuleDef.h"
#include "modulecrt\Streams.h"

#include <zlib.h>
//#include <python3.7\Python.h>

static const char* SIGNATURE = "RPA-3.0";
static const char* SPACE = " ";

struct RenPyArchive
{
    CFileStream* inputStream;
    char* unpackedData;
};

int MODULE_EXPORT OpenStorage(StorageOpenParams params, HANDLE* storage, StorageGeneralInfo* info)
{
    if (params.Data == nullptr) return SOR_INVALID_FILE;

    auto stream = CFileStream::Open(params.FilePath, true, false);
    if (stream == nullptr) return SOR_INVALID_FILE;

    char signatureCandidate[sizeof(SIGNATURE) - 1];
    stream->ReadBuffer(signatureCandidate, sizeof(signatureCandidate)); // TODO handle false
    if (strncmp(signatureCandidate, SIGNATURE, strlen(SIGNATURE) != 0)) return SOR_INVALID_FILE;

    auto archive = new RenPyArchive();
    archive->inputStream = stream;
    *storage = archive;

    memset(info, 0, sizeof(StorageGeneralInfo));
    wcscpy_s(info->Format, STORAGE_FORMAT_NAME_MAX_LEN, L"RenPy Archive");
    wcscpy_s(info->Compression, STORAGE_PARAM_MAX_LEN, L"zlib");
    wcscpy_s(info->Comment, STORAGE_PARAM_MAX_LEN, L"-");

    return SOR_SUCCESS;
}

void MODULE_EXPORT CloseStorage(HANDLE storage)
{
    auto archive = (RenPyArchive*)storage;
    if (archive == nullptr) return;

    if (archive->inputStream != nullptr)
        delete archive->inputStream;

    delete archive;
}

int MODULE_EXPORT PrepareFiles(HANDLE storage)
{
    auto archive = (RenPyArchive*)storage;
    if (archive == nullptr) return FALSE;

    char buffer[16];
    char* dummy;
    int64_t filesOffset;
    int32_t encryptionKey;

    archive->inputStream->Seek(strlen(SIGNATURE) + strlen(SPACE), STREAM_BEGIN); // TODO handle false
    archive->inputStream->ReadBuffer(&buffer, sizeof(filesOffset)); // TODO handle false
    filesOffset = std::strtoll(buffer, &dummy, 16);
    archive->inputStream->Seek(strlen(SPACE), STREAM_CURRENT); // TODO handle false
    memset(buffer, 0, sizeof(buffer));
    archive->inputStream->ReadBuffer(&buffer, sizeof(encryptionKey)); // TODO handle false
    encryptionKey = std::strtol(buffer, &dummy, 16);

    return TRUE;
}

int MODULE_EXPORT GetStorageItem(HANDLE storage, int item_index, StorageItemInfo* item_info)
{
    return GET_ITEM_NOMOREITEMS;
}

int MODULE_EXPORT ExtractItem(HANDLE storage, ExtractOperationParams params)
{
    return SER_ERROR_SYSTEM;
}

//////////////////////////////////////////////////////////////////////////
// Exported Functions
//////////////////////////////////////////////////////////////////////////

// {9486718F-8F0A-4DE7-9880-01146B336D6B}
static const GUID MODULE_GUID = { 0x9486718f, 0x8f0a, 0x4de7, { 0x98, 0x80, 0x01, 0x14, 0x6b, 0x33, 0x6d, 0x6b } };

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

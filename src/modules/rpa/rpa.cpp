// rpa.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ModuleDef.h"
#include "ModuleCRT.h"
#include "modulecrt\Streams.h"

#include <zlib.h>
//#include <python3.7\Python.h>

#define FILE_SIGNATURE_RPA30 "RPA-3.0"
#define WHITESPACE " "

// TODO rename
#define HANDLE_FILE_OPERATION(X) if (!X) return FALSE

struct RenPyArchive
{
    CFileStream* inputStream;
    char* unpackedData;
};

int MODULE_EXPORT OpenStorage(StorageOpenParams params, HANDLE* storage, StorageGeneralInfo* info)
{
    if (!SignatureMatchOrNull(params.Data, params.DataSize, FILE_SIGNATURE_RPA30))
        return SOR_INVALID_FILE;

    auto stream = CFileStream::Open(params.FilePath, true, false);
    if (stream == nullptr) return SOR_INVALID_FILE;

    char signatureCandidate[sizeof(FILE_SIGNATURE_RPA30) - 1];
    if (!stream->ReadBuffer(signatureCandidate, sizeof(signatureCandidate)))
        return SOR_INVALID_FILE;
    if (!SignatureMatchOrNull(signatureCandidate, strlen(FILE_SIGNATURE_RPA30), FILE_SIGNATURE_RPA30))
        return SOR_INVALID_FILE;

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

    char buffer[sizeof(int64_t)];
    char* dummy;
    HANDLE_FILE_OPERATION(archive->inputStream->Seek(strlen(FILE_SIGNATURE_RPA30) + strlen(WHITESPACE) + sizeof(int64_t), STREAM_BEGIN));
    HANDLE_FILE_OPERATION(archive->inputStream->ReadBuffer(&buffer, sizeof(int64_t)));
    int64_t filesOffset = std::strtoll(buffer, &dummy, 16);
    HANDLE_FILE_OPERATION(archive->inputStream->Seek(strlen(WHITESPACE), STREAM_CURRENT));
    memset(buffer, 0, sizeof(buffer));
    HANDLE_FILE_OPERATION(archive->inputStream->ReadBuffer(&buffer, sizeof(int64_t)));
    int64_t encryptionKey = std::strtoll(buffer, &dummy, 16);

    HANDLE_FILE_OPERATION(archive->inputStream->Seek(filesOffset, STREAM_BEGIN));

    //z_stream zStream;
    //zStream.next_in = (Bytef *)pvInBuffer;
    //zStream.avail_in = (uInt)cbInBuffer;
    //zStream.total_in = cbInBuffer;
    //zStream.next_out = (Bytef *)pvOutBuffer;
    //zStream.avail_out = *pcbOutBuffer;
    //zStream.total_out = 0;
    //zStream.zalloc = NULL;
    //zStream.zfree = NULL;

    //if (inflateInit(&compressedStream) != Z_OK)
    //    return FALSE;

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

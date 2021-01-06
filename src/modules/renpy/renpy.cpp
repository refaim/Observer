// renpy.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ModuleDef.h"
#include "ModuleCRT.h"
#include "modulecrt\Streams.h"

#include <Python.h>
#include <zlib.h>

#define FILE_SIGNATURE_RPA30 "RPA-3.0"
#define WHITESPACE " "

#define ENSURE_SUCCESS(SUCCESS_CONDITION) if (!(SUCCESS_CONDITION)) { success = false; goto cleanup; }
#define ENSURE_SUCCESS_EX(SUCCESS_CONDITION, ERROR) if (!(SUCCESS_CONDITION)) { status = ERROR; goto cleanup; }

#define DECLARE_PYOBJECT(VAR, EXPR) PyObject* VAR = EXPR; ENSURE_SUCCESS(VAR != nullptr); pyObjectsToRecycle.push_back(VAR);

enum PyIndexEntryValueTypeIndex
{
    OFFSET = 0,
    LENGTH = 1,
    PREFIX = 2,
};

struct RenPyIndexEntry
{
    std::string path;
    int64_t offset;
    int64_t size;
    char* prefixBytes;
    size_t prefixLength;
};

struct RenPyArchive
{
    CFileStream* inputStream;
    std::vector<RenPyIndexEntry*> index;
};

int MODULE_EXPORT OpenStorage(StorageOpenParams params, HANDLE* storage, StorageGeneralInfo* info)
{
    bool success = true;

    if (!SignatureMatchOrNull(params.Data, params.DataSize, FILE_SIGNATURE_RPA30))
        return SOR_INVALID_FILE;

    auto stream = CFileStream::Open(params.FilePath, true, false);
    if (stream == nullptr) return SOR_INVALID_FILE;

    auto archive = new RenPyArchive();
    archive->inputStream = stream;
    *storage = archive;

    memset(info, 0, sizeof(StorageGeneralInfo));
    ENSURE_SUCCESS(wcscpy_s(info->Format, STORAGE_FORMAT_NAME_MAX_LEN, L"RenPy Archive") == 0);
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
    auto archive = (RenPyArchive*)storage;
    if (archive == nullptr) return;

    if (archive->inputStream != nullptr) archive->inputStream->Close();
    delete archive->inputStream;

    for (auto i = archive->index.begin(); i != archive->index.end(); ++i)
    {
        auto entry = *i;
        delete entry->prefixBytes;
        delete entry;
    }
    archive->index.clear();

    delete archive;
}

int MODULE_EXPORT PrepareFiles(HANDLE storage)
{
    auto archive = (RenPyArchive*)storage;
    if (archive == nullptr) return FALSE;
    auto stream = archive->inputStream;

    bool success = true;
    Bytef* compressedData = nullptr;
    Bytef* uncompressedData = nullptr;
    RenPyIndexEntry* indexEntry = nullptr;
    std::vector<PyObject*> pyObjectsToRecycle;

    char buffer[sizeof(int64_t)];
    char* dummy;
    ENSURE_SUCCESS(stream->Seek(strlen(FILE_SIGNATURE_RPA30) + strlen(WHITESPACE) + sizeof(int64_t), STREAM_BEGIN));
    ENSURE_SUCCESS(stream->ReadBuffer(&buffer, sizeof(int64_t)));
    int64_t indexOffset = std::strtoll(buffer, &dummy, 16);
    ENSURE_SUCCESS(stream->Seek(strlen(WHITESPACE), STREAM_CURRENT));
    ENSURE_SUCCESS(stream->ReadBuffer(&buffer, sizeof(int64_t)));
    int64_t encryptionKey = std::strtoll(buffer, &dummy, 16);

    ENSURE_SUCCESS(stream->Seek(indexOffset, STREAM_BEGIN));
    int64_t archiveSize = stream->GetSize();
    ENSURE_SUCCESS(archiveSize > 0);
    int64_t archivePosition = stream->GetPos();
    ENSURE_SUCCESS(archivePosition > 0);
    uint32_t compressedSize = (uint32_t)(archiveSize - archivePosition);
    compressedData = new Bytef[(size_t)compressedSize]();
    ENSURE_SUCCESS(stream->ReadBuffer(compressedData, (size_t)compressedSize));

    uLongf uncompressedSize = 0;
    uLongf compressionMultiplier = 4;
    int zStatus = Z_BUF_ERROR;
    do
    {
        delete uncompressedData;
        uncompressedSize = compressedSize * compressionMultiplier;
        uncompressedData = new Bytef[(size_t)uncompressedSize]();
        zStatus = uncompress(uncompressedData, &uncompressedSize, compressedData, compressedSize);
        ++compressionMultiplier;
    }
    while (zStatus == Z_BUF_ERROR);
    ENSURE_SUCCESS(zStatus == Z_OK);
    delete[] compressedData;
    compressedData = nullptr;

    Py_Initialize();

    DECLARE_PYOBJECT(pyUncompressedData, PyByteArray_FromStringAndSize((const char*)uncompressedData, (Py_ssize_t)uncompressedSize));
    delete[] uncompressedData;
    uncompressedData = nullptr;

    DECLARE_PYOBJECT(pyPickleModuleName, PyUnicode_FromString("pickle"));
    DECLARE_PYOBJECT(pyPickleModule, PyImport_Import(pyPickleModuleName));
    DECLARE_PYOBJECT(pyPickleLoader, PyObject_GetAttrString(pyPickleModule, "loads"));

    PyObject* pyPickleLoaderArgs = PyTuple_New(1);
    ENSURE_SUCCESS(pyPickleLoaderArgs != nullptr);
    ENSURE_SUCCESS(PyTuple_SetItem(pyPickleLoaderArgs, 0, pyUncompressedData) == 0);

    DECLARE_PYOBJECT(pyPickleLoaderKwargs, PyDict_New());
    DECLARE_PYOBJECT(pyPickleEncoding, PyUnicode_FromString("latin1"));
    ENSURE_SUCCESS(PyDict_SetItemString(pyPickleLoaderKwargs, "encoding", pyPickleEncoding) == 0);

    DECLARE_PYOBJECT(pyIndexDictionary, PyObject_Call(pyPickleLoader, pyPickleLoaderArgs, pyPickleLoaderKwargs));
    archive->index.reserve(PyDict_Size(pyIndexDictionary));

    PyObject* pyPathInArchive;
    PyObject* pyIndexEntries;
    Py_ssize_t i = 0;
    while (PyDict_Next(pyIndexDictionary, &i, &pyPathInArchive, &pyIndexEntries))
    {
        ENSURE_SUCCESS(PyList_Size(pyIndexEntries) == 1); // TODO implement
        PyObject* pyIndexEntry = PyList_GetItem(pyIndexEntries, 0);
        ENSURE_SUCCESS(pyIndexEntry != nullptr);

        PyObject* pyFileOffset = PyTuple_GetItem(pyIndexEntry, PyIndexEntryValueTypeIndex::OFFSET);
        ENSURE_SUCCESS(pyFileOffset);
        int64_t fileOffset = PyLong_AsLongLong(pyFileOffset);
        ENSURE_SUCCESS(fileOffset != -1);

        PyObject* pyFileSize = PyTuple_GetItem(pyIndexEntry, PyIndexEntryValueTypeIndex::LENGTH);
        ENSURE_SUCCESS(pyFileSize);
        int64_t fileSize = PyLong_AsLongLong(pyFileSize);
        ENSURE_SUCCESS(fileSize != -1);

        Py_ssize_t prefixLength = 0;
        char* prefixBytes = nullptr;
        if (PyTuple_Size(pyIndexEntry) == 3)
        {
            PyObject* pyPrefixStringU = PyTuple_GetItem(pyIndexEntry, PyIndexEntryValueTypeIndex::PREFIX);
            ENSURE_SUCCESS(pyPrefixStringU != nullptr);

            DECLARE_PYOBJECT(pyPrefixStringL, PyUnicode_AsLatin1String(pyPrefixStringU));
            prefixBytes = PyBytes_AsString(pyPrefixStringL);
            ENSURE_SUCCESS(prefixBytes != nullptr);

            prefixLength = PyBytes_Size(pyPrefixStringL);
            if (prefixLength == 0) prefixBytes = nullptr;
        }

        indexEntry = new RenPyIndexEntry();
        indexEntry->offset = fileOffset ^ encryptionKey;
        indexEntry->size = fileSize ^ encryptionKey;

        indexEntry->prefixBytes = nullptr;
        if (prefixLength > 0 && prefixBytes != nullptr)
        {
            indexEntry->prefixBytes = new char[prefixLength];
            memcpy(indexEntry->prefixBytes, prefixBytes, prefixLength);
        }
        indexEntry->prefixLength = prefixLength;

        const char* referenceToPath = PyUnicode_AsUTF8(pyPathInArchive);
        ENSURE_SUCCESS(referenceToPath != nullptr);
        indexEntry->path.assign(referenceToPath);
        std::replace(indexEntry->path.begin(), indexEntry->path.end(), '/', '\\');

        archive->index.push_back(indexEntry);
        indexEntry = nullptr;
    }

cleanup:
    delete[] compressedData;
    delete[] uncompressedData;
    delete indexEntry;

    for (auto i = pyObjectsToRecycle.begin(); i != pyObjectsToRecycle.end(); ++i) Py_DECREF(*i);
    pyObjectsToRecycle.clear();
    Py_Finalize();

    return success ? TRUE : FALSE;
}

int MODULE_EXPORT GetStorageItem(HANDLE storage, int item_index, StorageItemInfo* item_info)
{
    auto archive = (RenPyArchive*)storage;
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

    auto archive = (RenPyArchive*)storage;
    if (archive == nullptr) return SER_ERROR_SYSTEM;

    if (params.ItemIndex < 0 || params.ItemIndex >= (int)archive->index.size())
        return SER_ERROR_SYSTEM;

    auto indexEntry = archive->index.at((size_t)params.ItemIndex);
    if (!archive->inputStream->Seek(indexEntry->offset, STREAM_BEGIN))
        return SER_ERROR_READ;

    outputStream = CFileStream::Open(params.DestPath, false, true);
    if (outputStream == nullptr) return SER_ERROR_WRITE;

    uint64_t bytesLeft = indexEntry->size;
    if (indexEntry->prefixLength > 0)
    {
        bytesLeft -= indexEntry->prefixLength;
        ENSURE_SUCCESS_EX(outputStream->WriteBuffer(indexEntry->prefixBytes, indexEntry->prefixLength), SER_ERROR_WRITE);
        ENSURE_SUCCESS_EX(params.Callbacks.FileProgress(params.Callbacks.signalContext, indexEntry->prefixLength), SER_USERABORT);
    }

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

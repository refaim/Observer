// rpa.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ModuleDef.h"
#include "ModuleCRT.h"
#include "modulecrt\Streams.h"

#include <python3.7\Python.h>
#include <zlib.h>

#define FILE_SIGNATURE_RPA30 "RPA-3.0"
#define WHITESPACE " "

// TODO rename module to renpy

#define ENSURE_SUCCESS(SUCCESS_CONDITION) if (!(SUCCESS_CONDITION)) { success = false; goto cleanup; }

enum PyIndexEntryValueTypeIndex
{
    OFFSET = 0,
    LENGTH = 1,
    PREFIX = 2,
};

struct RenPyArchive
{
    CFileStream* inputStream;
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
    auto stream = archive->inputStream;

    bool success = true;
    Bytef* compressedData = nullptr;
    Bytef* uncompressedData = nullptr;
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
    if (archiveSize == 0) return FALSE;
    int64_t archivePosition = stream->GetPos();
    if (archivePosition == 0) return FALSE;
    // TODO handle conversion of int64_t to uLongf
    uLongf compressedSize = archiveSize - archivePosition;
    compressedData = new Bytef[compressedSize]();
    ENSURE_SUCCESS(stream->ReadBuffer(compressedData, compressedSize));

    uLongf uncompressedSize = 0;
    uLongf compressionMultiplier = 2;
    int zStatus = Z_BUF_ERROR;
    do
    {
        delete uncompressedData;
        uncompressedSize = compressedSize * compressionMultiplier;
        uncompressedData = new Bytef[uncompressedSize]();
        zStatus = uncompress(uncompressedData, &uncompressedSize, compressedData, compressedSize);
        ++compressionMultiplier;
    }
    while (zStatus == Z_BUF_ERROR);
    ENSURE_SUCCESS(zStatus == Z_OK);
    delete compressedData;

    Py_Initialize();
    PyObject* pyPickleArgs = PyTuple_New(1);
    ENSURE_SUCCESS(pyPickleArgs != nullptr);
    pyObjectsToRecycle.push_back(pyPickleArgs);
    PyObject* pyUncompressedData = PyByteArray_FromStringAndSize((const char*)uncompressedData, uncompressedSize);
    ENSURE_SUCCESS(pyUncompressedData != nullptr);
    delete uncompressedData;
    ENSURE_SUCCESS(PyTuple_SetItem(pyPickleArgs, 0, pyUncompressedData) == 0);
    PyObject* pyPickleModuleName = PyUnicode_FromString("pickle");
    pyObjectsToRecycle.push_back(pyPickleModuleName);
    PyObject* pyPickleModule = PyImport_Import(pyPickleModuleName);
    pyObjectsToRecycle.push_back(pyPickleModule);
    PyObject* pyPickleLoader = PyObject_GetAttrString(pyPickleModule, "loads");
    ENSURE_SUCCESS(pyPickleLoader != nullptr);
    pyObjectsToRecycle.push_back(pyPickleLoader);
    PyObject* pyIndexDictionary = PyObject_CallObject(pyPickleLoader, pyPickleArgs);
    ENSURE_SUCCESS(pyIndexDictionary != nullptr);
    pyObjectsToRecycle.push_back(pyIndexDictionary);

    PyObject* pyPathInArchive;
    PyObject* pyIndexEntries;
    Py_ssize_t i = 0;
    while (PyDict_Next(pyIndexDictionary, &i, &pyPathInArchive, &pyIndexEntries))
    {
        std::string pathInArchive = PyUnicode_AsUTF8(pyPathInArchive);
        for (Py_ssize_t j = 0; j < PyList_Size(pyIndexEntries); ++j) // TODO handle multiple segments
        {
            PyObject* pyIndexEntry = PyList_GetItem(pyIndexEntries, j);
            ENSURE_SUCCESS(pyIndexEntry != nullptr);

            PyObject* pyFileOffset = PyTuple_GetItem(pyIndexEntry, PyIndexEntryValueTypeIndex::OFFSET);
            ENSURE_SUCCESS(pyFileOffset);
            PyObject* pyFileLength = PyTuple_GetItem(pyIndexEntry, PyIndexEntryValueTypeIndex::LENGTH);
            ENSURE_SUCCESS(pyFileLength);

            const char* prefixBytes = nullptr;
            if (PyTuple_Size(pyIndexEntry) == 3)
            {
                PyObject* pyPrefixString = PyTuple_GetItem(pyIndexEntry, PyIndexEntryValueTypeIndex::PREFIX);
                ENSURE_SUCCESS(pyPrefixString != nullptr);

                // TODO
            }
        }
    //    Py_ssize_t prefixSize = 0;
    //    char* prefixBytes = nullptr;
    //    if (PyTuple_Size(pyIndexEntry) == 3)
    //    {
    //        PyObject* pyPrefixString = PyTuple_GetItem(pyIndexEntry, 2);
    //    }
    //    //def deobfuscate_entry(key: int, entry : IndexEntry)->ComplexIndexEntry:
    //    //return[
    //    //    (offset ^ key, length ^ key, start)
    //    //        for offset, length, start in UnRPA.normalise_entry(entry)
    //    //]

    //        //return[
    //        //    (*cast(SimpleIndexPart, part), b"")
    //        //        if len(part) == 2
    //        //        else cast(ComplexIndexPart, part)
    //        //            for part in entry
    //        //]

    //    // path: UnRPA.deobfuscate_entry(key, entry) for path, entry in index.items()
    }

    Py_Finalize();

cleanup:
    delete compressedData;
    delete uncompressedData;

    for (auto const& pyObject : pyObjectsToRecycle) Py_DECREF(pyObject);
    pyObjectsToRecycle.clear();

    return success ? TRUE : FALSE;
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

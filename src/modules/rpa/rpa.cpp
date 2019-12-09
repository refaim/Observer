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

struct RenPyIndexEntry
{
    std::string *path;
    int64_t offset;
    int64_t length;
};

struct RenPyArchive
{
    CFileStream* inputStream;
    std::vector<RenPyIndexEntry*>* index;
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
    archive->index = new std::vector<RenPyIndexEntry*>();
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

    if (archive->inputStream != nullptr) archive->inputStream->Close();
    delete archive->inputStream;

    if (archive->index != nullptr)
    {
        for (auto i = archive->index->begin(); i != archive->index->end(); ++i)
        {
            RenPyIndexEntry* entry = *i;
            delete entry->path;
            delete entry;
        }
        archive->index->clear();
    }
    delete archive->index;

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
    uLongf compressedSize = archiveSize - archivePosition; // TODO handle conversion of int64_t to uLongf
    compressedData = new Bytef[compressedSize]();
    ENSURE_SUCCESS(stream->ReadBuffer(compressedData, compressedSize));

    uLongf uncompressedSize = 0;
    uLongf compressionMultiplier = 4;
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
    compressedData = nullptr;

    Py_Initialize();
    PyObject* pyPickleArgs = PyTuple_New(1);
    ENSURE_SUCCESS(pyPickleArgs != nullptr);
    pyObjectsToRecycle.push_back(pyPickleArgs);
    PyObject* pyUncompressedData = PyByteArray_FromStringAndSize((const char*)uncompressedData, uncompressedSize);
    ENSURE_SUCCESS(pyUncompressedData != nullptr);
    delete uncompressedData;
    uncompressedData = nullptr;
    ENSURE_SUCCESS(PyTuple_SetItem(pyPickleArgs, 0, pyUncompressedData) == 0);
    PyObject* pyPickleModuleName = PyUnicode_FromString("pickle");
    pyObjectsToRecycle.push_back(pyPickleModuleName);
    PyObject* pyPickleModule = PyImport_Import(pyPickleModuleName);
    pyObjectsToRecycle.push_back(pyPickleModule);
    PyObject* pyPickleLoader = PyObject_GetAttrString(pyPickleModule, "loads");
    ENSURE_SUCCESS(pyPickleLoader != nullptr);
    pyObjectsToRecycle.push_back(pyPickleLoader);
    // TODO this fails on "The Flower Shop.rpa"
    PyObject* pyIndexDictionary = PyObject_CallObject(pyPickleLoader, pyPickleArgs);
    ENSURE_SUCCESS(pyIndexDictionary != nullptr);
    pyObjectsToRecycle.push_back(pyIndexDictionary);

    archive->index->reserve(PyDict_Size(pyIndexDictionary));

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
        PyObject* pyFileLength = PyTuple_GetItem(pyIndexEntry, PyIndexEntryValueTypeIndex::LENGTH);
        ENSURE_SUCCESS(pyFileLength);

        Py_ssize_t prefixLength = 0;
        const char* prefixBytes = nullptr;
        if (PyTuple_Size(pyIndexEntry) == 3)
        {
            PyObject* pyPrefixString = PyTuple_GetItem(pyIndexEntry, PyIndexEntryValueTypeIndex::PREFIX);
            ENSURE_SUCCESS(pyPrefixString != nullptr);
            ENSURE_SUCCESS(PyUnicode_GetLength(pyPrefixString) == 0); // TODO implement

            //// TODO The returned buffer always has an extra null byte appended (not included in size), regardless of whether there are any other null code points.
            //prefixBytes = PyUnicode_AsUTF8AndSize(pyPrefixString, &prefixLength);
            //ENSURE_SUCCESS(prefixBytes != nullptr);
            //if (prefixLength == 0) prefixBytes = nullptr;
        }

        const char* referenceToPath = PyUnicode_AsUTF8(pyPathInArchive);
        ENSURE_SUCCESS(referenceToPath != nullptr);
        std::string* ownedPath = new std::string(referenceToPath);
        std::replace(ownedPath->begin(), ownedPath->end(), '/', '\\');

        indexEntry = new RenPyIndexEntry();
        indexEntry->path = ownedPath;
        indexEntry->offset = PyLong_AsLongLong(pyFileOffset) ^ encryptionKey;
        indexEntry->length = PyLong_AsLongLong(pyFileLength) ^ encryptionKey;
        ENSURE_SUCCESS(indexEntry->offset != -1 && indexEntry->length != -1);
        archive->index->push_back(indexEntry);
        indexEntry = nullptr;
    }

cleanup:
    delete compressedData;
    delete uncompressedData;
    delete indexEntry;

    for (auto i = pyObjectsToRecycle.begin(); i != pyObjectsToRecycle.end(); ++i) Py_XDECREF(*i);
    pyObjectsToRecycle.clear();
    Py_Finalize();

    return success ? TRUE : FALSE;
}

int MODULE_EXPORT GetStorageItem(HANDLE storage, int item_index, StorageItemInfo* item_info)
{
    auto archive = (RenPyArchive*)storage;
    if (archive == nullptr || item_index < 0)
        return GET_ITEM_ERROR;
    if (item_index >= archive->index->size())
        return GET_ITEM_NOMOREITEMS;

    RenPyIndexEntry *indexEntry = archive->index->at(item_index);

    memset(item_info, 0, sizeof(StorageItemInfo));
    item_info->Attributes = FILE_ATTRIBUTE_NORMAL;
    item_info->Size = indexEntry->length;
    item_info->PackedSize = indexEntry->length;
    if (MultiByteToWideChar(CP_ACP, 0, indexEntry->path->c_str(), -1, item_info->Path, STRBUF_SIZE(item_info->Path)) == 0)
        return GET_ITEM_ERROR;

    return GET_ITEM_OK;
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

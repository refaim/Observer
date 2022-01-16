// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "stdafx.h"

#include "ModuleWin.h"
#include "ModuleDef.h"

#include "kriabal\kriabal.h"
#include "archive.h"

int MODULE_EXPORT OpenStorage(StorageOpenParams params, HANDLE* storage, StorageGeneralInfo* info) //-V813
{
    auto archive = new zanzarah::Archive();
    try
    {
        archive->Open(params);
        archive->FillGeneralInfo(info);
        *storage = archive;
        return SOR_SUCCESS;
    }
    catch (kriabal::RuntimeError&)
    {
        delete archive;
        return SOR_INVALID_FILE;
    }
}

void MODULE_EXPORT CloseStorage(HANDLE storage)
{
    if (storage == nullptr) return;

    auto archive = static_cast<zanzarah::Archive*>(storage);
    delete archive;
}

int MODULE_EXPORT PrepareFiles(HANDLE storage)
{
    if (storage == nullptr) return FALSE;

    auto archive = static_cast<zanzarah::Archive*>(storage);
    try
    {
        archive->PrepareItems();
        return TRUE;
    }
    catch (kriabal::RuntimeError&)
    {
        return FALSE;
    }
}

int MODULE_EXPORT GetStorageItem(HANDLE storage, int item_index, StorageItemInfo* item_info)
{
    if (storage == nullptr) return GET_ITEM_ERROR;

    auto archive = static_cast<zanzarah::Archive*>(storage);
    try
    {
        archive->FillItemInfo(archive->GetItem(item_index), item_info);
        return GET_ITEM_OK;
    }
    catch (kriabal::ItemIndexTooLargeError&)
    {
        return GET_ITEM_NOMOREITEMS;
    }
    catch (kriabal::RuntimeError&)
    {
        return GET_ITEM_ERROR;
    }
}

int MODULE_EXPORT ExtractItem(HANDLE storage, ExtractOperationParams params) //-V813
{
    if (storage == nullptr) return SER_ERROR_SYSTEM;

    auto archive = static_cast<zanzarah::Archive*>(storage);
    try
    {
        archive->ExtractItem(archive->GetItem(params.ItemIndex), params);
        return SER_SUCCESS;
    }
    catch (kriabal::stream::ReadError&)
    {
        return SER_ERROR_READ;
    }
    catch (kriabal::stream::WriteError&)
    {
        return SER_ERROR_WRITE;
    }
    catch (kriabal::stream::FileOpenError&)
    {
        return SER_ERROR_WRITE;
    }
    catch (kriabal::UserInterrupt&)
    {
        return SER_USERABORT;
    }
    catch (kriabal::RuntimeError&)
    {
        return SER_ERROR_SYSTEM;
    }
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

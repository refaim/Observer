// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "stdafx.h"

#include "ModuleDef.h"

int MODULE_EXPORT OpenStorage(StorageOpenParams params, HANDLE *storage, StorageGeneralInfo* info)
{
    return SOR_INVALID_FILE;
}

void MODULE_EXPORT CloseStorage(HANDLE storage)
{
}

int MODULE_EXPORT PrepareFiles(HANDLE storage)
{
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

// %GUID_STRING%
static const GUID MODULE_GUID = %GUID_STRUCT%;

int MODULE_EXPORT LoadSubModule(ModuleLoadParameters* LoadParams) noexcept
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

void MODULE_EXPORT UnloadSubModule() noexcept
{
}

#ifndef KRIABAL_SUBMODULE_H_
#define KRIABAL_SUBMODULE_H_

#include "ModuleWin.h"
#include "ModuleDef.h"

#include "kriabal.h"

namespace kriabal::submodule
{
    template<class StorageClass>
    int MODULE_EXPORT load(int major_version, int minor_version, const GUID& guid, ModuleLoadParameters* load_params)
    {
        load_params->ModuleId = guid;
        load_params->ModuleVersion = MAKEMODULEVERSION(major_version, minor_version);
        load_params->ApiVersion = ACTUAL_API_VERSION;
        load_params->ApiFuncs.OpenStorage = __internal::open_storage<StorageClass>;
        load_params->ApiFuncs.CloseStorage = __internal::close_storage<StorageClass>;
        load_params->ApiFuncs.GetItem = __internal::get_storage_item<StorageClass>;
        load_params->ApiFuncs.ExtractItem = __internal::extract_item<StorageClass>;
        load_params->ApiFuncs.PrepareFiles = __internal::prepare_files<StorageClass>;
        return TRUE;
    }

    namespace __internal
    {
        template<class StorageClass>
        int MODULE_EXPORT open_storage(StorageOpenParams params, HANDLE* storage, StorageGeneralInfo* info)
        {
            auto archive = new StorageClass();
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

        template<class StorageClass>
        void MODULE_EXPORT close_storage(HANDLE storage)
        {
            if (storage == nullptr) return;

            auto archive = static_cast<StorageClass*>(storage);
            delete archive;
        }

        template<class StorageClass>
        int MODULE_EXPORT prepare_files(HANDLE storage)
        {
            if (storage == nullptr) return FALSE;

            auto archive = static_cast<StorageClass*>(storage);
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

        template<class StorageClass>
        int MODULE_EXPORT get_storage_item(HANDLE storage, int item_index, StorageItemInfo* item_info)
        {
            if (storage == nullptr) return GET_ITEM_ERROR;

            auto archive = static_cast<StorageClass*>(storage);
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

        template<class StorageClass>
        int MODULE_EXPORT extract_item(HANDLE storage, ExtractOperationParams params) //-V813
        {
            if (storage == nullptr) return SER_ERROR_SYSTEM;

            auto archive = static_cast<StorageClass*>(storage);
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
    }
}

#endif // KRIABAL_SUBMODULE_H_

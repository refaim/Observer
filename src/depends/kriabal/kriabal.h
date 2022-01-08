#ifndef kriabal_h__
#define kriabal_h__

#include <string>
#include <vector>

#include "_windef.h"
#include "ModuleDef.h"

#include "_common.h"
#include "_stream.h"

namespace kriabal
{
    class ItemIndexTooLargeError : public RuntimeError {};
    class UserInterrupt : public RuntimeError {};

    struct Item
    {
        int64_t offset;
        int64_t size_in_bytes;
        std::string path;
    };

    class Tome
    {
    public:
        Tome(std::wstring format, std::string signature) : format_(format), signature_(signature) {};
        void Open(StorageOpenParams params, StorageGeneralInfo* info);
        virtual void PrepareItems() = 0;
        const Item& GetItem(int64_t index) const;
        void FillItemInfo(const Item& item, StorageItemInfo* output);
        void ExtractItem(const Item& item, const ExtractOperationParams& params);
    protected:
        std::unique_ptr<stream::FileStream> stream_;

        void SkipSignature();
        void ReserveItems(int64_t count);
        void PushItem(std::unique_ptr<Item> item);
        virtual size_t ExtractItemHeader(const Item& item, stream::FileStream& output_stream);

        static void Assert(bool condition);
    private:
        std::wstring format_;
        std::string signature_;
        std::vector<std::unique_ptr<Item>> items_;

        void ReportExtractionProgress(const ExtractProcessCallbacks& callbacks, int64_t num_of_bytes);
    };
}

#endif

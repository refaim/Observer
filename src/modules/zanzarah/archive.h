#ifndef archive_h__
#define archive_h__

#include "kriabal\kriabal.h"

namespace zanzarah
{
    struct ArchiveItem : kriabal::Item {};

    class Archive : public kriabal::Tome
    {
    public:
        Archive() : kriabal::Tome(L"Zanzarah", std::vector<unsigned char>({0, 0, 0, 0})), num_of_files_(0) {}
        void Open(StorageOpenParams params);
        void PrepareItems();
    private:
        int32_t num_of_files_;
    };
}

#endif

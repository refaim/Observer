#ifndef ZANZARAH_ARCHIVE_H_
#define ZANZARAH_ARCHIVE_H_

#include <cstdint>

#include <vector>

#include "ModuleDef.h"

#include "kriabal/kriabal.h"

namespace zanzarah
{
    struct ArchiveItem : kriabal::Item {};

    class Archive : public kriabal::Tome
    {
    public:
        Archive() : kriabal::Tome(L"Zanzarah", std::vector<unsigned char>({0, 0, 0, 0})), num_of_files_(0) {}
        void Open(StorageOpenParams params) override;
        void PrepareItems() override;
    private:
        int32_t num_of_files_;
    };
}

#endif // ZANZARAH_ARCHIVE_H_

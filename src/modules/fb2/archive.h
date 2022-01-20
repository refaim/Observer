#ifndef FB2_ARCHIVE_H_
#define FB2_ARCHIVE_H_

#include "kriabal/kriabal.h"

#include "ModuleDef.h"

namespace fb2
{
    struct ArchiveItem : kriabal::Item {};

    class Archive : public kriabal::Tome
    {
    public:
        Archive() : kriabal::Tome(L"FictionBook2") {}
        void PrepareItems() override;
    };
}

#endif // FB2_ARCHIVE_H_

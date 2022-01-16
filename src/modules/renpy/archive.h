#ifndef RENPY_ARCHIVE_H_
#define RENPY_ARCHIVE_H_

#include "kriabal\kriabal.h"

#include "python.h"

namespace renpy
{
    struct ArchiveItem : kriabal::Item {};

    class Archive : public kriabal::Tome
    {
    public:
        Archive() : kriabal::Tome(L"RenPy", "RPA-3.0") {}
        void PrepareItems();
    private:
        static void UncompressZlibStream(std::string& input_buffer, std::string& output_buffer);
        void ParsePythonIndex(std::string& input_buffer, int64_t encryption_key);
    };
}

#endif // RENPY_ARCHIVE_H_

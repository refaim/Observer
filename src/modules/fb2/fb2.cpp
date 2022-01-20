// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "stdafx.h"

#include "ModuleWin.h"
#include "ModuleDef.h"

#include "kriabal/submodule.h"
#include "archive.h"

//////////////////////////////////////////////////////////////////////////
// Exported Functions
//////////////////////////////////////////////////////////////////////////

// {9A9EBE20-05A5-4049-A1C9-082C4E06979E}
static const GUID MODULE_GUID = { 0x9a9ebe20, 0x05a5, 0x4049, { 0xa1, 0xc9, 0x08, 0x2c, 0x4e, 0x06, 0x97, 0x9e } };

int MODULE_EXPORT LoadSubModule(ModuleLoadParameters* LoadParams) noexcept
{
    return kriabal::submodule::load<fb2::Archive>(1, 0, MODULE_GUID, LoadParams);
}

void MODULE_EXPORT UnloadSubModule() noexcept
{
}

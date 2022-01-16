// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "stdafx.h"

#include "ModuleWin.h"
#include "ModuleDef.h"

#include "kriabal\submodule.h"
#include "archive.h"

// {86E7E4C3-BC44-4E8E-90AF-BDBD1CB61A83}
static const GUID MODULE_GUID = { 0x86e7e4c3, 0xbc44, 0x4e8e, { 0x90, 0xaf, 0xbd, 0xbd, 0x1c, 0xb6, 0x1a, 0x83 } };

int MODULE_EXPORT LoadSubModule(ModuleLoadParameters* LoadParams)
{
    return kriabal::submodule::load<zanzarah::Archive>(1, 0, MODULE_GUID, LoadParams);
}

void MODULE_EXPORT UnloadSubModule()
{
}

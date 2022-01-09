**This is a fork of https://github.com/lazyhamster/Observer. This fork was created to develop additional modules (RenPy, Zanzarah).**

# Observer
Compound plugin for [Far Manager](https://www.farmanager.com) 2.0/3.0 that allows users to browse various file containers and extract content from them.
All formats support files listing and extraction.

## How to install
Download prebuilt binaries and place them in Plugins directory of your FAR Manager installation.

### System requirements
* MS Visual C++ 2017 Redistributable Package
* MS Windows Installer 4 or higher (for .msi files support)
* MS .NET Framework 4 (for virtual disks support)

## Supported file formats
#### Installation packages
  - CreateInstall
  - Install Shield 3+
  - MSI
  - NSIS
  - Setup Factory 5-9
  - WISE Installer
#### Game containers
  - Containers from games made by Relic (Homeworld 1/2, CoH, WH40k DOW 1/2)
  - Egosoft packages (from X-series games)
  - MoPaQ containers (from Blizzard Entertainment games)
  - Steam containers (GCF, WAD, XZP, PAK, BSP, VBSP)
  - Volition Pack V2 files (from FreeSpace 1/2/Open)
  - Ren'Py Visual Novel Engine archives (RPA)
  - Zanzarah: The Hidden Portal game data (PAK)
#### Mail archives
  - MIME containers
  - MS Outlook databases
  - Mail containers in MBox format
#### Misc:
  - CD disk images: ISO-9660 (incl. Joliet), CUE/BIN, MDF/MDS
  - UDF (ISO 13346) images
  - Virtual disk images (VMDK, VDI, VHD, XVA)
  - PDF files (embedded files)

## License
Observer is [free](http://www.gnu.org/philosophy/free-sw.html) software: you can use it, redistribute it and/or modify it under the terms of the [GNU Lesser General Public License](http://www.gnu.org/licenses/lgpl.html) as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

## Links
[Support Forum (in Russian)](https://forum.farmanager.com/viewtopic.php?f=5&t=4644)

## Credits
The modules are inspired by the following sources:
- RenPy: [rpatool](https://github.com/Shizmob/rpatool) by [Shizmob](https://github.com/Shizmob)
- Zanzarah: [zanzapak](http://aluigi.altervista.org/papers.htm#others-file) by [Luigi Auriemma](http://aluigi.altervista.org) and [IKS Tools](https://github.com/bartlomiejduda/Tools/tree/master/OLD%20Python%20Tools/IKS%20ZANZARAH%20TOOLS) by [bartlomiejduda](https://github.com/bartlomiejduda)

## How to build from source
Project is developed under MS Visual Studio 2017.

Observer modules depend on several libraries that are not supplied with sources.
The easiest way to use these libraries is with [vcpkg](https://github.com/Microsoft/vcpkg) tool.

For x86 version run:
* vcpkg install zlib bzip2 glib gmime libmspack --triplet x86-windows

For x64 version run:
* vcpkg install zlib bzip2 glib gmime libmspack --triplet x64-windows

### Additional requirements
* Boost C++ Libraries.
* M4 Macro Processor (must be in %PATH%).

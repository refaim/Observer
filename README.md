[![License: LGPL v3](https://img.shields.io/badge/License-LGPL_v3-blue.svg)](https://www.gnu.org/licenses/lgpl-3.0)
[![Continuous Integration](https://github.com/refaim/Observer/actions/workflows/main.yaml/badge.svg?branch=master)](https://github.com/refaim/Observer/actions/workflows/main.yaml)

**This repository is a fork of https://github.com/lazyhamster/Observer**

It adds support of the following formats:
  - [Ren'Py Visual Novel Engine](https://www.renpy.org/) archives (RPA)
  - [Zanzarah: The Hidden Portal](https://en.wikipedia.org/wiki/ZanZarah:_The_Hidden_Portal) game data (PAK)

## How to install

Download [nigthly build](https://nightly.link/refaim/Observer/workflows/main.yaml/master), follow the instructions inside the archive.

## How to build from source
Project is developed under MS Visual Studio 2017.

Observer modules depend on several libraries that are not supplied with sources.
Observer uses [vcpkg Manifest Mode MSBuild Integration](https://vcpkg.readthedocs.io/en/latest/users/manifests/#msbuild-integration).
All additional libraries will be installed according to `vcpkg.json` manifest file the first time you build the project.

### Additional requirements
* M4 Macro Processor (must be in %PATH%)

## Links

[Support Forum (in Russian)](https://forum.farmanager.com/viewtopic.php?t=12729)
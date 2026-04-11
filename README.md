------------------------------------------------
HALF-LIFE: INSECURE
Version 1.5
Source Code
------------------------------------------------

This is the code used in HL: Insecure version 1.5.
Forked from TWHL-Community Half-Life Updated SDK, 
as well as others. Ready to compile for Windows
and Linux.

------------------------------------------------
HOW TO USE:
------------------------------------------------

Windows:

- Download Visual Studio Community 2026 through the
Windows store or Microsoft website.
(https://visualstudio.microsoft.com/downloads/)

- Then at the installer, select the following components:
  MSVC Build Tools for x64/x86 
  Windows 11 SDK.
  
- After finishing the installation, it should be
ready to use.

- The compiled binaries are located in the default
Half-Life folder through Steam.
(C:\Program Files (x86)\Steam\steamapps\common\Half-Life\insecure)

Linux:

This is only applicable for Ubuntu/Debian 22.04, you
may have to look for similar commands based in your 
distro.

- Open your terminal, and use the commands listed below:
sudo apt-get install git
sudo apt-get install build-essential gcc-multilib g++-multilib libgl1-mesa-dev

- Once finished, select the "gendbg.sh" script and 
give it permission to run, otherwise it will fail
to compile.

- Then, open your terminal at the "linux" folder 
and type "make".

WARNING: You might encounter an "could not load library"
error after compiling, this usually occurs when compiling
with Ubuntu/Debian distros after 22.04. In which case,
look at the following link:
(https://twhl.info/wiki/page/Half-Life_Programming_-_Debugging#Debugging__could_not_load_library_client__errors)

------------------------------------------------
USED CODE FROM:
------------------------------------------------

- TWHL-Community Half-Life Updated SDK:
(https://github.com/twhl-community/halflife-updated)

- TWHL-Community Opposing Force Updated SDK:
(https://github.com/twhl-community/halflife-op4-updated)

- TWHL-Community Blue Shift Updated SDK:
(https://github.com/twhl-community/halflife-bs-updated)

- Spirits of Half-Life v1.8 SDK
(https://github.com/sohl-modders/Spirit-of-Half-Life-1.8)

------------------------------------------------
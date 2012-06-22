# Lorris - updater
This is Lorris' updater app for win32. It will check for Lorris' version,
download ZIP file from web if new version is available and extracts it.
It is really simple, written in C++ using only native Win32 functions
to make it as small as possible.

### Usage:
    updater.exe
If Lorris.exe is in same folder as updater, it will check for its version
and proceeds with update if new version is available.
If it is not, updater will just download lastest stable version

    updater.exe *version* *revision*
    updater.exe 0.5.0-dev 421
This is used by Lorris' built-in update check, so that updater does not
have to check for version again.

### License
My own files (main.cpp, ui.cpp and work.cpp) are GNU GPLv3.

Unzip files are from here:
http://www.codeproject.com/Articles/7530/Zip-Utils-clean-elegant-simple-C-Win32

File downloading is example modified to suit my needs:
http://www.dreamincode.net/forums/topic/101532-download-file-from-url/
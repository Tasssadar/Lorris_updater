# cmake -DCMAKE_TOOLCHAIN_FILE=../mingw32-toolchain.cmake -DCMAKE_BUILD_TYPE=Release

cmake_minimum_required (VERSION 2.6)

project (LorrisUpdater)

set(CMAKE_EXE_LINKER_FLAGS "-static")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Os -s")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os -s")

add_definitions(-D_UNICODE)
add_definitions(-DUNICODE)

add_executable(LorrisUpdater
    download.cpp
    changelog.cpp
    main.cpp
    ui.cpp
    unzip.cpp
    work.cpp
    resource.rc

    easy-ecc/ecc.c
    signtool/sha256.c
    signtool/util.c
    signtool/verify.c)

set_target_properties(LorrisUpdater PROPERTIES COMPILE_FLAGS "-mwindows" )
set_target_properties(LorrisUpdater PROPERTIES LINK_FLAGS "-mwindows" )
set_target_properties(LorrisUpdater PROPERTIES OUTPUT_NAME updater)
target_link_libraries(LorrisUpdater comctl32 wsock32 ws2_32 wininet)

add_executable(signtool
    easy-ecc/ecc.c
    signtool/sha256.c
    signtool/signtool.c
    signtool/util.c
    signtool/verify.c)

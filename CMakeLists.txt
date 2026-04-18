cmake_minimum_required(VERSION 3.12)
project(Horion)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wno-dynamic-class-memaccess -Wno-microsoft-goto -Wno-delete-incomplete /EHsc /Ob1)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/MP /Ob1)
endif()

add_compile_definitions(WIN32_LEAN_AND_MEAN)
add_compile_definitions(NOMINMAX)
if(CMAKE_BUILD_TYPE MATCHES Release)

elseif(CMAKE_BUILD_TYPE MATCHES BetaConf)
    add_compile_definitions(_BETA)
endif()

include_directories(.)

link_directories(minhook)

# NOTE: This is a STRIPPED-DOWN ".xp command only" build of Horion. All
# modules, GUIs, scripting, friend-list, pathfinding, etc. were removed.
# See the README for details on bringing the remaining byte-signatures up
# to date for Minecraft Bedrock Edition v1.26.13.
add_library(Horion SHARED
        Horion/Command/Commands/ICommand.cpp
        Horion/Command/Commands/ICommand.h
        Horion/Command/Commands/XpCommand.cpp
        Horion/Command/Commands/XpCommand.h
        Horion/Command/CommandMgr.cpp
        Horion/Command/CommandMgr.h
        Horion/Loader.cpp
        Memory/GameData.cpp
        Memory/GameData.h
        Memory/Hooks.cpp
        Memory/Hooks.h
        Memory/MinHook.h
        Memory/SlimMem.cpp
        Memory/SlimMem.h
        SDK/CClientInstance.h
        SDK/CEntity.h
        SDK/TextHolder.cpp
        SDK/TextHolder.h
        Utils/Logger.cpp
        Utils/Logger.h
        Utils/TextFormat.cpp
        Utils/TextFormat.h
        Utils/Utils.cpp
        Utils/Utils.h
        Utils/xorstr.h)

target_link_libraries(Horion PRIVATE libMinHook.x64.lib)

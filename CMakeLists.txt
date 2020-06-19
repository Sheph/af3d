cmake_minimum_required(VERSION 2.6)
cmake_policy(SET CMP0014 NEW)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(CROSSCOMPILING "Enable cross-compiling")

if (CROSSCOMPILING)
    if (NOT CMAKE_FIND_ROOT_PATH_MODE_PROGRAM)
        set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    endif ()
    if (NOT CMAKE_FIND_ROOT_PATH_MODE_LIBRARY)
        set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    endif ()
    if (NOT CMAKE_FIND_ROOT_PATH_MODE_INCLUDE)
        set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    endif ()
endif ()

project(AF3D)
enable_language(ASM)
include(CheckTypeSize)

#platform setup

if (WIN32)
    set(PLATFORM_NAME "win32")
else ()
    CHECK_TYPE_SIZE("void*" PTR_SIZE BUILTIN_TYPES_ONLY)
    if (PTR_SIZE EQUAL 8)
        set(PLATFORM_NAME "linux64")
    else ()
        set(PLATFORM_NAME "linux32")
    endif ()
endif ()

# USER SETTINGS

if (NOT WIN32)
    if (CMAKE_BUILD_TYPE STREQUAL "")
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: None (CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE)
    endif ()
endif ()

option(USE_LUAJIT "Use LuaJIT" FALSE)
option(USE_LUAJIT_VALGRIND "Enable LuaJIT+valgrind" FALSE)
option(USE_BT_SSE "Use Bullet SSE" TRUE)
option(FORCE_OPTIMIZE "Force compiler optimizations" FALSE)
option(LINK_GL "Link libGL in binary" FALSE)

# END USER SETTINGS

set(INSTALL_OUT_DIR "out" CACHE STRING "Where to install output files")

string(TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_LOWER)

#out dir

set(AF3D_OUT_DIR ${AF3D_BINARY_DIR}/out)

#set includes/libs

set(AF3D_INCLUDE_DIR "${AF3D_SOURCE_DIR}/include")
include_directories(${AF3D_INCLUDE_DIR})

#find packages

find_package(Threads)
if (NOT WIN32)
    find_package(X11 REQUIRED)
endif ()
set(ZLIB_INCLUDE_DIRS "${AF3D_SOURCE_DIR}/zlib-1.2.8")
set(ZLIB_LIBRARIES "z")

#find programs

add_definitions(-DBOOST_ALL_NO_LIB -DBOOST_CHRONO_HEADER_ONLY)

if (NOT USE_LUAJIT)
    add_definitions(-DLUABIND_CPLUSPLUS_LUA)
endif ()

if (USE_LUAJIT_VALGRIND)
    add_definitions(-DLUAJIT_USE_VALGRIND -DLUAJIT_USE_SYSMALLOC)
    if (NOT BUILD_TYPE_LOWER STREQUAL "debug")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
    endif ()
endif ()

if (NOT WIN32)
    if (BUILD_TYPE_LOWER STREQUAL "debug")
        if (USE_LUAJIT)
            add_definitions(-DLUA_USE_APICHECK -DLUA_USE_ASSERT)
        else ()
            add_definitions(-DLUA_USE_APICHECK -Dlua_assert=assert)
        endif ()
        add_definitions(-D_DEBUG)
    endif ()
else ()
    if (USE_LUAJIT)
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /DLUA_USE_APICHECK /DLUA_USE_ASSERT /MTd /bigobj")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DLUA_USE_APICHECK /DLUA_USE_ASSERT /MTd /bigobj")
    else ()
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /DLUA_USE_APICHECK /Dlua_assert=assert /MTd /bigobj")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DLUA_USE_APICHECK /Dlua_assert=assert /MTd /bigobj")
    endif ()
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /D_DEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D_DEBUG")
    foreach (CONFIGURATION RELEASE MINSIZEREL RELWITHDEBINFO)
        set(CMAKE_C_FLAGS_${CONFIGURATION} "${CMAKE_C_FLAGS_${CONFIGURATION}} /MT /bigobj")
        set(CMAKE_CXX_FLAGS_${CONFIGURATION} "${CMAKE_CXX_FLAGS_${CONFIGURATION}} /MT /bigobj")
    endforeach ()
endif ()

if (NOT WIN32)
    include_directories(${X11_INCLUDE_DIR})
endif ()
include_directories(${ZLIB_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIRS})
include_directories(${AF3D_SOURCE_DIR}/libRocket/Include)
include_directories(${AF3D_INCLUDE_DIR}/bullet)

if (USE_LUAJIT)
    include_directories(${AF3D_INCLUDE_DIR}/luajit)
else ()
    include_directories(${AF3D_INCLUDE_DIR}/lua)
endif ()

#fix output directories

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${AF3D_OUT_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${AF3D_OUT_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${AF3D_OUT_DIR}/lib)

if (WIN32)
    add_definitions(-DBYPASS_ECLIPSE_INDEXER -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS -D_WINSOCK_DEPRECATED_NO_WARNINGS -D_ENABLE_EXTENDED_ALIGNED_STORAGE)
else ()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -D_REENTRANT -fPIC -DPIC -DBYPASS_ECLIPSE_INDEXER")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -D_REENTRANT -fPIC -DPIC -DBYPASS_ECLIPSE_INDEXER")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedefs")
endif ()
add_definitions(-DROCKET_STATIC_LIB)
add_definitions(-DASSIMP_BUILD_NO_OWN_ZLIB)
add_definitions("-DIMGUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT=struct ImDrawVert{ImVec2 pos\;float posZ\;ImVec2 uv\;ImU32 col\;}")

if (USE_BT_SSE)
    add_definitions(-DUSE_SIMD)
    add_definitions(-DBT_USE_SSE)
    add_definitions(-DBT_USE_SSE_IN_API)
    add_definitions(-DBT_USE_SIMD_VECTOR3)
endif ()

add_definitions(-DSTBI_NO_STDIO)
add_definitions(-DSTBI_WRITE_NO_STDIO)

if (LINK_GL)
    add_definitions(-DLINK_GL)
endif ()

message(STATUS "Platform - " ${CMAKE_SYSTEM_NAME})
message(STATUS "Install prefix - " ${CMAKE_INSTALL_PREFIX})

if (NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=gold -Wl,--no-undefined -Wl,--exclude-libs,ALL")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold -Wl,--no-undefined -Wl,--exclude-libs,ALL")
    if (FORCE_OPTIMIZE AND (BUILD_TYPE_LOWER STREQUAL "debug"))
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
    endif ()
endif ()

add_subdirectory(zlib-1.2.8)
if (USE_LUAJIT)
    add_subdirectory(LuaJIT-2.0.4)
else ()
    add_subdirectory(lua-5.1.5)
endif ()
add_subdirectory(luabind-0.9.1)
add_subdirectory(stb)
add_subdirectory(dds-ktx)
add_subdirectory(freetype-2.6.2)
add_subdirectory(libRocket)
add_subdirectory(boost_1_72_0)
add_subdirectory(bullet-2.89)
add_subdirectory(log4cplus-2.0.2)
add_subdirectory(jsoncpp)
add_subdirectory(assimp-5.0.1)
add_subdirectory(imgui-1.75)
add_subdirectory(af3dutil)
add_subdirectory(game)

set(CMAKE_EXTRA_GENERATOR_CXX_SYSTEM_DEFINED_MACROS "${CMAKE_EXTRA_GENERATOR_CXX_SYSTEM_DEFINED_MACROS}__cplusplus;201103L")

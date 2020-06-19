set(SOURCES
    system/error_code.cpp
    thread/future.cpp
    filesystem/codecvt_error_category.cpp
    filesystem/utf8_codecvt_facet.cpp
    filesystem/portability.cpp
    filesystem/unique_path.cpp
    filesystem/path_traits.cpp
    filesystem/exception.cpp
    filesystem/operations.cpp
    filesystem/windows_file_codecvt.cpp
    filesystem/path.cpp
    filesystem/directory.cpp
)

if (NOT WIN32)
    set(SOURCES ${SOURCES}
        thread/pthread/thread.cpp
        thread/pthread/once_atomic.cpp
    )
else ()
    set(SOURCES ${SOURCES}
        thread/win32/thread.cpp
        thread/win32/thread_primitives.cpp
        thread/win32/tss_dll.cpp
        thread/win32/tss_pe.cpp
    )
endif ()

add_definitions(-DBOOST_SYSTEM_STATIC_LINK=1 -DBOOST_THREAD_BUILD_LIB=1)
if (NOT WIN32)
    add_definitions(-DBOOST_THREAD_POSIX)
endif ()

add_library(boost STATIC ${SOURCES})

target_link_libraries(boost ${CMAKE_THREAD_LIBS_INIT})

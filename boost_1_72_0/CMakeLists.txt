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

set(SOURCES ${SOURCES}
    thread/pthread/thread.cpp
    thread/pthread/once_atomic.cpp
)

add_definitions(-DBOOST_SYSTEM_STATIC_LINK=1 -DBOOST_THREAD_BUILD_LIB=1)
add_definitions(-DBOOST_THREAD_POSIX)

add_library(boost STATIC ${SOURCES})

target_link_libraries(boost ${CMAKE_THREAD_LIBS_INIT})

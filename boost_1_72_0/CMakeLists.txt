set(SOURCES
    system/error_code.cpp
    thread/future.cpp
)

set(SOURCES ${SOURCES}
    thread/pthread/thread.cpp
    thread/pthread/once_atomic.cpp
)

add_definitions(-DBOOST_SYSTEM_STATIC_LINK=1 -DBOOST_THREAD_BUILD_LIB=1)
add_definitions(-DBOOST_THREAD_POSIX)

add_library(boost STATIC ${SOURCES})

target_link_libraries(boost ${CMAKE_THREAD_LIBS_INIT})

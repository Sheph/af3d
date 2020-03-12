set(SOURCES
    Assert.cpp
    Logger.cpp
    Utils.cpp
    StreamAppConfig.cpp
    SequentialAppConfig.cpp
    OAL.cpp
    Frustum.cpp
    Plane.cpp
)

add_library(af3dutil STATIC ${SOURCES})

target_link_libraries(af3dutil log4cplus boost)

set(SOURCES
    Assert.cpp
    Logger.cpp
    Utils.cpp
    StreamAppConfig.cpp
    SequentialAppConfig.cpp
    OAL.cpp
    Frustum.cpp
    Plane.cpp
    PNGDecoder.cpp
    PNGUtils.cpp
    ImageReader.cpp
    BinaryReader.cpp
    FBXParser.cpp
    FBXSceneBuilder.cpp
    FBXMeshBuilder.cpp
    FBXProperties70Builder.cpp
    FBXMaterialTemplateBuilder.cpp
    FBXTextureTemplateBuilder.cpp
    TPS.cpp
    Ray.cpp
)

add_library(af3dutil STATIC ${SOURCES})

target_link_libraries(af3dutil log4cplus boost png stb jsoncpp)

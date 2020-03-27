set(SOURCES
    ${CMAKE_CURRENT_BINARY_DIR}/config.ini.cpp
    Component.cpp
    RenderMeshComponent.cpp
    RenderComponentManager.cpp
    PhasedComponentManager.cpp
    CameraComponent.cpp
    DummyShell.cpp
    Game.cpp
    GameLogAppender.cpp
    GameShell.cpp
    InputGamepad.cpp
    InputKeyboard.cpp
    InputManager.cpp
    InputMouse.cpp
    Level.cpp
    Logger.cpp
    main_x11.cpp
    OGL.cpp
    Scene.cpp
    SceneObject.cpp
    SceneObjectFactory.cpp
    SceneObjectManager.cpp
    Settings.cpp
    Platform.cpp
    PlatformLinux.cpp
    Utils.cpp
    HardwareContext.cpp
    HardwareResource.cpp
    HardwareBuffer.cpp
    HardwareVertexBuffer.cpp
    HardwareIndexBuffer.cpp
    HardwareProgram.cpp
    HardwareResourceManager.cpp
    HardwareShader.cpp
    HardwareTexture.cpp
    HardwareVertexArray.cpp
    Material.cpp
    MaterialManager.cpp
    MaterialType.cpp
    Mesh.cpp
    MeshManager.cpp
    Renderer.cpp
    SubMesh.cpp
    Resource.cpp
    Texture.cpp
    TextureManager.cpp
    VertexArray.cpp
    VertexArrayLayout.cpp
    VertexArraySlice.cpp
    RenderList.cpp
    RenderNode.cpp
    Light.cpp
    DirectionalLight.cpp
    PointLight.cpp
    LightComponent.cpp
    PhasedComponent.cpp
    UIComponent.cpp
    RenderComponent.cpp
    AssimpIOSystem.cpp
    AssimpIOStream.cpp
    AssimpLogStream.cpp
    BoxMeshGenerator.cpp
    AssimpMeshLoader.cpp
    UIComponentManager.cpp
    ImGuiManager.cpp
    ImGuiComponent.cpp
    VertexArrayWriter.cpp
    APropertyType.cpp
    APropertyTypes.cpp
    APropertyValue.cpp
    AClass.cpp
    AObject.cpp
    AClassRegistry.cpp
    Image.cpp
    ImageManager.cpp
    RenderQuadComponent.cpp
    editor/Workspace.cpp
    editor/ActionMainPopup.cpp
    editor/ActionAddObject.cpp
    editor/MainPopup.cpp
    editor/CommandAddObject.cpp
    editor/CommandHistory.cpp
    editor/CommandHistoryWindow.cpp
)

include_directories(.)

file(READ "config.ini" _INI)

string(REGEX REPLACE ";" "\\\\;" _INI "${_INI}")
string(REGEX REPLACE "\n" ";" _INI "${_INI}")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/config.ini.cpp" "extern const char configIniStr[];\n")
file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/config.ini.cpp" "const char configIniStr[] =")

if (WIN32)
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/config.ini.cpp" "\n\"assets=./assets\\n\"")

    foreach (_CONFIGURATION ${CMAKE_CONFIGURATION_TYPES})
        add_custom_command(
            OUTPUT ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_CONFIGURATION}/config.ini
            COMMAND ${CMAKE_COMMAND} -E echo assets=../../../../game/assets > ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_CONFIGURATION}/config.ini
            COMMAND type config.ini >> ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_CONFIGURATION}/config.ini
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS config.ini
            VERBATIM
        )
        set(SOURCES ${SOURCES}
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_CONFIGURATION}/config.ini
        )
    endforeach ()
    set(SOURCES ${SOURCES}
        main_win32.cpp
    )
else ()
    set(SOURCES ${SOURCES}
        main_x11.cpp
    )
endif ()

foreach (_ENTRY ${_INI})
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/config.ini.cpp" "\n\"${_ENTRY}\\n\"")
endforeach()

file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/config.ini.cpp" ";\n")

add_executable(af3d WIN32 ${SOURCES})

target_link_libraries(af3d af3dutil log4cplus bullet assimp imgui)

if (NOT WIN32)
    target_link_libraries(af3d ${X11_LIBRARIES} ${X11_Xxf86vm_LIB} rt dl)

    file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets)
    configure_file(config.ini ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config.ini COPYONLY)
endif ()

install(
    TARGETS af3d
    RUNTIME DESTINATION "${INSTALL_OUT_DIR}"
)

set(BETA_LIST
    test.txt
)

string(REGEX REPLACE ";" "\\\\;" BETA_LIST "${BETA_LIST}")
string(REGEX REPLACE "\n" ";" BETA_LIST "${BETA_LIST}")

file(GLOB_RECURSE ALL_LIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/assets FALSE assets/*)

list(REMOVE_ITEM ALL_LIST ${BETA_LIST})

foreach (_ENTRY ${ALL_LIST})
    get_filename_component(_NAME ${_ENTRY} NAME)
    get_filename_component(_DIR ${_ENTRY} DIRECTORY)
    get_filename_component(_EXT ${_ENTRY} EXT)

    install(
        FILES
        "assets/${_ENTRY}"
        DESTINATION "${INSTALL_OUT_DIR}/assets/${_DIR}")
    if("${_EXT}" STREQUAL ".lua")
        if (WIN32)
            foreach (_CONFIGURATION ${CMAKE_CONFIGURATION_TYPES})
                if (USE_LUAJIT)
                    install(CODE "set(ENV{LUA_PATH} ${AF3D_SOURCE_DIR}/LuaJIT-2.0.4/src/?.lua)\nexecute_process(COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_CONFIGURATION}/luac -bg ${_ENTRY} ${CMAKE_INSTALL_PREFIX}/${INSTALL_OUT_DIR}/assets/${_ENTRY} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets)" CONFIGURATIONS ${_CONFIGURATION})
                else ()
                    install(CODE "execute_process(COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_CONFIGURATION}/luac -o ${CMAKE_INSTALL_PREFIX}/${INSTALL_OUT_DIR}/assets/${_ENTRY} ${_ENTRY} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets)" CONFIGURATIONS ${_CONFIGURATION})
                endif ()
            endforeach()
        else()
            if (USE_LUAJIT)
                install(CODE "set(ENV{LUA_PATH} ${AF3D_SOURCE_DIR}/LuaJIT-2.0.4/src/?.lua)\nexecute_process(COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/luac -bg ${_ENTRY} ${CMAKE_INSTALL_PREFIX}/${INSTALL_OUT_DIR}/assets/${_ENTRY} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets)")
            else ()
                install(CODE "execute_process(COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/luac -o ${CMAKE_INSTALL_PREFIX}/${INSTALL_OUT_DIR}/assets/${_ENTRY} ${_ENTRY} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets)")
            endif ()
        endif()
    endif()
endforeach ()

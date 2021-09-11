include (CreateSourceGroups)

set(SOURCES
    ${CMAKE_CURRENT_BINARY_DIR}/config.ini.cpp
    AClass.h
    AClassRegistry.h
    ACommand.h
    AJsonReader.h
    AJsonSerializer.h
    AJsonWriter.h
    AObject.h
    AParameterized.h
    AProperty.h
    APropertyType.h
    APropertyTypes.h
    APropertyTypeVisitor.h
    APropertyValue.h
    APropertyValueMap.h
    Asset.h
    AssetManager.h
    AssetModel.h
    AssetTexture.h
    AssimpIOStream.h
    AssimpIOSystem.h
    AssimpLogStream.h
    AssimpMeshLoader.h
    AWeakObject.h
    BoxMeshGenerator.h
    CameraComponent.h
    CameraFilter.h
    Camera.h
    CameraLayer.h
    CameraRenderer.h
    CameraUsageComponent.h
    CollisionComponent.h
    CollisionComponentManager.h
    CollisionFilter.h
    CollisionMatrix.h
    CollisionSensorComponent.h
    CollisionShapeBox.h
    CollisionShapeCapsule.h
    CollisionShapeCompound.h
    CollisionShapeCone.h
    CollisionShapeConvexMesh.h
    CollisionShapeCylinder.h
    CollisionShape.h
    CollisionShapePlane.h
    CollisionShapeSphere.h
    CollisionShapeStaticMesh.h
    Component.h
    ComponentManager.h
    Const.h
    DirectionalLight.h
    Drawable.h
    DummyShell.h
    Equirect2CubeComponent.h
    FPComponent.h
    Game.h
    GameLogAppender.h
    GameShell.h
    HardwareBuffer.h
    HardwareContext.h
    HardwareDataBuffer.h
    HardwareFramebuffer.h
    HardwareIndexBuffer.h
    HardwareMRT.h
    HardwareProgram.h
    HardwareRenderbuffer.h
    HardwareRenderTarget.h
    HardwareResource.h
    HardwareResourceManager.h
    HardwareSampler.h
    HardwareShader.h
    HardwareTexture.h
    HardwareVertexArray.h
    Image.h
    ImGuiComponent.h
    ImGuiFileDialog.h
    ImGuiManager.h
    ImGuiUtils.h
    InputGamepad.h
    InputKeyboard.h
    InputManager.h
    InputMouse.h
    Joint6DOF.h
    JointConeTwist.h
    Joint.h
    JointHinge.h
    JointPointToPoint.h
    JointSlider.h
    Layer.h
    Level.h
    Light.h
    LightProbeComponent.h
    Logger.h
    Material.h
    MaterialManager.h
    MaterialType.h
    Mesh.h
    MeshImportComponent.h
    MeshImportSettings.h
    MeshManager.h
    MotionState.h
    OGL.h
    PhasedComponent.h
    PhasedComponentManager.h
    PhysicsBodyComponent.h
    PhysicsComponent.h
    PhysicsComponentManager.h
    PhysicsDebugDraw.h
    PhysicsJointComponent.h
    Platform.h
    PlatformLinux.h
    PlatformWin32.h
    PointLight.h
    RenderAxesComponent.h
    RenderCollisionShapeComponent.h
    RenderComponent.h
    RenderComponentManager.h
    Renderer.h
    RenderFilterComponent.h
    RenderGizmoAxesComponent.h
    RenderGizmoRotateComponent.h
    RenderGridComponent.h
    RenderJointComponent.h
    RenderList.h
    RenderMeshComponent.h
    RenderNode.h
    RenderPassCluster.h
    RenderPassCSM.h
    RenderPassGeometry.h
    RenderPass.h
    RenderPassPrepass.h
    RenderProxyComponent.h
    RenderQuadComponent.h
    RenderSkyBoxComponent.h
    RenderTarget.h
    Resource.h
    ResourceManager.h
    SceneAsset.h
    SceneEnvironment.h
    Scene.h
    SceneObjectFactory.h
    SceneObject.h
    SceneObjectManager.h
    ScriptCollisionComponent.h
    ScriptComponent.h
    ScriptConverters.h
    Script.h
    ScriptImpl.h
    ScriptSensorListener.h
    ScriptUITimerComponent.h
    SensorListener.h
    SequentialTweening.h
    Settings.h
    ShaderDataTypes.h
    ShadowManager.h
    ShadowMapCSM.h
    ShadowMap.h
    SingleTweening.h
    SpotLight.h
    SSAOComponent.h
    SubMesh.h
    TAAComponent.h
    Texture.h
    TextureManager.h
    TVComponent.h
    Tweening.h
    UIComponent.h
    UIComponentManager.h
    UITimerComponent.h
    Utils.h
    VertexArray.h
    VertexArrayLayout.h
    VertexArraySlice.h
    VertexArrayWriter.h
    Component.cpp
    RenderMeshComponent.cpp
    RenderComponentManager.cpp
    PhasedComponentManager.cpp
    CameraComponent.cpp
    CameraUsageComponent.cpp
    Camera.cpp
    CameraRenderer.cpp
    RenderPassPrepass.cpp
    RenderPassCluster.cpp
    RenderPassGeometry.cpp
    RenderPassCSM.cpp
    FPComponent.cpp
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
    OGL.cpp
    Scene.cpp
    SceneObject.cpp
    SceneObjectFactory.cpp
    SceneObjectManager.cpp
    SceneEnvironment.cpp
    Settings.cpp
    Platform.cpp
    Utils.cpp
    Layer.cpp
    HardwareContext.cpp
    HardwareResource.cpp
    HardwareBuffer.cpp
    HardwareDataBuffer.cpp
    HardwareIndexBuffer.cpp
    HardwareProgram.cpp
    HardwareResourceManager.cpp
    HardwareShader.cpp
    HardwareTexture.cpp
    HardwareVertexArray.cpp
    HardwareSampler.cpp
    HardwareFramebuffer.cpp
    HardwareRenderbuffer.cpp
    Material.cpp
    MaterialManager.cpp
    MaterialType.cpp
    Mesh.cpp
    MeshManager.cpp
    MeshImportComponent.cpp
    MeshImportSettings.cpp
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
    LightProbeComponent.cpp
    DirectionalLight.cpp
    PointLight.cpp
    SpotLight.cpp
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
    ImGuiUtils.cpp
    ImGuiFileDialog.cpp
    VertexArrayWriter.cpp
    APropertyType.cpp
    APropertyTypes.cpp
    APropertyValue.cpp
    AClass.cpp
    AObject.cpp
    AWeakObject.cpp
    AClassRegistry.cpp
    Image.cpp
    AssetManager.cpp
    Asset.cpp
    AssetTexture.cpp
    AssetModel.cpp
    RenderQuadComponent.cpp
    RenderGizmoAxesComponent.cpp
    RenderGizmoRotateComponent.cpp
    RenderGridComponent.cpp
    RenderAxesComponent.cpp
    RenderCollisionShapeComponent.cpp
    RenderJointComponent.cpp
    RenderFilterComponent.cpp
    RenderSkyBoxComponent.cpp
    RenderProxyComponent.cpp
    AJsonReader.cpp
    AJsonWriter.cpp
    SceneAsset.cpp
    PhysicsComponentManager.cpp
    PhysicsComponent.cpp
    PhysicsBodyComponent.cpp
    PhysicsJointComponent.cpp
    CollisionMatrix.cpp
    CollisionFilter.cpp
    CollisionShape.cpp
    CollisionShapeCompound.cpp
    CollisionShapeBox.cpp
    CollisionShapeCapsule.cpp
    CollisionShapeSphere.cpp
    CollisionShapeCylinder.cpp
    CollisionShapeCone.cpp
    CollisionShapePlane.cpp
    CollisionShapeStaticMesh.cpp
    CollisionShapeConvexMesh.cpp
    CollisionComponentManager.cpp
    CollisionComponent.cpp
    CollisionSensorComponent.cpp
    ScriptSensorListener.cpp
    ScriptCollisionComponent.cpp
    PhysicsDebugDraw.cpp
    MotionState.cpp
    Script.cpp
    ScriptComponent.cpp
    ScriptUITimerComponent.cpp
    SequentialTweening.cpp
    SingleTweening.cpp
    UITimerComponent.cpp
    Joint.cpp
    JointPointToPoint.cpp
    JointConeTwist.cpp
    JointHinge.cpp
    JointSlider.cpp
    Joint6DOF.cpp
    TVComponent.cpp
    Equirect2CubeComponent.cpp
    TAAComponent.cpp
    SSAOComponent.cpp
    ShadowManager.cpp
    ShadowMapCSM.cpp        
    editor/Action.h
    editor/CollisionMatrixEditor.h
    editor/CommandAdd.h
    editor/CommandDelete.h
    editor/CommandDup.h
    editor/Command.h
    editor/CommandHistory.h
    editor/CommandHistoryWindow.h
    editor/CommandSelect.h
    editor/CommandSetProperty.h
    editor/EditModeCollision.h
    editor/EditModeCollisionImpl.h
    editor/EditMode.h
    editor/EditModeImpl.h
    editor/EditModeJoint.h
    editor/EditModeJointImpl.h
    editor/EditModeLight.h
    editor/EditModeLightImpl.h
    editor/EditModeObject.h
    editor/EditModeObjectImpl.h
    editor/EditModeScene.h
    editor/EditModeSceneImpl.h
    editor/EditModeVisual.h
    editor/EditModeVisualImpl.h
    editor/EditPart.h
    editor/EditPartTransform.h
    editor/JsonSerializer.h
    editor/MainPopup.h
    editor/ObjectComponent.h
    editor/Playbar.h
    editor/PropertyEditor.h
    editor/Toolbox.h
    editor/ToolGizmo.h
    editor/Tool.h
    editor/ToolMove.h
    editor/ToolRotate.h
    editor/ToolScale.h
    editor/ToolSelect.h
    editor/Workspace.h   
    editor/Action.cpp
    editor/Workspace.cpp
    editor/MainPopup.cpp
    editor/CommandAdd.cpp
    editor/CommandSelect.cpp
    editor/CommandHistory.cpp
    editor/CommandHistoryWindow.cpp
    editor/CommandSetProperty.cpp
    editor/CommandDelete.cpp
    editor/CommandDup.cpp
    editor/ObjectComponent.cpp
    editor/EditModeImpl.cpp
    editor/EditModeObjectImpl.cpp
    editor/EditModeVisualImpl.cpp
    editor/EditModeLightImpl.cpp
    editor/EditModeSceneImpl.cpp
    editor/EditModeCollisionImpl.cpp
    editor/EditModeJointImpl.cpp
    editor/PropertyEditor.cpp
    editor/JsonSerializer.cpp
    editor/Toolbox.cpp
    editor/Tool.cpp
    editor/ToolGizmo.cpp
    editor/ToolSelect.cpp
    editor/ToolMove.cpp
    editor/ToolRotate.cpp
    editor/ToolScale.cpp
    editor/Playbar.cpp
    editor/EditPart.cpp
    editor/EditPartTransform.cpp
    editor/CollisionMatrixEditor.cpp
)

create_source_groups(FILE_PREFIX "${AF3D_SOURCE_DIR}" FILES ${SOURCES})

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
        configure_file(${AF3D_SOURCE_DIR}/lib/win32/OpenAL32.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_CONFIGURATION}/OpenAL32.dll COPYONLY)
        configure_file(${AF3D_SOURCE_DIR}/lib/win32/wrap_oal.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_CONFIGURATION}/wrap_oal.dll COPYONLY)
    endforeach ()
    set(SOURCES ${SOURCES}
        PlatformWin32.cpp
        main_win32.cpp
    )
else ()
    set(SOURCES ${SOURCES}
        PlatformLinux.cpp
        main_x11.cpp
    )
endif ()

foreach (_ENTRY ${_INI})
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/config.ini.cpp" "\n\"${_ENTRY}\\n\"")
endforeach()

file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/config.ini.cpp" ";\n")

add_executable(af3d WIN32 ${SOURCES})

target_link_libraries(af3d af3dutil log4cplus bullet assimp imgui luabind lua)

if (NOT WIN32)
    target_link_libraries(af3d ${X11_LIBRARIES} ${X11_Xxf86vm_LIB} rt dl)

    file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets)
    configure_file(config.ini ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config.ini COPYONLY)
endif ()

if (LINK_GL)
    target_link_libraries(af3d GL)
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

if (WIN32)
    install(
        FILES
        "${AF3D_SOURCE_DIR}/lib/win32/OpenAL32.dll"
        "${AF3D_SOURCE_DIR}/lib/win32/wrap_oal.dll"
        DESTINATION "${INSTALL_OUT_DIR}")
endif()

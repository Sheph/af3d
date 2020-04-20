/*
 * Copyright (c) 2020, Stanislav Vorobiov
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "editor/Workspace.h"
#include "editor/MainPopup.h"
#include "editor/CommandHistoryWindow.h"
#include "editor/PropertyEditor.h"
#include "editor/Toolbox.h"
#include "editor/CommandAdd.h"
#include "editor/CommandSetProperty.h"
#include "editor/CommandDelete.h"
#include "editor/CommandDup.h"
#include "Const.h"
#include "InputManager.h"
#include "AClassRegistry.h"
#include "Scene.h"
#include "SceneObject.h"
#include "RenderMeshComponent.h"
#include "RenderGridComponent.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "CollisionShapeBox.h"
#include "CollisionShapeCapsule.h"
#include "CollisionShapeSphere.h"
#include "CollisionShapeCylinder.h"
#include "CollisionShapeCone.h"
#include "CollisionShapePlane.h"
#include "CollisionShapeStaticMesh.h"
#include "CollisionShapeConvexMesh.h"
#include "PhysicsBodyComponent.h"
#include "MeshManager.h"
#include "AssetManager.h"
#include "ImGuiManager.h"
#include "ImGuiFileDialog.h"
#include "AJsonWriter.h"
#include "Logger.h"
#include "Platform.h"
#include "Settings.h"
#include "imgui.h"
#include <set>
#include <fstream>

namespace af3d {
    ACLASS_NS_DEFINE_BEGIN(editor, Workspace, UIComponent)
    ACLASS_NS_DEFINE_END(editor, Workspace)

namespace editor {
    Workspace::Workspace()
    : UIComponent(AClass_editorWorkspace, zOrderEditorWorkspace),
      emObject_(new EditModeObjectImpl(this)),
      emVisual_(new EditModeVisualImpl(this)),
      emLight_(new EditModeLightImpl(this)),
      emCollision_(new EditModeCollisionImpl(this)),
      emScene_(new EditModeSceneImpl(this)),
      toolSelect_(new ToolSelect(this)),
      toolMove_(new ToolMove(this)),
      toolRotate_(new ToolRotate(this)),
      toolScale_(new ToolScale(this))
    {
        static const char* prefix = "SceneObject";

        ems_.push_back(emObject_.get());
        ems_.push_back(emVisual_.get());
        ems_.push_back(emLight_.get());
        ems_.push_back(emCollision_.get());
        ems_.push_back(emScene_.get());
        em_ = emObject_.get();

        tools_.push_back(toolSelect_.get());
        tools_.push_back(toolMove_.get());
        tools_.push_back(toolRotate_.get());
        tools_.push_back(toolScale_.get());
        currentTool_ = toolSelect_.get();

        std::set<std::string> sortedKinds;

        for (const auto& kv : AClassRegistry::instance().map()) {
            if (kv.second->super() == &AClass_SceneObject) {
                sortedKinds.insert(kv.second->name());
            }
        }

        objectKinds_.push_back("Empty");

        for (const auto& k : sortedKinds) {
            if (k.find(prefix) == 0) {
                objectKinds_.push_back(k.substr(std::strlen(prefix)));
            }
        }

        setupActions();
    }

    const AClass& Workspace::staticKlass()
    {
        return AClass_editorWorkspace;
    }

    AObjectPtr Workspace::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<Workspace>();
        obj->propertiesSet(propVals);
        return obj;
    }

    void Workspace::update(float dt)
    {
        float mainMenuH = mainMenu();
        mainToolbar(mainMenuH);

        if (!locked()) {
            if (needNewSceneDlg_) {
                needNewSceneDlg_ = false;
                ImGui::OpenPopup("New scene");
            }

            if (needOpenSceneDlg_) {
                needOpenSceneDlg_ = false;
                ImGui::OpenPopup("Open scene");
            }

            if (auto dlg = ImGuiFileDialog::beginAssetsModal("New scene", "noname.af3", "Scene files,.af3;All files")) {
                if (dlg->ok() && !dlg->fileName().empty()) {
                    scene()->setNextLevel(dlg->filePath());
                }
                dlg->endModal();
            }

            if (auto dlg = ImGuiFileDialog::beginAssetsModal("Open scene", "", "Scene files,.af3;All files")) {
                if (dlg->ok() && !dlg->fileName().empty()) {
                    scene()->setNextLevel(dlg->filePath());
                }
                dlg->endModal();
            }

            ImGuiIO& io = ImGui::GetIO();

            if (!io.WantCaptureMouse && !io.WantCaptureKeyboard) {
                bool triggered = false;
                for (auto action : actions_) {
                    if (!action->shortcut().empty() && inputManager.keyboard().triggered(action->shortcut())) {
                        action->trigger();
                        triggered = true;
                        break;
                    }
                }
                if (!triggered) {
                    for (size_t i = 0; i < tools().size(); ++i) {
                        if (!tools()[i]->shortcut().empty() &&
                            (tools()[i] != currentTool()) &&
                            tools()[i]->canWork() &&
                            inputManager.keyboard().triggered(tools()[i]->shortcut())) {
                            currentTool()->activate(false);
                            setCurrentTool(tools()[i]);
                            if (toolsActive()) {
                                currentTool()->activate(true);
                            }
                        }
                    }
                }
            }
        }

        for (auto tool : tools_) {
            tool->update(dt);
        }
    }

    void Workspace::render(RenderList& rl)
    {
    }

    void Workspace::setToolsActive(bool value)
    {
        if (toolsActive_ == value) {
            return;
        }
        toolsActive_ = value;
        currentTool()->activate(value);
    }

    void Workspace::addObject(const std::string& kind)
    {
        std::string actualKind;

        if (kind != "Empty") {
            actualKind = kind;
        }

        auto klass = AClassRegistry::instance().classFind("SceneObject" + actualKind);

        if (!klass || ((klass->super() != &AClass_SceneObject) && !actualKind.empty())) {
            LOG4CPLUS_WARN(logger(), "Cannot addObject, bad kind - " << kind);
            return;
        }

        APropertyValueMap initVals;
        initVals.set(AProperty_WorldTransform, scene()->camera()->transform() * toTransform(btVector3_forward * 5.0f));

        cmdHistory_.add(
             std::make_shared<CommandAdd>(scene(), scene()->sharedThis(), *klass, "\"" + kind + "\" object",
                 initVals));
    }

    void Workspace::setProperty(const AObjectPtr& obj,
        const std::string& name, const APropertyValue& value, bool isParam)
    {
        cmdHistory_.add(
            std::make_shared<CommandSetProperty>(scene(), obj, name, value, isParam));
    }

    void Workspace::setEditMode(EditModeImpl* value)
    {
        if (value->active()) {
            return;
        }
        em_->leave();
        em_ = value;
        em_->enter();
    }

    void Workspace::setOverrideEditMode(EditMode* value)
    {
        if (value) {
            btAssert(!overriddenEm_);
            overriddenEm_ = em_;
            value->activate();
        } else {
            btAssert(overriddenEm_);
            overriddenEm_->activate();
            overriddenEm_ = nullptr;
        }
    }

    void Workspace::deleteObject(const AObjectPtr& obj)
    {
        cmdHistory_.add(
            std::make_shared<CommandDelete>(scene(), obj));
    }

    void Workspace::duplicateObject(const AObjectPtr& obj)
    {
        cmdHistory_.add(
            std::make_shared<CommandDup>(scene(), obj));
    }

    void Workspace::onRegister()
    {
        grid_ = std::make_shared<SceneObject>();
        grid_->setTransform(makeLookDir(btVector3_zero, btVector3_up, btVector3_forward));
        grid_->addComponent(std::make_shared<RenderGridComponent>());
        scene()->addObject(grid_);

        emScene_->setSelected(EditMode::AWeakList{AWeakObject(scene()->sharedThis())});
        em_->enter();
        currentTool_->activate(true);

        if (imGuiManager.cfgGetBool(ImGuiManager::strCommandHistoryOpened, true)) {
            actionCommandHistory().trigger();
        }
        if (imGuiManager.cfgGetBool(ImGuiManager::strPropertyEditorOpened, true)) {
            actionPropertyEditor().trigger();
        }
        if (imGuiManager.cfgGetBool(ImGuiManager::strToolBoxOpened, true)) {
            actionToolbox().trigger();
        }
    }

    void Workspace::onUnregister()
    {
        currentTool_->activate(false);
        em_->leave();

        emObject_.reset();
        emVisual_.reset();
        emLight_.reset();
        emCollision_.reset();
        emScene_.reset();
        em_ = nullptr;
        overriddenEm_ = nullptr;
        ems_.clear();

        toolSelect_.reset();
        toolMove_.reset();
        toolRotate_.reset();
        toolScale_.reset();
        currentTool_ = nullptr;
        tools_.clear();

        grid_->removeFromParent();
        grid_.reset();
    }

    void Workspace::setupActions()
    {
        actionSceneNew_ = Action("New scene", []() {
            return Action::State(true);
        }, [this]() {
            needNewSceneDlg_ = true;
        }, assetManager.getImage("common1/scene_new.png"));

        actionSceneOpen_ = Action("Open scene", []() {
            return Action::State(true);
        }, [this]() {
            needOpenSceneDlg_ = true;
        }, assetManager.getImage("common1/scene_open.png"), KeySequence(KM_CTRL, KI_O));

        actionSceneSave_ = Action("Save scene", [this]() {
            return Action::State(true);
        }, [this]() {
            saveAs(scene()->assetPath());
            cmdHistory_.resetDirty();
        }, assetManager.getImage("common1/scene_save.png"), KeySequence(KM_CTRL, KI_S));

        actionModeScene_ = Action("Edit scene settings", [this]() {
            return Action::State(true, emScene_->active());
        }, [this]() {
            emScene_->activate();
        }, assetManager.getImage("common1/mode_scene.png"), KeySequence(KI_U));

        actionModeObject_ = Action("Edit objects", [this]() {
            return Action::State(true, emObject_->active());
        }, [this]() {
            emObject_->activate();
        }, assetManager.getImage("common1/mode_object.png"), KeySequence(KI_O));

        actionModeVisual_ = Action("Edit visuals", [this]() {
            return Action::State(true, emVisual_->active());
        }, [this]() {
            emVisual_->activate();
        }, assetManager.getImage("common1/mode_visual.png"), KeySequence(KI_V));

        actionModeLight_ = Action("Edit lights", [this]() {
            return Action::State(true, emLight_->active());
        }, [this]() {
            emLight_->activate();
        }, assetManager.getImage("common1/mode_light.png"), KeySequence(KI_L));

        actionModeCollision_ = Action("Edit collision", [this]() {
            return Action::State(true, emCollision_->active());
        }, [this]() {
            emCollision_->activate();
        }, assetManager.getImage("common1/mode_collision.png"), KeySequence(KI_F));

        actionUndo_ = Action("Undo", [this]() {
            return Action::State(cmdHistory_.pos() > 0);
        }, [this]() {
            cmdHistory_.undo(1);
        }, assetManager.getImage("common1/undo.png"), KeySequence(KM_CTRL, KI_Z));

        actionRedo_ = Action("Redo", [this]() {
            return Action::State(cmdHistory_.pos() < static_cast<int>(cmdHistory_.list().size()));
        }, [this]() {
            cmdHistory_.redo(1);
        }, assetManager.getImage("common1/redo.png"), KeySequence(KM_CTRL, KI_Y));

        actionDelete_ = Action("Delete", [this]() {
            return Action::State(!em_->selected().empty());
        }, [this]() {
            auto sel = em_->selected();
            for (const auto& wobj : sel) {
                deleteObject(wobj.lock());
            }
        }, assetManager.getImage("common1/action_delete.png"), KeySequence(KI_DELETE));

        actionDup_ = Action("Duplicate", [this]() {
            return Action::State(!em_->selected().empty());
        }, [this]() {
            auto sel = em_->selected();
            for (const auto& wobj : sel) {
                duplicateObject(wobj.lock());
            }
        }, assetManager.getImage("common1/action_dup.png"), KeySequence(KM_CTRL, KI_D));

        actionPlay_ = Action("Play", []() {
            return Action::State(true);
        }, [this]() {
            settings.editor.playing = true;
            saveAs("_play.af3");
            scene()->setNextLevel("_play.af3");
        }, assetManager.getImage("common1/action_play.png"), KeySequence(KM_CTRL, KI_R));

        actionOpMenu_ = Action("", []() {
            return Action::State(true);
        }, [this]() {
            actionOpMenuAdd_.doMenu();
            actionOpMenuRemove_.doMenu();
        });

        actionOpMenuAdd_ = Action("Add", []() {
            return Action::State(true);
        }, [this]() {
            actionOpMenuAddObject_.doMenu();
            actionOpMenuAddLight_.doMenu();
            actionOpMenuAddCollision_.doMenu();
            actionOpMenuAddMesh_.doMenuItem();
            actionOpMenuAddPhysicsBody_.doMenuItem();
        });

        actionOpMenuAddObject_ = Action("Object", []() {
            return Action::State(true);
        }, [this]() {
            for (const auto& kind : objectKinds()) {
                if (ImGui::MenuItem(kind.c_str())) {
                    addObject(kind);
                }
            }
        });

        actionOpMenuAddMesh_ = Action("Mesh", [this]() {
            return Action::State(!emObject_->selected().empty());
        }, [this]() {
            APropertyValueMap initVals;
            initVals.set("mesh", APropertyValue(meshManager.loadMesh("cube.fbx")));

            cmdHistory_.add(
                std::make_shared<CommandAdd>(scene(),
                    emObject_->selected().back().lock(),
                    RenderMeshComponent::staticKlass(), "Mesh", initVals));
        });

        actionOpMenuAddLight_ = Action("Light", [this]() {
            return Action::State(!emObject_->selected().empty());
        }, [this]() {
            actionOpMenuAddLightDirectional_.doMenuItem();
            actionOpMenuAddLightPoint_.doMenuItem();
        });

        actionOpMenuAddLightDirectional_ = Action("Directional", [this]() {
            return Action::State(!emObject_->selected().empty());
        }, [this]() {
            APropertyValueMap initVals;
            cmdHistory_.add(
                std::make_shared<CommandAdd>(scene(),
                    emObject_->selected().back().lock(),
                    DirectionalLight::staticKlass(), "Directional light", initVals));
        });

        actionOpMenuAddLightPoint_ = Action("Point", [this]() {
            return Action::State(!emObject_->selected().empty());
        }, [this]() {
            APropertyValueMap initVals;
            cmdHistory_.add(
                std::make_shared<CommandAdd>(scene(),
                    emObject_->selected().back().lock(),
                    PointLight::staticKlass(), "Point light", initVals));
        });

        actionOpMenuAddCollision_ = Action("Collision", [this]() {
            return Action::State(objectWithPhysicsBodySelected());
        }, [this]() {
            actionOpMenuAddCollisionBox_.doMenuItem();
            actionOpMenuAddCollisionCapsule_.doMenuItem();
            actionOpMenuAddCollisionSphere_.doMenuItem();
            actionOpMenuAddCollisionCylinder_.doMenuItem();
            actionOpMenuAddCollisionCone_.doMenuItem();
            actionOpMenuAddCollisionPlane_.doMenuItem();
            actionOpMenuAddCollisionStaticMesh_.doMenuItem();
            actionOpMenuAddCollisionConvexMesh_.doMenuItem();
        });

        actionOpMenuAddCollisionBox_ = Action("Box", [this]() {
            return Action::State(objectWithPhysicsBodySelected());
        }, [this]() {
            APropertyValueMap initVals;
            cmdHistory_.add(
                std::make_shared<CommandAdd>(scene(),
                    emObject_->selectedTyped().back()->findComponent<PhysicsBodyComponent>(),
                    CollisionShapeBox::staticKlass(), "Box collision", initVals));
        });

        actionOpMenuAddCollisionCapsule_ = Action("Capsule", [this]() {
            return Action::State(objectWithPhysicsBodySelected());
        }, [this]() {
            APropertyValueMap initVals;
            cmdHistory_.add(
                std::make_shared<CommandAdd>(scene(),
                    emObject_->selectedTyped().back()->findComponent<PhysicsBodyComponent>(),
                    CollisionShapeCapsule::staticKlass(), "Capsule collision", initVals));
        });

        actionOpMenuAddCollisionSphere_ = Action("Sphere", [this]() {
            return Action::State(objectWithPhysicsBodySelected());
        }, [this]() {
            APropertyValueMap initVals;
            cmdHistory_.add(
                std::make_shared<CommandAdd>(scene(),
                    emObject_->selectedTyped().back()->findComponent<PhysicsBodyComponent>(),
                    CollisionShapeSphere::staticKlass(), "Sphere collision", initVals));
        });

        actionOpMenuAddCollisionCylinder_ = Action("Cylinder", [this]() {
            return Action::State(objectWithPhysicsBodySelected());
        }, [this]() {
            APropertyValueMap initVals;
            cmdHistory_.add(
                std::make_shared<CommandAdd>(scene(),
                    emObject_->selectedTyped().back()->findComponent<PhysicsBodyComponent>(),
                    CollisionShapeCylinder::staticKlass(), "Cylinder collision", initVals));
        });

        actionOpMenuAddCollisionCone_ = Action("Cone", [this]() {
            return Action::State(objectWithPhysicsBodySelected());
        }, [this]() {
            APropertyValueMap initVals;
            cmdHistory_.add(
                std::make_shared<CommandAdd>(scene(),
                    emObject_->selectedTyped().back()->findComponent<PhysicsBodyComponent>(),
                    CollisionShapeCone::staticKlass(), "Cone collision", initVals));
        });

        actionOpMenuAddCollisionPlane_ = Action("Plane", [this]() {
            return Action::State(objectWithPhysicsBodySelected());
        }, [this]() {
            APropertyValueMap initVals;
            cmdHistory_.add(
                std::make_shared<CommandAdd>(scene(),
                    emObject_->selectedTyped().back()->findComponent<PhysicsBodyComponent>(),
                    CollisionShapePlane::staticKlass(), "Plane collision", initVals));
        });

        actionOpMenuAddCollisionStaticMesh_ = Action("Static Mesh", [this]() {
            return Action::State(objectWithPhysicsBodySelected());
        }, [this]() {
            APropertyValueMap initVals;
            initVals.set("mesh", APropertyValue(meshManager.loadMesh("cube.fbx")));
            cmdHistory_.add(
                std::make_shared<CommandAdd>(scene(),
                    emObject_->selectedTyped().back()->findComponent<PhysicsBodyComponent>(),
                    CollisionShapeStaticMesh::staticKlass(), "Static mesh collision", initVals));
        });

        actionOpMenuAddCollisionConvexMesh_ = Action("Convex Mesh", [this]() {
            return Action::State(objectWithPhysicsBodySelected());
        }, [this]() {
            APropertyValueMap initVals;
            initVals.set("mesh", APropertyValue(meshManager.loadMesh("cube.fbx")));
            cmdHistory_.add(
                std::make_shared<CommandAdd>(scene(),
                    emObject_->selectedTyped().back()->findComponent<PhysicsBodyComponent>(),
                    CollisionShapeConvexMesh::staticKlass(), "Convex mesh collision", initVals));
        });

        actionOpMenuAddPhysicsBody_ = Action("Physics body", [this]() {
            bool enabled = false;
            if (!emObject_->selected().empty()) {
                auto obj = emObject_->selectedTyped().back();
                enabled = !obj->findComponent<PhysicsBodyComponent>();
            }
            return Action::State(enabled);
        }, [this]() {
            APropertyValueMap initVals;
            cmdHistory_.add(
                std::make_shared<CommandAdd>(scene(),
                    emObject_->selected().back().lock(),
                    PhysicsBodyComponent::staticKlass(), "Physics body", initVals));
        });

        actionOpMenuRemove_ = Action("Remove", []() {
            return Action::State(true);
        }, [this]() {
            actionOpMenuRemovePhysicsBody_.doMenuItem();
        });

        actionOpMenuRemovePhysicsBody_ = Action("Physics body", [this]() {
            return Action::State(objectWithPhysicsBodySelected());
        }, [this]() {
            deleteObject(emObject_->selectedTyped().back()->findComponent<PhysicsBodyComponent>());
        });

        actionMainPopup_ = Action("", [this]() {
            return Action::State(!parent()->findComponent<MainPopup>());
        }, [this]() {
            auto popup = std::make_shared<MainPopup>();
            parent()->addComponent(popup);
        }, Image(), KeySequence(KI_I));

        actionCommandHistory_ = Action("Command History", [this]() {
            return Action::State(!parent()->findComponent<CommandHistoryWindow>());
        }, [this]() {
            auto w = std::make_shared<CommandHistoryWindow>();
            parent()->addComponent(w);
        });

        actionPropertyEditor_ = Action("Properties", [this]() {
            return Action::State(!parent()->findComponent<PropertyEditor>());
        }, [this]() {
            auto w = std::make_shared<PropertyEditor>();
            parent()->addComponent(w);
        });

        actionToolbox_ = Action("Toolbox", [this]() {
            return Action::State(!parent()->findComponent<Toolbox>());
        }, [this]() {
            auto w = std::make_shared<Toolbox>();
            parent()->addComponent(w);
        });

        actions_.push_back(&actionSceneNew_);
        actions_.push_back(&actionSceneOpen_);
        actions_.push_back(&actionSceneSave_);
        actions_.push_back(&actionModeScene_);
        actions_.push_back(&actionModeObject_);
        actions_.push_back(&actionModeVisual_);
        actions_.push_back(&actionModeLight_);
        actions_.push_back(&actionModeCollision_);
        actions_.push_back(&actionUndo_);
        actions_.push_back(&actionRedo_);
        actions_.push_back(&actionDelete_);
        actions_.push_back(&actionDup_);
        actions_.push_back(&actionPlay_);
        actions_.push_back(&actionOpMenu_);
        actions_.push_back(&actionOpMenuAdd_);
        actions_.push_back(&actionOpMenuAddObject_);
        actions_.push_back(&actionOpMenuAddMesh_);
        actions_.push_back(&actionOpMenuAddLight_);
        actions_.push_back(&actionOpMenuAddLightDirectional_);
        actions_.push_back(&actionOpMenuAddLightPoint_);
        actions_.push_back(&actionOpMenuAddCollision_);
        actions_.push_back(&actionOpMenuAddCollisionBox_);
        actions_.push_back(&actionOpMenuAddCollisionCapsule_);
        actions_.push_back(&actionOpMenuAddCollisionSphere_);
        actions_.push_back(&actionOpMenuAddCollisionCylinder_);
        actions_.push_back(&actionOpMenuAddCollisionCone_);
        actions_.push_back(&actionOpMenuAddCollisionPlane_);
        actions_.push_back(&actionOpMenuAddCollisionStaticMesh_);
        actions_.push_back(&actionOpMenuAddCollisionConvexMesh_);
        actions_.push_back(&actionOpMenuAddPhysicsBody_);
        actions_.push_back(&actionOpMenuRemove_);
        actions_.push_back(&actionOpMenuRemovePhysicsBody_);
        actions_.push_back(&actionMainPopup_);
        actions_.push_back(&actionCommandHistory_);
        actions_.push_back(&actionPropertyEditor_);
        actions_.push_back(&actionToolbox_);
    }

    float Workspace::mainMenu()
    {
        const ImGuiStyle& style = ImGui::GetStyle();

        auto c1 = style.Colors[ImGuiCol_WindowBg];
        auto c2 = style.Colors[ImGuiCol_MenuBarBg];
        c2.w = c1.w;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_MenuBarBg, c2);

        float h = 0.0f;

        ImGui::SetNextWindowBgAlpha(0.0f);
        if (ImGui::BeginMainMenuBar()) {
            mainMenuContents();

            ImGui::SameLine();
            ImGui::Text("| %s%s", scene()->assetPath().c_str(), (cmdHistory_.dirty() ? "*" : ""));

            h = ImGui::GetWindowHeight();

            ImGui::EndMainMenuBar();
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor();

        return h;
    }

    void Workspace::mainMenuContents()
    {
        if (ImGui::BeginMenu("File")) {
            actionSceneNew().doMenuItem();
            actionSceneOpen().doMenuItem();
            actionSceneSave().doMenuItem();
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                scene()->setQuit(true);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            actionModeObject().doMenuItem();
            actionModeVisual().doMenuItem();
            actionModeLight().doMenuItem();
            actionModeCollision().doMenuItem();
            actionModeScene().doMenuItem();
            ImGui::Separator();
            actionUndo().doMenuItem();
            actionRedo().doMenuItem();
            ImGui::Separator();
            actionDelete().doMenuItem();
            actionDup().doMenuItem();
            ImGui::Separator();
            actionPlay().doMenuItem();
            ImGui::Separator();
            actionOpMenu_.trigger();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Window")) {
            actionCommandHistory().doMenuItem();
            actionPropertyEditor().doMenuItem();
            actionToolbox().doMenuItem();
            ImGui::EndMenu();
        }
    }

    void Workspace::mainToolbar(float offsetY)
    {
        ImGuiIO& io = ImGui::GetIO();

        ImGui::SetNextWindowPos(ImVec2(0, offsetY));
        ImGui::SetNextWindowSizeConstraints(ImVec2(io.DisplaySize.x, -1), ImVec2(io.DisplaySize.x, -1));

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings;

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 5));

        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0));
        ImGui::Begin("MainToolbar", nullptr, windowFlags);
        ImGui::PopStyleVar(4);

        mainToolbarContents();

        ImGui::End();

        ImGui::PopStyleVar();
    }

    void Workspace::mainToolbarContents()
    {
        toolbarButton(actionSceneNew());
        toolbarButton(actionSceneOpen());
        toolbarButton(actionSceneSave());

        toolbarSep();

        toolbarButton(actionModeObject());
        toolbarButton(actionModeVisual());
        toolbarButton(actionModeLight());
        toolbarButton(actionModeCollision());
        toolbarButton(actionModeScene());

        toolbarSep();

        toolbarButton(actionUndo());
        toolbarButton(actionRedo());

        toolbarSep();

        toolbarButton(actionDelete());
        toolbarButton(actionDup());

        toolbarSep();

        toolbarButton(actionPlay());
    }

    void Workspace::saveAs(const std::string& path)
    {
        Json::Value val(Json::arrayValue);
        AJsonSerializerDefault defS;
        AJsonWriter writer(val, defS);
        writer.write(scene()->sharedThis());
        std::ofstream os(platform->assetsPath() + "/" + path,
            std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
        os << Json::StyledWriter().write(val);
    }

    bool Workspace::objectWithPhysicsBodySelected() const
    {
        bool res = false;
        if (!emObject_->selected().empty()) {
            auto pc = emObject_->selectedTyped().back()->findComponent<PhysicsBodyComponent>();
            res = pc && ((pc->aflags() & AObjectEditable) != 0);
        }
        return res;
    }

    void Workspace::toolbarButton(Action& action)
    {
        action.doButton(28.0f);
        ImGui::SameLine();
    }

    void Workspace::toolbarSep()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 8));
        ImGui::AlignTextToFramePadding();
        ImGui::Text("|");
        ImGui::PopStyleVar();
        ImGui::SameLine();
    }
} }

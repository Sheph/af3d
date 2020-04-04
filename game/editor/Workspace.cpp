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
#include "editor/CommandAddObject.h"
#include "editor/CommandAddComponent.h"
#include "editor/CommandSetProperty.h"
#include "editor/CommandDelete.h"
#include "Const.h"
#include "InputManager.h"
#include "AClassRegistry.h"
#include "Scene.h"
#include "SceneObject.h"
#include "RenderMeshComponent.h"
#include "MeshManager.h"
#include "AssetManager.h"
#include "ImGuiManager.h"
#include "ImGuiFileDialog.h"
#include "AJsonWriter.h"
#include "Logger.h"
#include "Platform.h"
#include "imgui.h"
#include <set>
#include <fstream>

namespace af3d {
    ACLASS_NS_DEFINE_BEGIN(editor, Workspace, UIComponent)
    ACLASS_NS_DEFINE_END(editor, Workspace)

namespace editor {
    Workspace::Workspace()
    : UIComponent(AClass_editorWorkspace, zOrderEditor),
      emObject_(new EditModeObjectImpl(this)),
      emVisual_(new EditModeVisualImpl(this)),
      toolSelect_(new ToolSelect(this)),
      toolMove_(new ToolMove(this)),
      toolRotate_(new ToolRotate(this))
    {
        static const char* prefix = "SceneObject";

        ems_.push_back(emObject_.get());
        ems_.push_back(emVisual_.get());
        em_ = emObject_.get();

        tools_.push_back(toolSelect_.get());
        tools_.push_back(toolMove_.get());
        tools_.push_back(toolRotate_.get());
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
                if (inputManager.keyboard().triggered(KI_I)) {
                    actionMainPopup().trigger();
                } else if (inputManager.keyboard().triggered(KI_DELETE)) {
                    auto sel = em_->selected();
                    for (const auto& wobj : sel) {
                        deleteObject(wobj.lock());
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

        cmdHistory_.add(
             std::make_shared<CommandAddObject>(scene(), *klass, kind,
                 scene()->camera()->transform() * toTransform(btVector3_forward * 5.0f)));
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

    void Workspace::deleteObject(const AObjectPtr& obj)
    {
        cmdHistory_.add(
            std::make_shared<CommandDelete>(scene(), obj));
    }

    void Workspace::onRegister()
    {
        em_->enter();
        currentTool_->activate(true);

        actionCommandHistory().trigger();
        actionPropertyEditor().trigger();
        actionToolbox().trigger();
    }

    void Workspace::onUnregister()
    {
        currentTool_->activate(false);
        em_->leave();

        emObject_.reset();
        emVisual_.reset();
        em_ = nullptr;
        ems_.clear();

        toolSelect_.reset();
        toolMove_.reset();
        toolRotate_.reset();
        currentTool_ = nullptr;
        tools_.clear();
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
        }, assetManager.getImage("common1/scene_open.png"));

        actionSceneSave_ = Action("Save scene", [this]() {
            return Action::State(cmdHistory_.dirty());
        }, [this]() {
            Json::Value val(Json::arrayValue);
            AJsonSerializerDefault defS;
            AJsonWriter writer(val, defS);
            writer.write(scene()->sharedThis());
            std::ofstream os(platform->assetsPath() + "/" + scene()->assetPath(),
                std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
            os << Json::StyledWriter().write(val);
            cmdHistory_.resetDirty();
        }, assetManager.getImage("common1/scene_save.png"));

        actionModeScene_ = Action("Edit scene settings", []() {
            return Action::State(false);
        }, []() {
        }, assetManager.getImage("common1/mode_scene.png"));

        actionModeObject_ = Action("Edit objects", [this]() {
            return Action::State(true, emObject_->active());
        }, [this]() {
            emObject_->activate();
        }, assetManager.getImage("common1/mode_object.png"));

        actionModeVisual_ = Action("Edit visuals", [this]() {
            return Action::State(true, emVisual_->active());
        }, [this]() {
            emVisual_->activate();
        }, assetManager.getImage("common1/mode_visual.png"));

        actionModeLight_ = Action("Edit lights", []() {
            return Action::State(false);
        }, []() {
        }, assetManager.getImage("common1/mode_light.png"));

        actionUndo_ = Action("Undo", [this]() {
            return Action::State(cmdHistory_.pos() > 0);
        }, [this]() {
            cmdHistory_.undo(1);
        }, assetManager.getImage("common1/undo.png"));

        actionRedo_ = Action("Redo", [this]() {
            return Action::State(cmdHistory_.pos() < static_cast<int>(cmdHistory_.list().size()));
        }, [this]() {
            cmdHistory_.redo(1);
        }, assetManager.getImage("common1/redo.png"));

        actionOpMenu_ = Action("", []() {
            return Action::State(true);
        }, [this]() {
            actionOpMenuAdd_.doMenu();
        });

        actionOpMenuAdd_ = Action("Add", []() {
            return Action::State(true);
        }, [this]() {
            actionOpMenuAddObject_.doMenu();
            actionOpMenuAddMesh_.doMenuItem();
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
            initVals.set("mesh", APropertyValue(meshManager.loadMesh("muro.fbx")));
            initVals.set(AProperty_Scale, btVector3(0.01f, 0.01f, 0.01f));

            cmdHistory_.add(
                std::make_shared<CommandAddComponent>(scene(),
                    emObject_->selected().back().lock(),
                    RenderMeshComponent::staticKlass(), "Mesh", initVals));
        });

        actionMainPopup_ = Action("", [this]() {
            return Action::State(!parent()->findComponent<MainPopup>());
        }, [this]() {
            auto popup = std::make_shared<MainPopup>();
            parent()->addComponent(popup);
        });

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
            ImGui::Text("| %s%s", scene()->assetPath().c_str(), (actionSceneSave().state().enabled ? "*" : ""));

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
            actionModeScene().doMenuItem();
            ImGui::Separator();
            actionUndo().doMenuItem();
            actionRedo().doMenuItem();
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
        toolbarButton(actionModeScene());

        toolbarSep();

        toolbarButton(actionUndo());
        toolbarButton(actionRedo());
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

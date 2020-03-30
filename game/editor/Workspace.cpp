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
#include "editor/CommandAddObject.h"
#include "editor/CommandAddComponent.h"
#include "editor/CommandSetProperty.h"
#include "Const.h"
#include "InputManager.h"
#include "AClassRegistry.h"
#include "Scene.h"
#include "SceneObject.h"
#include "CameraComponent.h"
#include "RenderMeshComponent.h"
#include "MeshManager.h"
#include "ImageManager.h"
#include "ImGuiManager.h"
#include "Logger.h"
#include "imgui.h"
#include <set>

namespace af3d {
    ACLASS_NS_DEFINE_BEGIN(editor, Workspace, UIComponent)
    ACLASS_NS_DEFINE_END(editor, Workspace)

namespace editor {
    Workspace::Workspace()
    : UIComponent(AClass_editorWorkspace, zOrderEditor),
      emObject_(new EditModeObjectImpl(this))
    {
        static const char* prefix = "SceneObject";

        em_ = emObject_.get();

        std::set<std::string> sortedKinds;

        for (const auto& kv : AClassRegistry::instance().map()) {
            if (kv.second->super() == &AClass_SceneObject) {
                sortedKinds.insert(kv.second->name());
            }
        }

        for (const auto& k : sortedKinds) {
            if (k.find(prefix) == 0) {
                objectKinds_.push_back(k.substr(std::strlen(prefix)));
            }
        }

        imgSceneNew_ = imageManager.getImage("common1/scene_new.png");
        imgSceneOpen_ = imageManager.getImage("common1/scene_open.png");
        imgSceneSave_ = imageManager.getImage("common1/scene_save.png");
        imgModeScene_ = imageManager.getImage("common1/mode_scene.png");
        imgModeObject_ = imageManager.getImage("common1/mode_object.png");
        imgModeVisual_ = imageManager.getImage("common1/mode_visual.png");
        imgModeLight_ = imageManager.getImage("common1/mode_light.png");
        imgUndo_ = imageManager.getImage("common1/undo.png");
        imgRedo_ = imageManager.getImage("common1/redo.png");
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

        ImGuiIO& io = ImGui::GetIO();

        em_->setHovered(EditMode::AList());

        if (io.WantCaptureMouse || io.WantCaptureKeyboard) {
            return;
        }

        if (inputManager.keyboard().triggered(KI_I)) {
            openMainPopup();
        }

        auto cc = scene()->camera()->findComponent<CameraComponent>();

        auto res = em_->rayCast(cc->getFrustum(), cc->screenPointToRay(inputManager.mouse().pos()));

        if (res) {
            em_->setHovered(EditMode::AList{res});
            if (inputManager.mouse().triggered(true)) {
                em_->select(EditMode::AList{res});
            }
        }
    }

    void Workspace::render(RenderList& rl)
    {
    }

    void Workspace::openMainPopup()
    {
        auto popup = parent()->findComponent<MainPopup>();
        if (popup) {
            popup->removeFromParent();
        }
        popup = std::make_shared<MainPopup>();
        parent()->addComponent(popup);
    }

    void Workspace::openCommandHistory()
    {
        auto w = parent()->findComponent<CommandHistoryWindow>();
        if (w) {
            w->removeFromParent();
        }
        w = std::make_shared<CommandHistoryWindow>();
        parent()->addComponent(w);
    }

    void Workspace::openPropertyEditor()
    {
        auto w = parent()->findComponent<PropertyEditor>();
        if (w) {
            w->removeFromParent();
        }
        w = std::make_shared<PropertyEditor>();
        parent()->addComponent(w);
    }

    void Workspace::addObject(const std::string& kind)
    {
        auto klass = AClassRegistry::instance().classFind("SceneObject" + kind);

        if (!klass || (klass->super() != &AClass_SceneObject)) {
            LOG4CPLUS_WARN(logger(), "Cannot addObject, bad kind - " << kind);
            return;
        }

        cmdHistory_.add(
             std::make_shared<CommandAddObject>(scene(), *klass, kind,
                 scene()->camera()->transform() * toTransform(btVector3_forward * 5.0f)));
    }

    void Workspace::setProperty(const AObjectPtr& obj,
        const std::string& name, const APropertyValue& value)
    {
        cmdHistory_.add(
            std::make_shared<CommandSetProperty>(scene(), obj, name, value));
    }

    void Workspace::addMesh()
    {
        if (emObject_->selected().empty()) {
            LOG4CPLUS_WARN(logger(), "Cannot addMesh, no object is selected");
            return;
        }

        APropertyValueMap initVals;
        initVals.set("mesh", APropertyValue(meshManager.loadMesh("muro.fbx")));
        initVals.set(AProperty_Scale, btVector3(0.02f, 0.02f, 0.02f));

        cmdHistory_.add(
            std::make_shared<CommandAddComponent>(scene(),
                emObject_->selected().back(),
                RenderMeshComponent::staticKlass(), "Mesh", initVals));
    }

    void Workspace::onRegister()
    {
        em_->enter();

        openCommandHistory();
        openPropertyEditor();
    }

    void Workspace::onUnregister()
    {
        em_->leave();

        emObject_.reset();
        em_ = nullptr;
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
            ImGui::MenuItem("Exit");
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            ImGui::MenuItem("Undo", "CTRL+Z");
            ImGui::MenuItem("Redo", "CTRL+Y", false, false);
            ImGui::Separator();
            ImGui::MenuItem("Cut", "CTRL+X");
            ImGui::MenuItem("Copy", "CTRL+C");
            ImGui::MenuItem("Paste", "CTRL+V");
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
        toolbarButton("sceneNew", imgSceneNew_, "New scene", false);
        toolbarButton("sceneOpen", imgSceneOpen_, "Open scene", false);
        toolbarButton("sceneSave", imgSceneSave_, "Save scene", false);

        toolbarSep();

        toolbarButton("modeObject", imgModeObject_, "Objects edit mode", true, true);
        toolbarButton("modeVisual", imgModeVisual_, "Visuals edit mode");
        toolbarButton("modeLight", imgModeLight_, "Lights edit mode", false);
        toolbarButton("modeScene", imgModeScene_, "Scene edit mode", false);

        toolbarSep();

        toolbarButton("undo", imgUndo_, "Undo");
        toolbarButton("redo", imgRedo_, "Redo");
    }

    bool Workspace::toolbarButton(const char* id, const Image& image, const char* tooltip, bool enabled, bool checked)
    {
        bool res = ImGuiUtils::imageButtonTooltip(id, image, 28.0f, tooltip, enabled, checked);
        ImGui::SameLine();
        return res;
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

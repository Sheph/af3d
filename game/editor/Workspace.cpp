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
#include "editor/CommandSetProperty.h"
#include "Const.h"
#include "InputManager.h"
#include "AClassRegistry.h"
#include "Scene.h"
#include "SceneObject.h"
#include "CameraComponent.h"
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

    void Workspace::onRegister()
    {
        auto w = std::make_shared<CommandHistoryWindow>();
        parent()->addComponent(w);

        auto w2 = std::make_shared<PropertyEditor>();
        parent()->addComponent(w2);

        em_->enter();
    }

    void Workspace::onUnregister()
    {
        em_->leave();

        emObject_.reset();
        em_ = nullptr;
    }
} }

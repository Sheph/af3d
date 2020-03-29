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

#include "editor/MainPopup.h"
#include "Const.h"
#include "Logger.h"
#include "Scene.h"
#include "imgui.h"

namespace af3d {
    ACLASS_NS_DEFINE_BEGIN(editor, MainPopup, UIComponent)
    ACLASS_NS_DEFINE_END(editor, MainPopup)

namespace editor {
    MainPopup::MainPopup()
    : UIComponent(AClass_editorMainPopup, zOrderEditor)
    {
    }

    const AClass& MainPopup::staticKlass()
    {
        return AClass_editorMainPopup;
    }

    AObjectPtr MainPopup::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<MainPopup>();
        obj->propertiesSet(propVals);
        return obj;
    }

    void MainPopup::update(float dt)
    {
        if (!ImGui::BeginPopup("MainPopup")) {
            removeFromParent();
            return;
        }

        menuAdd();

        ImGui::EndPopup();
    }

    void MainPopup::menuAdd()
    {
        if (!ImGui::BeginMenu("Add")) {
            return;
        }

        menuAddObject();

        bool objSelected = !w_->emObject()->selected().empty();

        if (ImGui::MenuItem("Mesh", nullptr, false, objSelected)) {
            w_->addMesh();
        }

        ImGui::EndMenu();
    }

    void MainPopup::menuAddObject()
    {
        if (!ImGui::BeginMenu("Object")) {
            return;
        }

        for (const auto& kind : w_->objectKinds()) {
            if (ImGui::MenuItem(kind.c_str())) {
                w_->addObject(kind);
            }
        }

        ImGui::EndMenu();
    }

    void MainPopup::onRegister()
    {
        w_ = scene()->workspace().get();
        LOG4CPLUS_DEBUG(logger(), "MainPopup open");
        ImGui::OpenPopup("MainPopup");
        update(0);
    }

    void MainPopup::onUnregister()
    {
        w_ = nullptr;
        LOG4CPLUS_DEBUG(logger(), "MainPopup closed");
    }
} }

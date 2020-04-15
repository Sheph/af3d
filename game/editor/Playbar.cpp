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

#include "editor/Playbar.h"
#include "Scene.h"
#include "Const.h"
#include "Logger.h"
#include "AssetManager.h"
#include "ImGuiUtils.h"

namespace af3d {
    ACLASS_NS_DEFINE_BEGIN(editor, Playbar, UIComponent)
    ACLASS_NS_DEFINE_END(editor, Playbar)

namespace editor {
    Playbar::Playbar()
    : UIComponent(AClass_editorPlaybar, zOrderEditor),
      iconStop_(assetManager.getImage("common1/action_stop.png"))
    {
    }

    const AClass& Playbar::staticKlass()
    {
        return AClass_editorPlaybar;
    }

    AObjectPtr Playbar::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<Playbar>();
        obj->propertiesSet(propVals);
        return obj;
    }

    void Playbar::update(float dt)
    {
        KeySequence ks(KM_CTRL, KI_R);

        if (inputManager.keyboard().triggered(ks)) {
            scene()->setQuit(true);
        }

        ImGuiIO& io = ImGui::GetIO();

        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 5.0f, 5.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
        ImGui::SetNextWindowBgAlpha(0.35f);
        if (ImGui::Begin("##playbar", nullptr,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav)) {
            if (ImGuiUtils::imageButtonTooltip("##stop", iconStop_, 20.0f, ("Stop (" + ks.str() + ")").c_str(), true, false)) {
                scene()->setQuit(true);
            }
        }
        ImGui::End();
    }

    void Playbar::onRegister()
    {
        LOG4CPLUS_DEBUG(logger(), "Playbar open");
    }

    void Playbar::onUnregister()
    {
        LOG4CPLUS_DEBUG(logger(), "Playbar closed");
    }
} }

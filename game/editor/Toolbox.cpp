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

#include "editor/Toolbox.h"
#include "editor/Workspace.h"
#include "Scene.h"
#include "Const.h"
#include "Logger.h"
#include "ImGuiUtils.h"

namespace af3d {
    ACLASS_NS_DEFINE_BEGIN(editor, Toolbox, UIComponent)
    ACLASS_NS_DEFINE_END(editor, Toolbox)

namespace editor {
    Toolbox::Toolbox()
    : UIComponent(AClass_editorToolbox, zOrderEditor)
    {
    }

    const AClass& Toolbox::staticKlass()
    {
        return AClass_editorToolbox;
    }

    AObjectPtr Toolbox::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<Toolbox>();
        obj->propertiesSet(propVals);
        return obj;
    }

    void Toolbox::update(float dt)
    {
        if (!show_) {
            removeFromParent();
            return;
        }

        ImGui::SetNextWindowPos(ImVec2(10, 70), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(240, 300), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("Toolbox", &show_)) {
            ImGui::End();
            return;
        }

        const ImGuiStyle& style = ImGui::GetStyle();

        const auto& ws = scene()->workspace();
        const auto& tools = ws->tools();
        Tool* curTool = ws->currentTool();

        float wx2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
        float iconSize = 28.0f;

        for (size_t i = 0; i < tools.size(); ++i) {
            bool enabled = tools[i]->canWork();
            bool checked = tools[i] == curTool;
            if (ImGuiUtils::imageButtonTooltip(tools[i]->name().c_str(), tools[i]->icon(), iconSize,
                tools[i]->tooltip().c_str(), enabled, checked) && enabled && !checked) {
                curTool->activate(false);
                curTool = tools[i];
                curTool->activate(true);
                ws->setCurrentTool(curTool);
            }
            float nx2 = ImGui::GetItemRectMax().x + style.ItemSpacing.x + iconSize;
            if ((i < tools.size() - 1) && (nx2 < wx2)) {
                ImGui::SameLine();
            }
        }

        ImGui::Spacing();

        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
        ImGui::BeginChild("Tool options", ImVec2(0,0), true, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar);

        if (ImGui::BeginMenuBar()) {
            ImGui::Text("%s options", curTool->name().c_str());
            ImGui::EndMenuBar();
        }

        ImGui::PushID(curTool->name().c_str());
        curTool->options();
        ImGui::PopID();

        ImGui::EndChild();
        ImGui::PopStyleVar();

        ImGui::End();
    }

    void Toolbox::onRegister()
    {
        LOG4CPLUS_DEBUG(logger(), "Toolbox open");
    }

    void Toolbox::onUnregister()
    {
        LOG4CPLUS_DEBUG(logger(), "Toolbox closed");
    }
} }

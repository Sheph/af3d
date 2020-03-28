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

#include "editor/PropertyEditor.h"
#include "Const.h"
#include "Logger.h"
#include "Scene.h"
#include "Settings.h"
#include "imgui.h"

namespace af3d {
    ACLASS_NS_DEFINE_BEGIN(editor, PropertyEditor, UIComponent)
    ACLASS_NS_DEFINE_END(editor, PropertyEditor)

namespace editor {
    PropertyEditor::PropertyEditor()
    : UIComponent(AClass_editorPropertyEditor, zOrderEditor)
    {
    }

    const AClass& PropertyEditor::staticKlass()
    {
        return AClass_editorPropertyEditor;
    }

    AObjectPtr PropertyEditor::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<PropertyEditor>();
        obj->propertiesSet(propVals);
        return obj;
    }

    void PropertyEditor::update(float dt)
    {
        if (!show_) {
            removeFromParent();
            return;
        }

        ImGui::SetNextWindowPos(ImVec2(settings.viewWidth - 250, 100), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(240, 400), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("Properties", &show_)) {
            ImGui::End();
            return;
        }

        AObjectPtr obj;
        ACookie newCookie = 0;
        const auto& sList = scene()->workspace()->em()->selected();
        if (!sList.empty()) {
            obj = sList.back();
            newCookie = obj->cookie();
        }

        if (newCookie != cookie_) {
            cookie_ = newCookie;
            properties_.clear();
            if (obj) {
                properties_ = obj->klass().getProperties();
            }
        }

        ImGuiStyle& style = ImGui::GetStyle();

        if (obj) {
            ImGui::Text("1 %s selected", scene()->workspace()->em()->name().c_str());
        } else {
            ImGui::Text("No selection");
        }
        ImGui::Columns(2);
        ImGui::Separator();
        ImGui::TextColored(style.Colors[ImGuiCol_HeaderActive], "Property"); ImGui::NextColumn();
        ImGui::TextColored(style.Colors[ImGuiCol_HeaderActive], "Value"); ImGui::NextColumn();

        if (obj) {
            ImGui::PushID(std::to_string(newCookie).c_str());

            for (const auto& prop : properties_) {
                ImGui::PushID(prop.name().c_str());

                ImGui::Separator();
                ImGui::Text("%s", prop.name().c_str());
                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::Text("%s", prop.tooltip().c_str());
                    ImGui::EndTooltip();
                }

                ImGui::NextColumn();
                ImGui::PushItemWidth(-1);

                auto val = obj->propertyGet(prop.name());
                if (ImGuiUtils::APropertyEdit(prop.type(), val, (prop.flags() & APropertyWritable) == 0)) {
                    obj->propertySet(prop.name(), val);
                }

                ImGui::PopItemWidth();
                ImGui::NextColumn();

                ImGui::PopID();
            }

            ImGui::PopID();
        }

        ImGui::Columns(1);
        ImGui::Separator();

        ImGui::End();
    }

    void PropertyEditor::onRegister()
    {
        LOG4CPLUS_DEBUG(logger(), "PropertyEditor open");
    }

    void PropertyEditor::onUnregister()
    {
        LOG4CPLUS_DEBUG(logger(), "PropertyEditor closed");
    }
} }

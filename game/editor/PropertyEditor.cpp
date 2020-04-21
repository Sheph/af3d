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
#include "SceneObject.h"
#include "PhysicsBodyComponent.h"
#include "ImGuiManager.h"
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
            imGuiManager.cfgSetBool(ImGuiManager::strPropertyEditorOpened, false);
            removeFromParent();
            return;
        }

        ImGui::SetNextWindowPos(ImVec2(settings.viewWidth - 250, 100), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(240, 400), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("Properties", &show_)) {
            ImGui::End();
            return;
        }

        auto em = scene()->workspace()->overriddenEm();

        AWeakObject newWobj;
        const auto& sList = em->selected();
        if (!sList.empty()) {
            newWobj = sList.back();
        }

        AObjectPtr obj = newWobj.lock();

        if (newWobj != wobj_) {
            wobj_ = newWobj;
            properties_.clear();
            if (obj) {
                auto props = obj->klass().getProperties();
                for (const auto& prop : props) {
                    properties_.emplace_back(prop);
                }
            }
        }

        ImGuiStyle& style = ImGui::GetStyle();

        if (obj) {
            ImGui::Text("1 %s (%s) selected", em->name().c_str(), obj->klass().name().c_str());
        } else {
            ImGui::Text("No selection");
        }
        ImGui::Columns(2);
        if (!columnWidthSet_) {
            columnWidthSet_ = true;
            ImGui::SetColumnWidth(0, imGuiManager.cfgGetFloat(ImGuiManager::strPropertyEditorColumnWidth, 150.0f));
        } else {
            imGuiManager.cfgSetFloat(ImGuiManager::strPropertyEditorColumnWidth, ImGui::GetColumnWidth(0));
        }
        ImGui::Separator();
        ImGui::TextColored(style.Colors[ImGuiCol_HeaderActive], "Property"); ImGui::NextColumn();
        ImGui::TextColored(style.Colors[ImGuiCol_HeaderActive], "Value"); ImGui::NextColumn();

        bool wasSet = false;

        if (obj) {
            auto sceneObj = aobjectCast<SceneObject>(obj);

            ImGui::PushID(std::to_string(wobj_.cookie()).c_str());

            for (auto& pi : properties_) {
                if ((pi.prop.flags() & APropertyEditable) == 0) {
                    continue;
                }

                if (sceneObj && (pi.prop.name() == AProperty_PhysicsActive)) {
                    ImGui::PushID("_body");
                    ImGui::Separator();
                    ImGui::Text("physics body");
                    if (ImGui::IsItemHovered()) {
                        ImGui::BeginTooltip();
                        ImGui::Text("Have physics body");
                        ImGui::EndTooltip();
                    }
                    ImGui::NextColumn();

                    auto& actionAddBody = scene()->workspace()->actionOpMenuAddPhysicsBody();
                    auto& actionRemoveBody = scene()->workspace()->actionOpMenuRemovePhysicsBody();

                    if (actionRemoveBody.state().enabled) {
                        if (ImGui::Button("Remove")) {
                            wasSet = true;
                            actionRemoveBody.trigger();
                        }
                    } else if (actionAddBody.state().enabled) {
                        if (ImGui::Button("Create")) {
                            wasSet = true;
                            actionAddBody.trigger();
                        }
                    } else if (sceneObj->findComponent<PhysicsBodyComponent>()) {
                        ImGui::Text("Yes");
                    } else {
                        ImGui::Text("No");
                    }
                    ImGui::NextColumn();
                    ImGui::PopID();
                }

                ImGui::PushID(pi.prop.name().c_str());

                bool isParam = (pi.prop.category() == APropertyCategory::Params);

                ImGui::Separator();
                if (isParam) {
                    ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.6f, style.Colors[ImGuiCol_Text].w), "%s", pi.prop.name().c_str());
                } else {
                    ImGui::Text("%s", pi.prop.name().c_str());
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::Text("%s (%s)", pi.prop.tooltip().c_str(), pi.prop.type().name());
                    ImGui::EndTooltip();
                }

                ImGui::NextColumn();

                auto val = obj->propertyGet(pi.prop.name());
                if (val != pi.initialVal) {
                    pi.initialVal = pi.val = val;
                }
                if (ImGuiUtils::APropertyEdit(scene(), pi.prop.type(), pi.val, !isParam && (pi.prop.flags() & APropertyWritable) == 0) &&
                    !wasSet && (pi.val != pi.initialVal)) {
                    scene()->workspace()->setProperty(obj, pi.prop.name(), pi.val, isParam);
                    pi.initialVal = pi.val;
                    wasSet = true;
                }

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
        imGuiManager.cfgSetBool(ImGuiManager::strPropertyEditorOpened, true);
    }

    void PropertyEditor::onUnregister()
    {
        LOG4CPLUS_DEBUG(logger(), "PropertyEditor closed");
        properties_.clear();
    }
} }

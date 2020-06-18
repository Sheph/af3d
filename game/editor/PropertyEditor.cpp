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
#include "PhasedComponent.h"
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
            newWobj = sList.back().obj();
        }

        AObjectPtr obj = newWobj.lock();
        auto sceneObj = aobjectCast<SceneObject>(obj);

        if (objs_.empty() || (newWobj != objs_[0].wobj) ||
            (sceneObj && (sceneObj->components().size() != prevNumComponents_))) {
            objs_.clear();
            prevNumComponents_ = 0;
            if (obj) {
                addObj(obj);
                if (sceneObj) {
                    prevNumComponents_ = sceneObj->components().size();
                    // Also add editable phased components.
                    auto cs = sceneObj->findComponents<PhasedComponent>();
                    for (const auto& c : cs) {
                        if ((c->aflags() & AObjectEditable) != 0) {
                            addObj(c);
                        }
                    }
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

        for (auto it = objs_.begin(); it != objs_.end(); ++it) {
            display(*it, (it == objs_.begin()));
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
        objs_.clear();
        prevNumComponents_ = 0;
    }

    void PropertyEditor::addObj(const AObjectPtr& obj)
    {
        objs_.emplace_back(AWeakObject(obj));
        auto props = obj->klass().getProperties();
        for (const auto& prop : props) {
            objs_.back().properties.emplace_back(prop);
        }
    }

    void PropertyEditor::display(ObjInfo& objInfo, bool main)
    {
        ImGuiStyle& style = ImGui::GetStyle();

        KeySequence ksCopy(KM_CTRL, KI_C);
        KeySequence ksPaste(KM_CTRL, KI_V);

        bool wasSet = false;

        AObjectPtr obj = objInfo.wobj.lock();

        auto sceneObj = aobjectCast<SceneObject>(obj);

        if (!main) {
            ImGui::Columns(1);
            bool open = true;
            bool res = ImGui::CollapsingHeader((obj->klass().name() + "##" + std::to_string(objInfo.wobj.cookie())).c_str(),
                &open, ImGuiTreeNodeFlags_DefaultOpen);
            ImGui::Columns(2);
            if (!open) {
                scene()->workspace()->deleteObject(obj);
            }
            if (!res || !open) {
                return;
            }
        }

        ImGui::PushID(std::to_string(objInfo.wobj.cookie()).c_str());

        for (auto& pi : objInfo.properties) {
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
            auto val = obj->propertyGet(pi.prop.name());
            bool readOnly = !isParam && ((pi.prop.flags() & APropertyWritable) == 0);

            ImGui::Separator();
            if (isParam) {
                ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.6f, style.Colors[ImGuiCol_Text].w), "%s", pi.prop.name().c_str());
            } else if ((pi.prop.flags() & APropertyTransient) != 0) {
                ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, style.Colors[ImGuiCol_Text].w), "%s", pi.prop.name().c_str());
            } else {
                ImGui::Text("%s", pi.prop.name().c_str());
            }
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("%s (%s)", pi.prop.tooltip().c_str(), pi.prop.type().name());
                ImGui::EndTooltip();

                if (inputManager.keyboard().triggered(ksCopy)) {
                    clipboard_ = val;
                } else if (!wasSet && !readOnly && clipboard_.convertibleTo(val.type()) &&
                    inputManager.keyboard().triggered(ksPaste)) {
                    val = clipboard_.convertTo(val.type());
                    scene()->workspace()->setProperty(obj, pi.prop.name(), val);
                    wasSet = true;
                }
            }
            if (ImGui::BeginPopupContextItem("##m")) {
                if (ImGui::MenuItem("Copy", ksCopy.str().c_str(), false, true)) {
                    clipboard_ = val;
                }
                if (ImGui::MenuItem("Paste", ksPaste.str().c_str(), false, !readOnly && clipboard_.convertibleTo(val.type())) && !wasSet) {
                    val = clipboard_.convertTo(val.type());
                    scene()->workspace()->setProperty(obj, pi.prop.name(), val);
                    wasSet = true;
                }
                ImGui::EndPopup();
            }

            ImGui::NextColumn();

            if (val != pi.initialVal) {
                pi.initialVal = pi.val = val;
            }
            if (ImGuiUtils::APropertyEdit(scene(), pi.prop.type(), pi.initialVal, pi.val, readOnly) &&
                !wasSet && (pi.val != pi.initialVal)) {
                scene()->workspace()->setProperty(obj, pi.prop.name(), pi.val);
                pi.initialVal = pi.val;
                wasSet = true;
            }

            ImGui::NextColumn();

            ImGui::PopID();
        }

        ImGui::PopID();
    }
} }

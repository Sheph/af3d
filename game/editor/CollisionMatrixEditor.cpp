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

#include "editor/CollisionMatrixEditor.h"
#include "Scene.h"
#include "Const.h"
#include "Logger.h"
#include "ImGuiUtils.h"
#include "ImGuiManager.h"

namespace af3d {
    ACLASS_NS_DEFINE_BEGIN(editor, CollisionMatrixEditor, UIComponent)
    ACLASS_NS_DEFINE_END(editor, CollisionMatrixEditor)

namespace editor {
    CollisionMatrixEditor::CollisionMatrixEditor()
    : UIComponent(AClass_editorCollisionMatrixEditor, zOrderEditor)
    {
    }

    const AClass& CollisionMatrixEditor::staticKlass()
    {
        return AClass_editorCollisionMatrixEditor;
    }

    AObjectPtr CollisionMatrixEditor::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<CollisionMatrixEditor>();
        obj->propertiesSet(propVals);
        return obj;
    }

    void CollisionMatrixEditor::update(float dt)
    {
        if (!show_) {
            imGuiManager.cfgSetBool(ImGuiManager::strCollisionMatrixEditorOpened, false);
            removeFromParent();
            return;
        }

        ImGui::SetNextWindowPos(ImVec2(470, 150), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(730, 530), ImGuiCond_FirstUseEver);

        CollisionMatrixPtr cm = scene()->collisionMatrix();

        if (!ImGui::Begin(("Collision matrix editor (" + cm->name() + ")###Collision matrix editor").c_str(), &show_)) {
            ImGui::End();
            return;
        }

        ImDrawList* dl = ImGui::GetWindowDrawList();

        bool changed = false;

        ImGui::Columns(static_cast<int>(Layer::Max) + 2);
        ImGui::SetColumnWidth(0, 160.0f);
        for (int i = 0; i <= static_cast<int>(Layer::Max); ++i) {
            ImGui::SetColumnWidth(1 + i, 40.0f);
        }

        for (int j = 0; j < 8; ++j) {
            ImGui::Text(" ");
        }
        ImGui::SameLine(130.0f);
        bool val = true;
        for (int i = 0; i <= static_cast<int>(Layer::Max); ++i) {
            auto layerI = static_cast<Layer>(i);
            for (int j = 0; j <= static_cast<int>(Layer::Max); ++j) {
                auto layerJ = static_cast<Layer>(j);
                if (!cm->row(layerI)[layerJ]) {
                    val = false;
                    break;
                }
            }
        }
        if (ImGui::Checkbox("##all", &val)) {
            changed = true;
            for (int i = 0; i <= static_cast<int>(Layer::Max); ++i) {
                auto layerI = static_cast<Layer>(i);
                for (int j = 0; j <= static_cast<int>(Layer::Max); ++j) {
                    auto layerJ = static_cast<Layer>(j);
                    if (val) {
                        cm->row(layerI).set(layerJ);
                    } else {
                        cm->row(layerI).reset(layerJ);
                    }
                }
            }
        }
        ImGui::NextColumn();
        for (int i = 0; i <= static_cast<int>(Layer::Max); ++i) {
            ImGui::PushID(i);
            auto layerI = static_cast<Layer>(i);
            for (int j = 0; j < 7; ++j) {
                ImGui::Text(" ");
            }
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImGuiUtils::addTextVertical(dl, (" " + APropertyType_Layer.enumerators()[i]).c_str(), pos, ImGui::GetColorU32(ImGuiCol_Text));
            bool val = true;
            for (int j = 0; j <= static_cast<int>(Layer::Max); ++j) {
                auto layerJ = static_cast<Layer>(j);
                if (!cm->row(layerJ)[layerI]) {
                    val = false;
                    break;
                }
            }
            if (ImGui::Checkbox("##col", &val)) {
                changed = true;
                for (int j = 0; j <= static_cast<int>(Layer::Max); ++j) {
                    auto layerJ = static_cast<Layer>(j);
                    if (val) {
                        cm->row(layerJ).set(layerI);
                    } else {
                        cm->row(layerJ).reset(layerI);
                    }
                }
            }
            ImGui::NextColumn();
            ImGui::PopID();
        }

        for (int i = 0; i <= static_cast<int>(Layer::Max); ++i) {
            ImGui::PushID(i);
            auto layerI = static_cast<Layer>(i);
            ImGui::Text("%s", APropertyType_Layer.enumerators()[i].c_str());
            ImGui::SameLine(130.0f);

            bool val = true;
            for (int j = 0; j <= static_cast<int>(Layer::Max); ++j) {
                auto layerJ = static_cast<Layer>(j);
                if (!cm->row(layerI)[layerJ]) {
                    val = false;
                    break;
                }
            }
            if (ImGui::Checkbox("##row", &val)) {
                changed = true;
                for (int j = 0; j <= static_cast<int>(Layer::Max); ++j) {
                    auto layerJ = static_cast<Layer>(j);
                    if (val) {
                        cm->row(layerI).set(layerJ);
                    } else {
                        cm->row(layerI).reset(layerJ);
                    }
                }
            }

            ImGui::NextColumn();
            for (int j = 0; j <= static_cast<int>(Layer::Max); ++j) {
                ImGui::PushID(j);
                auto layerJ = static_cast<Layer>(j);
                bool val = cm->row(layerI)[layerJ];
                if (ImGui::Checkbox("##val", &val)) {
                    changed = true;
                    if (val) {
                        cm->row(layerI).set(layerJ);
                    } else {
                        cm->row(layerI).reset(layerJ);
                    }
                }
                ImGui::NextColumn();
                ImGui::PopID();
            }
            ImGui::PopID();
        }

        ImGui::Columns(1);

        if (changed) {
            cm->save();
        }

        ImGui::End();
    }

    void CollisionMatrixEditor::onRegister()
    {
        LOG4CPLUS_DEBUG(logger(), "Collision matrix editor open");
        imGuiManager.cfgSetBool(ImGuiManager::strCollisionMatrixEditorOpened, true);
    }

    void CollisionMatrixEditor::onUnregister()
    {
        LOG4CPLUS_DEBUG(logger(), "Collision matrix editor closed");
    }
} }

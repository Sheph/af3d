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

#include "editor/EditMode.h"
#include "editor/Workspace.h"
#include "Scene.h"
#include "SceneObject.h"
#include "CameraComponent.h"
#include "InputManager.h"
#include "imgui.h"

namespace af3d { namespace editor
{
    EditMode::EditMode(Workspace* workspace)
    : workspace_(workspace)
    {
    }

    void EditMode::enter()
    {
        active_ = true;
    }

    void EditMode::update(float dt)
    {
        hovered_.clear();

        ImGuiIO& io = ImGui::GetIO();

        if (io.WantCaptureMouse) {
            return;
        }

        auto cc = scene()->camera()->findComponent<CameraComponent>();

        auto res = doHover(cc->getFrustum(), cc->screenPointToRay(inputManager.mouse().pos()));

        if (res) {
            hovered_.push_back(res);
            if (inputManager.mouse().triggered(true)) {
                selected_.clear();
                selected_.push_back(res);
            }
        }
    }

    void EditMode::leave()
    {
        hovered_.clear();
        active_ = false;
    }

    const EditMode::AList& EditMode::hovered()
    {
        for (auto it = hovered_.begin(); it != hovered_.end();) {
            if (doCheck(*it)) {
                ++it;
            } else {
                hovered_.erase(it++);
            }
        }
        return hovered_;
    }

    const EditMode::AList& EditMode::selected()
    {
        for (auto it = selected_.begin(); it != selected_.end();) {
            if (doCheck(*it)) {
                ++it;
            } else {
                selected_.erase(it++);
            }
        }
        return selected_;
    }

    bool EditMode::isHovered(const AObjectPtr& obj)
    {
        for (const auto& h : hovered_) {
             if (h == obj) {
                 return doCheck(h);
             }
        }
        return false;
    }

    bool EditMode::isSelected(const AObjectPtr& obj)
    {
        for (const auto& s : selected_) {
             if (s == obj) {
                 return doCheck(s);
             }
        }
        return false;
    }

    Workspace& EditMode::workspace()
    {
        return *workspace_;
    }

    Scene* EditMode::scene()
    {
        return workspace_->scene();
    }
} }

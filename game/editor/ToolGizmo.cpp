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

#include "editor/ToolGizmo.h"
#include "editor/Workspace.h"
#include "InputManager.h"
#include "Scene.h"
#include "SceneObject.h"
#include "CameraComponent.h"
#include "imgui.h"

namespace af3d { namespace editor
{
    ToolGizmo::ToolGizmo(Workspace* workspace, const std::string& name,
        const Image& icon, KeyIdentifier shortcut)
    : Tool(workspace, name, icon, shortcut)
    {
    }

    bool ToolGizmo::canWork() const
    {
        return canWork_;
    }

    void ToolGizmo::onActivate()
    {
    }

    void ToolGizmo::onDeactivate()
    {
        cleanup();
    }

    void ToolGizmo::doUpdate(float dt)
    {
        const auto& sel = workspace().em()->selected();
        if (sel.empty()) {
            cleanup();
            return;
        }

        auto obj = sel.back().lock();
        if (target_ != obj) {
            cleanup();
        }

        if (!target_) {
            if (!obj) {
                return;
            }

            if (!gizmoCreate(obj)) {
                canWork_ = false;
                return;
            }

            canWork_ = true;
            target_ = obj;
        }

        if (captured()) {
            bool canceled = inputManager.keyboard().triggered(KI_ESCAPE);
            if (!inputManager.mouse().pressed(true) || canceled) {
                gizmoRelease(canceled || (capturedMousePos_ == inputManager.mouse().pos()));
                capturedRay_ = Ray_empty;
                workspace().unlock();
            } else {
                auto cc = scene()->camera()->findComponent<CameraComponent>();
                auto ray = cc->screenPointToRay(inputManager.mouse().pos());
                gizmoMove(cc->getFrustum(), ray);
            }
        } else if (inputManager.mouse().triggered(true)) {
            if (workspace().lock()) {
                auto cc = scene()->camera()->findComponent<CameraComponent>();
                auto ray = cc->screenPointToRay(inputManager.mouse().pos());
                if (gizmoCapture(cc->getFrustum(), ray)) {
                    capturedRay_ = ray;
                    capturedMousePos_ = inputManager.mouse().pos();
                } else {
                    workspace().unlock();
                }
            }
        } else if (!inputManager.mouse().pressed(true)) {
            auto cc = scene()->camera()->findComponent<CameraComponent>();
            auto ray = cc->screenPointToRay(inputManager.mouse().pos());
            gizmoMove(cc->getFrustum(), ray);
        }
    }

    void ToolGizmo::doOptions()
    {
        int cur = static_cast<int>(orientation_);

        ImGui::SetNextItemWidth(80);
        if (ImGui::Combo("Orientation", &cur, "Local\0Global\0")) {
            orientation_ = static_cast<TransformOrientation>(cur);
        }
    }

    void ToolGizmo::cleanup()
    {
        canWork_ = true;

        if (!target_) {
            return;
        }

        if (captured()) {
            gizmoRelease(true);
            workspace().unlock();
            capturedRay_ = Ray_empty;
        }

        gizmoDestroy();

        target_.reset();
    }
} }

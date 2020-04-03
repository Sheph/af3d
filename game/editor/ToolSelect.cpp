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

#include "editor/ToolSelect.h"
#include "editor/Workspace.h"
#include "editor/EditMode.h"
#include "AssetManager.h"
#include "InputManager.h"
#include "Scene.h"
#include "SceneObject.h"
#include "CameraComponent.h"
#include "imgui.h"

namespace af3d { namespace editor
{
    ToolSelect::ToolSelect(Workspace* workspace)
    : Tool(workspace, "Select", assetManager.getImage("common1/tool_select.png"))
    {
    }

    void ToolSelect::onActivate()
    {
    }

    void ToolSelect::onDeactivate()
    {
        auto em = workspace().em();

        em->setHovered(EditMode::AWeakList());
    }

    void ToolSelect::doUpdate(float dt)
    {
        auto em = workspace().em();

        ImGuiIO& io = ImGui::GetIO();

        em->setHovered(EditMode::AWeakList());

        if (io.WantCaptureMouse || io.WantCaptureKeyboard) {
            return;
        }

        auto cc = scene()->camera()->findComponent<CameraComponent>();

        auto res = em->rayCast(cc->getFrustum(), cc->screenPointToRay(inputManager.mouse().pos()));

        if (res) {
            em->setHovered(EditMode::AWeakList{AWeakObject(res)});
            if (inputManager.mouse().triggered(true)) {
                em->select({res});
            }
        }
    }

    void ToolSelect::doOptions()
    {
    }
} }

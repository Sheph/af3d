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

#include "editor/ToolMove.h"
#include "editor/Workspace.h"
#include "AssetManager.h"
#include "InputManager.h"
#include "Scene.h"
#include "SceneObject.h"
#include "CameraComponent.h"

namespace af3d { namespace editor
{
    ToolMove::ToolMove(Workspace* workspace)
    : Tool(workspace, "Move", assetManager.getImage("common1/tool_move.png")),
      selTool_(workspace)
    {
    }

    void ToolMove::onActivate()
    {
        selTool_.activate(true);
    }

    void ToolMove::onDeactivate()
    {
        cleanup();
        selTool_.activate(false);
    }

    void ToolMove::doUpdate(float dt)
    {
        const auto& sel = workspace().em()->selected();
        if (sel.empty()) {
            cleanup();
            selTool_.update(dt);
            return;
        }

        auto obj = sel.back().lock();
        if (rc_ && (rc_->target() != obj)) {
            cleanup();
        }

        if (!rc_) {
            if (!obj || !obj->propertyCanGet(AProperty_WorldTransform)) {
                selTool_.update(dt);
                return;
            }

            rc_ = std::make_shared<RenderGizmoMoveComponent>();
            rc_->setTarget(obj);
            rc_->setAlphaInactive(0.4f);
            rc_->setAlphaActive(0.7f);
            workspace().parent()->addComponent(rc_);
        }

        if (mt_ != MoveType::None) {
            if (!inputManager.mouse().pressed(true)) {
                mt_ = MoveType::None;
                selTool_.activate(true);
                rc_->setAlphaActive(0.7f);
                auto xf = rc_->target()->propertyGet(AProperty_WorldTransform);
                rc_->target()->propertySet(AProperty_WorldTransform, targetXf_);
                workspace().setProperty(rc_->target(), AProperty_WorldTransform, xf, false);
            } else {
                auto diff = inputManager.mouse().pos() - mousePos_;
                if (mt_ == MoveType::AxisX) {
                    auto xf = targetXf_;
                    xf.getOrigin() += targetXf_.getBasis() * btVector3_right * diff.x() * 0.1f;
                    rc_->target()->propertySet(AProperty_WorldTransform, xf);
                }
            }
            selTool_.update(dt);
            return;
        }

        auto cc = scene()->camera()->findComponent<CameraComponent>();

        auto res = rc_->testRay(cc->getFrustum(), cc->screenPointToRay(inputManager.mouse().pos()));

        rc_->setMoveType(res);

        if ((res != MoveType::None) && inputManager.mouse().pressed(true)) {
            mt_ = res;
            rc_->setAlphaActive(1.0f);
            selTool_.activate(false);
            mousePos_ = inputManager.mouse().pos();
            targetXf_ = rc_->target()->propertyGet(AProperty_WorldTransform).toTransform();
        }

        selTool_.update(dt);
    }

    void ToolMove::doOptions()
    {
    }

    void ToolMove::cleanup()
    {
        if (rc_) {
            if (mt_ != MoveType::None) {
                rc_->target()->propertySet(AProperty_WorldTransform, targetXf_);
            }
            rc_->removeFromParent();
            rc_.reset();
            mt_ = MoveType::None;
            selTool_.activate(true);
        }
    }
} }

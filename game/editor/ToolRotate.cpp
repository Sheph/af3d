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

#include "editor/ToolRotate.h"
#include "editor/Workspace.h"
#include "AssetManager.h"
#include "SceneObject.h"

namespace af3d { namespace editor
{
    ToolRotate::ToolRotate(Workspace* workspace)
    : ToolGizmo(workspace, "Rotate", assetManager.getImage("common1/tool_rotate.png")),
      selTool_(workspace)
    {
    }

    void ToolRotate::onActivate()
    {
        ToolGizmo::onActivate();
        selTool_.activate(true);
    }

    void ToolRotate::onDeactivate()
    {
        ToolGizmo::onDeactivate();
        selTool_.activate(false);
    }

    void ToolRotate::doUpdate(float dt)
    {
        ToolGizmo::doUpdate(dt);
        selTool_.update(dt);
    }

    void ToolRotate::doOptions()
    {
    }

    bool ToolRotate::gizmoCreate(const AObjectPtr& obj)
    {
        if (!obj->propertyCanGet(AProperty_WorldTransform)) {
            return false;
        }

        rc_ = std::make_shared<RenderGizmoRotateComponent>();
        rc_->setTarget(obj);
        rc_->setAlphaInactive(0.4f);
        rc_->setAlphaActive(0.7f);
        workspace().parent()->addComponent(rc_);

        return true;
    }

    void ToolRotate::gizmoDestroy()
    {
        rc_->removeFromParent();
        rc_.reset();
    }

    bool ToolRotate::gizmoCapture(const Frustum& frustum, const Ray& ray)
    {
        auto res = rc_->testRay(frustum, ray);
        if (res != RotateType::None) {
            rc_->setRotateType(res);
            rc_->setAlphaActive(1.0f);
            capturedTargetXf_ = rc_->target()->propertyGet(AProperty_WorldTransform).toTransform();
            selTool_.activate(false);
            return true;
        } else {
            return false;
        }
    }

    void ToolRotate::gizmoRelease(bool canceled)
    {
        auto xf = rc_->target()->propertyGet(AProperty_WorldTransform);
        rc_->target()->propertySet(AProperty_WorldTransform, capturedTargetXf_);
        if (!canceled) {
            workspace().setProperty(rc_->target(), AProperty_WorldTransform, xf, false);
        }
        rc_->setRotateType(RotateType::None);
        rc_->setAlphaActive(0.7f);
        selTool_.activate(true);
    }

    void ToolRotate::gizmoMove(const Frustum& frustum, const Ray& ray)
    {
        if (!captured()) {
            rc_->setRotateType(rc_->testRay(frustum, ray));
            return;
        }
    }
} }

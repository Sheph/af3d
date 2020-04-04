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
#include "SceneObject.h"

namespace af3d { namespace editor
{
    ToolMove::ToolMove(Workspace* workspace)
    : ToolGizmo(workspace, "Move", assetManager.getImage("common1/tool_move.png")),
      selTool_(workspace)
    {
    }

    void ToolMove::onActivate()
    {
        ToolGizmo::onActivate();
        selTool_.activate(true);
    }

    void ToolMove::onDeactivate()
    {
        ToolGizmo::onDeactivate();
        selTool_.activate(false);
    }

    void ToolMove::doUpdate(float dt)
    {
        ToolGizmo::doUpdate(dt);
        selTool_.update(dt);
    }

    void ToolMove::doOptions()
    {
    }

    bool ToolMove::gizmoCreate(const AObjectPtr& obj)
    {
        if (!obj->propertyCanGet(AProperty_WorldTransform)) {
            return false;
        }

        rc_ = std::make_shared<RenderGizmoMoveComponent>();
        rc_->setTarget(obj);
        rc_->setAlphaInactive(0.4f);
        rc_->setAlphaActive(0.7f);
        workspace().parent()->addComponent(rc_);

        return true;
    }

    void ToolMove::gizmoDestroy()
    {
        rc_->removeFromParent();
        rc_.reset();
    }

    bool ToolMove::gizmoCapture(const Frustum& frustum, const Ray& ray)
    {
        auto res = rc_->testRay(frustum, ray);
        if (res != MoveType::None) {
            rc_->setMoveType(res);
            rc_->setAlphaActive(1.0f);
            capturedTargetXf_ = rc_->target()->propertyGet(AProperty_WorldTransform).toTransform();
            selTool_.activate(false);
            return true;
        } else {
            return false;
        }
    }

    void ToolMove::gizmoRelease(bool canceled)
    {
        auto xf = rc_->target()->propertyGet(AProperty_WorldTransform);
        rc_->target()->propertySet(AProperty_WorldTransform, capturedTargetXf_);
        if (!canceled) {
            workspace().setProperty(rc_->target(), AProperty_WorldTransform, xf, false);
        }
        rc_->setMoveType(MoveType::None);
        rc_->setAlphaActive(0.7f);
        selTool_.activate(true);
    }

    void ToolMove::gizmoMove(const Frustum& frustum, const Ray& ray)
    {
        if (!captured()) {
            rc_->setMoveType(rc_->testRay(frustum, ray));
            return;
        }

        btPlane plane = getMovePlane(frustum);

        auto r = capturedRay().testPlane(plane);
        if (r.first) {
            auto p1 = capturedRay().getAt(r.second);
            r = ray.testPlane(plane);
            if (r.first) {
                auto p2 = ray.getAt(r.second);
                auto xf = capturedTargetXf_;
                switch (rc_->moveType()) {
                case MoveType::AxisX: {
                    auto v = capturedTargetXf_.getBasis() * btVector3_right;
                    xf.getOrigin() += v * (p2 - p1).dot(v);
                    break;
                }
                case MoveType::AxisY: {
                    auto v = capturedTargetXf_.getBasis() * btVector3_up;
                    xf.getOrigin() += v * (p2 - p1).dot(v);
                    break;
                }
                case MoveType::AxisZ: {
                    auto v = capturedTargetXf_.getBasis() * btVector3_forward;
                    xf.getOrigin() += v * (p2 - p1).dot(v);
                    break;
                }
                default:
                    xf.getOrigin() += (p2 - p1);
                    break;
                }
                rc_->target()->propertySet(AProperty_WorldTransform, xf);
            }
        }
    }

    btPlane ToolMove::getMovePlane(const Frustum& frustum)
    {
        auto vRight = capturedTargetXf_.getBasis() * btVector3_right;
        auto vUp = capturedTargetXf_.getBasis() * btVector3_up;
        auto vForward = capturedTargetXf_.getBasis() * btVector3_forward;

        auto planeX = btPlaneMake(capturedTargetXf_.getOrigin(), vRight);
        auto planeY = btPlaneMake(capturedTargetXf_.getOrigin(), vUp);
        auto planeZ = btPlaneMake(capturedTargetXf_.getOrigin(), vForward);

        if (rc_->moveType() == MoveType::PlaneX) {
            return planeX;
        } else if (rc_->moveType() == MoveType::PlaneY) {
            return planeY;
        } else if (rc_->moveType() == MoveType::PlaneZ) {
            return planeZ;
        } else if (rc_->moveType() == MoveType::PlaneCurrent) {
            return btPlaneMake(capturedTargetXf_.getOrigin(),
                frustum.plane(Frustum::Plane::Far).normal);
        }

        auto px = btFabs(capturedRay().dir.dot(planeX.normal));
        auto py = btFabs(capturedRay().dir.dot(planeY.normal));
        auto pz = btFabs(capturedRay().dir.dot(planeZ.normal));

        if (rc_->moveType() == MoveType::AxisX) {
            return (py > pz) ? planeY : planeZ;
        } else if (rc_->moveType() == MoveType::AxisY) {
            return (px > pz) ? planeX : planeZ;
        } else {
            btAssert(rc_->moveType() == MoveType::AxisZ);
            return (px > py) ? planeX : planeY;
        }
    }
} }

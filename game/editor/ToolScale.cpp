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

#include "editor/ToolScale.h"
#include "editor/Workspace.h"
#include "AssetManager.h"
#include "SceneObject.h"

namespace af3d { namespace editor
{
    ToolScale::ToolScale(Workspace* workspace)
    : ToolGizmo(workspace, "Scale", assetManager.getImage("common1/tool_scale.png")),
      selTool_(workspace)
    {
    }

    void ToolScale::onActivate()
    {
        ToolGizmo::onActivate();
        selTool_.activate(true);
    }

    void ToolScale::onDeactivate()
    {
        ToolGizmo::onDeactivate();
        selTool_.activate(false);
    }

    void ToolScale::doUpdate(float dt)
    {
        ToolGizmo::doUpdate(dt);
        selTool_.update(dt);
    }

    void ToolScale::doOptions()
    {
    }

    bool ToolScale::gizmoCreate(const AObjectPtr& obj)
    {
        if (!obj->propertyCanGet(AProperty_Scale) ||
            !obj->propertyCanGet(AProperty_WorldTransform)) {
            return false;
        }

        rc_ = std::make_shared<RenderGizmoAxesComponent>();
        rc_->setKind(RenderGizmoAxesComponent::KindScale);
        rc_->setTarget(obj);
        rc_->setAlphaInactive(0.4f);
        rc_->setAlphaActive(0.7f);
        workspace().parent()->addComponent(rc_);

        return true;
    }

    void ToolScale::gizmoDestroy()
    {
        rc_->removeFromParent();
        rc_.reset();
    }

    bool ToolScale::gizmoCapture(const Frustum& frustum, const Ray& ray)
    {
        auto res = rc_->testRay(frustum, ray);
        if (res != MoveType::None) {
            rc_->setMoveType(res);
            rc_->setAlphaActive(1.0f);
            capturedTargetXf_ = rc_->target()->propertyGet(AProperty_WorldTransform).toTransform();
            capturedTargetScale_ = rc_->target()->propertyGet(AProperty_Scale).toVec3();
            selTool_.activate(false);
            return true;
        } else {
            return false;
        }
    }

    void ToolScale::gizmoRelease(bool canceled)
    {
        auto scale = rc_->target()->propertyGet(AProperty_Scale);
        rc_->target()->propertySet(AProperty_Scale, capturedTargetScale_);
        if (!canceled) {
            workspace().setProperty(rc_->target(), AProperty_Scale, scale, false);
        }
        rc_->setMoveType(MoveType::None);
        rc_->setAlphaActive(0.7f);
        selTool_.activate(true);
    }

    void ToolScale::gizmoMove(const Frustum& frustum, const Ray& ray)
    {
        if (!captured()) {
            rc_->setMoveType(rc_->testRay(frustum, ray));
            return;
        }

        btPlane plane = getScalePlane(frustum);

        auto r = capturedRay().testPlane(plane);
        if (r.first) {
            auto p1 = capturedRay().getAt(r.second);
            r = ray.testPlane(plane);
            if (r.first) {
                auto p2 = ray.getAt(r.second);
                auto scale = capturedTargetScale_;
                switch (rc_->moveType()) {
                case MoveType::AxisX: {
                    auto v = capturedTargetXf_.getBasis() * btVector3_right;
                    scale.setX((p2 - capturedTargetXf_.getOrigin()).dot(v) * scale.x() / (p1 - capturedTargetXf_.getOrigin()).dot(v));
                    break;
                }
                case MoveType::AxisY: {
                    auto v = capturedTargetXf_.getBasis() * btVector3_up;
                    scale.setY((p2 - capturedTargetXf_.getOrigin()).dot(v) * scale.y() / (p1 - capturedTargetXf_.getOrigin()).dot(v));
                    break;
                }
                case MoveType::AxisZ: {
                    auto v = capturedTargetXf_.getBasis() * btVector3_forward;
                    scale.setZ((p2 - capturedTargetXf_.getOrigin()).dot(v) * scale.z() / (p1 - capturedTargetXf_.getOrigin()).dot(v));
                    break;
                }
                case MoveType::PlaneX: {
                    auto vX = capturedTargetXf_.getBasis() * btVector3_forward;
                    auto vY = capturedTargetXf_.getBasis() * btVector3_up;
                    auto diff = p2 - capturedTargetXf_.getOrigin();
                    float x = diff.dot(vX);
                    float y = diff.dot(vY);

                    float tmp = scale.x();
                    scale.setX(0);

                    if (x + y < 0.0f) {
                        scale = -diff.length() * scale / (p1 - capturedTargetXf_.getOrigin()).length();
                    } else {
                        scale = diff.length() * scale / (p1 - capturedTargetXf_.getOrigin()).length();
                    }
                    scale.setX(tmp);
                    break;
                }
                case MoveType::PlaneY: {
                    auto vX = capturedTargetXf_.getBasis() * btVector3_forward;
                    auto vY = capturedTargetXf_.getBasis() * btVector3_right;
                    auto diff = p2 - capturedTargetXf_.getOrigin();
                    float x = diff.dot(vX);
                    float y = diff.dot(vY);

                    float tmp = scale.y();
                    scale.setY(0);

                    if (x + y < 0.0f) {
                        scale = -diff.length() * scale / (p1 - capturedTargetXf_.getOrigin()).length();
                    } else {
                        scale = diff.length() * scale / (p1 - capturedTargetXf_.getOrigin()).length();
                    }
                    scale.setY(tmp);
                    break;
                }
                case MoveType::PlaneZ: {
                    auto vX = capturedTargetXf_.getBasis() * btVector3_right;
                    auto vY = capturedTargetXf_.getBasis() * btVector3_up;
                    auto diff = p2 - capturedTargetXf_.getOrigin();
                    float x = diff.dot(vX);
                    float y = diff.dot(vY);

                    float tmp = scale.z();
                    scale.setZ(0);

                    if (x + y < 0.0f) {
                        scale = -diff.length() * scale / (p1 - capturedTargetXf_.getOrigin()).length();
                    } else {
                        scale = diff.length() * scale / (p1 - capturedTargetXf_.getOrigin()).length();
                    }
                    scale.setZ(tmp);
                    break;
                }
                case MoveType::PlaneCurrent: {
                    auto vX = frustum.transform().getBasis() * btVector3_right;
                    auto vY = frustum.transform().getBasis() * btVector3_up;
                    auto vP1 = p1 - capturedTargetXf_.getOrigin();
                    auto diff = p2 - capturedTargetXf_.getOrigin();

                    if (btFabs(Vector2f(vP1.dot(vX), vP1.dot(vY)).angle(Vector2f(diff.dot(vX), diff.dot(vY)))) > SIMD_HALF_PI) {
                        scale = -diff.length() * scale / vP1.length();
                    } else {
                        scale = diff.length() * scale / vP1.length();
                    }
                    break;
                }
                default:
                    btAssert(false);
                    break;
                }
                rc_->target()->propertySet(AProperty_Scale, scale);
            }
        }
    }

    btPlane ToolScale::getScalePlane(const Frustum& frustum)
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

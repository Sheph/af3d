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

        if (capturedMt_ != MoveType::None) {
            bool canceled = inputManager.keyboard().triggered(KI_ESCAPE);
            if (!inputManager.mouse().pressed(true) || canceled) {
                capturedMt_ = MoveType::None;
                selTool_.activate(true);
                rc_->setAlphaActive(0.7f);
                auto xf = rc_->target()->propertyGet(AProperty_WorldTransform);
                rc_->target()->propertySet(AProperty_WorldTransform, capturedTargetXf_);
                if (!canceled && capturedChanged_) {
                    workspace().setProperty(rc_->target(), AProperty_WorldTransform, xf, false);
                }
                rc_->setMoveType(MoveType::None);
                workspace().unlock();
            } else {
                auto mp = inputManager.mouse().pos();
                if (moveTarget(mp)) {
                    capturedChanged_ = true;
                }
            }
        } else {
            auto cc = scene()->camera()->findComponent<CameraComponent>();

            auto ray = cc->screenPointToRay(inputManager.mouse().pos());

            auto res = rc_->testRay(cc->getFrustum(), ray);

            if (!inputManager.mouse().pressed(true)) {
                rc_->setMoveType(res);
            } else if ((rc_->moveType() != MoveType::None) && inputManager.mouse().triggered(true)) {
                if (workspace().lock()) {
                    capturedMt_ = rc_->moveType();
                    rc_->setAlphaActive(1.0f);
                    selTool_.activate(false);
                    capturedMousePos_ = inputManager.mouse().pos();
                    capturedRay_ = ray;
                    capturedTargetXf_ = rc_->target()->propertyGet(AProperty_WorldTransform).toTransform();
                    capturedChanged_ = false;
                } else {
                    rc_->setMoveType(MoveType::None);
                }
            }
        }

        selTool_.update(dt);
    }

    void ToolMove::doOptions()
    {
    }

    void ToolMove::cleanup()
    {
        if (!rc_) {
            return;
        }

        if (capturedMt_ != MoveType::None) {
            rc_->target()->propertySet(AProperty_WorldTransform, capturedTargetXf_);
            workspace().unlock();
        }
        rc_->removeFromParent();
        rc_.reset();
        capturedMt_ = MoveType::None;
        selTool_.activate(true);
    }

    btPlane ToolMove::getMovePlane(const Frustum& frustum)
    {
        auto vRight = capturedTargetXf_.getBasis() * btVector3_right;
        auto vUp = capturedTargetXf_.getBasis() * btVector3_up;
        auto vForward = capturedTargetXf_.getBasis() * btVector3_forward;

        auto planeX = btPlaneMake(capturedTargetXf_.getOrigin(), vRight);
        auto planeY = btPlaneMake(capturedTargetXf_.getOrigin(), vUp);
        auto planeZ = btPlaneMake(capturedTargetXf_.getOrigin(), vForward);

        if (capturedMt_ == MoveType::PlaneYZ) {
            return planeX;
        } else if (capturedMt_ == MoveType::PlaneXZ) {
            return planeY;
        } else if (capturedMt_ == MoveType::PlaneXY) {
            return planeZ;
        } else if (capturedMt_ == MoveType::PlaneCurrent) {
            return btPlaneMake(capturedTargetXf_.getOrigin(),
                frustum.plane(Frustum::Plane::Far).normal);
        }

        auto px = btFabs(capturedRay_.dir.dot(planeX.normal));
        auto py = btFabs(capturedRay_.dir.dot(planeY.normal));
        auto pz = btFabs(capturedRay_.dir.dot(planeZ.normal));

        if (capturedMt_ == MoveType::AxisX) {
            return (py > pz) ? planeY : planeZ;
        } else if (capturedMt_ == MoveType::AxisY) {
            return (px > pz) ? planeX : planeZ;
        } else {
            btAssert(capturedMt_ == MoveType::AxisZ);
            return (px > py) ? planeX : planeY;
        }
    }

    bool ToolMove::moveTarget(const Vector2f& mp)
    {
        auto cc = scene()->camera()->findComponent<CameraComponent>();

        btPlane plane = getMovePlane(cc->getFrustum());

        auto r = capturedRay_.testPlane(plane);
        if (r.first) {
            auto p1 = capturedRay_.getAt(r.second);
            auto ray = cc->screenPointToRay(mp);
            r = ray.testPlane(plane);
            if (r.first) {
                auto p2 = ray.getAt(r.second);
                auto xf = capturedTargetXf_;
                switch (capturedMt_) {
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

        return (mp != capturedMousePos_);
    }
} }

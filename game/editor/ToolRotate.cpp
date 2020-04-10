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
#include "Scene.h"
#include "SceneObject.h"
#include "CameraComponent.h"
#include "imgui.h"

namespace af3d { namespace editor
{
    ToolRotate::ToolRotate(Workspace* workspace)
    : ToolGizmo(workspace, "Rotate", assetManager.getImage("common1/tool_rotate.png"), KeySequence(KI_R)),
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
        ToolGizmo::doOptions();
        if (captured()) {
            ImGui::Text("%s,%s,%s - Restrict plane", InputKeyboard::kiToStr(KI_X), InputKeyboard::kiToStr(KI_Y), InputKeyboard::kiToStr(KI_Z));
        }
    }

    bool ToolRotate::gizmoCreate(const AObjectPtr& obj)
    {
        if (!obj->propertyCanGet(AProperty_WorldTransform)) {
            return false;
        }

        rc_ = std::make_shared<RenderGizmoRotateComponent>();
        rc_->setOrientation(orientation());
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
        rc_->setOrientation(orientation());
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
        rc_->setOrientation(orientation());
        rc_->setRotateType(RotateType::None);
        rc_->setAlphaActive(0.7f);
        selTool_.activate(true);
    }

    void ToolRotate::gizmoMove(const Frustum& frustum, const Ray& ray)
    {
        rc_->setOrientation(orientation());

        if (!captured()) {
            rc_->setRotateType(rc_->testRay(frustum, ray));
            return;
        }

        ImGuiIO& io = ImGui::GetIO();
        if (!io.WantCaptureMouse && !io.WantCaptureKeyboard) {
            if (inputManager.keyboard().triggered(KeySequence(KI_X))) {
                rc_->setRotateType(RotateType::PlaneX);
            } else if (inputManager.keyboard().triggered(KeySequence(KI_Y))) {
                rc_->setRotateType(RotateType::PlaneY);
            } else if (inputManager.keyboard().triggered(KeySequence(KI_Z))) {
                rc_->setRotateType(RotateType::PlaneZ);
            }
        }

        btPlane plane = getRotatePlane(frustum);

        auto r = capturedRay().testPlane(plane);
        if (r.first) {
            auto p1 = capturedRay().getAt(r.second);
            r = ray.testPlane(plane);
            if (r.first) {
                auto p2 = ray.getAt(r.second);
                auto xf = capturedTargetXf_;

                if (rc_->rotateType() == RotateType::Trackball) {
                    if (capturedMousePos() != inputManager.mouse().pos()) {
                        auto cc = scene()->camera()->findComponent<CameraComponent>();

                        auto diff = cc->screenToViewport(inputManager.mouse().pos()) - cc->screenToViewport(capturedMousePos());

                        xf.setRotation(btQuaternion((p2 - p1).cross(plane.normal), -diff.length() * SIMD_PI * 2.0f) *
                            xf.getRotation());
                        rc_->target()->propertySet(AProperty_WorldTransform, xf);
                    }
                } else {
                    p1 -= capturedTargetXf_.getOrigin();
                    p2 -= capturedTargetXf_.getOrigin();

                    xf.setRotation(shortestArcQuatNormalize2(p1, p2) * xf.getRotation());
                    rc_->target()->propertySet(AProperty_WorldTransform, xf);
                }
            }
        }
    }

    btPlane ToolRotate::getRotatePlane(const Frustum& frustum)
    {
        auto txf = capturedTargetXfOriented();

        if (rc_->rotateType() == RotateType::PlaneX) {
            auto v = txf.getBasis() * btVector3_right;
            return btPlaneMake(txf.getOrigin(), v);
        } else if (rc_->rotateType() == RotateType::PlaneY) {
            auto v = txf.getBasis() * btVector3_up;
            return btPlaneMake(txf.getOrigin(), v);
        } else if (rc_->rotateType() == RotateType::PlaneZ) {
            auto v = txf.getBasis() * btVector3_forward;
            return btPlaneMake(txf.getOrigin(), v);
        } else if ((rc_->rotateType() == RotateType::PlaneCurrent) ||
            rc_->rotateType() == RotateType::Trackball) {
            return btPlaneMake(txf.getOrigin(),
                frustum.plane(Frustum::Plane::Far).normal);
        } else {
            btAssert(false);
            return btPlane();
        }
    }

    btTransform ToolRotate::capturedTargetXfOriented() const
    {
        switch (orientation()) {
        case TransformOrientation::Global:
            return toTransform(capturedTargetXf_.getOrigin());
        default:
            btAssert(false);
        case TransformOrientation::Local:
            return capturedTargetXf_;
        }
    }
} }

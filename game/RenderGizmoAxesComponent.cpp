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

#include "RenderGizmoAxesComponent.h"
#include "MaterialManager.h"
#include "SceneObject.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(RenderGizmoAxesComponent, RenderComponent)
    ACLASS_DEFINE_END(RenderGizmoAxesComponent)

    RenderGizmoAxesComponent::RenderGizmoAxesComponent()
    : RenderComponent(AClass_RenderGizmoAxesComponent)
    {
    }

    const AClass& RenderGizmoAxesComponent::staticKlass()
    {
        return AClass_RenderGizmoAxesComponent;
    }

    AObjectPtr RenderGizmoAxesComponent::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<RenderGizmoAxesComponent>();
        obj->propertiesSet(propVals);
        return obj;
    }

    void RenderGizmoAxesComponent::update(float dt)
    {
        btTransform newXf = getTargetXf();

        if ((newXf == targetXf_) && !dirty_) {
            return;
        }

        AABB aabb = calcAABB();

        btVector3 displacement = newXf.getOrigin() - targetXf_.getOrigin();

        manager()->moveAABB(cookie_, prevAABB_, aabb, displacement);

        targetXf_ = newXf;
        prevAABB_ = aabb;
        dirty_ = false;
    }

    void RenderGizmoAxesComponent::render(RenderList& rl, void* const* parts, size_t numParts)
    {
        auto sz = getSizes(rl.camera()->frustum());

        auto rop = rl.addGeometry(materialManager.matImmDefault(false, false), GL_TRIANGLES, 1.0f);

        auto txf = targetXfOriented();

        auto vForward = txf.getBasis() * btVector3_forward;
        auto vUp = txf.getBasis() * btVector3_up;
        auto vRight = txf.getBasis() * btVector3_right;

        if (kind_ == KindMove) {
            rop.addLineArrow(txf.getOrigin(), vRight * sz.lineLength, vUp * sz.lineRadius, sz.arrowSize,
                Color(1.0f, 0.0f, 0.0f, alpha(MoveType::AxisX)));
            rop.addLineArrow(txf.getOrigin(), vUp * sz.lineLength, vForward * sz.lineRadius, sz.arrowSize,
                Color(0.0f, 1.0f, 0.0f, alpha(MoveType::AxisY)));
            rop.addLineArrow(txf.getOrigin(), vForward * sz.lineLength, vUp * sz.lineRadius, sz.arrowSize,
                Color(0.0f, 0.0f, 1.0f, alpha(MoveType::AxisZ)));
        } else {
            auto asz = btVector3(sz.arrowSize.y(), sz.arrowSize.y(), sz.arrowSize.y()) * 1.5f;
            rop.addLineBox(txf.getOrigin(), vRight * sz.lineLength, vUp * sz.lineRadius, asz,
                Color(1.0f, 0.0f, 0.0f, alpha(MoveType::AxisX)));
            rop.addLineBox(txf.getOrigin(), vUp * sz.lineLength, vForward * sz.lineRadius, asz,
                Color(0.0f, 1.0f, 0.0f, alpha(MoveType::AxisY)));
            rop.addLineBox(txf.getOrigin(), vForward * sz.lineLength, vUp * sz.lineRadius, asz,
                Color(0.0f, 0.0f, 1.0f, alpha(MoveType::AxisZ)));
        }

        rop.addQuad(txf.getOrigin() + (vUp + vForward) * sz.quadOffset,
            {vUp * sz.quadSize, vForward * sz.quadSize}, Color(1.0f, 0.0f, 0.0f, alpha(MoveType::PlaneX)));
        rop.addQuad(txf.getOrigin() + (vRight + vForward) * sz.quadOffset,
            {vRight * sz.quadSize, vForward * sz.quadSize}, Color(0.0f, 1.0f, 0.0f, alpha(MoveType::PlaneY)));
        rop.addQuad(txf.getOrigin() + (vRight + vUp) * sz.quadOffset,
            {vRight * sz.quadSize, vUp * sz.quadSize}, Color(0.0f, 0.0f, 1.0f, alpha(MoveType::PlaneZ)));

        if (kind_ == KindMove) {
            rop.addBox(txf.getOrigin(),
                {vRight * sz.boxSize, vForward * sz.boxSize, vUp * sz.boxSize}, Color(1.0f, 1.0f, 1.0f, alpha(MoveType::PlaneCurrent)));
        } else {
            rop.addRing(txf.getOrigin(),
                rl.camera()->frustum().plane(Frustum::Plane::Far).normal * sz.lineRadius, sz.ringRadius, Color(1.0f, 1.0f, 1.0f, alpha(MoveType::PlaneCurrent)));
        }
    }

    std::pair<AObjectPtr, float> RenderGizmoAxesComponent::testRay(const Frustum& frustum, const Ray& ray, void* part)
    {
        auto res = ray.testSphere(Sphere(targetXf_.getOrigin(), radius_));
        if (res.first) {
            return std::make_pair(sharedThis(), res.second);
        } else {
            return std::make_pair(AObjectPtr(), 0.0f);
        }
    }

    MoveType RenderGizmoAxesComponent::testRay(const Frustum& frustum, const Ray& ray) const
    {
        auto sz = getSizes(frustum);

        auto txf = targetXfOriented();

        auto r2 = ray.getTransformed(txf.inverse());

        if (kind_ == KindMove) {
            if (r2.testAABB(AABB(btVector3_forward * sz.boxSize,
                (btVector3_up + btVector3_right) * sz.boxSize)).first) {
                return MoveType::PlaneCurrent;
            }
        } else {
            auto res = ray.testPlane(btPlaneMake(txf.getOrigin(), frustum.plane(Frustum::Plane::Far).normal));
            if (res.first) {
                auto l = (ray.getAt(res.second) - txf.getOrigin()).length();
                if (l >= (sz.ringRadius - sz.arrowSize.y()) && l <= (sz.ringRadius + sz.arrowSize.y())) {
                    return MoveType::PlaneCurrent;
                }
            }
        }

        if (r2.testAABB(AABB((btVector3_up + btVector3_forward) * sz.quadOffset + (btVector3_forward - btVector3_right) * sz.quadSize,
            (btVector3_up + btVector3_forward) * sz.quadOffset + (btVector3_up + btVector3_right) * sz.quadSize)).first) {
            return MoveType::PlaneX;
        } else if (r2.testAABB(AABB((btVector3_right + btVector3_forward) * sz.quadOffset + (btVector3_forward - btVector3_up) * sz.quadSize,
            (btVector3_right + btVector3_forward) * sz.quadOffset + (btVector3_up + btVector3_right) * sz.quadSize)).first) {
            return MoveType::PlaneY;
        } else if (r2.testAABB(AABB((btVector3_right + btVector3_up) * sz.quadOffset + btVector3_forward * sz.quadSize,
            (btVector3_right + btVector3_up) * sz.quadOffset + (btVector3_up + btVector3_right - btVector3_forward) * sz.quadSize)).first) {
            return MoveType::PlaneZ;
        } else if (r2.testAABB(AABB(btVector3(0.0f, -sz.arrowSize.y(), -sz.arrowSize.y()),
            btVector3(sz.lineLength + sz.arrowSize.x(), sz.arrowSize.y(), sz.arrowSize.y()))).first) {
            return MoveType::AxisX;
        } else if (r2.testAABB(AABB(btVector3(-sz.arrowSize.y(), 0.0f, -sz.arrowSize.y()),
            btVector3(sz.arrowSize.y(), sz.lineLength + sz.arrowSize.x(), sz.arrowSize.y()))).first) {
            return MoveType::AxisY;
        } else if (r2.testAABB(AABB(btVector3(-sz.arrowSize.y(), -sz.arrowSize.y(), - sz.lineLength - sz.arrowSize.x()),
            btVector3(sz.arrowSize.y(), sz.arrowSize.y(), 0.0f))).first) {
            return MoveType::AxisZ;
        }

        return MoveType::None;
    }

    void RenderGizmoAxesComponent::onRegister()
    {
        targetXf_ = getTargetXf();
        prevAABB_ = calcAABB();
        cookie_ = manager()->addAABB(this, prevAABB_, nullptr);
        dirty_ = false;
    }

    void RenderGizmoAxesComponent::onUnregister()
    {
        manager()->removeAABB(cookie_);
    }

    AABB RenderGizmoAxesComponent::calcAABB()
    {
        auto sz = btVector3(radius_, radius_, radius_);

        return AABB(targetXf_.getOrigin() - sz, targetXf_.getOrigin() + sz);
    }

    btTransform RenderGizmoAxesComponent::getTargetXf() const
    {
        return target_ ? target_->propertyGet(AProperty_WorldTransform).toTransform() : btTransform::getIdentity();
    }

    RenderGizmoAxesComponent::Sizes RenderGizmoAxesComponent::getSizes(const Frustum& frustum) const
    {
        auto viewExt = frustum.getExtents(targetXf_.getOrigin());

        Sizes ret;

        ret.lineLength = viewportLength_ * viewExt.y();
        ret.lineRadius = viewportRadius_ * viewExt.y();
        ret.arrowSize.setValue(ret.lineRadius * 15.0f, ret.lineRadius * 5.0f);
        ret.quadOffset = ret.lineLength * 0.3f;
        ret.quadSize = ret.lineLength * 0.18f;
        ret.boxSize = ret.lineLength * 0.14f;
        ret.ringRadius = ret.lineLength + ret.arrowSize.x();

        return ret;
    }

    btTransform RenderGizmoAxesComponent::targetXfOriented() const
    {
        switch (orientation_) {
        case TransformOrientation::Global:
            return toTransform(targetXf_.getOrigin());
        default:
            btAssert(false);
        case TransformOrientation::Local:
            return targetXf_;
        }
    }
}

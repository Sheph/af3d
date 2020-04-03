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

#include "RenderGizmoMoveComponent.h"
#include "MaterialManager.h"
#include "SceneObject.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(RenderGizmoMoveComponent, RenderComponent)
    ACLASS_DEFINE_END(RenderGizmoMoveComponent)

    RenderGizmoMoveComponent::RenderGizmoMoveComponent()
    : RenderComponent(AClass_RenderGizmoMoveComponent)
    {
        material_ = materialManager.createMaterial(MaterialTypeImm);
        material_->setBlendingParams(BlendingParams(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        material_->setDepthTest(false);
        material_->setCullFaceMode(0);
    }

    const AClass& RenderGizmoMoveComponent::staticKlass()
    {
        return AClass_RenderGizmoMoveComponent;
    }

    AObjectPtr RenderGizmoMoveComponent::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<RenderGizmoMoveComponent>();
        obj->propertiesSet(propVals);
        return obj;
    }

    void RenderGizmoMoveComponent::update(float dt)
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

    void RenderGizmoMoveComponent::render(RenderList& rl, void* const* parts, size_t numParts)
    {
        auto sz = getSizes(rl.frustum());

        auto rop = rl.addGeometry(material_, GL_TRIANGLES);

        auto vForward = targetXf_.getBasis() * btVector3_forward;
        auto vUp = targetXf_.getBasis() * btVector3_up;
        auto vRight = targetXf_.getBasis() * btVector3_right;

        rop.addLineArrow(targetXf_.getOrigin(), vRight * sz.lineLength, vUp * sz.lineRadius, sz.arrowSize,
            Color(1.0f, 0.0f, 0.0f, alpha(MoveType::AxisX)));
        rop.addLineArrow(targetXf_.getOrigin(), vUp * sz.lineLength, vForward * sz.lineRadius, sz.arrowSize,
            Color(0.0f, 1.0f, 0.0f, alpha(MoveType::AxisY)));
        rop.addLineArrow(targetXf_.getOrigin(), vForward * sz.lineLength, vUp * sz.lineRadius, sz.arrowSize,
            Color(0.0f, 0.0f, 1.0f, alpha(MoveType::AxisZ)));

        rop.addQuad(targetXf_.getOrigin() + (vUp + vForward) * sz.quadOffset,
            {vUp * sz.quadSize, vForward * sz.quadSize}, Color(1.0f, 0.0f, 0.0f, alpha(MoveType::PlaneYZ)));
        rop.addQuad(targetXf_.getOrigin() + (vRight + vForward) * sz.quadOffset,
            {vRight * sz.quadSize, vForward * sz.quadSize}, Color(0.0f, 1.0f, 0.0f, alpha(MoveType::PlaneXZ)));
        rop.addQuad(targetXf_.getOrigin() + (vRight + vUp) * sz.quadOffset,
            {vRight * sz.quadSize, vUp * sz.quadSize}, Color(0.0f, 0.0f, 1.0f, alpha(MoveType::PlaneXY)));

        rop.addBox(targetXf_.getOrigin(),
            {vRight * sz.boxSize, vForward * sz.boxSize, vUp * sz.boxSize}, Color(1.0f, 1.0f, 1.0f, alpha(MoveType::PlaneCurrent)));
    }

    std::pair<AObjectPtr, float> RenderGizmoMoveComponent::testRay(const Frustum& frustum, const Ray& ray, void* part)
    {
        auto res = ray.testSphere(Sphere(targetXf_.getOrigin(), radius_));
        if (res.first) {
            return std::make_pair(sharedThis(), res.second);
        } else {
            return std::make_pair(AObjectPtr(), 0.0f);
        }
    }

    void RenderGizmoMoveComponent::debugDraw()
    {
    }

    MoveType RenderGizmoMoveComponent::testRay(const Frustum& frustum, const Ray& ray) const
    {
        auto sz = getSizes(frustum);

        auto r2 = ray.getTransformed(targetXf_.inverse());

        if (r2.testAABB(AABB(btVector3_forward * sz.boxSize,
            (btVector3_up + btVector3_right) * sz.boxSize)).first) {
            return MoveType::PlaneCurrent;
        } else if (r2.testAABB(AABB((btVector3_up + btVector3_forward) * sz.quadOffset + (btVector3_forward - btVector3_right) * sz.quadSize,
            (btVector3_up + btVector3_forward) * sz.quadOffset + (btVector3_up + btVector3_right) * sz.quadSize)).first) {
            return MoveType::PlaneYZ;
        } else if (r2.testAABB(AABB((btVector3_right + btVector3_forward) * sz.quadOffset + (btVector3_forward - btVector3_up) * sz.quadSize,
            (btVector3_right + btVector3_forward) * sz.quadOffset + (btVector3_up + btVector3_right) * sz.quadSize)).first) {
            return MoveType::PlaneXZ;
        } else if (r2.testAABB(AABB((btVector3_right + btVector3_up) * sz.quadOffset + btVector3_forward * sz.quadSize,
            (btVector3_right + btVector3_up) * sz.quadOffset + (btVector3_up + btVector3_right - btVector3_forward) * sz.quadSize)).first) {
            return MoveType::PlaneXY;
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

    void RenderGizmoMoveComponent::onRegister()
    {
        targetXf_ = getTargetXf();
        prevAABB_ = calcAABB();
        cookie_ = manager()->addAABB(this, prevAABB_, nullptr);
        dirty_ = false;
    }

    void RenderGizmoMoveComponent::onUnregister()
    {
        manager()->removeAABB(cookie_);
    }

    AABB RenderGizmoMoveComponent::calcAABB()
    {
        auto sz = btVector3(radius_, radius_, radius_);

        return AABB(targetXf_.getOrigin() - sz, targetXf_.getOrigin() + sz);
    }

    btTransform RenderGizmoMoveComponent::getTargetXf() const
    {
        return target_ ? target_->propertyGet(AProperty_WorldTransform).toTransform() : btTransform::getIdentity();
    }

    RenderGizmoMoveComponent::Sizes RenderGizmoMoveComponent::getSizes(const Frustum& frustum) const
    {
        auto viewExt = frustum.getExtents(targetXf_.getOrigin());

        Sizes ret;

        ret.lineLength = viewportLength_ * viewExt.y();
        ret.lineRadius = viewportRadius_ * viewExt.y();
        ret.arrowSize.setValue(ret.lineRadius * 15.0f, ret.lineRadius * 5.0f);
        ret.quadOffset = ret.lineLength * 0.3f;
        ret.quadSize = ret.lineLength * 0.18f;
        ret.boxSize = ret.lineLength * 0.14f;

        return ret;
    }
}

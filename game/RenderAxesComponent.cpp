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

#include "RenderAxesComponent.h"
#include "MaterialManager.h"
#include "SceneObject.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(RenderAxesComponent, RenderComponent)
    ACLASS_DEFINE_END(RenderAxesComponent)

    RenderAxesComponent::RenderAxesComponent()
    : RenderComponent(AClass_RenderAxesComponent)
    {
        material_ = materialManager.createMaterial(MaterialTypeImm);
        material_->setBlendingParams(BlendingParams(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        material_->setDepthTest(false);
    }

    const AClass& RenderAxesComponent::staticKlass()
    {
        return AClass_RenderAxesComponent;
    }

    AObjectPtr RenderAxesComponent::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<RenderAxesComponent>();
        obj->propertiesSet(propVals);
        return obj;
    }

    void RenderAxesComponent::update(float dt)
    {
        if ((parent()->transform() == prevParentXf_) && !dirty_) {
            return;
        }

        dirty_ = false;

        AABB aabb = calcAABB();

        btVector3 displacement = parent()->transform().getOrigin() - prevParentXf_.getOrigin();

        manager()->moveAABB(cookie_, prevAABB_, aabb, displacement);

        prevParentXf_ = parent()->transform();
        prevAABB_ = aabb;
    }

    void RenderAxesComponent::render(RenderList& rl, void* const* parts, size_t numParts)
    {
        auto xf = parent()->transform() * xf_;

        auto vForward = xf.getBasis() * btVector3_forward;
        auto vUp = xf.getBasis() * btVector3_up;
        auto vRight = xf.getBasis() * btVector3_right;

        auto viewExt = rl.frustum().getExtents(xf.getOrigin());
        float lineLength = viewportLength_ * viewExt.y();
        float lineRadius = viewportRadius_ * viewExt.y();

        auto rop = rl.addGeometry(material_, GL_TRIANGLES, 1.0f);

        rop.addLine(xf.getOrigin(), vRight * lineLength, vUp * lineRadius, Color(1.0f, 0.0f, 0.0f, 0.5f));
        rop.addLine(xf.getOrigin(), vUp * lineLength, vForward * lineRadius, Color(0.0f, 1.0f, 0.0f, 0.5f));
        rop.addLine(xf.getOrigin(), vForward * lineLength, vUp * lineRadius, Color(0.0f, 0.0f, 1.0f, 0.5f));
    }

    std::pair<AObjectPtr, float> RenderAxesComponent::testRay(const Frustum& frustum, const Ray& ray, void* part)
    {
        auto res = ray.testAABB(prevAABB_);
        if (res.first) {
            return std::make_pair(sharedThis(), res.second);
        } else {
            return std::make_pair(AObjectPtr(), 0.0f);
        }
    }

    void RenderAxesComponent::debugDraw()
    {
    }

    void RenderAxesComponent::setTransform(const btTransform& value)
    {
        xf_ = value;
        dirty_ = true;
    }

    void RenderAxesComponent::onRegister()
    {
        prevParentXf_ = parent()->transform();
        prevAABB_ = calcAABB();
        cookie_ = manager()->addAABB(this, prevAABB_, nullptr);
        dirty_ = false;
    }

    void RenderAxesComponent::onUnregister()
    {
        manager()->removeAABB(cookie_);
    }

    AABB RenderAxesComponent::calcAABB() const
    {
        auto xf = parent()->transform() * xf_;

        auto sz = btVector3(radius_, radius_, radius_);

        return AABB(xf.getOrigin() - sz, xf.getOrigin() + sz);
    }
}

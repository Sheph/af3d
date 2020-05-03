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

#include "RenderGridComponent.h"
#include "SceneObject.h"
#include "MaterialManager.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(RenderGridComponent, RenderComponent)
    ACLASS_DEFINE_END(RenderGridComponent)

    RenderGridComponent::RenderGridComponent()
    : RenderComponent(AClass_RenderGridComponent, true)
    {
        material_ = materialManager.createMaterial(MaterialTypeGrid);
        material_->setBlendingParams(BlendingParams(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        material_->setDepthWrite(false);
        material_->setCullFaceMode(0);
        setXAxisColor(xAxisColor_);
        setYAxisColor(yAxisColor_);
    }

    const AClass& RenderGridComponent::staticKlass()
    {
        return AClass_RenderGridComponent;
    }

    AObjectPtr RenderGridComponent::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<RenderGridComponent>();
        obj->propertiesSet(propVals);
        return obj;
    }

    void RenderGridComponent::update(float dt)
    {
        plane_ = btPlaneMake(parent()->smoothTransform().getOrigin(), parent()->smoothTransform().getBasis() * btVector3_forward);

        material_->params().setUniform(UniformName::GridPos, parent()->smoothTransform().getOrigin());
        material_->params().setUniform(UniformName::GridRight, parent()->getSmoothRight());
        material_->params().setUniform(UniformName::GridUp, parent()->getSmoothUp());
    }

    void RenderGridComponent::render(RenderList& rl, void* const* parts, size_t numParts)
    {
        auto p = btPlaneProject(plane_, rl.camera()->frustum().transform().getOrigin());
        float dist = (p - rl.camera()->frustum().transform().getOrigin()).length();
        auto vRight = parent()->getSmoothRight();
        auto vUp = parent()->getSmoothUp();

        int power = dist > 0.0f ? btLog(dist / step_ / 5.0f) / btLog(10.0f) : 0;
        if (power < 0) {
            power = 0;
        }

        material_->params().setUniform(UniformName::GridStep, step_ * btPow(10.0f, power));

        auto sz = rl.camera()->frustum().farDist();

        auto p0 = p + (- vRight - vUp) * sz;
        auto p1 = p + (vRight - vUp) * sz;
        auto p2 = p + (vRight + vUp) * sz;
        auto p3 = p + (- vRight + vUp) * sz;

        auto rop = rl.addGeometry(material_, GL_TRIANGLES);

        Color c = color_;
        c.setW(lerp(c.w() * 2.0f, c.w(), btMin(dist, 5.0f) / 5.0f));

        rop.addVertex(p0, Vector2f(), c);
        rop.addVertex(p1, Vector2f(), c);
        rop.addVertex(p2, Vector2f(), c);

        rop.addVertex(p0, Vector2f(), c);
        rop.addVertex(p2, Vector2f(), c);
        rop.addVertex(p3, Vector2f(), c);
    }

    std::pair<AObjectPtr, float> RenderGridComponent::testRay(const Frustum& frustum, const Ray& ray, void* part)
    {
        auto res = ray.testPlane(plane_);
        if (res.first) {
            return std::make_pair(sharedThis(), res.second);
        } else {
            return std::make_pair(AObjectPtr(), 0.0f);
        }
    }

    void RenderGridComponent::setXAxisColor(const Color& value)
    {
        xAxisColor_ = value;
        material_->params().setUniform(UniformName::GridXColor, toVector3(value));
    }

    void RenderGridComponent::setYAxisColor(const Color& value)
    {
        yAxisColor_ = value;
        material_->params().setUniform(UniformName::GridYColor, toVector3(value));
    }

    void RenderGridComponent::onRegister()
    {
        plane_ = btPlaneMake(parent()->smoothTransform().getOrigin(), parent()->smoothTransform().getBasis() * btVector3_forward);
    }

    void RenderGridComponent::onUnregister()
    {
    }
}

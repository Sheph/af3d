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

#include "RenderProxyComponent.h"
#include "SceneObject.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(RenderProxyComponent, RenderComponent)
    ACLASS_DEFINE_END(RenderProxyComponent)

    RenderProxyComponent::RenderProxyComponent(const RenderCb& cb)
    : RenderComponent(AClass_RenderProxyComponent),
      cb_(cb)
    {
    }

    const AClass& RenderProxyComponent::staticKlass()
    {
        return AClass_RenderProxyComponent;
    }

    AObjectPtr RenderProxyComponent::create(const APropertyValueMap& propVals)
    {
        return AObjectPtr();
    }

    void RenderProxyComponent::update(float dt)
    {
        if ((parent()->smoothTransform() == prevParentXf_) && !dirty_) {
            return;
        }

        dirty_ = false;

        auto aabb = localAABB_.getTransformed(parent()->smoothTransform());

        btVector3 displacement = parent()->smoothTransform().getOrigin() - prevParentXf_.getOrigin();

        manager()->moveAABB(cookie_, prevAABB_, aabb, displacement);

        prevParentXf_ = parent()->smoothTransform();
        prevAABB_ = aabb;
    }

    void RenderProxyComponent::render(RenderList& rl, void* const* parts, size_t numParts)
    {
        cb_(rl);
    }

    std::pair<AObjectPtr, float> RenderProxyComponent::testRay(const Frustum& frustum, const Ray& ray, void* part)
    {
        auto res = ray.testAABB(prevAABB_);
        if (res.first) {
            return std::make_pair(sharedThis(), res.second);
        } else {
            return std::make_pair(AObjectPtr(), 0.0f);
        }
    }

    void RenderProxyComponent::onRegister()
    {
        prevParentXf_ = parent()->transform();
        prevAABB_ = localAABB_.getTransformed(prevParentXf_);
        cookie_ = manager()->addAABB(this, prevAABB_, nullptr);
    }

    void RenderProxyComponent::onUnregister()
    {
        manager()->removeAABB(cookie_);
    }
}

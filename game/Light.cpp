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

#include "Light.h"
#include "LightComponent.h"
#include "SceneObject.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN_ABSTRACT(Light, AObject)
    ACLASS_DEFINE_END(Light)

    Light::Light(const AClass& klass, int typeId)
    : AObject(klass),
      typeId_(typeId)
    {
        btAssert(typeId > 0);
    }

    Light::~Light()
    {
        btAssert(parent_ == nullptr);
        btAssert(cookie_ == nullptr);
    }

    const AClass& Light::staticKlass()
    {
        return AClass_Light;
    }

    SceneObject* Light::parentObject() const
    {
        return parent() ? parent()->parent() : nullptr;
    }

    void Light::setTransform(const btTransform& value)
    {
        xf_ = value;
        worldXf_ = parentXf_ * xf_;
        dirty_ = true;
    }

    AABB Light::getWorldAABB() const
    {
        return localAABB_.getTransformed(worldTransform());
    }

    void Light::remove()
    {
        if (parent()) {
            parent()->removeLight(sharedThis());
        }
    }

    void Light::setupMaterial(const btVector3& eyePos, MaterialParams& params) const
    {
        const auto& xf = worldTransform();
        params.setUniform(UniformName::EyePos, eyePos);
        params.setUniform(UniformName::LightPos, Vector4f(xf.getOrigin(), typeId_));
        params.setUniform(UniformName::LightColor, Vector3f(color_.x(), color_.y(), color_.z()) * color_.w());
        doSetupMaterial(eyePos, params);
    }

    void Light::adopt(LightComponent* parent)
    {
        parent_ = parent;
    }

    void Light::abandon()
    {
        parent_ = nullptr;
    }

    void Light::updateParentTransform()
    {
        btAssert(parent());
        auto obj = parent()->parent();
        btAssert(obj);
        parentXf_ = obj->transform();
        worldXf_ = parentXf_ * xf_;
        dirty_ = true;
    }

    bool Light::needsRenderUpdate(AABB& prevAABB, AABB& aabb, btVector3& displacement)
    {
        if (!dirty_) {
            return false;
        }

        aabb = getWorldAABB();
        prevAABB = prevWorldAABB_;

        auto worldPos = worldXf_.getOrigin();

        displacement = worldPos - prevWorldPos_;

        prevWorldPos_ = worldPos;
        prevWorldAABB_ = aabb;
        dirty_ = false;

        return true;
    }

    void Light::setLocalAABBImpl(const AABB& value)
    {
        localAABB_ = value;
        dirty_ = true;
    }
}

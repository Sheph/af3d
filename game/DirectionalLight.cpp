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

#include "DirectionalLight.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(DirectionalLight, Light)
    ACLASS_DEFINE_END(DirectionalLight)

    DirectionalLight::DirectionalLight()
    : Light(AClass_DirectionalLight, TypeId, true)
    {
        setLocalAABB(AABB(btVector3_one * -1000.0f, btVector3_one * 1000.0f));
    }

    const AClass& DirectionalLight::staticKlass()
    {
        return AClass_DirectionalLight;
    }

    AObjectPtr DirectionalLight::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<DirectionalLight>();
        obj->propertiesSet(propVals);
        return obj;
    }

    void DirectionalLight::setLocalAABB(const AABB& value)
    {
        setLocalAABBImpl(value);
    }

    void DirectionalLight::doSetupMaterial(const btVector3& eyePos, MaterialParams& params) const
    {
        params.setUniform(UniformName::LightDir, worldTransform().getBasis() * btVector3_forward);
    }

    void DirectionalLight::doSetupCluster(ShaderClusterLight& cLight) const
    {
        cLight.dir = Vector4f(worldTransform().getBasis() * btVector3_forward, 0.0f);
    }
}

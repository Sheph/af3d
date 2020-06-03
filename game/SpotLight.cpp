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

#include "SpotLight.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(SpotLight, Light)
    ACLASS_PROPERTY(SpotLight, Radius, "radius", "Radius", Float, 1.0f, Position, APropertyEditable)
    ACLASS_PROPERTY(SpotLight, Angle, "angle", "Angle", FloatRadian, btRadians(45.0f), Position, APropertyEditable)
    ACLASS_PROPERTY(SpotLight, InnerAngle, "inner angle", "Inner angle", FloatRadian, btRadians(15.0f), Position, APropertyEditable)
    ACLASS_PROPERTY(SpotLight, Power, "power", "Spot power", Float, 1.0f, Position, APropertyEditable)
    ACLASS_DEFINE_END(SpotLight)

    SpotLight::SpotLight()
    : Light(AClass_SpotLight, TypeId, true)
    {
        setRadius(1.0f);
    }

    const AClass& SpotLight::staticKlass()
    {
        return AClass_SpotLight;
    }

    AObjectPtr SpotLight::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<SpotLight>();
        obj->propertiesSet(propVals);
        return obj;
    }

    void SpotLight::setRadius(float value)
    {
        radius_ = value;
        setLocalAABBImpl(AABB(-btVector3_one * value, btVector3_one * value));
    }

    void SpotLight::setAngle(float value)
    {
        angle_ = value;
    }

    void SpotLight::setInnerAngle(float value)
    {
        innerAngle_ = value;
    }

    void SpotLight::setPower(float value)
    {
        power_ = value;
    }

    void SpotLight::doSetupCluster(ShaderClusterLight& cLight) const
    {
        cLight.dir = Vector4f(worldTransform().getBasis() * btVector3_forward * radius_, 0.0f);
        cLight.cutoffCos = btCos(angle_ * 0.5f);
        cLight.cutoffInnerCos = btCos(innerAngle_ * 0.5f);
        cLight.power = power_;
    }
}

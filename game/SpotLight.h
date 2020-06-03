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

#ifndef _SPOT_LIGHT_H_
#define _SPOT_LIGHT_H_

#include "Light.h"

namespace af3d
{
    class SpotLight : public std::enable_shared_from_this<SpotLight>,
        public Light
    {
    public:
        static const int TypeId = 3;

        explicit SpotLight();
        ~SpotLight() = default;

        static const AClass& staticKlass();

        static AObjectPtr create(const APropertyValueMap& propVals);

        AObjectPtr sharedThis() override { return shared_from_this(); }

        inline float radius() const { return radius_; }
        void setRadius(float value);

        inline float angle() const { return angle_; }
        void setAngle(float value);

        inline float innerAngle() const { return innerAngle_; }
        void setInnerAngle(float value);

        inline float power() const { return power_; }
        void setPower(float value);

        APropertyValue propertyRadiusGet(const std::string&) const { return radius(); }
        void propertyRadiusSet(const std::string&, const APropertyValue& value) { setRadius(value.toFloat()); }

        APropertyValue propertyAngleGet(const std::string&) const { return angle(); }
        void propertyAngleSet(const std::string&, const APropertyValue& value) { setAngle(value.toFloat()); }

        APropertyValue propertyInnerAngleGet(const std::string&) const { return innerAngle(); }
        void propertyInnerAngleSet(const std::string&, const APropertyValue& value) { setInnerAngle(value.toFloat()); }

        APropertyValue propertyPowerGet(const std::string&) const { return power(); }
        void propertyPowerSet(const std::string&, const APropertyValue& value) { setPower(value.toFloat()); }

    private:
        void doSetupCluster(ShaderClusterLight& cLight) const override;

        float radius_ = 0.0f;
        float angle_ = btRadians(45.0f);
        float innerAngle_ = btRadians(15.0f);
        float power_ = 1.0f;
    };

    using SpotLightPtr = std::shared_ptr<SpotLight>;

    ACLASS_DECLARE(SpotLight)
}

#endif

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

#ifndef _JOINTHINGE_H_
#define _JOINTHINGE_H_

#include "Joint.h"

namespace af3d
{
    class JointHinge : public std::enable_shared_from_this<JointHinge>,
        public Joint
    {
    public:
        explicit JointHinge(const SceneObjectPtr& objectA, const SceneObjectPtr& objectB,
            bool collideConnected = false);
        ~JointHinge() = default;

        static const AClass& staticKlass();

        static AObjectPtr create(const APropertyValueMap& propVals);

        AObjectPtr sharedThis() override { return shared_from_this(); }

        btHingeConstraint* constraint() override { return constraint_; }

        void render(bool drawA, PhysicsDebugDraw& dd, const btVector3& c, float sz) override;

        inline const btTransform& frameA() const { return frameA_; }
        void setFrameA(const btTransform& value);

        inline const btTransform& frameB() const { return frameB_; }
        void setFrameB(const btTransform& value);

        inline float lowerLimit() const { return lowerLimit_; }
        void setLowerLimit(float value);

        inline float upperLimit() const { return upperLimit_; }
        void setUpperLimit(float value);

        inline float softness() const { return softness_; }
        void setSoftness(float value);

        inline float biasFactor() const { return biasFactor_; }
        void setBiasFactor(float value);

        inline float relaxationFactor() const { return relaxationFactor_; }
        void setRelaxationFactor(float value);

        btTransform worldFrameA() const;
        void setWorldFrameA(const btTransform& value);

        btTransform worldFrameB() const;
        void setWorldFrameB(const btTransform& value);

        inline bool motorEnabled() const { return motorEnabled_; }
        void enableMotor(bool value);

        inline float maxMotorImpulse() const { return maxMotorImpulse_; }
        void setMaxMotorImpulse(float value);

        inline float motorVelocity() const { return motorVelocity_; }
        void setMotorVelocity(float value);

        APropertyValue propertyLocalFrameAGet(const std::string&) const { return frameA(); }
        void propertyLocalFrameASet(const std::string&, const APropertyValue& value) { setFrameA(value.toTransform()); }

        APropertyValue propertyLocalFrameBGet(const std::string&) const { return frameB(); }
        void propertyLocalFrameBSet(const std::string&, const APropertyValue& value) { setFrameB(value.toTransform()); }

        APropertyValue propertyWorldFrameAGet(const std::string&) const { return worldFrameA(); }
        void propertyWorldFrameASet(const std::string&, const APropertyValue& value) { setWorldFrameA(value.toTransform()); }

        APropertyValue propertyWorldFrameBGet(const std::string&) const { return worldFrameB(); }
        void propertyWorldFrameBSet(const std::string&, const APropertyValue& value) { setWorldFrameB(value.toTransform()); }

        APropertyValue propertyLowerLimitGet(const std::string&) const { return lowerLimit(); }
        void propertyLowerLimitSet(const std::string&, const APropertyValue& value) { setLowerLimit(value.toFloat()); }

        APropertyValue propertyUpperLimitGet(const std::string&) const { return upperLimit(); }
        void propertyUpperLimitSet(const std::string&, const APropertyValue& value) { setUpperLimit(value.toFloat()); }

        APropertyValue propertySoftnessGet(const std::string&) const { return softness(); }
        void propertySoftnessSet(const std::string&, const APropertyValue& value) { setSoftness(value.toFloat()); }

        APropertyValue propertyBiasFactorGet(const std::string&) const { return biasFactor(); }
        void propertyBiasFactorSet(const std::string&, const APropertyValue& value) { setBiasFactor(value.toFloat()); }

        APropertyValue propertyRelaxationFactorGet(const std::string&) const { return relaxationFactor(); }
        void propertyRelaxationFactorSet(const std::string&, const APropertyValue& value) { setRelaxationFactor(value.toFloat()); }

        APropertyValue propertyMotorEnabledGet(const std::string&) const { return motorEnabled(); }
        void propertyMotorEnabledSet(const std::string&, const APropertyValue& value) { enableMotor(value.toBool()); }

        APropertyValue propertyMaxMotorImpulseGet(const std::string&) const { return maxMotorImpulse(); }
        void propertyMaxMotorImpulseSet(const std::string&, const APropertyValue& value) { setMaxMotorImpulse(value.toFloat()); }

        APropertyValue propertyMotorVelocityGet(const std::string&) const { return motorVelocity(); }
        void propertyMotorVelocitySet(const std::string&, const APropertyValue& value) { setMotorVelocity(value.toFloat()); }

    private:
        void doRefresh(bool forceDelete) override;

        void doAdopt(bool withEdit) override;

        void doAbandon() override;

        static const btTransform& fixup();
        btTransform frameAWithFixup() const;
        btTransform frameBWithFixup() const;

        btTransform frameA_ = btTransform::getIdentity();
        btTransform frameB_ = btTransform::getIdentity();
        float lowerLimit_ = SIMD_HALF_PI;
        float upperLimit_ = -SIMD_HALF_PI;
        float softness_ = 0.9f;
        float biasFactor_ = 0.3f;
        float relaxationFactor_ = 1.0f;

        bool motorEnabled_ = false;
        float maxMotorImpulse_ = -1.0f;
        float motorVelocity_ = 0.0f;

        btHingeConstraint* constraint_ = nullptr;

        SceneObjectPtr editA_;
        SceneObjectPtr editB_;
    };

    using JointHingePtr = std::shared_ptr<JointHinge>;

    ACLASS_DECLARE(JointHinge)
}

#endif

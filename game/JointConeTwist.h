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

#ifndef _JOINTCONETWIST_H_
#define _JOINTCONETWIST_H_

#include "Joint.h"

namespace af3d
{
    class JointConeTwist : public std::enable_shared_from_this<JointConeTwist>,
        public Joint
    {
    public:
        explicit JointConeTwist(const SceneObjectPtr& objectA, const SceneObjectPtr& objectB = SceneObjectPtr(),
            bool collideConnected = false);
        ~JointConeTwist() = default;

        static const AClass& staticKlass();

        static AObjectPtr create(const APropertyValueMap& propVals);

        AObjectPtr sharedThis() override { return shared_from_this(); }

        btConeTwistConstraint* constraint() override { return constraint_; }

        inline const btTransform& frameA() const { return frameA_; }
        void setFrameA(const btTransform& value);

        inline const btTransform& frameB() const { return frameB_; }
        void setFrameB(const btTransform& value);

        inline float swing1() const { return swing1_; }
        void setSwing1(float value);

        inline float swing2() const { return swing2_; }
        void setSwing2(float value);

        inline float twist() const { return twist_; }
        void setTwist(float value);

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

        APropertyValue propertyLocalFrameAGet(const std::string&) const { return frameA(); }
        void propertyLocalFrameASet(const std::string&, const APropertyValue& value) { setFrameA(value.toTransform()); }

        APropertyValue propertyLocalFrameBGet(const std::string&) const { return frameB(); }
        void propertyLocalFrameBSet(const std::string&, const APropertyValue& value) { setFrameB(value.toTransform()); }

        APropertyValue propertyWorldFrameAGet(const std::string&) const { return worldFrameA(); }
        void propertyWorldFrameASet(const std::string&, const APropertyValue& value) { setWorldFrameA(value.toTransform()); }

        APropertyValue propertyWorldFrameBGet(const std::string&) const { return worldFrameB(); }
        void propertyWorldFrameBSet(const std::string&, const APropertyValue& value) { setWorldFrameB(value.toTransform()); }

        APropertyValue propertySwing1Get(const std::string&) const { return swing1(); }
        void propertySwing1Set(const std::string&, const APropertyValue& value) { setSwing1(value.toFloat()); }

        APropertyValue propertySwing2Get(const std::string&) const { return swing2(); }
        void propertySwing2Set(const std::string&, const APropertyValue& value) { setSwing2(value.toFloat()); }

        APropertyValue propertyTwistGet(const std::string&) const { return twist(); }
        void propertyTwistSet(const std::string&, const APropertyValue& value) { setTwist(value.toFloat()); }

        APropertyValue propertySoftnessGet(const std::string&) const { return softness(); }
        void propertySoftnessSet(const std::string&, const APropertyValue& value) { setSoftness(value.toFloat()); }

        APropertyValue propertyBiasFactorGet(const std::string&) const { return biasFactor(); }
        void propertyBiasFactorSet(const std::string&, const APropertyValue& value) { setBiasFactor(value.toFloat()); }

        APropertyValue propertyRelaxationFactorGet(const std::string&) const { return relaxationFactor(); }
        void propertyRelaxationFactorSet(const std::string&, const APropertyValue& value) { setRelaxationFactor(value.toFloat()); }

    private:
        void doRefresh(bool forceDelete) override;

        void doAdopt(bool withEdit) override;

        void doAbandon() override;

        btTransform frameA_ = btTransform::getIdentity();
        btTransform frameB_ = btTransform::getIdentity();
        float swing1_ = 0.0f;
        float swing2_ = 0.0f;
        float twist_ = 0.0f;
        float softness_ = 0.0f;
        float biasFactor_ = 0.0f;
        float relaxationFactor_ = 0.0f;

        btConeTwistConstraint* constraint_ = nullptr;

        SceneObjectPtr editA_;
        SceneObjectPtr editB_;
    };

    using JointConeTwistPtr = std::shared_ptr<JointConeTwist>;

    ACLASS_DECLARE(JointConeTwist)
}

#endif

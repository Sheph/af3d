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

#ifndef _JOINT6DOF_H_
#define _JOINT6DOF_H_

#include "Joint.h"

namespace af3d
{
    #define AF3D_JOINT_6DOF_ANGULAR_PROPS(X) \
        APropertyValue propertyLowerAngular##X##LimitGet(const std::string&) const { return lowerAngularLimit(Axis##X); } \
        void propertyLowerAngular##X##LimitSet(const std::string&, const APropertyValue& value) { setLowerAngularLimit(Axis##X, value.toFloat()); } \
        APropertyValue propertyUpperAngular##X##LimitGet(const std::string&) const { return upperAngularLimit(Axis##X); } \
        void propertyUpperAngular##X##LimitSet(const std::string&, const APropertyValue& value) { setUpperAngularLimit(Axis##X, value.toFloat()); } \
        APropertyValue propertyAngular##X##SoftnessGet(const std::string&) const { return angularSoftness(Axis##X); } \
        void propertyAngular##X##SoftnessSet(const std::string&, const APropertyValue& value) { setAngularSoftness(Axis##X, value.toFloat()); } \
        APropertyValue propertyAngular##X##DampingGet(const std::string&) const { return angularDamping(Axis##X); } \
        void propertyAngular##X##DampingSet(const std::string&, const APropertyValue& value) { setAngularDamping(Axis##X, value.toFloat()); } \
        APropertyValue propertyAngular##X##RestitutionGet(const std::string&) const { return angularRestitution(Axis##X); } \
        void propertyAngular##X##RestitutionSet(const std::string&, const APropertyValue& value) { setAngularRestitution(Axis##X, value.toFloat()); } \
        APropertyValue propertyAngular##X##MotorEnabledGet(const std::string&) const { return angularMotorEnabled(Axis##X); } \
        void propertyAngular##X##MotorEnabledSet(const std::string&, const APropertyValue& value) { enableAngularMotor(Axis##X, value.toBool()); } \
        APropertyValue propertyAngular##X##MotorVelocityGet(const std::string&) const { return angularMotorVelocity(Axis##X); } \
        void propertyAngular##X##MotorVelocitySet(const std::string&, const APropertyValue& value) { setAngularMotorVelocity(Axis##X, value.toFloat()); } \
        APropertyValue propertyMaxAngular##X##MotorForceGet(const std::string&) const { return maxAngularMotorForce(Axis##X); } \
        void propertyMaxAngular##X##MotorForceSet(const std::string&, const APropertyValue& value) { setMaxAngularMotorForce(Axis##X, value.toFloat()); } \
        APropertyValue propertyMaxAngular##X##LimitForceGet(const std::string&) const { return maxAngularLimitForce(Axis##X); } \
        void propertyMaxAngular##X##LimitForceSet(const std::string&, const APropertyValue& value) { setMaxAngularLimitForce(Axis##X, value.toFloat()); }

    class Joint6DOF : public std::enable_shared_from_this<Joint6DOF>,
        public Joint
    {
    public:
        enum Axis
        {
            AxisX = 0,
            AxisY = 1,
            AxisZ = 2
        };

        explicit Joint6DOF(const SceneObjectPtr& objectA, const SceneObjectPtr& objectB,
            bool collideConnected = false);
        ~Joint6DOF() = default;

        static const AClass& staticKlass();

        static AObjectPtr create(const APropertyValueMap& propVals);

        AObjectPtr sharedThis() override { return shared_from_this(); }

        btGeneric6DofConstraint* constraint() override { return constraint_; }

        void render(bool drawA, PhysicsDebugDraw& dd, const btVector3& c, float sz) override;

        inline const btTransform& frameA() const { return frameA_; }
        void setFrameA(const btTransform& value);

        inline const btTransform& frameB() const { return frameB_; }
        void setFrameB(const btTransform& value);

        inline const btVector3& lowerLinearLimit() const { return lowerLinearLimit_; }
        void setLowerLinearLimit(const btVector3& value);

        inline const btVector3& upperLinearLimit() const { return upperLinearLimit_; }
        void setUpperLinearLimit(const btVector3& value);

        inline float linearSoftness() const { return linearSoftness_; }
        void setLinearSoftness(float value);

        inline float linearDamping() const { return linearDamping_; }
        void setLinearDamping(float value);

        inline float linearRestitution() const { return linearRestitution_; }
        void setLinearRestitution(float value);

        inline bool linearMotorEnabled(Axis axis) const { return linearMotorEnabled_[axis]; }
        void enableLinearMotor(Axis axis, bool value);

        inline const btVector3& linearMotorVelocity() const { return linearMotorVelocity_; }
        void setLinearMotorVelocity(const btVector3& value);

        inline const btVector3& maxLinearMotorForce() const { return maxLinearMotorForce_; }
        void setMaxLinearMotorForce(const btVector3& value);

        inline float lowerAngularLimit(Axis axis) const { return angularCfg_[axis].lowerLimit; }
        void setLowerAngularLimit(Axis axis, float value);

        inline float upperAngularLimit(Axis axis) const { return angularCfg_[axis].upperLimit; }
        void setUpperAngularLimit(Axis axis, float value);

        inline float angularSoftness(Axis axis) const { return angularCfg_[axis].softness; }
        void setAngularSoftness(Axis axis, float value);

        inline float angularDamping(Axis axis) const { return angularCfg_[axis].damping; }
        void setAngularDamping(Axis axis, float value);

        inline float angularRestitution(Axis axis) const { return angularCfg_[axis].restitution; }
        void setAngularRestitution(Axis axis, float value);

        inline bool angularMotorEnabled(Axis axis) const { return angularCfg_[axis].motorEnabled; }
        void enableAngularMotor(Axis axis, bool value);

        inline float angularMotorVelocity(Axis axis) const { return angularCfg_[axis].motorVelocity; }
        void setAngularMotorVelocity(Axis axis, float value);

        inline float maxAngularMotorForce(Axis axis) const { return angularCfg_[axis].maxMotorForce; }
        void setMaxAngularMotorForce(Axis axis, float value);

        inline float maxAngularLimitForce(Axis axis) const { return angularCfg_[axis].maxLimitForce; }
        void setMaxAngularLimitForce(Axis axis, float value);

        inline bool linearSpringEnabled(Axis axis) const { return linearSpringEnabled_[axis]; }
        void enableLinearSpring(Axis axis, bool value);

        inline bool angularSpringEnabled(Axis axis) const { return angularSpringEnabled_[axis]; }
        void enableAngularSpring(Axis axis, bool value);

        inline const btVector3& linearSpringStiffness() const { return linearSpringStiffness_; }
        void setLinearSpringStiffness(const btVector3& value);

        inline const btVector3& angularSpringStiffness() const { return angularSpringStiffness_; }
        void setAngularSpringStiffness(const btVector3& value);

        inline const btVector3& linearSpringDamping() const { return linearSpringDamping_; }
        void setLinearSpringDamping(const btVector3& value);

        inline const btVector3& angularSpringDamping() const { return angularSpringDamping_; }
        void setAngularSpringDamping(const btVector3& value);

        inline const btTransform& springEquilibriumPoint() const { return springEquilibriumPoint_; }
        void setSpringEquilibriumPoint(const btTransform& value);

        btTransform worldSpringEquilibriumPoint() const;
        void setWorldSpringEquilibriumPoint(const btTransform& value);

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

        APropertyValue propertyLowerLinearLimitGet(const std::string&) const { return lowerLinearLimit(); }
        void propertyLowerLinearLimitSet(const std::string&, const APropertyValue& value) { setLowerLinearLimit(value.toVec3()); }

        APropertyValue propertyUpperLinearLimitGet(const std::string&) const { return upperLinearLimit(); }
        void propertyUpperLinearLimitSet(const std::string&, const APropertyValue& value) { setUpperLinearLimit(value.toVec3()); }

        APropertyValue propertyLinearSoftnessGet(const std::string&) const { return linearSoftness(); }
        void propertyLinearSoftnessSet(const std::string&, const APropertyValue& value) { setLinearSoftness(value.toFloat()); }

        APropertyValue propertyLinearDampingGet(const std::string&) const { return linearDamping(); }
        void propertyLinearDampingSet(const std::string&, const APropertyValue& value) { setLinearDamping(value.toFloat()); }

        APropertyValue propertyLinearRestitutionGet(const std::string&) const { return linearRestitution(); }
        void propertyLinearRestitutionSet(const std::string&, const APropertyValue& value) { setLinearRestitution(value.toFloat()); }

        APropertyValue propertyLinearXMotorEnabledGet(const std::string&) const { return linearMotorEnabled(AxisX); }
        void propertyLinearXMotorEnabledSet(const std::string&, const APropertyValue& value) { enableLinearMotor(AxisX, value.toBool()); }

        APropertyValue propertyLinearYMotorEnabledGet(const std::string&) const { return linearMotorEnabled(AxisY); }
        void propertyLinearYMotorEnabledSet(const std::string&, const APropertyValue& value) { enableLinearMotor(AxisY, value.toBool()); }

        APropertyValue propertyLinearZMotorEnabledGet(const std::string&) const { return linearMotorEnabled(AxisZ); }
        void propertyLinearZMotorEnabledSet(const std::string&, const APropertyValue& value) { enableLinearMotor(AxisZ, value.toBool()); }

        APropertyValue propertyLinearMotorVelocityGet(const std::string&) const { return linearMotorVelocity(); }
        void propertyLinearMotorVelocitySet(const std::string&, const APropertyValue& value) { setLinearMotorVelocity(value.toVec3()); }

        APropertyValue propertyMaxLinearMotorForceGet(const std::string&) const { return maxLinearMotorForce(); }
        void propertyMaxLinearMotorForceSet(const std::string&, const APropertyValue& value) { setMaxLinearMotorForce(value.toVec3()); }

        AF3D_JOINT_6DOF_ANGULAR_PROPS(X)
        AF3D_JOINT_6DOF_ANGULAR_PROPS(Y)
        AF3D_JOINT_6DOF_ANGULAR_PROPS(Z)

        APropertyValue propertyLinearXSpringEnabledGet(const std::string&) const { return linearSpringEnabled(AxisX); }
        void propertyLinearXSpringEnabledSet(const std::string&, const APropertyValue& value) { enableLinearSpring(AxisX, value.toBool()); }

        APropertyValue propertyLinearYSpringEnabledGet(const std::string&) const { return linearSpringEnabled(AxisY); }
        void propertyLinearYSpringEnabledSet(const std::string&, const APropertyValue& value) { enableLinearSpring(AxisY, value.toBool()); }

        APropertyValue propertyLinearZSpringEnabledGet(const std::string&) const { return linearSpringEnabled(AxisZ); }
        void propertyLinearZSpringEnabledSet(const std::string&, const APropertyValue& value) { enableLinearSpring(AxisZ, value.toBool()); }

        APropertyValue propertyAngularXSpringEnabledGet(const std::string&) const { return angularSpringEnabled(AxisX); }
        void propertyAngularXSpringEnabledSet(const std::string&, const APropertyValue& value) { enableAngularSpring(AxisX, value.toBool()); }

        APropertyValue propertyAngularYSpringEnabledGet(const std::string&) const { return angularSpringEnabled(AxisY); }
        void propertyAngularYSpringEnabledSet(const std::string&, const APropertyValue& value) { enableAngularSpring(AxisY, value.toBool()); }

        APropertyValue propertyAngularZSpringEnabledGet(const std::string&) const { return angularSpringEnabled(AxisZ); }
        void propertyAngularZSpringEnabledSet(const std::string&, const APropertyValue& value) { enableAngularSpring(AxisZ, value.toBool()); }

        APropertyValue propertyLinearSpringStiffnessGet(const std::string&) const { return linearSpringStiffness(); }
        void propertyLinearSpringStiffnessSet(const std::string&, const APropertyValue& value) { setLinearSpringStiffness(value.toVec3()); }

        APropertyValue propertyAngularSpringStiffnessGet(const std::string&) const { return angularSpringStiffness(); }
        void propertyAngularSpringStiffnessSet(const std::string&, const APropertyValue& value) { setAngularSpringStiffness(value.toVec3()); }

        APropertyValue propertyLinearSpringDampingGet(const std::string&) const { return linearSpringDamping(); }
        void propertyLinearSpringDampingSet(const std::string&, const APropertyValue& value) { setLinearSpringDamping(value.toVec3()); }

        APropertyValue propertyAngularSpringDampingGet(const std::string&) const { return angularSpringDamping(); }
        void propertyAngularSpringDampingSet(const std::string&, const APropertyValue& value) { setAngularSpringDamping(value.toVec3()); }

        APropertyValue propertyLocalSpringEquilibriumPointGet(const std::string&) const { return springEquilibriumPoint(); }
        void propertyLocalSpringEquilibriumPointSet(const std::string&, const APropertyValue& value) { setSpringEquilibriumPoint(value.toTransform()); }

        APropertyValue propertyWorldSpringEquilibriumPointGet(const std::string&) const { return worldSpringEquilibriumPoint(); }
        void propertyWorldSpringEquilibriumPointSet(const std::string&, const APropertyValue& value) { setWorldSpringEquilibriumPoint(value.toTransform()); }

    private:
        struct AngularConfig
        {
            float lowerLimit = 1.0f;
            float upperLimit = -1.0f;

            float softness = 0.5f;
            float damping = 1.0f;
            float restitution = 0.0f;

            bool motorEnabled = false;
            float motorVelocity = 0.0f;
            float maxMotorForce = 6.0f;
            float maxLimitForce = 300.0f;
        };

        void doRefresh(bool forceDelete) override;

        void doAdopt(bool withEdit) override;

        void doAbandon() override;

        btTransform frameA_ = btTransform::getIdentity();
        btTransform frameB_ = btTransform::getIdentity();
        btVector3 lowerLinearLimit_ = btVector3_zero;
        btVector3 upperLinearLimit_ = btVector3_zero;
        float linearSoftness_ = 0.7f;
        float linearDamping_ = 1.0f;
        float linearRestitution_ = 0.5f;

        bool linearMotorEnabled_[3] = {false, false, false};
        btVector3 linearMotorVelocity_ = btVector3_zero;
        btVector3 maxLinearMotorForce_ = btVector3_zero;

        AngularConfig angularCfg_[3];

        bool linearSpringEnabled_[3] = {false, false, false};
        bool angularSpringEnabled_[3] = {false, false, false};

        btVector3 linearSpringStiffness_ = btVector3_zero;
        btVector3 angularSpringStiffness_ = btVector3_zero;

        btVector3 linearSpringDamping_ = btVector3(1.0f, 1.0f, 1.0f);
        btVector3 angularSpringDamping_ = btVector3(1.0f, 1.0f, 1.0f);

        btTransform springEquilibriumPoint_ = btTransform::getIdentity();

        btGeneric6DofSpringConstraint* constraint_ = nullptr;

        SceneObjectPtr editA_;
        SceneObjectPtr editB_;
    };

    using Joint6DOFPtr = std::shared_ptr<Joint6DOF>;

    ACLASS_DECLARE(Joint6DOF)
}

#endif

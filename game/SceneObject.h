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

#ifndef _SCENEOBJECT_H_
#define _SCENEOBJECT_H_

#include "SceneObjectManager.h"
#include "Component.h"
#include "bullet/btBulletDynamicsCommon.h"
#include <memory>

namespace af3d
{
    class MotionState;

    enum class BodyType
    {
        Static = 0,
        Kinematic = 1,
        Dynamic = 2,
        Max = Dynamic
    };

    extern const APropertyTypeEnumImpl<BodyType> APropertyType_BodyType;

    class SceneObject : public std::enable_shared_from_this<SceneObject>,
        public SceneObjectManager
    {
    public:
        SceneObject();
        ~SceneObject();

        static const AClass& staticKlass();

        static AObjectPtr create(const APropertyValueMap& propVals);

        static SceneObjectPtr createWithParams(const AClass& klass, const APropertyValueMap& propVals, AClass::CreateFn fn);

        AObjectPtr sharedThis() override { return shared_from_this(); }

        static SceneObject* fromBody(btRigidBody* body);

        static SceneObject* fromShape(btCollisionShape* shape);

        static btCollisionShape* getBodyShape(btCollisionObject* body, int childIdx);

        static const btCollisionShape* getBodyShape(const btCollisionObject* body, int childIdx);

        void addComponent(const ComponentPtr& component);

        void removeComponent(const ComponentPtr& component);

        inline const std::vector<ComponentPtr>& components() const { return components_; }

        template <class T>
        inline std::shared_ptr<T> findComponent() const
        {
            for (const auto& c : components_) {
                const auto& ct = aobjectCast<T>(c);
                if (ct) {
                    return ct;
                }
            }
            return std::shared_ptr<T>();
        }

        template <class T>
        std::shared_ptr<T> findComponentByName(const std::string& name) const
        {
            for (const auto& c : components_) {
                const auto& ct = aobjectCast<T>(c);
                if (ct && (ct->name() == name)) {
                    return ct;
                }
            }
            return std::shared_ptr<T>();
        }

        template <class T>
        std::vector<std::shared_ptr<T>> findComponents(const std::string& name) const
        {
            std::vector<std::shared_ptr<T>> res;

            for (const auto& c : components_) {
                const auto& ct = aobjectCast<T>(c);
                if (ct && (ct->name() == name)) {
                    res.push_back(ct);
                }
            }

            return res;
        }

        template <class T>
        std::vector<std::shared_ptr<T>> findComponents() const
        {
            std::vector<std::shared_ptr<T>> res;

            for (const auto& c : components_) {
                const auto& ct = aobjectCast<T>(c);
                if (ct) {
                    res.push_back(ct);
                }
            }

            return res;
        }

        SceneObject* parentObject();

        SceneObjectPtr script_parentObject();

        void removeFromParent();

        void removeFromParentRecursive();

        inline SceneObjectType type() const { return type_; }
        void setType(SceneObjectType value) { type_ = value; }

        inline btRigidBody* body() { return body_; }
        inline const btRigidBody* body() const { return body_; }
        void setBody(btRigidBody* value);

        BodyType bodyType() const;
        void setBodyType(BodyType value);

        bool isSensor() const;
        void setIsSensor(bool value);

        const btTransform& transform() const;
        void setTransform(const btVector3& pos, const btQuaternion& rot);
        void setTransform(const btTransform& t);

        void setTransformRecursive(const btVector3& pos, const btQuaternion& rot);
        void setTransformRecursive(const btTransform& t);

        const btTransform& smoothTransform() const;

        const btVector3& pos() const;
        void setPos(const btVector3& value);

        const btVector3& smoothPos() const;

        const btMatrix3x3& basis() const;
        void setBasis(const btMatrix3x3& value);

        const btMatrix3x3& smoothBasis() const;

        btQuaternion rotation() const;
        void setRotation(const btQuaternion& value);

        btQuaternion smoothRotation() const;

        const btTransform& worldCenter() const;
        const btTransform& localCenter() const;
        void setLocalCenter(const btTransform& t);

        float mass() const;

        btVector3 scaleByMomentOfInertia(const btVector3& v) const;

        const btVector3& linearVelocity() const;
        void setLinearVelocity(const btVector3& value);

        const btVector3& angularVelocity() const;
        void setAngularVelocity(const btVector3& value);

        float linearDamping() const;
        void setLinearDamping(float value);

        float angularDamping() const;
        void setAngularDamping(float value);

        float friction() const;
        void setFriction(float value);

        float restitution() const;
        void setRestitution(float value);

        void applyForce(const btVector3& force, const btVector3& point);
        void applyForceToCenter(const btVector3& force);
        void applyTorque(const btVector3& torque);
        void applyLinearImpulse(const btVector3& impulse, const btVector3& point);
        void applyAngularImpulse(const btVector3& impulse);

        bool physicsActive() const;
        void setPhysicsActive(bool value);

        btVector3 getWorldPoint(const btVector3& localPoint) const;

        btVector3 getLocalPoint(const btVector3& worldPoint) const;

        btVector3 getSmoothWorldPoint(const btVector3& localPoint) const;

        btVector3 getSmoothLocalPoint(const btVector3& worldPoint) const;

        btVector3 getForward() const;

        btVector3 getRight() const;

        btVector3 getUp() const;

        btVector3 getSmoothForward() const;

        btVector3 getSmoothRight() const;

        btVector3 getSmoothUp() const;

        btVector3 getLinearVelocityFromWorldPoint(const btVector3& worldPoint) const;

        btVector3 getLinearVelocityFromLocalPoint(const btVector3& localPoint) const;

        inline bool freezable() const { return flags_[Flag::Freezable]; }
        inline void setFreezable(bool value) { flags_[Flag::Freezable] = value; }

        inline float freezeRadius() const { return freezeRadius_; }
        inline void setFreezeRadius(float value) { freezeRadius_ = value; }

        inline bool freezePhysics() const { return flags_[Flag::FreezePhysics]; }
        inline void setFreezePhysics(bool value) { flags_[Flag::FreezePhysics] = value; }

        inline bool frozen() const { return flags_[Flag::Frozen]; }

        void collectIslandObjects(std::unordered_set<SceneObjectPtr>& objs);

        bool collidesWith(btCollisionObject* other);

        inline const APropertyValueMap& params() const { return params_; }
        inline void setParams(const APropertyValueMap& value) { params_ = value; }

        /*
         * Internal, do not call.
         * @{
         */
        void freeze();
        void thaw();

        /*
         * @}
         */

        APropertyValue propertyTransformGet(const std::string&) const { return transform(); }
        void propertyTransformSet(const std::string&, const APropertyValue& value) { setTransformRecursive(value.toTransform()); }

        APropertyValue propertyTypeGet(const std::string&) const { return static_cast<int>(type()); }
        void propertyTypeSet(const std::string&, const APropertyValue& value) { setType(static_cast<SceneObjectType>(value.toInt())); }

        APropertyValue propertyPhysicsActiveGet(const std::string&) const { return physicsActive(); }
        void propertyPhysicsActiveSet(const std::string&, const APropertyValue& value) { setPhysicsActive(value.toBool()); }

        APropertyValue propertyBodyTypeGet(const std::string&) const { return static_cast<int>(bodyType()); }
        void propertyBodyTypeSet(const std::string&, const APropertyValue& value) { setBodyType(static_cast<BodyType>(value.toInt())); }

        APropertyValue propertyIsSensorGet(const std::string&) const { return isSensor(); }
        void propertyIsSensorSet(const std::string&, const APropertyValue& value) { setIsSensor(value.toBool()); }

        APropertyValue propertyMassGet(const std::string&) const { return mass(); }

        APropertyValue propertyFrictionGet(const std::string&) const { return friction(); }
        void propertyFrictionSet(const std::string&, const APropertyValue& value) { setFriction(value.toFloat()); }

        APropertyValue propertyRestitutionGet(const std::string&) const { return restitution(); }
        void propertyRestitutionSet(const std::string&, const APropertyValue& value) { setRestitution(value.toFloat()); }

        APropertyValue propertyLinearDampingGet(const std::string&) const { return linearDamping(); }
        void propertyLinearDampingSet(const std::string&, const APropertyValue& value) { setLinearDamping(value.toFloat()); }

        APropertyValue propertyAngularDampingGet(const std::string&) const { return angularDamping(); }
        void propertyAngularDampingSet(const std::string&, const APropertyValue& value) { setAngularDamping(value.toFloat()); }

        APropertyValue propertyLinearVelocityGet(const std::string&) const { return linearVelocity(); }
        void propertyLinearVelocitySet(const std::string&, const APropertyValue& value) { setLinearVelocity(value.toVec3()); }

        APropertyValue propertyAngularVelocityGet(const std::string&) const { return angularVelocity(); }
        void propertyAngularVelocitySet(const std::string&, const APropertyValue& value) { setAngularVelocity(value.toVec3()); }

        APropertyValue propertyParamGet(const std::string& key) const { return params_.get(key); }

    private:
        enum class Flag
        {
            Freezable = 0,
            Frozen,
            FreezePhysics,
            Max = FreezePhysics
        };

        struct PhysicsBodyConstructionInfo
        {
            BodyType bodyType = BodyType::Static;
            bool isSensor = false;
            btTransform xf = btTransform::getIdentity();
            bool active = true;
            float linearDamping = 0.0f;
            float angularDamping = 0.0f;
            float friction = 0.5f;
            float restitution = 0.0f;
            btVector3 linearVelocity = btVector3_zero;
            btVector3 angularVelocity = btVector3_zero;
        };

        using Flags = EnumSet<Flag>;

        std::vector<AObjectPtr> getChildren() const override;

        void setChildren(const std::vector<AObjectPtr>& value) override;

        SceneObjectType type_ = SceneObjectType::Other;

        mutable PhysicsBodyConstructionInfo bodyCi_;
        btRigidBody* body_ = nullptr;
        MotionState* bodyMs_ = nullptr;

        float freezeRadius_ = 0.0f;

        std::vector<ComponentPtr> components_;

        Flags flags_;

        APropertyValueMap params_;
    };

    ACLASS_DECLARE(SceneObject)

    #define SCENEOBJECT_DEFINE_BEGIN(Name) \
        extern const AClass AClass_SceneObject##Name; \
        static SceneObjectPtr SceneObject##Name##create(const APropertyValueMap& params)

    #define SCENEOBJECT_DEFINE_PROPS_NO_RESTRICT(Name) \
        static AObjectPtr SceneObject##Name##createWrapper(const APropertyValueMap& propVals) \
        { \
            return SceneObject::createWithParams(AClass_SceneObject##Name, propVals, (AClass::CreateFn)&SceneObject##Name##create); \
        } \
        const AClass AClass_SceneObject##Name{"SceneObject" #Name, AClass_SceneObject, &SceneObject##Name##createWrapper, { \

    #define SCENEOBJECT_DEFINE_PROPS(Name) \
        SCENEOBJECT_DEFINE_PROPS_NO_RESTRICT(Name) \
        ACLASS_PROPERTY_RO(SceneObject, Type, "type", "Scene object type", SceneObjectType, General, APropertyEditable|APropertyTransient) \
        ACLASS_PROPERTY_RO(SceneObject, PhysicsActive, AProperty_PhysicsActive, "Physics is active", Bool, Physics, APropertyEditable|APropertyTransient) \
        ACLASS_PROPERTY_RO(SceneObject, BodyType, "body type", "Physics body type", BodyType, Physics, APropertyEditable|APropertyTransient) \
        ACLASS_PROPERTY_RO(SceneObject, IsSensor, "is sensor", "Is sensor", Bool, Physics, APropertyEditable|APropertyTransient) \
        ACLASS_PROPERTY_RO(SceneObject, Friction, "friction", "Friction", Float, Physics, APropertyEditable|APropertyTransient) \
        ACLASS_PROPERTY_RO(SceneObject, Restitution, "restitution", "Restitution", Float, Physics, APropertyEditable|APropertyTransient) \
        ACLASS_PROPERTY_RO(SceneObject, LinearDamping, "linear damping", "Linear damping", Float, Physics, APropertyEditable|APropertyTransient) \
        ACLASS_PROPERTY_RO(SceneObject, AngularDamping, "angular damping", "Angular damping", Float, Physics, APropertyEditable|APropertyTransient)

    #define SCENEOBJECT_PARAM(SName, Name, Tooltip, Type, Def) \
        {Name, Tooltip, APropertyType_##Type, APropertyValue(Def), APropertyCategory::Params, APropertyReadable|APropertyEditable, (APropertyGetter)&SceneObject::propertyParamGet, nullptr},

    #define SCENEOBJECT_DEFINE_END(Name) }};
}

#endif

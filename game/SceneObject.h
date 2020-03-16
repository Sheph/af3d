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
    class SceneObject : public std::enable_shared_from_this<SceneObject>,
        public SceneObjectManager
    {
    public:
        SceneObject();
        ~SceneObject();

        inline std::uint32_t cookie() const { return cookie_; }

        inline const std::string& name() const { return name_; }
        inline void setName(const std::string& value) { name_ = value; }

        void addComponent(const ComponentPtr& component);

        void removeComponent(const ComponentPtr& component);

        inline const std::vector<ComponentPtr>& components() const { return components_; }

        template <class T>
        inline std::shared_ptr<T> findComponent() const
        {
            for (const auto& c : components_) {
                const auto& ct = std::dynamic_pointer_cast<T>(c);
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
                const auto& ct = std::dynamic_pointer_cast<T>(c);
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
                const auto& ct = std::dynamic_pointer_cast<T>(c);
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
                const auto& ct = std::dynamic_pointer_cast<T>(c);
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

        const btRigidBody::btRigidBodyConstructionInfo& bodyDef() const { return bodyDef_; }
        void setBodyDef(const btRigidBody::btRigidBodyConstructionInfo& value);

        const btTransform& transform() const;
        void setTransform(const btVector3& pos, const btQuaternion& rot);
        void setTransform(const btTransform& t);

        //void setTransformRecursive(const btVector3& pos, const btQuaternion& rot);

        //void setTransformRecursive(const btTransform& t);

        const btVector3& pos() const;
        void setPos(const btVector3& value);
        //void setPosRecursive(const btVector3& value);

        //void setPosSmoothed(const btVector3& value);

        const btMatrix3x3& basis() const;
        void setBasis(const btMatrix3x3& value);

        btQuaternion rotation() const;
        void setRotation(const btQuaternion& value);
        //void setRotationRecursive(const btQuaternion& value);

        //void setRotationSmoothed(const btQuaternion& value);

        const btVector3& worldCenter() const;
        const btVector3& localCenter() const;

        float mass() const;

        btVector3 scaleByMomentOfInertia(const btVector3& v) const;

        const btVector3& linearVelocity() const;
        void setLinearVelocity(const btVector3& value);
        //void setLinearVelocityRecursive(const btVector3& value);

        const btVector3& angularVelocity() const;
        void setAngularVelocity(const btVector3& value);
        //void setAngularVelocityRecursive(const btVector3& value);

        float linearDamping() const;
        void setLinearDamping(float value);

        float angularDamping() const;
        void setAngularDamping(float value);

        void applyForce(const btVector3& force, const btVector3& point);
        void applyForceToCenter(const btVector3& force);
        void applyTorque(const btVector3& torque);
        void applyLinearImpulse(const btVector3& impulse, const btVector3& point);
        void applyAngularImpulse(const btVector3& impulse);

        bool physicsActive() const;
        void setPhysicsActive(bool value);
        //void setActiveRecursive(bool value);

        btVector3 getWorldPoint(const btVector3& localPoint) const;

        btVector3 getLocalPoint(const btVector3& worldPoint) const;

        //b2Vec2 getSmoothWorldPoint(const b2Vec2& localPoint) const;

        //b2Vec2 getSmoothLocalPoint(const b2Vec2& worldPoint) const;

        btVector3 getForward() const;

        btVector3 getRight() const;

        btVector3 getUp() const;

        //b2Vec2 getSmoothDirection(float length) const;

        btVector3 getLinearVelocityFromWorldPoint(const btVector3& worldPoint) const;

        btVector3 getLinearVelocityFromLocalPoint(const btVector3& localPoint) const;

        //void resetSmooth();
        //void updateSmooth(float fixedTimestepAccumulatorRatio);

        //b2Transform getSmoothTransform() const;

        //b2Vec2 smoothPos() const;

        //float smoothAngle() const;

        inline bool freezable() const { return flags_[Flag::Freezable]; }
        inline void setFreezable(bool value) { flags_[Flag::Freezable] = value; }

        inline float freezeRadius() const { return freezeRadius_; }
        inline void setFreezeRadius(float value) { freezeRadius_ = value; }

        inline bool freezePhysics() const { return flags_[Flag::FreezePhysics]; }
        inline void setFreezePhysics(bool value) { flags_[Flag::FreezePhysics] = value; }

        inline bool frozen() const { return flags_[Flag::Frozen]; }

        void collectIslandObjects(std::unordered_set<SceneObjectPtr>& objs);

        bool collidesWith(btCollisionObject* other);

        /*
         * Internal, do not call.
         * @{
         */
        void freeze();
        void thaw();

        /*
         * @}
         */

    private:
        enum class Flag
        {
            Freezable = 0,
            Frozen,
            FreezePhysics,
            Max = FreezePhysics
        };

        using Flags = EnumSet<Flag>;

        std::uint32_t cookie_;

        SceneObjectType type_;

        std::string name_;

        btRigidBody::btRigidBodyConstructionInfo bodyDef_;
        btRigidBody* body_;

        float freezeRadius_;

        std::vector<ComponentPtr> components_;

        Flags flags_;

        bool physicsActive_;
    };
}

#endif

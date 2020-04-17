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

#include "SceneObject.h"
#include "PhysicsBodyComponent.h"
#include "MotionState.h"
#include "Scene.h"
#include "Utils.h"
#include "Settings.h"
#include "Logger.h"

namespace af3d
{
    const APropertyTypeEnumImpl<BodyType> APropertyType_BodyType{"BodyType",
        {
            "Static",
            "Kinematic",
            "Dynamic"
        }
    };

    ACLASS_DEFINE_BEGIN(SceneObject, SceneObjectManager)
    ACLASS_PROPERTY(SceneObject, Transform, AProperty_WorldTransform, "World transform", Transform, btTransform::getIdentity(), Position, APropertyEditable)
    ACLASS_PROPERTY(SceneObject, Type, "type", "Scene object type", SceneObjectType, static_cast<int>(SceneObjectType::Other), General, APropertyEditable)
    ACLASS_PROPERTY(SceneObject, PhysicsActive, AProperty_PhysicsActive, "Physics is active", Bool, true, Physics, APropertyEditable)
    ACLASS_PROPERTY(SceneObject, BodyType, "body type", "Physics body type", BodyType, static_cast<int>(BodyType::Static), Physics, APropertyEditable)
    ACLASS_PROPERTY_RO(SceneObject, Mass, "mass", "Mass", Float, Physics, APropertyEditable|APropertyTransient)
    ACLASS_PROPERTY(SceneObject, Friction, "friction", "Friction", Float, 0.5f, Physics, APropertyEditable)
    ACLASS_PROPERTY(SceneObject, Restitution, "restitution", "Restitution", Float, 0.0f, Physics, APropertyEditable)
    ACLASS_PROPERTY(SceneObject, LinearDamping, "linear damping", "Linear damping", Float, 0.0f, Physics, APropertyEditable)
    ACLASS_PROPERTY(SceneObject, AngularDamping, "angular damping", "Angular damping", Float, 0.0f, Physics, APropertyEditable)
    ACLASS_PROPERTY(SceneObject, LinearVelocity, "linear velocity", "Linear velocity", Vec3f, btVector3(0.0f, 0.0f, 0.0f), Physics, APropertyEditable)
    ACLASS_PROPERTY(SceneObject, AngularVelocity, "angular velocity", "Angular velocity", Vec3f, btVector3(0.0f, 0.0f, 0.0f), Physics, APropertyEditable)
    ACLASS_DEFINE_END(SceneObject)

    static void insertComponent(std::vector<ComponentPtr>& components,
        const ComponentPtr& component)
    {
        for (const auto& c : components) {
            if (c == component) {
                return;
            }
        }

        components.push_back(component);
    }

    static bool eraseComponent(std::vector<ComponentPtr>& components,
        const ComponentPtr& component)
    {
        for (auto it = components.begin(); it != components.end(); ++it) {
            if (*it == component) {
                components.erase(it);
                return true;
            }
        }

        return false;
    }

    SceneObject::SceneObject()
    : SceneObjectManager(AClass_SceneObject)
    {
        setFreezePhysics(true);
    }

    SceneObject::~SceneObject()
    {
        btAssert(!scene());

        while (!components_.empty()) {
            ComponentPtr component = *components_.begin();

            removeComponent(component);
        }

        removeAllObjects();

        if (body_) {
            runtime_assert(!body_->isInWorld());
            CollisionShape::fromShape(body_->getCollisionShape())->resetUserPointer();
            body_->setCollisionShape(nullptr);
            delete bodyMs_;
            body_->setMotionState(nullptr);
            delete body_;
            body_ = nullptr;
            bodyMs_ = nullptr;
        }
    }

    const AClass& SceneObject::staticKlass()
    {
        return AClass_SceneObject;
    }

    AObjectPtr SceneObject::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<SceneObject>();
        obj->propertiesSet(propVals);
        return obj;
    }

    SceneObjectPtr SceneObject::createWithParams(const AClass& klass, const APropertyValueMap& propVals, AClass::CreateFn fn)
    {
        APropertyValueMap params;
        for (const auto& prop : klass.thisProperties()) {
            if (prop.category() == APropertyCategory::Params) {
                params.set(prop.name(), propVals.get(prop.name()));
            }
        }
        auto obj = std::static_pointer_cast<SceneObject>(fn(params));
        if (!obj) {
            return obj;
        }
        obj->propertiesSet(propVals);
        obj->setKlass(klass);
        if (settings.editor.enabled) {
            obj->setParams(params);
        }
        return obj;
    }

    SceneObject* SceneObject::fromBody(btRigidBody* body)
    {
        return static_cast<SceneObject*>(body->getUserPointer());
    }

    void SceneObject::addComponent(const ComponentPtr& component)
    {
        btAssert(!component->parent());

        insertComponent(components_, component);
        component->setParent(this);

        if (scene()) {
            scene()->registerComponent(component);
        }
    }

    void SceneObject::removeComponent(const ComponentPtr& component)
    {
        /*
         * Hold on to this component while
         * removing.
         */
        ComponentPtr tmp = component;

        if (eraseComponent(components_, tmp)) {
            if (scene()) {
                scene()->unregisterComponent(tmp);
            }

            tmp->setParent(nullptr);
        }
    }

    SceneObject* SceneObject::parentObject()
    {
        if (parent() != scene()) {
            return static_cast<SceneObject*>(parent());
        } else {
            return nullptr;
        }
    }

    SceneObjectPtr SceneObject::script_parentObject()
    {
        SceneObject* obj = parentObject();
        return obj ? obj->shared_from_this() : SceneObjectPtr();
    }

    void SceneObject::removeFromParent()
    {
        if (parent()) {
            parent()->removeObject(shared_from_this());
        }
    }

    void SceneObject::removeFromParentRecursive()
    {
        SceneObject* obj = this;

        while (obj->parentObject()) {
            obj = obj->parentObject();
        }

        obj->removeFromParent();
    }

    void SceneObject::setBody(btRigidBody* value)
    {
        btAssert(value);
        btAssert(!body_);

        body_ = value;
        body_->setUserPointer(this);
        bodyMs_ = static_cast<MotionState*>(body_->getMotionState());
        bodyCi_.active = body_->isInWorld();

        if (bodyType() == BodyType::Kinematic) {
            if (!bodyCi_.linearVelocity.fuzzyZero() || !bodyCi_.angularVelocity.fuzzyZero()) {
                // Non-zero velocity for kinematic body, disable deactivation so that it can move.
                body_->setActivationState(DISABLE_DEACTIVATION);
            }
        }
    }

    BodyType SceneObject::bodyType() const
    {
        if (body_) {
            if (body_->isKinematicObject()) {
                return BodyType::Kinematic;
            } else if (body_->isStaticObject()) {
                return BodyType::Static;
            } else {
                return BodyType::Dynamic;
            }
        } else {
            return bodyCi_.bodyType;
        }
    }

    void SceneObject::setBodyType(BodyType value)
    {
        if (body_) {
            if (value == bodyType()) {
                return;
            }
            if (bodyType() == BodyType::Kinematic) {
                // Was kinematic body, copy velocities and reset deactivation
                body_->setLinearVelocity(bodyCi_.linearVelocity);
                body_->setAngularVelocity(bodyCi_.angularVelocity);
                if (body_->getActivationState() == DISABLE_DEACTIVATION) {
                    body_->forceActivationState(ACTIVE_TAG);
                    body_->activate(true);
                }
            } else if (value == BodyType::Kinematic) {
                // Becomes kinematic, copy velocities
                bodyCi_.linearVelocity = body_->getLinearVelocity();
                bodyCi_.angularVelocity = body_->getAngularVelocity();
                if (!bodyCi_.linearVelocity.fuzzyZero() || !bodyCi_.angularVelocity.fuzzyZero()) {
                    // Non-zero velocity for kinematic body, disable deactivation so that it can move.
                    body_->setActivationState(DISABLE_DEACTIVATION);
                }
            }
            switch (value) {
            case BodyType::Static:
                body_->setCollisionFlags((body_->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT) |
                    btCollisionObject::CF_STATIC_OBJECT);
                break;
            case BodyType::Kinematic:
                body_->setCollisionFlags((body_->getCollisionFlags() & ~btCollisionObject::CF_STATIC_OBJECT) |
                    btCollisionObject::CF_KINEMATIC_OBJECT);
                break;
            case BodyType::Dynamic:
                body_->setCollisionFlags((body_->getCollisionFlags() &
                    ~(btCollisionObject::CF_KINEMATIC_OBJECT | btCollisionObject::CF_STATIC_OBJECT)));
                break;
            default:
                btAssert(false);
                break;
            }
            auto pc = findComponent<PhysicsBodyComponent>();
            if (pc) {
                pc->updateBodyCollision(true);
            }
        } else {
            bodyCi_.bodyType = value;
        }
    }

    const btTransform& SceneObject::transform() const
    {
        if (body_) {
            bodyCi_.xf = worldCenter() * localCenter().inverse();
        }
        return bodyCi_.xf;
    }

    void SceneObject::setTransform(const btVector3& pos, const btQuaternion& rot)
    {
        btAssert(btIsValid(pos));
        btAssert(btIsValid(rot));

        setTransform(btTransform(rot, pos));
    }

    void SceneObject::setTransform(const btTransform& t)
    {
        if (body_) {
            auto relXf = worldCenter().inverse() * bodyMs_->smoothXf;
            body_->setCenterOfMassTransform(t * localCenter());
            bodyMs_->smoothXf = worldCenter() * relXf;
            if (body_->isKinematicObject()) {
                body_->setInterpolationWorldTransform(worldCenter());
            }
            body_->activate();
        } else {
            bodyCi_.xf = t;
        }
    }

    const btTransform& SceneObject::smoothTransform() const
    {
        if (body_) {
            return bodyMs_->smoothXf;
        } else {
            return bodyCi_.xf;
        }
    }

    const btVector3& SceneObject::pos() const
    {
        return transform().getOrigin();
    }

    void SceneObject::setPos(const btVector3& value)
    {
        auto xf = transform();
        xf.setOrigin(value);
        setTransform(xf);
    }

    const btVector3& SceneObject::smoothPos() const
    {
        return smoothTransform().getOrigin();
    }

    const btMatrix3x3& SceneObject::basis() const
    {
        return transform().getBasis();
    }

    void SceneObject::setBasis(const btMatrix3x3& value)
    {
        auto xf = transform();
        xf.setBasis(value);
        setTransform(xf);
    }

    const btMatrix3x3& SceneObject::smoothBasis() const
    {
        return smoothTransform().getBasis();
    }

    btQuaternion SceneObject::rotation() const
    {
        btQuaternion ret;
        basis().getRotation(ret);
        return ret;
    }

    void SceneObject::setRotation(const btQuaternion& value)
    {
        auto xf = transform();
        xf.setRotation(value);
        setTransform(xf);
    }

    btQuaternion SceneObject::smoothRotation() const
    {
        btQuaternion ret;
        smoothBasis().getRotation(ret);
        return ret;
    }

    const btTransform& SceneObject::worldCenter() const
    {
        if (body_) {
            return body_->getWorldTransform();
        } else {
            return bodyCi_.xf;
        }
    }

    const btTransform& SceneObject::localCenter() const
    {
        if (body_) {
            return bodyMs_->centerOfMassXf;
        } else {
            return btTransform::getIdentity();
        }
    }

    void SceneObject::setLocalCenter(const btTransform& t)
    {
        if (body_) {
            auto xf = transform();
            auto smoothXf = bodyMs_->smoothXf;
            bodyMs_->centerOfMassXf = t;
            setTransform(xf);
            bodyMs_->smoothXf = smoothXf;
        }
    }

    float SceneObject::mass() const
    {
        if (body_) {
            return body_->getMass();
        } else {
            return 0.0f;
        }
    }

    btVector3 SceneObject::scaleByMomentOfInertia(const btVector3& v) const
    {
        //TODO: v(world) -> local space -> body space -> scale -> local space -> world
        return v;
    }

    const btVector3& SceneObject::linearVelocity() const
    {
        if (body_ && !body_->isKinematicObject()) {
            return body_->getLinearVelocity();
        } else {
            return bodyCi_.linearVelocity;
        }
    }

    void SceneObject::setLinearVelocity(const btVector3& value)
    {
        btAssert(btIsValid(value));

        if (body_ && !body_->isKinematicObject()) {
            body_->setLinearVelocity(value);
            body_->activate();
        } else {
            bodyCi_.linearVelocity = value;
            if (body_) {
                if (value.fuzzyZero() && bodyCi_.angularVelocity.fuzzyZero()) {
                    if (body_->getActivationState() == DISABLE_DEACTIVATION) {
                        body_->forceActivationState(ACTIVE_TAG);
                        body_->activate(true);
                    }
                } else {
                    body_->setActivationState(DISABLE_DEACTIVATION);
                }
            }
        }
    }

    const btVector3& SceneObject::angularVelocity() const
    {
        if (body_ && !body_->isKinematicObject()) {
            return body_->getAngularVelocity();
        } else {
            return bodyCi_.angularVelocity;
        }
    }

    void SceneObject::setAngularVelocity(const btVector3& value)
    {
        btAssert(btIsValid(value));

        if (body_ && !body_->isKinematicObject()) {
            body_->setAngularVelocity(value);
            body_->activate();
        } else {
            bodyCi_.angularVelocity = value;
            if (body_) {
                if (value.fuzzyZero() && bodyCi_.linearVelocity.fuzzyZero()) {
                    if (body_->getActivationState() == DISABLE_DEACTIVATION) {
                        body_->forceActivationState(ACTIVE_TAG);
                        body_->activate(true);
                    }
                } else {
                    body_->setActivationState(DISABLE_DEACTIVATION);
                }
            }
        }
    }

    float SceneObject::linearDamping() const
    {
        if (body_) {
            return body_->getLinearDamping();
        } else {
            return bodyCi_.linearDamping;
        }
    }

    void SceneObject::setLinearDamping(float value)
    {
        btAssert(btIsValid(value));

        if (body_) {
            body_->setDamping(value, body_->getAngularDamping());
        } else {
            bodyCi_.linearDamping = value;
        }
    }

    float SceneObject::angularDamping() const
    {
        if (body_) {
            return body_->getAngularDamping();
        } else {
            return bodyCi_.angularDamping;
        }
    }

    void SceneObject::setAngularDamping(float value)
    {
        btAssert(btIsValid(value));

        if (body_) {
            body_->setDamping(body_->getLinearDamping(), value);
        } else {
            bodyCi_.angularDamping = value;
        }
    }

    float SceneObject::friction() const
    {
        if (body_) {
            return body_->getFriction();
        } else {
            return bodyCi_.friction;
        }
    }

    void SceneObject::setFriction(float value)
    {
        btAssert(btIsValid(value));

        if (body_) {
            body_->setFriction(value);
        } else {
            bodyCi_.friction = value;
        }
    }

    float SceneObject::restitution() const
    {
        if (body_) {
            return body_->getRestitution();
        } else {
            return bodyCi_.restitution;
        }
    }

    void SceneObject::setRestitution(float value)
    {
        btAssert(btIsValid(value));

        if (body_) {
            body_->setRestitution(value);
        } else {
            bodyCi_.restitution = value;
        }
    }

    void SceneObject::applyForce(const btVector3& force, const btVector3& point)
    {
        btAssert(btIsValid(force));
        btAssert(btIsValid(point));

        if (body_) {
            body_->applyForce(force, worldCenter().inverse() * point);
            body_->activate();
        }
    }

    void SceneObject::applyForceToCenter(const btVector3& force)
    {
        btAssert(btIsValid(force));

        if (body_) {
            body_->applyCentralForce(force);
            body_->activate();
        }
    }

    void SceneObject::applyTorque(const btVector3& torque)
    {
        btAssert(btIsValid(torque));

        if (body_) {
            body_->applyTorque(torque);
            body_->activate();
        }
    }

    void SceneObject::applyLinearImpulse(const btVector3& impulse, const btVector3& point)
    {
        btAssert(btIsValid(impulse));
        btAssert(btIsValid(point));

        if (body_) {
            body_->applyImpulse(impulse, worldCenter().inverse() * point);
            body_->activate();
        }
    }

    void SceneObject::applyAngularImpulse(const btVector3& impulse)
    {
        btAssert(btIsValid(impulse));

        if (body_) {
            body_->applyTorqueImpulse(impulse);
            body_->activate();
        }
    }

    bool SceneObject::physicsActive() const
    {
        return bodyCi_.active;
    }

    void SceneObject::setPhysicsActive(bool value)
    {
        bodyCi_.active = value;
        if (body_) {
            if (!frozen()) {
                auto pc = findComponent<PhysicsBodyComponent>();
                if (pc) {
                    pc->setActive(value);
                }
            }
        }
    }

    btVector3 SceneObject::getWorldPoint(const btVector3& localPoint) const
    {
        return transform() * localPoint;
    }

    btVector3 SceneObject::getLocalPoint(const btVector3& worldPoint) const
    {
        return transform().inverse() * worldPoint;
    }

    btVector3 SceneObject::getSmoothWorldPoint(const btVector3& localPoint) const
    {
        return smoothTransform() * localPoint;
    }

    btVector3 SceneObject::getSmoothLocalPoint(const btVector3& worldPoint) const
    {
        return smoothTransform().inverse() * worldPoint;
    }

    btVector3 SceneObject::getForward() const
    {
        return transform().getBasis() * btVector3_forward;
    }

    btVector3 SceneObject::getRight() const
    {
        return transform().getBasis() * btVector3_right;
    }

    btVector3 SceneObject::getUp() const
    {
        return transform().getBasis() * btVector3_up;
    }

    btVector3 SceneObject::getSmoothForward() const
    {
        return smoothTransform().getBasis() * btVector3_forward;
    }

    btVector3 SceneObject::getSmoothRight() const
    {
        return smoothTransform().getBasis() * btVector3_right;
    }

    btVector3 SceneObject::getSmoothUp() const
    {
        return smoothTransform().getBasis() * btVector3_up;
    }

    btVector3 SceneObject::getLinearVelocityFromWorldPoint(const btVector3& worldPoint) const
    {
        return getLinearVelocityFromLocalPoint(getLocalPoint(worldPoint));
    }

    btVector3 SceneObject::getLinearVelocityFromLocalPoint(const btVector3& localPoint) const
    {
        if (body_) {
            return body_->getVelocityInLocalPoint(localCenter().inverse() * localPoint);
        } else {
            return btVector3_zero;
        }
    }

    void SceneObject::collectIslandObjects(std::unordered_set<SceneObjectPtr>& objs)
    {
        if (!body_) {
            return;
        }

        objs.insert(shared_from_this());
    }

    bool SceneObject::collidesWith(btCollisionObject* other)
    {
        btAssert(body_);

        if (!other->hasContactResponse()) {
            return false;
        }

        btOverlapFilterCallback* filter = nullptr; // TODO.

        return filter->needBroadphaseCollision(body_->getBroadphaseHandle(), other->getBroadphaseHandle());
    }

    void SceneObject::freeze()
    {
        btAssert(!flags_[Flag::Frozen]);

        flags_[Flag::Frozen] = true;

        auto tmpObjs = objects();
        auto tmpComponents = components();

        for (const auto& obj : tmpObjs) {
            obj->freeze();
        }

        for (const auto& c : tmpComponents) {
            scene()->freezeComponent(c);
        }
    }

    void SceneObject::thaw()
    {
        btAssert(flags_[Flag::Frozen]);

        flags_[Flag::Frozen] = false;

        auto tmpObjs = objects();
        auto tmpComponents = components();

        for (const auto& obj : tmpObjs) {
            obj->thaw();
        }

        for (const auto& c : tmpComponents) {
            scene()->thawComponent(c);
        }
    }

    std::vector<AObjectPtr> SceneObject::getChildren() const
    {
        std::vector<AObjectPtr> res;
        res.reserve(components_.size() + objects().size());
        for (const auto& c : components_) {
            if ((c->aflags() & AObjectEditable) != 0) {
                res.push_back(c);
            }
        }
        for (const auto& obj : objects()) {
            if ((obj->aflags() & AObjectEditable) != 0) {
                res.push_back(obj);
            }
        }
        return res;
    }

    void SceneObject::setChildren(const std::vector<AObjectPtr>& value)
    {
        auto objs = objects();
        for (const auto& obj : objs) {
            if ((obj->aflags() & AObjectEditable) != 0) {
                obj->removeFromParent();
            }
        }
        for (const auto& c : components_) {
            if ((c->aflags() & AObjectEditable) != 0) {
                c->removeFromParent();
            }
        }
        for (const auto& obj : value) {
            if (auto sObj = aobjectCast<SceneObject>(obj)) {
                addObject(sObj);
            } else if (auto c = aobjectCast<Component>(obj)) {
                addComponent(c);
            } else {
                LOG4CPLUS_ERROR(logger(), "Bad child object \"" << obj->name() << "\", class - \"" << obj->klass().name() << "\"");
            }
        }
    }
}

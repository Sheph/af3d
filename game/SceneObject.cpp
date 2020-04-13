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
#include "Scene.h"
#include "Utils.h"
#include "Settings.h"
#include "Logger.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(SceneObject, SceneObjectManager)
    ACLASS_PROPERTY(SceneObject, Transform, AProperty_WorldTransform, "World transform", Transform, btTransform::getIdentity(), Position, APropertyEditable)
    ACLASS_PROPERTY(SceneObject, Type, "type", "Scene object type", SceneObjectType, static_cast<int>(SceneObjectType::Other), General, APropertyEditable)
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
            auto ms = body_->getMotionState();
            delete ms;
            body_->setMotionState(nullptr);
            delete body_;
            body_ = nullptr;
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
        obj->setParams(params);
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
        physicsActive_ = body_->isInWorld();
    }

    const btTransform& SceneObject::transform() const
    {
        if (body_) {
            return body_->getWorldTransform();
        } else {
            return bodyCi_.xf;
        }
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
            body_->setWorldTransform(t);
        } else {
            bodyCi_.xf = t;
        }
    }

/*    void SceneObject::setTransformRecursive(const b2Vec2& pos, float32 angle)
    {
        setPosRecursive(pos);
        setAngleRecursive(angle);
    }

    void SceneObject::setTransformRecursive(const b2Transform& t)
    {
        setTransformRecursive(t.p, t.q.GetAngle());
    }*/

    const btVector3& SceneObject::pos() const
    {
        if (body_) {
            return body_->getWorldTransform().getOrigin();
        } else {
            return bodyCi_.xf.getOrigin();
        }
    }

    void SceneObject::setPos(const btVector3& value)
    {
        btAssert(btIsValid(value));

        //b2Vec2 tmpPos = smoothPos_ - pos();
        //b2Vec2 tmpPrevPos = smoothPrevPos_ - pos();

        if (body_) {
            body_->setWorldTransform(btTransform(body_->getWorldTransform().getBasis(), value));
        } else {
            bodyCi_.xf.setOrigin(value);
        }

        //smoothPos_ = pos() + tmpPos;
        //smoothPrevPos_ = pos() + tmpPrevPos;
    }

/*    void SceneObject::setPosRecursive(const b2Vec2& value)
    {
        b2Vec2 tmp = value - pos();
        setPos(value);
        for (std::set<SceneObjectPtr>::const_iterator it = objects().begin();
             it != objects().end();
             ++it ) {
            (*it)->setPosRecursive((*it)->pos() + tmp);
        }
    }

    void SceneObject::setPosSmoothed(const b2Vec2& value)
    {
        assert(value.IsValid());

        b2Vec2 tmpPos = smoothPos_ - pos();

        if (body_) {
            body_->SetTransform(value, body_->GetAngle());
        } else {
            bodyDef_.position = value;
        }

        smoothPos_ = pos() + tmpPos;
    }*/

    const btMatrix3x3& SceneObject::basis() const
    {
        if (body_) {
            return body_->getWorldTransform().getBasis();
        } else {
            return bodyCi_.xf.getBasis();
        }
    }

    void SceneObject::setBasis(const btMatrix3x3& value)
    {
        if (body_) {
            body_->setWorldTransform(btTransform(value, body_->getWorldTransform().getOrigin()));
        } else {
            bodyCi_.xf.setBasis(value);
        }
    }

    btQuaternion SceneObject::rotation() const
    {
        if (body_) {
            return body_->getWorldTransform().getRotation();
        } else {
            return bodyCi_.xf.getRotation();
        }
    }

    void SceneObject::setRotation(const btQuaternion& value)
    {
        btAssert(btIsValid(value));

        //float tmpAng = smoothAngle_ - angle();
        //float tmpPrevAng = smoothPrevAngle_ - angle();

        if (body_) {
            body_->setWorldTransform(btTransform(value, body_->getWorldTransform().getOrigin()));
        } else {
            bodyCi_.xf.setRotation(value);
        }

        //smoothAngle_ = angle() + tmpAng;
        //smoothPrevAngle_ = angle() + tmpPrevAng;
    }

/*    void SceneObject::setAngleRecursive(float value)
    {
        float tmp = value - angle();
        setAngle(value);
        b2Rot rot(tmp);
        for (std::set<SceneObjectPtr>::const_iterator it = objects().begin();
             it != objects().end();
             ++it ) {
            (*it)->setPosRecursive(pos() + b2Mul(rot, (*it)->pos() - pos()));
            (*it)->setAngleRecursive((*it)->angle() + tmp);
        }
    }

    void SceneObject::setAngleSmoothed(float value)
    {
        float tmpAng = smoothAngle_ - angle();

        if (body_) {
            body_->SetTransform(body_->GetPosition(), value);
        } else {
            bodyDef_.angle = value;
        }

        smoothAngle_ = angle() + tmpAng;
    }*/

    const btVector3& SceneObject::worldCenter() const
    {
        if (body_) {
            return body_->getWorldTransform().getOrigin();
        } else {
            return bodyCi_.xf.getOrigin();
        }
    }

    const btVector3& SceneObject::localCenter() const
    {
        return btVector3_zero;
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
        if (body_) {
            return body_->getLinearVelocity();
        } else {
            return bodyCi_.linearVelocity;
        }
    }

    void SceneObject::setLinearVelocity(const btVector3& value)
    {
        btAssert(btIsValid(value));

        if (body_) {
            body_->setLinearVelocity(value);
        } else {
            bodyCi_.linearVelocity = value;
        }
    }

/*    void SceneObject::setLinearVelocityRecursive(const b2Vec2& value)
    {
        setLinearVelocity(value);
        for (std::set<SceneObjectPtr>::const_iterator it = objects().begin();
             it != objects().end();
             ++it ) {
            (*it)->setLinearVelocityRecursive(value);
        }
    }*/

    const btVector3& SceneObject::angularVelocity() const
    {
        if (body_) {
            return body_->getAngularVelocity();
        } else {
            return bodyCi_.angularVelocity;
        }
    }

    void SceneObject::setAngularVelocity(const btVector3& value)
    {
        btAssert(btIsValid(value));

        if (body_) {
            body_->setAngularVelocity(value);
        } else {
            bodyCi_.angularVelocity = value;
        }
    }

/*    void SceneObject::setAngularVelocityRecursive(float value)
    {
        setAngularVelocity(value);
        for (std::set<SceneObjectPtr>::const_iterator it = objects().begin();
             it != objects().end();
             ++it ) {
            (*it)->setAngularVelocityRecursive(value);
        }
    }*/

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
            body_->applyForce(force, point);
        }
    }

    void SceneObject::applyForceToCenter(const btVector3& force)
    {
        btAssert(btIsValid(force));

        if (body_) {
            body_->applyCentralForce(force);
        }
    }

    void SceneObject::applyTorque(const btVector3& torque)
    {
        btAssert(btIsValid(torque));

        if (body_) {
            body_->applyTorque(torque);
        }
    }

    void SceneObject::applyLinearImpulse(const btVector3& impulse, const btVector3& point)
    {
        btAssert(btIsValid(impulse));
        btAssert(btIsValid(point));

        if (body_) {
            body_->applyImpulse(impulse, point);
        }
    }

    void SceneObject::applyAngularImpulse(const btVector3& impulse)
    {
        btAssert(btIsValid(impulse));

        if (body_) {
            body_->applyTorqueImpulse(impulse);
        }
    }

    bool SceneObject::physicsActive() const
    {
        if (body_) {
            return physicsActive_;
        } else {
            return bodyCi_.active;
        }
    }

    void SceneObject::setPhysicsActive(bool value)
    {
        if (body_) {
            physicsActive_ = value;
            if (!frozen()) {
                auto pc = findComponent<PhysicsBodyComponent>();
                if (!value && body_->isInWorld() && pc && pc->manager()) {
                    pc->manager()->world().removeRigidBody(body_);
                } else if (value && !body_->isInWorld() && pc && pc->manager()) {
                    pc->manager()->world().addRigidBody(body_);
                }
            }
        } else {
            bodyCi_.active = value;
        }
    }

/*    void SceneObject::setActiveRecursive(bool value)
    {
        setActive(value);
        for (std::set<SceneObjectPtr>::const_iterator it = objects().begin();
             it != objects().end();
             ++it ) {
            (*it)->setActiveRecursive(value);
        }
    }*/

    btVector3 SceneObject::getWorldPoint(const btVector3& localPoint) const
    {
        return transform() * localPoint;
    }

    btVector3 SceneObject::getLocalPoint(const btVector3& worldPoint) const
    {
        return transform().inverse() * worldPoint;
    }

/*    b2Vec2 SceneObject::getSmoothWorldPoint(const b2Vec2& localPoint) const
    {
        if (body_) {
            return b2Mul(getSmoothTransform(), localPoint);
        } else {
            return getWorldPoint(localPoint);
        }
    }

    b2Vec2 SceneObject::getSmoothLocalPoint(const b2Vec2& worldPoint) const
    {
        if (body_) {
            return b2MulT(getSmoothTransform(), worldPoint);
        } else {
            return getLocalPoint(worldPoint);
        }
    }*/

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

/*    b2Vec2 SceneObject::getSmoothDirection(float length) const
    {
        return b2Mul(getSmoothTransform().q, b2Vec2(length, 0.0f));
    }*/

    btVector3 SceneObject::getLinearVelocityFromWorldPoint(const btVector3& worldPoint) const
    {
        return getLinearVelocityFromLocalPoint(getLocalPoint(worldPoint));
    }

    btVector3 SceneObject::getLinearVelocityFromLocalPoint(const btVector3& localPoint) const
    {
        if (body_) {
            return body_->getVelocityInLocalPoint(localPoint);
        } else {
            return btVector3_zero;
        }
    }

/*    void SceneObject::resetSmooth()
    {
        if (body_) {
            smoothPos_ = smoothPrevPos_ = body_->GetPosition();
            smoothAngle_ = smoothPrevAngle_ = body_->GetAngle();
        } else {
            smoothPos_ = smoothPrevPos_ = bodyDef_.position;
            smoothAngle_ = smoothPrevAngle_ = bodyDef_.angle;
        }
    }

    void SceneObject::updateSmooth(float fixedTimestepAccumulatorRatio)
    {
        if (body_) {
            const float oneMinusRatio = 1.0f - fixedTimestepAccumulatorRatio;

            if (body_->GetPosition() != smoothPrevPos_) {
                smoothPos_ = fixedTimestepAccumulatorRatio * body_->GetPosition() +
                    oneMinusRatio * smoothPrevPos_;
            } else {
                smoothPos_ = body_->GetPosition();
            }

            if (body_->GetAngle() != smoothPrevAngle_) {
                smoothAngle_ = fixedTimestepAccumulatorRatio * body_->GetAngle() +
                    oneMinusRatio * smoothPrevAngle_;
            } else {
                smoothAngle_ = body_->GetAngle();
            }
        }
    }

    b2Transform SceneObject::getSmoothTransform() const
    {
        if (smoothOverrideJoint_) {
            if (smoothOverrideJoint_->valid()) {
                if (smoothOverrideJoint_->referenceAngle() != smoothRotAngle_) {
                    smoothRotAngle_ = smoothOverrideJoint_->referenceAngle();
                    smoothRot_.Set(smoothOverrideJoint_->referenceAngle());
                }

                b2Transform xf = smoothOverrideJoint_->objectA()->getSmoothTransform();
                b2Rot idR;
                idR.SetIdentity();
                xf = b2Mul(xf, b2Transform(smoothOverrideJoint_->localAnchorA(), idR));
                xf = b2Mul(xf, b2Transform(b2Vec2_zero, smoothRot_));
                xf = b2Mul(xf, b2Transform(-smoothOverrideJoint_->localAnchorB(), idR));
                return xf;
            }
            smoothOverrideJoint_.reset();
        }

        if (smoothAngle_ != smoothRotAngle_) {
            smoothRotAngle_ = smoothAngle_;
            smoothRot_.Set(smoothRotAngle_);
        }

        return b2Transform(smoothPos_, smoothRot_);
    }

    b2Vec2 SceneObject::smoothPos() const
    {
        if (smoothOverrideJoint_) {
            if (smoothOverrideJoint_->valid()) {
                return smoothOverrideJoint_->objectA()->getSmoothWorldPoint(smoothOverrideJoint_->localAnchorA()) - smoothOverrideJoint_->localAnchorB();
            }
            smoothOverrideJoint_.reset();
        }
        return smoothPos_;
    }

    float SceneObject::smoothAngle() const
    {
        if (smoothOverrideJoint_) {
            if (smoothOverrideJoint_->valid()) {
                return smoothOverrideJoint_->objectA()->smoothAngle() + smoothOverrideJoint_->referenceAngle();
            }
            smoothOverrideJoint_.reset();
        }
        return smoothAngle_;
    }

    void SceneObject::setLife(float value)
    {
        if ((maxLife_ >= 0) && (value > maxLife_)) {
            life_ = maxLife_;
        } else {
            life_ = value;
        }
    }

    bool SceneObject::changeLife(float value, bool shock)
    {
        SceneObject* pobj;

        if (propagateDamage_ && (pobj = parentObject())) {
            return pobj->changeLife(value, shock);
        }

        if (dead() ||
            ((value < 0.0f) && (type_ == SceneObjectTypePlayer) && scene() && scene()->cutscene()) ||
            ((value < 0.0f) && (flags_[FlagInvulnerable] || findComponent<ShockedComponent>()))) {
            return false;
        }

        setLife(life_ + value);

        if (shock && (value < 0.0f) && !dead()) {
            addComponent(boost::make_shared<ShockedComponent>(shockDuration_));
        } else if (dead()) {
            ShockedComponentPtr c = findComponent<ShockedComponent>();
            if (c) {
                c->removeFromParent();
            }
        }

        return true;
    }

    bool SceneObject::changeLife2(SceneObject* missile, float value, bool shock)
    {
        return changeLife2(missile->type(), value, shock);
    }

    bool SceneObject::changeLife2(SceneObjectType missileType, float value, bool shock)
    {
        if (missileType == SceneObjectTypeNeutralMissile) {
            return changeLife(value);
        } else if (missileType == SceneObjectTypeEnemyMissile) {
            if ((type_ != SceneObjectTypeEnemy) && (type_ != SceneObjectTypeEnemyBuilding)) {
                return changeLife(value, shock);
            }
        } else {
            if ((type_ != SceneObjectTypePlayer) && (type_ != SceneObjectTypeAlly)) {
                return changeLife(value, shock);
            }
        }
        return false;
    }

    void SceneObject::setMaxLife(float value)
    {
        maxLife_ = value;

        if ((maxLife_ >= 0) && (life_ > maxLife_)) {
            life_ = maxLife_;
        }
    }

    float SceneObject::lifePercent() const
    {
        if (dead()) {
            return 0.0f;
        }

        if (maxLife_ < 0) {
            return 1.0f;
        }

        assert(life_ <= maxLife_);

        return static_cast<float>(life_) / maxLife_;
    }

    SceneObject* SceneObject::damageReceiver()
    {
        SceneObject* pobj = this;

        while (pobj->propagateDamage_ && pobj->parentObject()) {
            pobj = pobj->parentObject();
        }

        return pobj;
    }*/

    void SceneObject::collectIslandObjects(std::unordered_set<SceneObjectPtr>& objs)
    {
        if (!body_) {
            return;
        }

        objs.insert(shared_from_this());
    }

/*    void SceneObject::becomeDeadbody()
    {
        static CollisionFilterPtr deadbodyFilter;

        if (!deadbodyFilter) {
            deadbodyFilter = boost::make_shared<CollisionDeadbodyFilter>();
        }

        type_ = SceneObjectTypeDeadbody;
        setCollisionFilter(deadbodyFilter);

        PhysicsBodyComponentPtr pc = findComponent<PhysicsBodyComponent>();
        if (pc) {
            pc->refilter();
        }
    }*/

    bool SceneObject::collidesWith(btCollisionObject* other)
    {
        btAssert(body_);

        if (!other->hasContactResponse()) {
            return false;
        }

        btOverlapFilterCallback* filter = nullptr; // TODO.

        return filter->needBroadphaseCollision(body_->getBroadphaseHandle(), other->getBroadphaseHandle());
    }

/*    bool SceneObject::visible() const
    {
        for (std::vector<ComponentPtr>::const_iterator it = components_.begin();
             it != components_.end();
             ++it ) {
            const RenderComponentPtr& rc =
                boost::dynamic_pointer_cast<RenderComponent>(*it);
            if (rc && rc->visible()) {
                return true;
            }
            const LightComponentPtr& lc =
                boost::dynamic_pointer_cast<LightComponent>(*it);
            if (lc) {
                for (std::vector<LightPtr>::const_iterator jt = lc->lights().begin();
                     jt != lc->lights().end();
                     ++jt ) {
                    if ((*jt)->visible()) {
                        return true;
                    }
                }
            }
        }

        return false;
    }

    void SceneObject::setVisible(bool value)
    {
        for (std::vector<ComponentPtr>::const_iterator it = components_.begin();
             it != components_.end();
             ++it ) {
            const RenderComponentPtr& rc =
                boost::dynamic_pointer_cast<RenderComponent>(*it);
            if (rc) {
                rc->setVisible(value);
            }
            const LightComponentPtr& lc =
                boost::dynamic_pointer_cast<LightComponent>(*it);
            if (lc) {
                for (std::vector<LightPtr>::const_iterator jt = lc->lights().begin();
                     jt != lc->lights().end();
                     ++jt ) {
                    (*jt)->setVisible(value);
                }
            }
        }
    }

    void SceneObject::setVisibleRecursive(bool value)
    {
        setVisible(value);
        for (std::set<SceneObjectPtr>::const_iterator it = objects().begin();
             it != objects().end();
             ++it ) {
            (*it)->setVisibleRecursive(value);
        }
    }

    Color SceneObject::color() const
    {
        for (std::vector<ComponentPtr>::const_iterator it = components_.begin();
             it != components_.end();
             ++it ) {
            const RenderComponentPtr& rc =
                boost::dynamic_pointer_cast<RenderComponent>(*it);
            if (rc) {
                return rc->color();
            }
            const LightComponentPtr& lc =
                boost::dynamic_pointer_cast<LightComponent>(*it);
            if (lc) {
                for (std::vector<LightPtr>::const_iterator jt = lc->lights().begin();
                     jt != lc->lights().end();
                     ++jt ) {
                    return (*jt)->color();
                }
            }
        }

        return Color(1.0f, 1.0f, 1.0f, 1.0f);
    }

    void SceneObject::setColor(const Color& value)
    {
        for (std::vector<ComponentPtr>::const_iterator it = components_.begin();
             it != components_.end();
             ++it ) {
            const RenderComponentPtr& rc =
                boost::dynamic_pointer_cast<RenderComponent>(*it);
            if (rc) {
                rc->setColor(value);
            }
            const LightComponentPtr& lc =
                boost::dynamic_pointer_cast<LightComponent>(*it);
            if (lc) {
                for (std::vector<LightPtr>::const_iterator jt = lc->lights().begin();
                     jt != lc->lights().end();
                     ++jt ) {
                    (*jt)->setColor(value);
                }
            }
        }
    }

    void SceneObject::setColorRecursive(const Color& value)
    {
        setColor(value);
        for (std::set<SceneObjectPtr>::const_iterator it = objects().begin();
             it != objects().end();
             ++it ) {
            (*it)->setColorRecursive(value);
        }
    }

    float SceneObject::alpha() const
    {
        for (std::vector<ComponentPtr>::const_iterator it = components_.begin();
             it != components_.end();
             ++it ) {
            const RenderComponentPtr& rc =
                boost::dynamic_pointer_cast<RenderComponent>(*it);
            if (rc) {
                return rc->color().rgba[3];
            }
            const LightComponentPtr& lc =
                boost::dynamic_pointer_cast<LightComponent>(*it);
            if (lc) {
                for (std::vector<LightPtr>::const_iterator jt = lc->lights().begin();
                     jt != lc->lights().end();
                     ++jt ) {
                    return (*jt)->color().rgba[3];
                }
            }
        }

        return 1.0f;
    }

    void SceneObject::setAlpha(float value)
    {
        for (std::vector<ComponentPtr>::const_iterator it = components_.begin();
             it != components_.end();
             ++it ) {
            const RenderComponentPtr& rc =
                boost::dynamic_pointer_cast<RenderComponent>(*it);
            if (rc) {
                Color c = rc->color();
                c.rgba[3] = value;
                rc->setColor(c);
            }
            const LightComponentPtr& lc =
                boost::dynamic_pointer_cast<LightComponent>(*it);
            if (lc) {
                for (std::vector<LightPtr>::const_iterator jt = lc->lights().begin();
                     jt != lc->lights().end();
                     ++jt ) {
                    Color c = (*jt)->color();
                    c.rgba[3] = value;
                    (*jt)->setColor(c);
                }
            }
        }
    }

    void SceneObject::setAlphaRecursive(float value)
    {
        setAlpha(value);
        for (std::set<SceneObjectPtr>::const_iterator it = objects().begin();
             it != objects().end();
             ++it ) {
            (*it)->setAlphaRecursive(value);
        }
    }

    void SceneObject::setZOrder(int value)
    {
        std::vector<RenderComponentPtr> rcs = findComponents<RenderComponent>();

        int minZ = (std::numeric_limits<int>::max)();
        for (size_t i = 0; i < rcs.size(); ++i) {
            if (rcs[i]->zOrder() < minZ) {
                minZ = rcs[i]->zOrder();
            }
        }
        for (size_t i = 0; i < rcs.size(); ++i) {
            rcs[i]->setZOrder(rcs[i]->zOrder() - minZ + value);
        }
    }

    b2BodyType SceneObject::script_bodyType() const
    {
        if (body_) {
            return body_->GetType();
        } else {
            return bodyDef_.type;
        }
    }

    void SceneObject::script_setBodyType(b2BodyType value)
    {
        if (body_) {
            body_->SetType(value);
        } else {
            bodyDef_.type = value;
        }
    }

    void SceneObject::script_changePosSmoothed(float x, float y)
    {
        setPosSmoothed(pos() + b2Vec2(x, y));
    }*/

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

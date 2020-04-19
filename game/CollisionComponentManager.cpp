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

#include "CollisionComponentManager.h"
#include "CollisionComponent.h"
#include "SceneObject.h"

namespace af3d
{
    SceneObject* Contact::getOther(SceneObject* obj) const
    {
        SceneObject* objA = SceneObject::fromShape(shapeA->shape());

        return (obj == objA) ? SceneObject::fromShape(shapeB->shape()) : objA;
    }

    const CollisionShapePtr& Contact::getOtherShape(SceneObject* obj) const
    {
        return (obj == SceneObject::fromShape(shapeA->shape())) ? shapeB : shapeA;
    }

    const CollisionShapePtr& Contact::getThisShape(SceneObject* obj) const
    {
        return (obj == SceneObject::fromShape(shapeA->shape())) ? shapeA : shapeB;
    }

    CollisionComponentManager::Manifold::Manifold(btPersistentManifold* manifold)
    : manifold(manifold),
      objA(SceneObject::fromBody(const_cast<btRigidBody*>(btRigidBody::upcast(manifold->getBody0())))->shared_from_this()),
      objB(SceneObject::fromBody(const_cast<btRigidBody*>(btRigidBody::upcast(manifold->getBody1())))->shared_from_this())
    {
    }

    CollisionComponentManager::~CollisionComponentManager()
    {
        btAssert(components_.empty());
        btAssert(frozenComponents_.empty());
    }

    void CollisionComponentManager::cleanup()
    {
        btAssert(components_.empty());
        btAssert(frozenComponents_.empty());

        manifolds_.clear();
        events_.clear();
    }

    void CollisionComponentManager::addComponent(const ComponentPtr& component)
    {
        CollisionComponentPtr cComponent = std::static_pointer_cast<CollisionComponent>(component);

        btAssert(!component->manager());

        components_.insert(cComponent);
        cComponent->setManager(this);
    }

    void CollisionComponentManager::removeComponent(const ComponentPtr& component)
    {
        CollisionComponentPtr cComponent = std::static_pointer_cast<CollisionComponent>(component);

        if (components_.erase(cComponent) ||
            frozenComponents_.erase(cComponent)) {
            cComponent->setManager(nullptr);
        }
    }

    void CollisionComponentManager::freezeComponent(const ComponentPtr& component)
    {
        CollisionComponentPtr cComponent = std::static_pointer_cast<CollisionComponent>(component);

        components_.erase(cComponent);
        frozenComponents_.insert(cComponent);
        component->onFreeze();
    }

    void CollisionComponentManager::thawComponent(const ComponentPtr& component)
    {
        CollisionComponentPtr cComponent = std::static_pointer_cast<CollisionComponent>(component);

        frozenComponents_.erase(cComponent);
        components_.insert(cComponent);
        component->onThaw();
    }

    bool CollisionComponentManager::update(float dt)
    {
        static std::vector<CollisionComponentPtr> tmp;

        tmp.reserve(components_.size());

        for (const auto& c : components_) {
            tmp.push_back(c);
        }

        for (const auto& c : tmp) {
            if (c->manager()) {
                c->update(dt);
            }
        }

        tmp.resize(0);

        return true;
    }

    void CollisionComponentManager::debugDraw(RenderList& rl)
    {
        for (const auto& c : components_) {
            c->debugDraw(rl);
        }
    }

    void CollisionComponentManager::step(btCollisionWorld* world)
    {
        btDispatcher* dispatcher = world->getDispatcher();
        int numManifolds = dispatcher->getNumManifolds();

        btAssert(numManifolds >= static_cast<int>(manifolds_.size()));

        for (int i = 0; i < numManifolds; ++i) {
            btPersistentManifold* cm = dispatcher->getManifoldByIndexInternal(i);

            auto it = manifolds_.find(cm);
            if (it == manifolds_.end()) {
                if (cm->getNumContacts() <= 0) {
                    continue;
                }
                it = manifolds_.emplace(cm, Manifold(cm)).first;
            }

            addEvents(it->second);

            if (cm->getNumContacts() <= 0) {
                endContact(cm);
            }
        }
    }

    void CollisionComponentManager::flushPending()
    {
        static ContactEvents events;

        while (!events_.empty()) {
            events.swap(events_);

            for (const auto& event : events) {
                if (event.contact.pointCount <= 0) {
                    if (event.cA && event.cA->manager()) {
                        event.cA->endContact(event.contact);
                    }
                    if (event.cB && event.cB->manager()) {
                        event.cB->endContact(event.contact);
                    }
                } else {
                    if (event.cA && event.cA->manager()) {
                        event.cA->updateContact(event.contact, event.isNew);
                    }
                    if (event.cB && event.cB->manager()) {
                        event.cB->updateContact(event.contact, event.isNew);
                    }
                }
            }

            events.clear();
        }
    }

    void CollisionComponentManager::endContact(btPersistentManifold* manifold)
    {
        auto it = manifolds_.find(manifold);
        if (it == manifolds_.end()) {
            return;
        }

        btAssert(it->second.manifold->getBody0() == it->second.objA->body());
        btAssert(it->second.manifold->getBody1() == it->second.objB->body());

        if (!it->second.contacts.empty()) {
            auto cA = it->second.objA->findComponent<CollisionComponent>();
            auto cB = it->second.objB->findComponent<CollisionComponent>();

            if (cA || cB) {
                for (size_t i = 0; i < it->second.contacts.size(); ++i) {
                    if (it->second.contacts[i].cookie) {
                        // Contact removed.
                        events_.emplace_back();
                        auto& evt = events_.back();

                        evt.contact.cookie = it->second.contacts[i].cookie;
                        evt.contact.shapeA = it->second.contacts[i].shapeA;
                        evt.contact.shapeB = it->second.contacts[i].shapeB;
                        evt.objA = it->second.objA;
                        evt.objB = it->second.objB;
                        evt.cA = cA;
                        evt.cB = cB;
                    }
                }
            }
        }

        manifolds_.erase(it);
    }

    void CollisionComponentManager::addEvents(Manifold& m)
    {
        auto cA = m.objA->findComponent<CollisionComponent>();
        auto cB = m.objB->findComponent<CollisionComponent>();
        bool needEvent = cA || cB;

        auto bodyA = m.manifold->getBody0();
        auto bodyB = m.manifold->getBody1();

        btAssert(bodyA == m.objA->body());
        btAssert(bodyB == m.objB->body());

        int numNewContacts = 0;
        static std::array<ContactInfo, MANIFOLD_CACHE_SIZE> newContacts;

        for (int i = 0; i < m.manifold->getNumContacts(); ++i) {
            auto& pt = m.manifold->getContactPoint(i);
            auto shapeA = const_cast<btCollisionShape*>(SceneObject::getBodyShape(bodyA, pt.m_childIdx0));
            auto shapeB = const_cast<btCollisionShape*>(SceneObject::getBodyShape(bodyB, pt.m_childIdx1));

            bool found = false;
            for (size_t j = 0; j < m.contacts.size(); ++j) {
                if (!m.contacts[j].cookie) {
                    continue;
                }
                if ((m.contacts[j].shapeA->shape() == shapeA) &&
                    (m.contacts[j].shapeB->shape() == shapeB)) {
                    m.contacts[j].points[m.contacts[j].numPoints++] = &pt;
                    found = true;
                    break;
                }
            }

            if (!found) {
                for (int j = 0; j < numNewContacts; ++j) {
                    if ((newContacts[j].shapeA->shape() == shapeA) &&
                        (newContacts[j].shapeB->shape() == shapeB)) {
                        newContacts[j].points[newContacts[j].numPoints++] = &pt;
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    newContacts[numNewContacts].cookie = nextCookie_++;
                    newContacts[numNewContacts].shapeA = std::static_pointer_cast<CollisionShape>(CollisionShape::fromShape(shapeA)->sharedThis());
                    newContacts[numNewContacts].shapeB = std::static_pointer_cast<CollisionShape>(CollisionShape::fromShape(shapeB)->sharedThis());
                    newContacts[numNewContacts].points[newContacts[numNewContacts].numPoints++] = &pt;
                    ++numNewContacts;
                }
            }
        }

        for (size_t i = 0; i < m.contacts.size(); ++i) {
            if (m.contacts[i].cookie && (m.contacts[i].numPoints <= 0)) {
                if (needEvent) {
                    // Contact removed.
                    events_.emplace_back();
                    auto& evt = events_.back();

                    evt.contact.cookie = m.contacts[i].cookie;
                    evt.contact.shapeA = m.contacts[i].shapeA;
                    evt.contact.shapeB = m.contacts[i].shapeB;
                    evt.objA = m.objA;
                    evt.objB = m.objB;
                    evt.cA = cA;
                    evt.cB = cB;
                }

                m.contacts[i].cookie = 0;
                m.contacts[i].shapeA.reset();
                m.contacts[i].shapeB.reset();
            } else if (m.contacts[i].numPoints > 0) {
                btAssert(m.contacts[i].cookie);
                if (needEvent) {
                    // Contact updated.
                    events_.emplace_back();
                    auto& evt = events_.back();

                    evt.contact.cookie = m.contacts[i].cookie;
                    evt.contact.shapeA = m.contacts[i].shapeA;
                    evt.contact.shapeB = m.contacts[i].shapeB;
                    evt.contact.pointCount = m.contacts[i].numPoints;
                    for (int j = 0; j < evt.contact.pointCount; ++j) {
                        evt.contact.points[j] = *m.contacts[i].points[j];
                    }
                    evt.objA = m.objA;
                    evt.objB = m.objB;
                    evt.cA = cA;
                    evt.cB = cB;
                }
                m.contacts[i].numPoints = 0;
            }
        }

        for (int i = 0; i < numNewContacts; ++i) {
            bool found = false;
            for (size_t j = 0; j < m.contacts.size(); ++j) {
                if (!m.contacts[j].cookie) {
                    m.contacts[j] = newContacts[i];
                    if (needEvent) {
                        // New contact.
                        events_.emplace_back();
                        auto& evt = events_.back();

                        evt.contact.cookie = m.contacts[j].cookie;
                        evt.contact.shapeA = m.contacts[j].shapeA;
                        evt.contact.shapeB = m.contacts[j].shapeB;
                        evt.contact.pointCount = m.contacts[j].numPoints;
                        for (int k = 0; k < evt.contact.pointCount; ++k) {
                            evt.contact.points[k] = *m.contacts[j].points[k];
                        }
                        evt.isNew = true;
                        evt.objA = m.objA;
                        evt.objB = m.objB;
                        evt.cA = cA;
                        evt.cB = cB;
                    }
                    m.contacts[j].numPoints = 0;
                    found = true;
                    break;
                }
            }
            btAssert(found);
            newContacts[i].shapeA.reset();
            newContacts[i].shapeB.reset();
            newContacts[i].numPoints = 0;
        }
    }
}

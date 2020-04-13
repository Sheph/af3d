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

#include "PhysicsComponentManager.h"
#include "PhysicsComponent.h"
#include "Settings.h"

namespace af3d
{
    PhysicsComponentManager::PhysicsComponentManager()
    : collisionDispatcher_(&collisionCfg_),
      world_(&collisionDispatcher_, &broadphase_, &solver_, &collisionCfg_)
    {
        world_.setWorldUserInfo(this);
    }

    PhysicsComponentManager::~PhysicsComponentManager()
    {
        btAssert(components_.empty());
        btAssert(frozenComponents_.empty());
    }

    PhysicsComponentManager* PhysicsComponentManager::fromWorld(btDynamicsWorld* world)
    {
        return static_cast<PhysicsComponentManager*>(world->getWorldUserInfo());
    }

    void PhysicsComponentManager::cleanup()
    {
        btAssert(components_.empty());
        btAssert(frozenComponents_.empty());
    }

    void PhysicsComponentManager::addComponent(const ComponentPtr& component)
    {
        PhysicsComponentPtr physicsComponent = std::static_pointer_cast<PhysicsComponent>(component);

        btAssert(!component->manager());

        components_.insert(physicsComponent);
        physicsComponent->setManager(this);
    }

    void PhysicsComponentManager::removeComponent(const ComponentPtr& component)
    {
        PhysicsComponentPtr physicsComponent = std::static_pointer_cast<PhysicsComponent>(component);

        if (components_.erase(physicsComponent) ||
            frozenComponents_.erase(physicsComponent)) {
            physicsComponent->setManager(NULL);
        }
    }

    void PhysicsComponentManager::freezeComponent(const ComponentPtr& component)
    {
        PhysicsComponentPtr physicsComponent = std::static_pointer_cast<PhysicsComponent>(component);

        components_.erase(physicsComponent);
        frozenComponents_.insert(physicsComponent);
        component->onFreeze();
    }

    void PhysicsComponentManager::thawComponent(const ComponentPtr& component)
    {
        PhysicsComponentPtr physicsComponent = std::static_pointer_cast<PhysicsComponent>(component);

        frozenComponents_.erase(physicsComponent);
        components_.insert(physicsComponent);
        component->onThaw();
    }

    bool PhysicsComponentManager::update(float dt)
    {
        return world_.stepSimulation(dt, settings.physics.maxSteps, settings.physics.fixedTimestep) > 0;
    }

    void PhysicsComponentManager::debugDraw()
    {
    }
}

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

#include "RenderComponentManager.h"
#include "RenderComponent.h"

namespace af3d
{
    RenderComponentManager::~RenderComponentManager()
    {
        assert(components_.empty());
        assert(frozenComponents_.empty());
    }

    void RenderComponentManager::cleanup()
    {
        assert(components_.empty());
        assert(frozenComponents_.empty());
    }

    void RenderComponentManager::addComponent(const ComponentPtr& component)
    {
        auto renderComponent = std::static_pointer_cast<RenderComponent>(component);

        btAssert(!component->manager());

        components_.insert(renderComponent);
        renderComponent->setManager(this);
    }

    void RenderComponentManager::removeComponent(const ComponentPtr& component)
    {
        auto renderComponent = std::static_pointer_cast<RenderComponent>(component);

        if (components_.erase(renderComponent) ||
            frozenComponents_.erase(renderComponent)) {
            renderComponent->setManager(NULL);
        }
    }

    void RenderComponentManager::freezeComponent(const ComponentPtr& component)
    {
        auto renderComponent = std::static_pointer_cast<RenderComponent>(component);

        components_.erase(renderComponent);
        frozenComponents_.insert(renderComponent);
        component->onFreeze();
    }

    void RenderComponentManager::thawComponent(const ComponentPtr& component)
    {
        auto renderComponent = std::static_pointer_cast<RenderComponent>(component);

        frozenComponents_.erase(renderComponent);
        components_.insert(renderComponent);
        component->onThaw();
    }

    void RenderComponentManager::update(float dt)
    {
        for (const auto& c : components_) {
            c->update(dt);
            if (c->renderAlways()) {
                // include in render list...
            }
        }
    }

    void RenderComponentManager::debugDraw()
    {
        for (const auto& c : components_) {
            c->debugDraw();
        }
    }

    void RenderComponentManager::render()
    {
    }
}

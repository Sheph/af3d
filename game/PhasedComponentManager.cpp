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

#include "PhasedComponentManager.h"
#include "PhasedComponent.h"

namespace af3d
{
    bool PhasedComponentComparer::operator()(const PhasedComponentPtr& l, const PhasedComponentPtr& r) const
    {
        if (l->order() == r->order()) {
            return l.get() < r.get();
        } else {
            return l->order() < r->order();
        }
    }

    PhasedComponentManager::~PhasedComponentManager()
    {
        btAssert(thinkComponents_.empty());
        btAssert(preRenderComponents_.empty());
        btAssert(frozenComponents_.empty());
    }

    void PhasedComponentManager::cleanup()
    {
        btAssert(thinkComponents_.empty());
        btAssert(preRenderComponents_.empty());
        btAssert(frozenComponents_.empty());
    }

    void PhasedComponentManager::addComponent(const ComponentPtr& component)
    {
        auto phasedComponent = std::static_pointer_cast<PhasedComponent>(component);

        btAssert(!component->manager());

        if ((phasedComponent->phases() & phaseThink) != 0) {
            thinkComponents_.insert(phasedComponent);
        }
        if ((phasedComponent->phases() & phasePreRender) != 0) {
            preRenderComponents_.insert(phasedComponent);
        }
        phasedComponent->setManager(this);
    }

    void PhasedComponentManager::removeComponent(const ComponentPtr& component)
    {
        auto phasedComponent = std::static_pointer_cast<PhasedComponent>(component);

        bool res = (thinkComponents_.erase(phasedComponent) > 0);
        res |= (preRenderComponents_.erase(phasedComponent) > 0);

        if (res || frozenComponents_.erase(phasedComponent)) {
            phasedComponent->setManager(nullptr);
        }
    }

    void PhasedComponentManager::freezeComponent(const ComponentPtr& component)
    {
        auto phasedComponent = std::static_pointer_cast<PhasedComponent>(component);

        thinkComponents_.erase(phasedComponent);
        preRenderComponents_.erase(phasedComponent);
        frozenComponents_.insert(phasedComponent);
        component->onFreeze();
    }

    void PhasedComponentManager::thawComponent(const ComponentPtr& component)
    {
        auto phasedComponent = std::static_pointer_cast<PhasedComponent>(component);

        frozenComponents_.erase(phasedComponent);
        if ((phasedComponent->phases() & phaseThink) != 0) {
            thinkComponents_.insert(phasedComponent);
        }
        if ((phasedComponent->phases() & phasePreRender) != 0) {
            preRenderComponents_.insert(phasedComponent);
        }
        component->onThaw();
    }

    bool PhasedComponentManager::update(float dt)
    {
        static std::vector<PhasedComponentPtr> tmp;

        tmp.reserve(thinkComponents_.size());

        for (const auto& c : thinkComponents_) {
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

    void PhasedComponentManager::preRender(float dt)
    {
        static std::vector<PhasedComponentPtr> tmp;

        tmp.reserve(preRenderComponents_.size());

        for (const auto& c : preRenderComponents_) {
            tmp.push_back(c);
        }

        for (const auto& c : tmp) {
            if (c->manager()) {
                c->preRender(dt);
            }
        }

        tmp.resize(0);
    }

    void PhasedComponentManager::debugDraw(RenderList& rl)
    {
        for (const auto& c : thinkComponents_) {
            c->debugDraw(rl);
        }
        for (const auto& c : preRenderComponents_) {
            c->debugDraw(rl);
        }
    }
}

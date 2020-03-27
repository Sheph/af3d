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

#include "LightComponent.h"
#include "SceneObject.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(LightComponent, RenderComponent)
    ACLASS_PROPERTY_RO(LightComponent, Children, AProperty_Children, "Children", ArrayLight, Hierarchy)
    ACLASS_DEFINE_END(LightComponent)

    LightComponent::LightComponent()
    : RenderComponent(AClass_LightComponent)
    {
    }

    LightComponent::~LightComponent()
    {
        while (!lights_.empty()) {
            lights_.back()->remove();
        }
    }

    const AClass& LightComponent::staticKlass()
    {
        return AClass_LightComponent;
    }

    AObjectPtr LightComponent::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<LightComponent>();
        obj->propertiesSet(propVals);
        return obj;
    }

    void LightComponent::update(float dt)
    {
        if (parent()->transform() != prevParentXf_) {
            prevParentXf_ = parent()->transform();
            for (const auto& light : lights_) {
                light->updateParentTransform();
            }
        }

        AABB prevAABB;
        AABB aabb;
        btVector3 displacement;

        for (const auto& light : lights_) {
            if (light->needsRenderUpdate(prevAABB, aabb, displacement)) {
                manager()->moveAABB(light->cookie(), prevAABB, aabb, displacement);
            }
        }
    }

    void LightComponent::render(RenderList& rl, void* const* parts, size_t numParts)
    {
        for (size_t i = 0; i < numParts; ++i) {
            Light* light = static_cast<Light*>(parts[i]);
            rl.addLight(light->sharedThis());
        }
    }

    std::pair<AObjectPtr, float> LightComponent::testRay(const Frustum& frustum, const Ray& ray, void* part)
    {
        Light* light = static_cast<Light*>(part);
        auto res = ray.testAABB(light->getWorldAABB());
        if (res.first) {
            return std::make_pair(light->sharedThis(), res.second);
        } else {
            return std::make_pair(AObjectPtr(), 0.0f);
        }
    }

    void LightComponent::debugDraw()
    {
    }

    void LightComponent::addLight(const LightPtr& light)
    {
        runtime_assert(!light->parent());

        light->adopt(this);

        if (scene()) {
            AABB prevAABB;
            AABB aabb;
            btVector3 displacement;

            runtime_assert(light->needsRenderUpdate(prevAABB, aabb, displacement));

            auto cookie = manager()->addAABB(this, aabb, light.get());
            light->setCookie(cookie);
        }

        lights_.push_back(light);
    }

    void LightComponent::removeLight(const LightPtr& light)
    {
        runtime_assert(light->parent() == this);

        /*
         * Hold on to this light while removing.
         */
        LightPtr tmp = light;

        if (light->cookie()) {
            manager()->removeAABB(light->cookie());
            light->setCookie(nullptr);
        }

        light->abandon();

        for (auto it = lights_.begin(); it != lights_.end(); ++it) {
            if (*it == tmp) {
                lights_.erase(it);
                break;
            }
        }
    }

    LightComponent::Lights LightComponent::getLights(const std::string& name) const
    {
        Lights res;

        for (const auto& light : lights_) {
            if (light->name() == name) {
                res.push_back(light);
            }
        }

        return res;
    }

    void LightComponent::onRegister()
    {
        prevParentXf_ = parent()->transform();
        for (const auto& light : lights_) {
            AABB prevAABB;
            AABB aabb;
            btVector3 displacement;

            runtime_assert(light->needsRenderUpdate(prevAABB, aabb, displacement));

            auto cookie = manager()->addAABB(this, aabb, light.get());
            light->setCookie(cookie);
        }
    }

    void LightComponent::onUnregister()
    {
        for (const auto& light : lights_) {
            manager()->removeAABB(light->cookie());
            light->setCookie(nullptr);
        }
    }
}

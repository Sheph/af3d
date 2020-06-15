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

#include "DirectionalLight.h"
#include "Scene.h"
#include "Logger.h"
#include "Settings.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(DirectionalLight, Light)
    ACLASS_DEFINE_END(DirectionalLight)

    DirectionalLight::DirectionalLight()
    : Light(AClass_DirectionalLight, TypeId, true)
    {
        setLocalAABB(AABB(btVector3_one * -1000.0f, btVector3_one * 1000.0f));
    }

    const AClass& DirectionalLight::staticKlass()
    {
        return AClass_DirectionalLight;
    }

    AObjectPtr DirectionalLight::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<DirectionalLight>();
        obj->propertiesSet(propVals);
        return obj;
    }

    void DirectionalLight::update(float dt)
    {
        Light::update(dt);

        for (auto it = shadowMaps_.begin(); it != shadowMaps_.end();) {
            if (it->second.immCameraIdx >= -1) {
                it->second.immCameraIdx = -2;
                ++it;
            } else {
                it->second.csm->remove();
                LOG4CPLUS_TRACE(logger(), "CSM for " << it->first << " removed = " << it->second.csm.get());
                shadowMaps_.erase(it++);
            }
        }
    }

    void DirectionalLight::render(RenderList& rl, void* const* parts, size_t numParts)
    {
        Light::render(rl, parts, numParts);

        if (!castShadow()) {
            return;
        }

        if (!rl.camera()->canSeeShadows()) {
            return;
        }

        auto it = shadowMaps_.find(rl.camera()->cookie());
        if (it == shadowMaps_.end()) {
            auto csm = std::make_shared<ShadowMapCSM>(scene());
            if (!scene()->addShadowMap(csm.get())) {
                return;
            }
            it = shadowMaps_.emplace(rl.camera()->cookie(), ShadowMapInfo(csm)).first;
            LOG4CPLUS_TRACE(logger(), "CSM for " << it->first << " added = " << csm.get());
        }
        it->second.csm->update(rl.camera()->frustum(), worldTransform());
        it->second.immCameraIdx = scene()->getImmCameraIdx(rl.camera()->cookie());
    }

    void DirectionalLight::setLocalAABB(const AABB& value)
    {
        setLocalAABBImpl(value);
    }

    void DirectionalLight::onUnregister()
    {
        Light::onUnregister();

        for (auto& kv : shadowMaps_) {
            kv.second.csm->remove();
        }
        shadowMaps_.clear();
    }

    void DirectionalLight::doSetupCluster(ShaderClusterLightImpl& cLight) const
    {
        cLight.dir = Vector4f(worldTransform().getBasis() * btVector3_forward, 0.0f);
        for (std::uint32_t i = 0; i < settings.maxImmCameras + 1; ++i) {
            btAssert(i < sizeof(cLight.shadowIdx) / sizeof(cLight.shadowIdx[0]));
            cLight.shadowIdx[i] = -1;
        }
        for (const auto &kv : shadowMaps_) {
            if (kv.second.immCameraIdx < 0) {
                continue;
            }
            btAssert(kv.second.immCameraIdx > 0);
            btAssert(kv.second.immCameraIdx < static_cast<int>(settings.maxImmCameras + 1));
            cLight.shadowIdx[kv.second.immCameraIdx] = kv.second.csm->index();
        }
    }

    void DirectionalLight::doSetCastShadow(bool value)
    {
        if (!value) {
            for (auto& kv : shadowMaps_) {
                kv.second.csm->remove();
            }
            shadowMaps_.clear();
        }
    }
}

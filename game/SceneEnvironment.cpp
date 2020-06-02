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

#include "SceneEnvironment.h"
#include "LightProbeComponent.h"
#include "Light.h"
#include "HardwareResourceManager.h"
#include "Renderer.h"
#include "Settings.h"
#include "Logger.h"
#include "ShaderDataTypes.h"

namespace af3d
{
    namespace
    {
        struct LightsSSBOUpdate : boost::noncopyable
        {
            HardwareDataBufferPtr ssbo;
            bool recreate = false;
            std::pair<int, int> indexRange{std::numeric_limits<int>::max(), 0};
            std::vector<std::pair<int, ShaderClusterLight>> lights;
        };
    };

    SceneEnvironment::SceneEnvironment()
    : lightsSSBO_(hwManager.createDataBuffer(HardwareBuffer::Usage::DynamicDraw, sizeof(ShaderClusterLight)))
    {
        for (int i = 0; i < static_cast<int>(settings.cluster.maxLights); ++i) {
            lightsFreeIndices_.insert(i);
        }
    }

    SceneEnvironment::~SceneEnvironment()
    {
        btAssert(lights_.empty());
        btAssert(lightProbes_.empty());
        btAssert(globalProbe_ == nullptr);
    }

    void SceneEnvironment::update(float realDt, float dt)
    {
        realDt_ = realDt;
        dt_ = dt;
        time_ += dt;
    }

    void SceneEnvironment::preSwap()
    {
        defaultVa_.upload();

        bool recreate = lightsSSBO_->setValid();
        if (!lights_.empty() || !lightsRemovedIndices_.empty() || recreate) {
            auto upd = std::make_shared<LightsSSBOUpdate>();
            upd->ssbo = lightsSSBO_;
            upd->recreate = recreate;
            upd->lights.reserve(lights_.size() + lightsRemovedIndices_.size());
            for (auto light : lights_) {
                upd->indexRange.first = std::min(upd->indexRange.first, light->index());
                upd->indexRange.second = std::max(upd->indexRange.second, light->index());
                upd->lights.emplace_back(light->index(), ShaderClusterLight());
                light->setupCluster(upd->lights.back().second);
            }
            for (auto idx : lightsRemovedIndices_) {
                upd->indexRange.first = std::min(upd->indexRange.first, idx);
                upd->indexRange.second = std::max(upd->indexRange.second, idx);
                upd->lights.emplace_back(idx, ShaderClusterLight());
            }
            lightsRemovedIndices_.clear();
            renderer.scheduleHwOp([upd](HardwareContext& ctx) {
                ShaderClusterLight* ptr;
                if (upd->recreate) {
                    upd->ssbo->resize(settings.cluster.maxLights, ctx);
                    ptr = (ShaderClusterLight*)upd->ssbo->lock(HardwareBuffer::WriteOnly, ctx);
                    auto ptrBase = ptr;
                    for (int i = 0; i < upd->ssbo->count(ctx); ++i, ++ptr) {
                        ptr->enabled = 0;
                    }
                    ptr = ptrBase + upd->indexRange.first;
                } else {
                    btAssert(upd->indexRange.second >= upd->indexRange.first);
                    ptr = (ShaderClusterLight*)upd->ssbo->lock(upd->indexRange.first,
                        upd->indexRange.second - upd->indexRange.first + 1, HardwareBuffer::ReadWrite, ctx);
                }
                for (const auto& lp : upd->lights) {
                    auto dest = ptr + (lp.first - upd->indexRange.first);
                    *dest = lp.second;
                }
                upd->ssbo->unlock(ctx);
            });
        }
    }

    int SceneEnvironment::addLight(Light* light)
    {
        if (lightsFreeIndices_.empty()) {
            LOG4CPLUS_WARN(logger(), "Too many lights...");
            return -1;
        }
        int idx = *lightsFreeIndices_.begin();
        lightsFreeIndices_.erase(lightsFreeIndices_.begin());
        lights_.insert(light);
        lightsRemovedIndices_.erase(idx);
        return idx;
    }

    void SceneEnvironment::removeLight(Light* light)
    {
        if (lights_.erase(light) <= 0) {
            return;
        }
        btAssert(light->index() >= 0);
        bool res = lightsFreeIndices_.insert(light->index()).second;
        btAssert(res);
        lightsRemovedIndices_.insert(light->index());
    }

    void SceneEnvironment::addLightProbe(LightProbeComponent* probe)
    {
        if (probe->isGlobal()) {
            globalProbe_ = probe;
        } else {
            lightProbes_.insert(probe);
        }
    }

    void SceneEnvironment::removeLightProbe(LightProbeComponent* probe)
    {
        if (probe == globalProbe_) {
            globalProbe_ = nullptr;
        } else {
            lightProbes_.erase(probe);
        }
    }

    void SceneEnvironment::updateLightProbes()
    {
        if (globalProbe_) {
            globalProbe_->recreate();
        }
        for (auto probe : lightProbes_) {
            probe->recreate();
        }
    }

    LightProbeComponent* SceneEnvironment::getLightProbeFor(const btVector3& pos)
    {
        return lightProbes_.empty() ? globalProbe_ : *lightProbes_.begin();
    }
}

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

#ifndef _SCENE_ENVIRONMENT_H_
#define _SCENE_ENVIRONMENT_H_

#include "VertexArrayWriter.h"
#include "RenderTarget.h"

namespace af3d
{
    class Light;
    class LightProbeComponent;

    class SceneEnvironment : boost::noncopyable
    {
    public:
        SceneEnvironment();
        ~SceneEnvironment();

        inline float realDt() const { return realDt_; }
        inline float dt() const { return dt_; }
        inline float time() const { return time_; }
        inline const VertexArrayWriter& defaultVa() const { return defaultVa_; }
        inline VertexArrayWriter& defaultVa() { return defaultVa_; }
        inline LightProbeComponent* globalLightProbe() { return globalProbe_; }

        void update(float realDt, float dt);

        void preSwap();

        int addLight(Light* light);

        void removeLight(Light* light);

        LightProbeRenderTarget addLightProbe(LightProbeComponent* probe);

        void removeLightProbe(LightProbeComponent* probe);

        void updateLightProbes();

        inline const HardwareDataBufferPtr& lightsSSBO() const { return lightsSSBO_; }

        inline const HardwareDataBufferPtr& probesSSBO() const { return probesSSBO_; }

        inline const TexturePtr& irradianceTexture() const { return irradianceTexture_; }

        inline const TexturePtr& specularTexture() const { return specularTexture_; }

    private:
        using IndexSet = std::set<int>;

        void preSwapLights();

        void preSwapProbes();

        void updateProbeTextures(LightProbeComponent* probe);

        float realDt_ = 0.0f;
        float dt_ = 0.0f;
        float time_ = 0.0f;
        VertexArrayWriter defaultVa_;
        HardwareDataBufferPtr lightsSSBO_;
        HardwareDataBufferPtr probesSSBO_;
        TexturePtr irradianceTexture_;
        std::uint32_t irradianceTextureGeneration_ = std::numeric_limits<std::uint32_t>::max();
        TexturePtr specularTexture_;

        std::unordered_set<Light*> lights_;
        IndexSet lightsFreeIndices_;
        IndexSet lightsRemovedIndices_;

        LightProbeComponent* globalProbe_ = nullptr;
        std::unordered_set<LightProbeComponent*> probes_;
        IndexSet probesFreeIndices_;
        std::unordered_set<LightProbeComponent*> probesToCheck_;
        bool probesNeedUpdate_ = true;
    };

    using SceneEnvironmentPtr = std::shared_ptr<SceneEnvironment>;
}

#endif

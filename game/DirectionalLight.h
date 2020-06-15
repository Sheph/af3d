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

#ifndef _DIRECTIONAL_LIGHT_H_
#define _DIRECTIONAL_LIGHT_H_

#include "Light.h"
#include "ShadowMapCSM.h"

namespace af3d
{
    class DirectionalLight : public std::enable_shared_from_this<DirectionalLight>,
        public Light
    {
    public:
        static const int TypeId = 1;

        explicit DirectionalLight();
        ~DirectionalLight() = default;

        static const AClass& staticKlass();

        static AObjectPtr create(const APropertyValueMap& propVals);

        AObjectPtr sharedThis() override { return shared_from_this(); }

        void update(float dt) override;

        void render(RenderList& rl, void* const* parts, size_t numParts) override;

        void setLocalAABB(const AABB& value);

    private:
        struct ShadowMapInfo
        {
            ShadowMapInfo() = default;
            explicit ShadowMapInfo(const ShadowMapCSMPtr& csm) : csm(csm) {}

            ShadowMapCSMPtr csm;
            int immCameraIdx = -1;
        };

        using ShadowMaps = std::unordered_map<ACookie, ShadowMapInfo>; // viewCam cookie -> shadow map info.

        void onUnregister() override;

        void doSetupCluster(ShaderClusterLightImpl& cLight) const override;

        void doSetCastShadow(bool value) override;

        ShadowMaps shadowMaps_;
    };

    using DirectionalLightPtr = std::shared_ptr<DirectionalLight>;

    ACLASS_DECLARE(DirectionalLight)
}

#endif

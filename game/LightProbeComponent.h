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

#ifndef _LIGHTPROBECOMPONENT_H_
#define _LIGHTPROBECOMPONENT_H_

#include "PhasedComponent.h"
#include "RenderFilterComponent.h"
#include "RenderMeshComponent.h"
#include "RenderProxyComponent.h"
#include "Equirect2CubeComponent.h"
#include "Texture.h"
#include "ShaderDataTypes.h"
#include <boost/optional.hpp>

namespace af3d
{
    class LightProbeComponent : public std::enable_shared_from_this<LightProbeComponent>,
        public PhasedComponent
    {
    public:
        explicit LightProbeComponent(const boost::optional<AABB>& bounds = boost::optional<AABB>(), bool spherical = false,
            const Color& ambientColor = Color_one, const Color& specularColor = Color_zero);
        ~LightProbeComponent() = default;

        static const AClass& staticKlass();

        static AObjectPtr create(const APropertyValueMap& propVals);

        AObjectPtr sharedThis() override { return shared_from_this(); }

        void preRender(float dt) override;

        bool recreate();

        bool resetDirty();

        void setupCluster(ShaderClusterProbe& cProbe);

        inline int index() const { return rt_.index; }

        inline bool isGlobal() const { return !bounds_; }

        inline const AABB& bounds() const { return bounds_ ? *bounds_ : AABB_empty; }

        inline bool hasIrradiance() const { return !!irrEquirect2cube_; }
        inline bool hasSpecular() const { return !!specularEquirect2cube_; }
        inline const TexturePtr& specularLUTTexture() const { return specularLUTTexture_; }

        inline const Color& ambientColor() const { return ambientColor_; }
        inline const Color& specularColor() const { return specularColor_; }

    private:
        static const std::uint32_t sceneCaptureSize = 512;
        static const std::uint32_t specularLUTSize = 512;

        void onRegister() override;

        void onUnregister() override;

        void startIrradianceGen();

        void stopIrradianceGen();

        void startSpecularGen();

        void stopSpecularGen();

        std::string getIrradianceTexName();

        std::string getSpecularTexName();

        std::string getSpecularLUTTexName();

        void renderBounds(RenderList& rl);

        bool dirty_ = true;
        btTransform prevXf_ = btTransform::getIdentity();

        boost::optional<AABB> bounds_;
        bool spherical_;
        Color ambientColor_;
        Color specularColor_;

        LightProbeRenderTarget rt_;
        TexturePtr specularLUTTexture_;

        std::array<CameraPtr, 6> sceneCaptureCameras_;
        std::array<RenderFilterComponentPtr, 6> irrGenFilters_;
        RenderFilterComponentPtr irrCube2equirectFilter_;

        std::vector<RenderFilterComponentPtr> specularGenFilters_;
        std::vector<RenderFilterComponentPtr> specularCube2EquirectFilters_;
        RenderFilterComponentPtr specularLUTGenFilter_;

        Equirect2CubeComponentPtr irrEquirect2cube_;
        Equirect2CubeComponentPtr specularEquirect2cube_;

        RenderMeshComponentPtr markerRc_;
        RenderProxyComponentPtr boundsRc_;
    };

    using LightProbeComponentPtr = std::shared_ptr<LightProbeComponent>;

    ACLASS_DECLARE(LightProbeComponent)
}

#endif

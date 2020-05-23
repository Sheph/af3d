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

#ifndef _RENDERSKYBOXCOMPONENT_H_
#define _RENDERSKYBOXCOMPONENT_H_

#include "RenderComponent.h"
#include "Equirect2CubeComponent.h"

namespace af3d
{
    class RenderSkyBoxComponent : public std::enable_shared_from_this<RenderSkyBoxComponent>,
        public RenderComponent
    {
    public:
        explicit RenderSkyBoxComponent(const TexturePtr& texture);
        ~RenderSkyBoxComponent() = default;

        static const AClass& staticKlass();

        static AObjectPtr create(const APropertyValueMap& propVals);

        AObjectPtr sharedThis() override { return shared_from_this(); }

        void update(float dt) override;

        void render(RenderList& rl, void* const* parts, size_t numParts) override;

        std::pair<AObjectPtr, float> testRay(const Frustum& frustum, const Ray& ray, void* part) override;

        // Sky box should always go into light probes.
        const CameraFilter& getCameraFilterWithFixup() const override { return cameraFilter(); }

    private:
        void onRegister() override;

        void onUnregister() override;

        Equirect2CubeComponentPtr equirect2cube_;
        MeshPtr mesh_;

        boost::optional<Matrix4f> prevModelMat_;
        boost::optional<Matrix4f> modelMat_;
    };

    using RenderSkyBoxComponentPtr = std::shared_ptr<RenderSkyBoxComponent>;

    ACLASS_DECLARE(RenderSkyBoxComponent)
}

#endif
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

#ifndef _TAACOMPONENT_H_
#define _TAACOMPONENT_H_

#include "PhasedComponent.h"
#include "RenderFilterComponent.h"
#include "Camera.h"

namespace af3d
{
    class TAAComponent : public std::enable_shared_from_this<TAAComponent>,
        public PhasedComponent
    {
    public:
        TAAComponent(const CameraPtr& srcCamera,
            const TexturePtr& inputTexture,
            const TexturePtr& velocityTexture,
            const TexturePtr& depthTexture,
            const std::vector<MaterialPtr>& destMaterials, int camOrder);
        ~TAAComponent() = default;

        static const AClass& staticKlass();

        static AObjectPtr create(const APropertyValueMap& propVals);

        AObjectPtr sharedThis() override { return shared_from_this(); }

        void preRender(float dt) override;

    private:
        static float haltonNumber(int base, int index);

        static float catmullRom(float x);

        void onRegister() override;

        void onUnregister() override;

        void updateTextureBindings(const TexturePtr& prevTex, const TexturePtr& outTex);

        CameraPtr srcCamera_;
        std::vector<MaterialPtr> destMaterials_;
        RenderFilterComponentPtr taaFilter_;

        TexturePtr prevTex_;
        TexturePtr outTex_;

        std::vector<Vector2f> jitters_;
        size_t jitterIdx_ = 0;
        std::vector<float> sampleWeights_;
        std::vector<float> lowpassWeights_;
        std::vector<float> plusWeights_;
    };

    using TAAComponentPtr = std::shared_ptr<TAAComponent>;

    ACLASS_DECLARE(TAAComponent)
}

#endif

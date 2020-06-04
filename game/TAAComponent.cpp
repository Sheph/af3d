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

#include "TAAComponent.h"
#include "TextureManager.h"
#include "SceneObject.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(TAAComponent, PhasedComponent)
    ACLASS_DEFINE_END(TAAComponent)

    TAAComponent::TAAComponent(const CameraPtr& srcCamera, const std::vector<MaterialPtr>& destMaterials, int camOrder)
    : PhasedComponent(AClass_TAAComponent, phasePreRender, 9999),
      srcCamera_(srcCamera),
      destMaterials_(destMaterials),
      sampleWeights_(9),
      lowpassWeights_(9),
      plusWeights_(5)
    {
        std::vector<Byte> data(srcCamera->renderTarget().texture()->width() * srcCamera->renderTarget().texture()->height() * 3);
        prevTex_ = textureManager.createRenderTextureScaled(TextureType2D, 1.0f, 0, GL_RGB16F, GL_RGB, GL_UNSIGNED_BYTE, false, std::move(data));
        std::vector<Byte> data2(srcCamera->renderTarget().texture()->width() * srcCamera->renderTarget().texture()->height() * 3);
        outTex_ = textureManager.createRenderTextureScaled(TextureType2D, 1.0f, 0, GL_RGB16F, GL_RGB, GL_UNSIGNED_BYTE, false, std::move(data2));

        taaFilter_ = std::make_shared<RenderFilterComponent>(MaterialTypeFilterTAA);
        taaFilter_->material()->setTextureBinding(SamplerName::Main,
            TextureBinding(srcCamera->renderTarget().texture(),
                SamplerParams(GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR)));
        taaFilter_->material()->setTextureBinding(SamplerName::Noise,
            TextureBinding(srcCamera->renderTarget(AttachmentPoint::Color1).texture(),
                SamplerParams(GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR)));
        taaFilter_->material()->setTextureBinding(SamplerName::Depth,
            TextureBinding(srcCamera->renderTarget(AttachmentPoint::Depth).texture(),
                SamplerParams(GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR)));
        taaFilter_->camera()->setOrder(camOrder);

        for (int i = 1; i <= 16; ++i) {
            jitters_.emplace_back((haltonNumber(2, i) - 0.5f) * 2.0f, (haltonNumber(3, i) - 0.5f) * 2.0f);
        }
    }

    const AClass& TAAComponent::staticKlass()
    {
        return AClass_TAAComponent;
    }

    AObjectPtr TAAComponent::create(const APropertyValueMap& propVals)
    {
        return AObjectPtr();
    }

    void TAAComponent::preRender(float dt)
    {
        static const float sampleOffsets[9][2] =
        {
            { -1.0f, -1.0f },
            {  0.0f, -1.0f },
            {  1.0f, -1.0f },
            { -1.0f,  0.0f },
            {  0.0f,  0.0f },
            {  1.0f,  0.0f },
            { -1.0f,  1.0f },
            {  0.0f,  1.0f },
            {  1.0f,  1.0f },
        };

        updateTextureBindings(prevTex_, outTex_);

        Vector2f jitter = jitters_[jitterIdx_++] * 0.9f / Vector2f(prevTex_->width(), prevTex_->height());
        jitterIdx_ = jitterIdx_ % jitters_.size();

        srcCamera_->setJitter(jitter);

        float jitterX = srcCamera_->jitter().x() * 0.5f;
        float jitterY = srcCamera_->jitter().y() * -0.5f;

        float sharpness = 1.0f;

        float weights[9];
        float weightsLow[9];
        float weightsPlus[5];
        float totalWeight = 0.0f;
        float totalWeightLow = 0.0f;
        float totalWeightPlus = 0.0f;

        for (int i = 0; i < 9; ++i) {
            float pixelOffsetX = sampleOffsets[i][0] - jitterX;
            float pixelOffsetY = sampleOffsets[i][1] - jitterY;

            if (sharpness > 1.0f) {
                weights[i] = catmullRom(pixelOffsetX ) * catmullRom(pixelOffsetY);
                totalWeight += weights[i];
            } else {
                // Exponential fit to Blackman-Harris 3.3
                pixelOffsetX *= 1.0f + sharpness * 0.5f;
                pixelOffsetY *= 1.0f + sharpness * 0.5f;
                weights[i] = btExp(-2.29f * (pixelOffsetX * pixelOffsetX + pixelOffsetY * pixelOffsetY));
                totalWeight += weights[i];
            }

            // Lowpass.
            pixelOffsetX = sampleOffsets[i][0] - jitterX;
            pixelOffsetY = sampleOffsets[i][1] - jitterY;
            pixelOffsetX *= 0.25f;
            pixelOffsetY *= 0.25f;
            pixelOffsetX *= 1.0f + sharpness * 0.5f;
            pixelOffsetY *= 1.0f + sharpness * 0.5f;
            weightsLow[i] = btExp(-2.29f * (pixelOffsetX * pixelOffsetX + pixelOffsetY * pixelOffsetY));
            totalWeightLow += weightsLow[i];
        }

        weightsPlus[0] = weights[1];
        weightsPlus[1] = weights[3];
        weightsPlus[2] = weights[4];
        weightsPlus[3] = weights[5];
        weightsPlus[4] = weights[7];
        totalWeightPlus = weights[1] + weights[3] + weights[4] + weights[5] + weights[7];

        for (int i = 0; i < 9; ++i) {
            sampleWeights_[i] = weights[i] / totalWeight;
            lowpassWeights_[i] = weightsLow[i] / totalWeightLow;
        }

        for (int i = 0; i < 5; ++i) {
            plusWeights_[i] = weightsPlus[i] / totalWeightPlus;
        }

        taaFilter_->material()->params().setUniform(UniformName::ArgPrevViewProjMatrix, srcCamera_->prevViewProjMat());
        taaFilter_->material()->params().setUniform(UniformName::ArgViewProjMatrix, srcCamera_->frustum().viewProjMat());
        taaFilter_->material()->params().setUniform(UniformName::SampleWeights, sampleWeights_, true);
        taaFilter_->material()->params().setUniform(UniformName::LowpassWeights, lowpassWeights_, true);
        taaFilter_->material()->params().setUniform(UniformName::PlusWeights, plusWeights_);

        std::swap(prevTex_, outTex_);
    }

    float TAAComponent::haltonNumber(int base, int index)
    {
        float result = 0.0f;
        float f = 1.0f;
        while (index > 0) {
            f /= base;
            result += f * (index % base);
            index = std::floor(index / base);
        }
        return result;
    }

    float TAAComponent::catmullRom(float x)
    {
        float ax = btFabs(x);
        if (ax > 1.0f) {
            return ((-0.5f * ax + 2.5f ) * ax - 4.0f) * ax + 2.0f;
        } else {
            return (1.5f * ax - 2.5f) * ax * ax + 1.0f;
        }
    }

    void TAAComponent::onRegister()
    {
        parent()->addComponent(taaFilter_);
    }

    void TAAComponent::onUnregister()
    {
        parent()->removeComponent(taaFilter_);
    }

    void TAAComponent::updateTextureBindings(const TexturePtr& prevTex, const TexturePtr& outTex)
    {
        taaFilter_->material()->setTextureBinding(SamplerName::Prev,
            TextureBinding(prevTex,
                SamplerParams(GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR)));
        taaFilter_->camera()->setRenderTarget(AttachmentPoint::Color0, RenderTarget(outTex));

        for (const auto& m : destMaterials_) {
            auto params = m->textureBinding(SamplerName::Main).params;
            m->setTextureBinding(SamplerName::Main, TextureBinding(outTex, params));
        }
    }
}

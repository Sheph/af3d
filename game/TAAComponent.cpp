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

    static float CatmullRom( float x )
    {
        float ax = std::fabs(x);
        if( ax > 1.0f )
            return ( ( -0.5f * ax + 2.5f ) * ax - 4.0f ) *ax + 2.0f;
        else
            return ( 1.5f * ax - 2.5f ) * ax*ax + 1.0f;
    }

    TAAComponent::TAAComponent(const CameraPtr& srcCamera, const std::vector<MaterialPtr>& destMaterials, int camOrder)
    : PhasedComponent(AClass_TAAComponent, phasePreRender, 9999),
      srcCamera_(srcCamera),
      destMaterials_(destMaterials),
      curTex_(srcCamera->renderTarget().texture()),
      outTex_(textureManager.createRenderTextureScaled(TextureType2D, 1.0f, GL_RGBA32F, GL_RGBA, GL_FLOAT))
    {
        std::vector<Byte> data(srcCamera->renderTarget().texture()->width() * srcCamera->renderTarget().texture()->height() * 4);
        prevTex_ = textureManager.createRenderTextureScaled(TextureType2D, 1.0f, GL_RGBA32F, GL_RGBA, GL_UNSIGNED_BYTE, false, std::move(data));

        taaFilter_ = std::make_shared<RenderFilterComponent>(MaterialTypeFilterTAA);
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
        updateTextureBindings(prevTex_, curTex_, outTex_);

        Vector2f jitter = jitters_[jitterIdx_++] * 0.9f / Vector2f(curTex_->width(), curTex_->height());
        jitterIdx_ = jitterIdx_ % jitters_.size();

        srcCamera_->setJitter(jitter);

        float JitterX = srcCamera_->jitter().x() *  0.5f;
        float JitterY = srcCamera_->jitter().y() * -0.5f;

        //taaFilter_->material()->params().setUniform(UniformName::ArgJitter, srcCamera_->jitter() * -0.5f);
        taaFilter_->material()->params().setUniform(UniformName::ArgPrevViewProjMatrix, srcCamera_->prevViewProjMat());
        taaFilter_->material()->params().setUniform(UniformName::ArgViewProjMatrix, srcCamera_->frustum().viewProjMat());

        static const float SampleOffsets[9][2] =
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

        float Sharpness = 1.0f;

        float Weights[9];
        float WeightsLow[9];
        float WeightsPlus[5];
        float TotalWeight = 0.0f;
        float TotalWeightLow = 0.0f;
        float TotalWeightPlus = 0.0f;
        for( int i = 0; i < 9; i++ )
        {
            float PixelOffsetX = SampleOffsets[i][0] - JitterX;
            float PixelOffsetY = SampleOffsets[i][1] - JitterY;

            if( Sharpness > 1.0f )
            {
                Weights[i] = CatmullRom( PixelOffsetX ) * CatmullRom( PixelOffsetY );
                TotalWeight += Weights[i];
            }
            else
            {
                // Exponential fit to Blackman-Harris 3.3
                PixelOffsetX *= 1.0f + Sharpness * 0.5f;
                PixelOffsetY *= 1.0f + Sharpness * 0.5f;
                Weights[i] = btExp( -2.29f * ( PixelOffsetX * PixelOffsetX + PixelOffsetY * PixelOffsetY ) );
                TotalWeight += Weights[i];
            }

            // Lowpass.
            PixelOffsetX = SampleOffsets[i][0] - JitterX;
            PixelOffsetY = SampleOffsets[i][1] - JitterY;
            PixelOffsetX *= 0.25f;
            PixelOffsetY *= 0.25f;
            PixelOffsetX *= 1.0f + Sharpness * 0.5f;
            PixelOffsetY *= 1.0f + Sharpness * 0.5f;
            WeightsLow[i] = btExp( -2.29f * ( PixelOffsetX * PixelOffsetX + PixelOffsetY * PixelOffsetY ) );
            TotalWeightLow += WeightsLow[i];
        }

        WeightsPlus[0] = Weights[1];
        WeightsPlus[1] = Weights[3];
        WeightsPlus[2] = Weights[4];
        WeightsPlus[3] = Weights[5];
        WeightsPlus[4] = Weights[7];
        TotalWeightPlus = Weights[1] + Weights[3] + Weights[4] + Weights[5] + Weights[7];

        std::vector<float> sampleWeights(9);
        std::vector<float> lowpassWeights(9);
        std::vector<float> plusWeights(5);

        for( int i = 0; i < 9; i++ )
        {
            sampleWeights[i] = Weights[i] / TotalWeight;
            lowpassWeights[i] = WeightsLow[i] / TotalWeightLow;
        }

        for( int i = 0; i < 5; i++ )
        {
            plusWeights[i] = WeightsPlus[i] / TotalWeightPlus;
        }

        //taaFilter_->material()->params().setUniform(UniformName::SampleWeights, sampleWeights);
        //taaFilter_->material()->params().setUniform(UniformName::LowpassWeights, lowpassWeights);
        taaFilter_->material()->params().setUniform(UniformName::PlusWeights, plusWeights);
        taaFilter_->material()->params().setUniform(UniformName::VelocityScaling, 1.0f);

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

    void TAAComponent::onRegister()
    {
        parent()->addComponent(taaFilter_);
    }

    void TAAComponent::onUnregister()
    {
        parent()->removeComponent(taaFilter_);
    }

    void TAAComponent::updateTextureBindings(const TexturePtr& prevTex,
        const TexturePtr& curTex, const TexturePtr& outTex)
    {
        srcCamera_->setRenderTarget(AttachmentPoint::Color0, RenderTarget(curTex));

        taaFilter_->material()->setTextureBinding(SamplerName::Main,
            TextureBinding(curTex,
                SamplerParams(GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR)));
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

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

        taaFilter_->material()->params().setUniform(UniformName::ArgJitter, srcCamera_->jitter() * -0.5f);
        taaFilter_->material()->params().setUniform(UniformName::ArgPrevViewProjMatrix, srcCamera_->prevViewProjMat());
        taaFilter_->material()->params().setUniform(UniformName::ArgViewProjMatrix, srcCamera_->frustum().viewProjMat());

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

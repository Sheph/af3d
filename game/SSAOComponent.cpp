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

#include "SSAOComponent.h"
#include "TextureManager.h"
#include "SceneObject.h"
#include "Const.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(SSAOComponent, PhasedComponent)
    ACLASS_DEFINE_END(SSAOComponent)

    SSAOComponent::SSAOComponent(const CameraPtr& srcCamera,
        const TexturePtr& depthTexture,
        const TexturePtr& normalTexture,
        int ksize, int camOrder)
    : PhasedComponent(AClass_SSAOComponent, phasePreRender, phaseOrderSSAO),
      srcCamera_(srcCamera)
    {
        int blurKSize = 11;
        float blurSigma = 2.0f;

        auto outTex1 = textureManager.createRenderTextureScaled(TextureType2D,
            2.0f, 0, GL_R16F, GL_RED, GL_FLOAT);
        auto outTex2 = textureManager.createRenderTextureScaled(TextureType2D,
            2.0f, 0, GL_R16F, GL_RED, GL_FLOAT);

        ssaoFilter_ = std::make_shared<RenderFilterComponent>(MaterialTypeFilterSSAO);
        ssaoFilter_->material()->setTextureBinding(SamplerName::Depth,
            TextureBinding(depthTexture,
                SamplerParams(GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST)));
        ssaoFilter_->material()->setTextureBinding(SamplerName::Normal,
            TextureBinding(normalTexture,
                SamplerParams(GL_NEAREST, GL_NEAREST)));
        ssaoFilter_->material()->setTextureBinding(SamplerName::Noise,
            TextureBinding(textureManager.ssaoNoise(),
                SamplerParams(GL_NEAREST, GL_NEAREST)));
        ssaoFilter_->camera()->setOrder(camOrder);
        ssaoFilter_->camera()->setRenderTarget(AttachmentPoint::Color0, RenderTarget(outTex1));
        setSSAOKernelParams(ssaoFilter_->material()->params(), ksize);
        ssaoFilter_->material()->params().setUniform(UniformName::Radius, 0.5f);

        blurFilter_[0] = std::make_shared<RenderFilterComponent>(MaterialTypeFilterSSAOBlur);
        blurFilter_[0]->material()->setTextureBinding(SamplerName::Main,
            TextureBinding(outTex1,
                SamplerParams(GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR)));
        blurFilter_[0]->material()->setTextureBinding(SamplerName::Depth,
            TextureBinding(depthTexture,
                SamplerParams(GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST)));
        blurFilter_[0]->camera()->setOrder(camOrder + 1);
        blurFilter_[0]->camera()->setRenderTarget(AttachmentPoint::Color0, RenderTarget(outTex2));
        setGaussianBlurParams(blurFilter_[0]->material()->params(), blurKSize, blurSigma, false);

        blurFilter_[1] = std::make_shared<RenderFilterComponent>(MaterialTypeFilterSSAOBlur);
        blurFilter_[1]->material()->setTextureBinding(SamplerName::Main,
            TextureBinding(outTex2,
                SamplerParams(GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR)));
        blurFilter_[1]->material()->setTextureBinding(SamplerName::Depth,
            TextureBinding(depthTexture,
                SamplerParams(GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST)));
        blurFilter_[1]->camera()->setOrder(camOrder + 2);
        blurFilter_[1]->camera()->setRenderTarget(AttachmentPoint::Color0, RenderTarget(outTex1));
        setGaussianBlurParams(blurFilter_[1]->material()->params(), blurKSize, blurSigma, true);
    }

    const AClass& SSAOComponent::staticKlass()
    {
        return AClass_SSAOComponent;
    }

    AObjectPtr SSAOComponent::create(const APropertyValueMap& propVals)
    {
        return AObjectPtr();
    }

    void SSAOComponent::preRender(float dt)
    {
        ssaoFilter_->material()->params().setUniform(UniformName::ArgViewProjMatrix, srcCamera_->frustum().jitteredViewProjMat());

        Vector2f nearFar(srcCamera_->frustum().nearDist(), srcCamera_->frustum().farDist());

        ssaoFilter_->material()->params().setUniform(UniformName::ArgNearFar, nearFar);
        blurFilter_[0]->material()->params().setUniform(UniformName::ArgNearFar, nearFar);
        blurFilter_[1]->material()->params().setUniform(UniformName::ArgNearFar, nearFar);
    }

    void SSAOComponent::onRegister()
    {
        parent()->addComponent(ssaoFilter_);
        parent()->addComponent(blurFilter_[0]);
        parent()->addComponent(blurFilter_[1]);
    }

    void SSAOComponent::onUnregister()
    {
        parent()->removeComponent(ssaoFilter_);
        parent()->removeComponent(blurFilter_[0]);
        parent()->removeComponent(blurFilter_[1]);
    }
}

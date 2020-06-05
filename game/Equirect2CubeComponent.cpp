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

#include "Equirect2CubeComponent.h"
#include "MaterialManager.h"
#include "MeshManager.h"
#include "Const.h"
#include "SceneObject.h"
#include "Logger.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(Equirect2CubeComponent, PhasedComponent)
    ACLASS_DEFINE_END(Equirect2CubeComponent)

    Equirect2CubeComponent::Equirect2CubeComponent(const TexturePtr& src, const TexturePtr& target, std::uint32_t layer, int camOrder, std::uint32_t numMipLevels)
    : PhasedComponent(AClass_Equirect2CubeComponent, phasePreRender),
      targetGeneration_(target->generation())
    {
        auto filterMaterial = materialManager.createMaterial(MaterialTypeFilterEquirect2Cube);
        filterMaterial->setDepthTest(false);
        filterMaterial->setDepthWrite(false);
        filterMaterial->setCullFaceMode(0);
        filterMaterial->setTextureBinding(SamplerName::Main,
            TextureBinding(src, SamplerParams((numMipLevels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR), GL_LINEAR)));
        auto baseMesh = meshManager.createBoxMesh(btVector3(2.0f, 2.0f, 2.0f), filterMaterial);
        for (std::uint32_t mip = 0; mip < numMipLevels; ++mip) {
            auto mesh = baseMesh->clone();
            mesh->subMeshes()[0]->material()->params().setUniform(UniformName::MipLevel, static_cast<float>(mip));
            for (size_t i = 0; i < 6; ++i) {
                auto face = static_cast<TextureCubeFace>(i);
                auto filter = std::make_shared<RenderFilterComponent>(mesh);
                filter->camera()->setOrder(camOrder);
                filter->camera()->setFov(btRadians(90.0f));
                filter->camera()->setAspect(1.0f);
                filter->camera()->setTransform(btTransform(textureCubeFaceBasis(face)));
                filter->camera()->setRenderTarget(AttachmentPoint::Color0, RenderTarget(target, mip, face, layer));
                filters_.push_back(filter);
            }
        }
    }

    const AClass& Equirect2CubeComponent::staticKlass()
    {
        return AClass_Equirect2CubeComponent;
    }

    AObjectPtr Equirect2CubeComponent::create(const APropertyValueMap& propVals)
    {
        return AObjectPtr();
    }

    void Equirect2CubeComponent::preRender(float dt)
    {
        if (filters_[0]->scene()) {
            if (filters_[0]->numFramesRendered() > 0) {
                for (size_t i = 0; i < filters_.size(); ++i) {
                    filters_[i]->removeFromParent();
                }
            }
        } else if (filters_[0]->camera()->renderTarget().texture()->generation() != targetGeneration_) {
            LOG4CPLUS_INFO(logger(), "Regenerating equirect2cube for " << parent()->name());
            targetGeneration_ = filters_[0]->camera()->renderTarget().texture()->generation();
            for (size_t i = 0; i < filters_.size(); ++i) {
                parent()->addComponent(filters_[i]);
            }
        }
    }

    void Equirect2CubeComponent::onRegister()
    {
        for (size_t i = 0; i < filters_.size(); ++i) {
            parent()->addComponent(filters_[i]);
        }
    }

    void Equirect2CubeComponent::onUnregister()
    {
        if (filters_[0]->scene()) {
            for (size_t i = 0; i < filters_.size(); ++i) {
                filters_[i]->removeFromParent();
            }
        }
    }
}

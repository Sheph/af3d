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

#include "RenderSkyBoxComponent.h"
#include "SceneObject.h"
#include "MaterialManager.h"
#include "TextureManager.h"
#include "MeshManager.h"
#include "Const.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(RenderSkyBoxComponent, RenderComponent)
    ACLASS_DEFINE_END(RenderSkyBoxComponent)

    RenderSkyBoxComponent::RenderSkyBoxComponent(const TexturePtr& texture)
    : RenderComponent(AClass_RenderSkyBoxComponent, true)
    {
        std::uint32_t cubeSize = 2048;
        auto cubeTexture = textureManager.createRenderTexture(TextureTypeCubeMap,
            cubeSize, cubeSize, GL_RGB16F, GL_RGB, GL_FLOAT);
        equirect2cube_ = std::make_shared<Equirect2CubeComponent>(texture, cubeTexture, camOrderSkyBox);

        auto material = materialManager.createMaterial(MaterialTypeSkyBox);
        material->setTextureBinding(SamplerName::Main, TextureBinding(cubeTexture, SamplerParams(GL_LINEAR)));
        material->setCullFaceMode(GL_FRONT);

        mesh_ = meshManager.createBoxMesh(btVector3(2.0f, 2.0f, 2.0f), material);
    }

    const AClass& RenderSkyBoxComponent::staticKlass()
    {
        return AClass_RenderSkyBoxComponent;
    }

    AObjectPtr RenderSkyBoxComponent::create(const APropertyValueMap& propVals)
    {
        return AObjectPtr();
    }

    void RenderSkyBoxComponent::update(float dt)
    {
        if (modelMat_) {
            prevModelMat_ = *modelMat_;
        }
    }

    void RenderSkyBoxComponent::render(RenderList& rl, void* const* parts, size_t numParts)
    {
        auto xf = parent()->smoothTransform();
        xf.setOrigin(rl.camera()->transform().getOrigin());

        modelMat_ = Matrix4f(xf);
        for (const auto& subMesh : mesh_->subMeshes()) {
            rl.addGeometry(*modelMat_, (prevModelMat_ ? *prevModelMat_ : *modelMat_), AABB_empty,
                subMesh->material(), subMesh->vaSlice(), GL_TRIANGLES);
        }
    }

    std::pair<AObjectPtr, float> RenderSkyBoxComponent::testRay(const Frustum& frustum, const Ray& ray, void* part)
    {
        return std::make_pair(AObjectPtr(), 0.0f);
    }

    void RenderSkyBoxComponent::onRegister()
    {
        parent()->addComponent(equirect2cube_);
    }

    void RenderSkyBoxComponent::onUnregister()
    {
        equirect2cube_->removeFromParent();
    }
}

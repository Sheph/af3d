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

#include "LightProbeComponent.h"
#include "CameraComponent.h"
#include "TextureManager.h"
#include "MaterialManager.h"
#include "MeshManager.h"
#include "SceneObject.h"
#include "Scene.h"
#include "Logger.h"
#include "Settings.h"
#include "Const.h"
#include "af3d/ImageWriter.h"
#include <fstream>

namespace af3d
{
    ACLASS_DEFINE_BEGIN(LightProbeComponent, PhasedComponent)
    ACLASS_DEFINE_END(LightProbeComponent)

    LightProbeComponent::LightProbeComponent(std::uint32_t irradianceResolution, std::uint32_t specularResolution,
        std::uint32_t specularMipLevels)
    : PhasedComponent(AClass_LightProbeComponent, phasePreRender),
      irradianceResolution_(irradianceResolution),
      specularResolution_(specularResolution),
      specularMipLevels_(specularMipLevels)
    {
        runtime_assert(specularMipLevels_ > 0);
    }

    const AClass& LightProbeComponent::staticKlass()
    {
        return AClass_LightProbeComponent;
    }

    AObjectPtr LightProbeComponent::create(const APropertyValueMap& propVals)
    {
        return AObjectPtr();
    }

    void LightProbeComponent::preRender(float dt)
    {
        if (irrCube2equirectFilter_ && (irrCube2equirectFilter_->numFramesRendered() > 0)) {
            auto tex = irrCube2equirectFilter_->camera()->renderTarget().texture();
            std::vector<Byte> pixels(tex->width() * tex->height() * 3 * sizeof(float));
            tex->download(GL_RGB, GL_FLOAT, pixels);

            {
                std::string fname = platform->assetsPath() + "/" + getIrradianceTexPath();
                std::ofstream os(fname,
                    std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
                ImageWriter writer(fname, os);
                writer.writeHDR(tex->width(), tex->height(), 3, pixels);
            }

            startSpecularGen();

            stopIrradianceGen();

            btAssert(!irrEquirect2cube_);
            auto equirectTex = textureManager.loadTexture(getIrradianceTexPath(), false);
            equirectTex->invalidate();
            equirectTex->load();
            irrEquirect2cube_ = std::make_shared<Equirect2CubeComponent>(equirectTex, irradianceTexture_, camOrderLightProbe);
            parent()->addComponent(irrEquirect2cube_);
        } else if (specularLUTGenFilter_ && (specularLUTGenFilter_->numFramesRendered() > 0)) {
            stopSpecularGen();

            LOG4CPLUS_INFO(logger(), "LightProbe(" << parent()->name() << "): done");
        }
    }

    void LightProbeComponent::recreate()
    {
        btAssert(scene());

        if (irrCube2equirectFilter_ || specularLUTGenFilter_) {
            LOG4CPLUS_WARN(logger(), "LightProbe(" << parent()->name() << "): recreation still in progress...");
            return;
        }

        LOG4CPLUS_INFO(logger(), "LightProbe(" << parent()->name() << "): recreating...");

        startIrradianceGen();
    }

    void LightProbeComponent::onRegister()
    {
        scene()->addLightProbe(this);
        auto tex = textureManager.loadTexture(getIrradianceTexPath(), false);
        if (tex) {
            irradianceTexture_ = textureManager.createRenderTexture(TextureTypeCubeMap,
                irradianceResolution_, irradianceResolution_, GL_RGB16F, GL_RGB, GL_FLOAT);
            irrEquirect2cube_ = std::make_shared<Equirect2CubeComponent>(tex, irradianceTexture_, camOrderLightProbe);
            parent()->addComponent(irrEquirect2cube_);
        } else {
            // No saved irradiance map, use camera clear color.
            PackedColor color = toPackedColor(scene()->mainCamera()->findComponent<CameraComponent>()->camera()->clearColor());
            std::vector<Byte> data(irradianceResolution_ * irradianceResolution_ * 3);
            for (size_t i = 0; i < data.size(); i += 3) {
                data[i + 0] = color.x();
                data[i + 1] = color.y();
                data[i + 2] = color.z();
            }
            irradianceTexture_ = textureManager.createRenderTexture(TextureTypeCubeMap,
                irradianceResolution_, irradianceResolution_, GL_RGB16F, GL_RGB, GL_UNSIGNED_BYTE, false, std::move(data));
        }

        specularTexture_ = textureManager.createRenderTexture(TextureTypeCubeMap,
            specularResolution_, specularResolution_, GL_RGB16F, GL_RGB, GL_FLOAT, true);
        specularLUTTexture_ = textureManager.createRenderTexture(TextureType2D,
            specularLUTSize, specularLUTSize, GL_RG16F, GL_RG, GL_FLOAT);
    }

    void LightProbeComponent::onUnregister()
    {
        scene()->removeLightProbe(this);
        stopIrradianceGen();
        stopSpecularGen();
        if (irrEquirect2cube_) {
            irrEquirect2cube_->removeFromParent();
            irrEquirect2cube_.reset();
        }
    }

    void LightProbeComponent::startIrradianceGen()
    {
        if (irrEquirect2cube_) {
            irrEquirect2cube_->removeFromParent();
            irrEquirect2cube_.reset();
        }

        auto sceneCaptureTexture = textureManager.createRenderTexture(TextureTypeCubeMap,
            sceneCaptureSize, sceneCaptureSize, GL_RGB16F, GL_RGB, GL_FLOAT);

        auto mainCamera = scene()->mainCamera()->findComponent<CameraComponent>()->camera();

        auto filterMaterial = materialManager.createMaterial(MaterialTypeFilterIrradianceConv);
        filterMaterial->setDepthTest(false);
        filterMaterial->setDepthWrite(false);
        filterMaterial->setCullFaceMode(0);
        filterMaterial->setTextureBinding(SamplerName::Main,
            TextureBinding(sceneCaptureTexture,
                SamplerParams(GL_LINEAR, GL_LINEAR)));
        auto mesh = meshManager.createBoxMesh(btVector3(2.0f, 2.0f, 2.0f), filterMaterial);

        for (size_t i = 0; i < 6; ++i) {
            auto face = static_cast<TextureCubeFace>(i);
            auto cam = std::make_shared<Camera>();
            cam->setOrder(camOrderLightProbe);
            cam->setFov(btRadians(90.0f));
            cam->setAspect(1.0f);
            cam->setClearColor(mainCamera->clearColor());
            cam->setAmbientColor(mainCamera->ambientColor());
            cam->setTransform(btTransform(textureCubeFaceBasis(face), parent()->pos()));
            cam->setRenderTarget(RenderTarget(sceneCaptureTexture, 0, face));
            scene()->addCamera(cam);
            sceneCaptureCameras_[i] = cam;

            irrGenFilters_[i] = std::make_shared<RenderFilterComponent>(mesh);
            irrGenFilters_[i]->camera()->setOrder(camOrderLightProbe + 1);
            irrGenFilters_[i]->camera()->setFov(btRadians(90.0f));
            irrGenFilters_[i]->camera()->setAspect(1.0f);
            irrGenFilters_[i]->camera()->setTransform(btTransform(textureCubeFaceBasis(face)));
            irrGenFilters_[i]->camera()->setRenderTarget(RenderTarget(irradianceTexture_, 0, face));
            parent()->addComponent(irrGenFilters_[i]);
        }

        irrCube2equirectFilter_ = std::make_shared<RenderFilterComponent>(MaterialTypeFilterCube2Equirect);
        irrCube2equirectFilter_->material()->setTextureBinding(SamplerName::Main,
            TextureBinding(irradianceTexture_,
                SamplerParams(GL_LINEAR, GL_LINEAR)));
        irrCube2equirectFilter_->camera()->setOrder(camOrderLightProbe + 2);

        std::uint32_t equirectW = std::ceil(irradianceTexture_->width() * SIMD_PI);
        if ((equirectW % 2) != 0) {
            ++equirectW;
        }
        std::uint32_t equirectH = equirectW / 2;

        irrCube2equirectFilter_->camera()->setRenderTarget(RenderTarget(textureManager.createRenderTexture(TextureType2D,
            equirectW, equirectH, GL_RGB16F, GL_RGB, GL_FLOAT)));
        parent()->addComponent(irrCube2equirectFilter_);
    }

    void LightProbeComponent::stopIrradianceGen()
    {
        if (irrCube2equirectFilter_) {
            irrCube2equirectFilter_->removeFromParent();
            irrCube2equirectFilter_.reset();
            for (size_t i = 0; i < irrGenFilters_.size(); ++i) {
                irrGenFilters_[i]->removeFromParent();
                irrGenFilters_[i].reset();
            }
            for (size_t i = 0; i < sceneCaptureCameras_.size(); ++i) {
                scene()->removeCamera(sceneCaptureCameras_[i]);
                sceneCaptureCameras_[i].reset();
            }
        }
    }

    void LightProbeComponent::startSpecularGen()
    {
        auto sceneCaptureTexture = sceneCaptureCameras_[0]->renderTarget().texture();
        sceneCaptureTexture->generateMipmap();

        auto filterMaterial = materialManager.createMaterial(MaterialTypeFilterSpecularCM);
        filterMaterial->setDepthTest(false);
        filterMaterial->setDepthWrite(false);
        filterMaterial->setCullFaceMode(0);
        filterMaterial->setTextureBinding(SamplerName::Main,
            TextureBinding(sceneCaptureTexture,
                SamplerParams(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR)));
        auto baseMesh = meshManager.createBoxMesh(btVector3(2.0f, 2.0f, 2.0f), filterMaterial);

        for (std::uint32_t mip = 0; mip < specularMipLevels_; ++mip) {
            float roughness = (float)mip / (float)(specularMipLevels_ - 1);

            auto mesh = baseMesh->clone();
            mesh->subMeshes()[0]->material()->params().setUniform(UniformName::Roughness, roughness);

            for (size_t i = 0; i < 6; ++i) {
                auto face = static_cast<TextureCubeFace>(i);
                auto filter = std::make_shared<RenderFilterComponent>(mesh);
                filter->camera()->setOrder(camOrderLightProbe);
                filter->camera()->setFov(btRadians(90.0f));
                filter->camera()->setAspect(1.0f);
                filter->camera()->setTransform(btTransform(textureCubeFaceBasis(face)));
                filter->camera()->setRenderTarget(RenderTarget(specularTexture_, mip, face));
                parent()->addComponent(filter);
                specularGenFilters_.push_back(filter);
            }
        }

        specularLUTGenFilter_ = std::make_shared<RenderFilterComponent>(MaterialTypeFilterSpecularLUT);
        specularLUTGenFilter_->camera()->setOrder(camOrderLightProbe);
        specularLUTGenFilter_->camera()->setRenderTarget(RenderTarget(specularLUTTexture_));
        parent()->addComponent(specularLUTGenFilter_);
    }

    void LightProbeComponent::stopSpecularGen()
    {
        if (specularLUTGenFilter_) {
            specularLUTGenFilter_->removeFromParent();
            specularLUTGenFilter_.reset();
            for (size_t i = 0; i < specularGenFilters_.size(); ++i) {
                specularGenFilters_[i]->removeFromParent();
            }
            specularGenFilters_.clear();
        }
    }

    std::string LightProbeComponent::getIrradianceTexPath()
    {
        return "lp_" + scene()->name() + "_" + parent()->name() + "_irr.hdr";
    }
}

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
        std::uint32_t specularMipLevels, bool isGlobal)
    : PhasedComponent(AClass_LightProbeComponent, phasePreRender),
      isGlobal_(isGlobal),
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
                std::string fname = platform->assetsPath() + "/" + getIrradianceTexName();
                std::ofstream os(fname,
                    std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
                ImageWriter writer(fname, os);
                writer.writeHDR(tex->width(), tex->height(), 3, pixels);
            }

            startSpecularGen();

            stopIrradianceGen();

            btAssert(!irrEquirect2cube_);
            auto equirectTex = textureManager.loadTexture(getIrradianceTexName(), false);
            equirectTex->invalidate();
            equirectTex->load();
            irrEquirect2cube_ = std::make_shared<Equirect2CubeComponent>(equirectTex, irradianceTexture_, camOrderLightProbe);
            parent()->addComponent(irrEquirect2cube_);
        } else if (specularLUTGenFilter_ && (specularLUTGenFilter_->numFramesRendered() > 0)) {
            btAssert(specularMipLevels_ == specularCube2EquirectFilters_.size());
            auto mip0Tex = specularCube2EquirectFilters_[0]->camera()->renderTarget().texture();

            std::uint32_t numPixels = 0;
            for (std::uint32_t mip = 0; mip < specularMipLevels_; ++mip) {
                numPixels += textureMipSize(mip0Tex->width(), mip) * textureMipSize(mip0Tex->height(), mip);
            }
            std::uint32_t height = (numPixels + mip0Tex->width() - 1) / mip0Tex->width();

            std::vector<Byte> pixels(mip0Tex->width() * height * 3 * sizeof(float), 0);
            numPixels = 0;
            for (std::uint32_t mip = 0; mip < specularMipLevels_; ++mip) {
                auto tex = specularCube2EquirectFilters_[mip]->camera()->renderTarget().texture();
                tex->download(GL_RGB, GL_FLOAT, &pixels[0] + (numPixels * 3 * sizeof(float)));
                numPixels += textureMipSize(mip0Tex->width(), mip) * textureMipSize(mip0Tex->height(), mip);
            }

            {
                std::string fname = platform->assetsPath() + "/" + getSpecularTexName();
                std::ofstream os(fname,
                    std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
                ImageWriter writer(fname, os);
                writer.writeHDR(mip0Tex->width(), height, 3, pixels);
            }

            auto tex = specularLUTGenFilter_->camera()->renderTarget().texture();
            pixels.resize(tex->width() * tex->height() * 3 * sizeof(float));
            tex->download(GL_RGB, GL_FLOAT, pixels);
            pixels.resize(specularLUTSize * specularLUTSize * 3 * sizeof(float));

            {
                std::string fname = platform->assetsPath() + "/" + getSpecularLUTTexName();
                std::ofstream os(fname,
                    std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
                ImageWriter writer(fname, os);
                writer.writeHDR(specularLUTSize, specularLUTSize, 3, pixels);
            }

            stopSpecularGen();

            btAssert(!specularEquirect2cube_);
            auto equirectTex = textureManager.loadTexture(getSpecularTexName(), false);
            equirectTex->invalidate();
            equirectTex->load();
            specularEquirect2cube_ = std::make_shared<Equirect2CubeComponent>(equirectTex, specularTexture_, camOrderLightProbe, specularMipLevels_);
            parent()->addComponent(specularEquirect2cube_);

            specularLUTTexture_->invalidate();
            specularLUTTexture_->load();

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
        auto tex = textureManager.loadTexture(getIrradianceTexName(), false);
        if (tex) {
            irradianceTexture_ = textureManager.createRenderTexture(TextureTypeCubeMap,
                irradianceResolution_, irradianceResolution_, GL_RGB16F, GL_RGB, GL_FLOAT);
            irrEquirect2cube_ = std::make_shared<Equirect2CubeComponent>(tex, irradianceTexture_, camOrderLightProbe);
            parent()->addComponent(irrEquirect2cube_);
        } else {
            // No saved irradiance map, use camera clear color.
            PackedColor color = toPackedColor(gammaToLinear(scene()->mainCamera()->findComponent<CameraComponent>()->camera()->clearColor()));
            std::vector<Byte> data(irradianceResolution_ * irradianceResolution_ * 3);
            for (size_t i = 0; i < data.size(); i += 3) {
                data[i + 0] = color.x();
                data[i + 1] = color.y();
                data[i + 2] = color.z();
            }
            irradianceTexture_ = textureManager.createRenderTexture(TextureTypeCubeMap,
                irradianceResolution_, irradianceResolution_, GL_RGB16F, GL_RGB, GL_UNSIGNED_BYTE, false, std::move(data));
        }

        tex = textureManager.loadTexture(getSpecularTexName(), false);
        if (tex) {
            specularTexture_ = textureManager.createRenderTexture(TextureTypeCubeMap,
                specularResolution_, specularResolution_, GL_RGB16F, GL_RGB, GL_FLOAT, true);
            specularEquirect2cube_ = std::make_shared<Equirect2CubeComponent>(tex, specularTexture_, camOrderLightProbe, specularMipLevels_);
            parent()->addComponent(specularEquirect2cube_);
        } else {
            // No saved specular map, use black.
            std::vector<Byte> data(specularResolution_ * specularResolution_ * 3, 0);
            specularTexture_ = textureManager.createRenderTexture(TextureTypeCubeMap,
                specularResolution_, specularResolution_, GL_RGB16F, GL_RGB, GL_UNSIGNED_BYTE, true, std::move(data));
        }

        specularLUTTexture_ = textureManager.loadTexture(getSpecularLUTTexName(), false);
        runtime_assert(specularLUTTexture_);
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
        if (specularEquirect2cube_) {
            specularEquirect2cube_->removeFromParent();
            specularEquirect2cube_.reset();
        }
    }

    void LightProbeComponent::startIrradianceGen()
    {
        if (irrEquirect2cube_) {
            irrEquirect2cube_->removeFromParent();
            irrEquirect2cube_.reset();
        }
        if (specularEquirect2cube_) {
            specularEquirect2cube_->removeFromParent();
            specularEquirect2cube_.reset();
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
            cam->setLayer(isGlobal_ ? CameraLayer::SkyBox : CameraLayer::LightProbe);
            cam->setFov(btRadians(90.0f));
            cam->setAspect(1.0f);
            cam->setClearColor(AttachmentPoint::Color0, mainCamera->clearColor());
            cam->setAmbientColor(mainCamera->ambientColor());
            cam->setTransform(btTransform(textureCubeFaceBasis(face), parent()->pos()));
            cam->setRenderTarget(AttachmentPoint::Color0, RenderTarget(sceneCaptureTexture, 0, face));
            scene()->addCamera(cam);
            sceneCaptureCameras_[i] = cam;

            irrGenFilters_[i] = std::make_shared<RenderFilterComponent>(mesh);
            irrGenFilters_[i]->camera()->setOrder(camOrderLightProbe + 1);
            irrGenFilters_[i]->camera()->setFov(btRadians(90.0f));
            irrGenFilters_[i]->camera()->setAspect(1.0f);
            irrGenFilters_[i]->camera()->setTransform(btTransform(textureCubeFaceBasis(face)));
            irrGenFilters_[i]->camera()->setRenderTarget(AttachmentPoint::Color0, RenderTarget(irradianceTexture_, 0, face));
            parent()->addComponent(irrGenFilters_[i]);
        }

        auto equirectSz = cubeSize2equirect(irradianceTexture_->width());
        irrCube2equirectFilter_ = std::make_shared<RenderFilterComponent>(MaterialTypeFilterCube2Equirect);
        irrCube2equirectFilter_->material()->setTextureBinding(SamplerName::Main,
            TextureBinding(irradianceTexture_,
                SamplerParams(GL_LINEAR, GL_LINEAR)));
        irrCube2equirectFilter_->material()->params().setUniform(UniformName::MipLevel, 0.0f);
        irrCube2equirectFilter_->camera()->setOrder(camOrderLightProbe + 2);
        irrCube2equirectFilter_->camera()->setRenderTarget(AttachmentPoint::Color0, RenderTarget(textureManager.createRenderTexture(TextureType2D,
            equirectSz.x(), equirectSz.y(), GL_RGB16F, GL_RGB, GL_FLOAT)));
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

        auto equirectSz0 = cubeSize2equirect(specularResolution_);

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
                filter->camera()->setRenderTarget(AttachmentPoint::Color0, RenderTarget(specularTexture_, mip, face));
                parent()->addComponent(filter);
                specularGenFilters_.push_back(filter);
            }

            auto filter = std::make_shared<RenderFilterComponent>(MaterialTypeFilterCube2Equirect);
            filter->material()->setTextureBinding(SamplerName::Main,
                TextureBinding(specularTexture_,
                    SamplerParams(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR)));
            filter->material()->params().setUniform(UniformName::MipLevel, static_cast<float>(mip));
            filter->camera()->setOrder(camOrderLightProbe + 1);
            filter->camera()->setRenderTarget(AttachmentPoint::Color0, RenderTarget(textureManager.createRenderTexture(TextureType2D,
                textureMipSize(equirectSz0.x(), mip), textureMipSize(equirectSz0.y(), mip), GL_RGB16F, GL_RGB, GL_FLOAT)));
            parent()->addComponent(filter);
            specularCube2EquirectFilters_.push_back(filter);
        }

        specularLUTGenFilter_ = std::make_shared<RenderFilterComponent>(MaterialTypeFilterSpecularLUT);
        specularLUTGenFilter_->camera()->setOrder(camOrderLightProbe);
        specularLUTGenFilter_->camera()->setRenderTarget(AttachmentPoint::Color0, RenderTarget(specularLUTTexture_));
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
            for (size_t i = 0; i < specularCube2EquirectFilters_.size(); ++i) {
                specularCube2EquirectFilters_[i]->removeFromParent();
            }
            specularCube2EquirectFilters_.clear();
        }
    }

    std::string LightProbeComponent::getIrradianceTexName()
    {
        return "lp_" + scene()->name() + "_" + (isGlobal_ ? "global" : parent()->name()) + "_irr.hdr";
    }

    std::string LightProbeComponent::getSpecularTexName()
    {
        return "lp_" + scene()->name() + "_" + (isGlobal_ ? "global" : parent()->name()) + "_spec.hdr";
    }

    std::string LightProbeComponent::getSpecularLUTTexName()
    {
        return "lp_spec_lut.hdr";
    }
}

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
#include "PhysicsDebugDraw.h"
#include "af3d/ImageWriter.h"
#include <fstream>

namespace af3d
{
    ACLASS_DEFINE_BEGIN(LightProbeComponent, PhasedComponent)
    ACLASS_DEFINE_END(LightProbeComponent)

    LightProbeComponent::LightProbeComponent(const boost::optional<AABB>& bounds)
    : PhasedComponent(AClass_LightProbeComponent, phasePreRender),
      bounds_(bounds)
    {
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
        if (prevXf_ != parent()->smoothTransform()) {
            prevXf_ = parent()->smoothTransform();
            dirty_ = true;
        }

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
            irrEquirect2cube_ = std::make_shared<Equirect2CubeComponent>(equirectTex, rt_.irradianceTexture, rt_.index, camOrderLightProbe);
            parent()->addComponent(irrEquirect2cube_);
        } else if (specularLUTGenFilter_ && (specularLUTGenFilter_->numFramesRendered() > 0)) {
            btAssert(settings.lightProbe.specularMipLevels == specularCube2EquirectFilters_.size());
            auto mip0Tex = specularCube2EquirectFilters_[0]->camera()->renderTarget().texture();

            std::uint32_t numPixels = 0;
            for (std::uint32_t mip = 0; mip < settings.lightProbe.specularMipLevels; ++mip) {
                numPixels += textureMipSize(mip0Tex->width(), mip) * textureMipSize(mip0Tex->height(), mip);
            }
            std::uint32_t height = (numPixels + mip0Tex->width() - 1) / mip0Tex->width();

            std::vector<Byte> pixels(mip0Tex->width() * height * 3 * sizeof(float), 0);
            numPixels = 0;
            for (std::uint32_t mip = 0; mip < settings.lightProbe.specularMipLevels; ++mip) {
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
            specularEquirect2cube_ = std::make_shared<Equirect2CubeComponent>(equirectTex, rt_.specularTexture, rt_.index, camOrderLightProbe, settings.lightProbe.specularMipLevels);
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

    bool LightProbeComponent::resetDirty()
    {
        bool wasDirty = dirty_;
        dirty_ = false;
        return wasDirty;
    }

    void LightProbeComponent::setupCluster(ShaderClusterProbe& cProbe)
    {
        cProbe.pos = Vector4f(parent()->smoothPos(), 0.0f);
        const auto& b = bounds();
        auto mat = Matrix4f(parent()->smoothTransform() * toTransform(b.getCenter())).scaled(b.getExtents());
        cProbe.invModel = mat.inverse();
        cProbe.cubeIdx = rt_.index;
        cProbe.enabled = 1;
    }

    void LightProbeComponent::onRegister()
    {
        rt_ = scene()->addLightProbe(this);
        prevXf_ = parent()->smoothTransform();
        auto tex = textureManager.loadTexture(getIrradianceTexName(), false);
        if (tex) {
            irrEquirect2cube_ = std::make_shared<Equirect2CubeComponent>(tex, rt_.irradianceTexture, rt_.index, camOrderLightProbe);
            parent()->addComponent(irrEquirect2cube_);
        }

        tex = textureManager.loadTexture(getSpecularTexName(), false);
        if (tex) {
            specularEquirect2cube_ = std::make_shared<Equirect2CubeComponent>(tex, rt_.specularTexture, rt_.index, camOrderLightProbe, settings.lightProbe.specularMipLevels);
            parent()->addComponent(specularEquirect2cube_);
        }

        specularLUTTexture_ = textureManager.loadTexture(getSpecularLUTTexName(), false);
        runtime_assert(specularLUTTexture_);

        if (!isGlobal() && settings.editor.enabled && !settings.editor.playing) {
            // Editor stuff.
            auto mesh = meshManager.loadMesh("light_probe.fbx")->clone();
            for (const auto& sm : mesh->subMeshes()) {
                sm->material()->setBlendingParams(BlendingParams(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
                Color c;
                if (sm->material()->params().getUniform(UniformName::MainColor, c, true)) {
                    c.setW(0.5f);
                    sm->material()->params().setUniform(UniformName::MainColor, c);
                }
            }

            markerRc_ = std::make_shared<RenderMeshComponent>();
            markerRc_->cameraFilter().layers() = CameraLayer::Main;
            markerRc_->setMesh(mesh);
            markerRc_->setScale(btVector3_one * 0.5f);
            parent()->addComponent(markerRc_);

            boundsRc_ = std::make_shared<RenderProxyComponent>(
                std::bind(&LightProbeComponent::renderBounds, this, std::placeholders::_1));
            boundsRc_->cameraFilter().layers() = CameraLayer::Main;
            boundsRc_->setLocalAABB(*bounds_);
            parent()->addComponent(boundsRc_);
        }
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
        if (markerRc_) {
            markerRc_->removeFromParent();
            markerRc_.reset();
        }
        if (boundsRc_) {
            boundsRc_->removeFromParent();
            boundsRc_.reset();
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
            sceneCaptureSize, sceneCaptureSize, 0, GL_RGB16F, GL_RGB, GL_FLOAT);

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
            cam->setLayer(isGlobal() ? CameraLayer::SkyBox : CameraLayer::LightProbe);
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
            irrGenFilters_[i]->camera()->setRenderTarget(AttachmentPoint::Color0, RenderTarget(rt_.irradianceTexture, 0, face, rt_.index));
            parent()->addComponent(irrGenFilters_[i]);
        }

        auto equirectSz = cubeSize2equirect(rt_.irradianceTexture->width());
        irrCube2equirectFilter_ = std::make_shared<RenderFilterComponent>(MaterialTypeFilterCube2Equirect);
        irrCube2equirectFilter_->material()->setTextureBinding(SamplerName::Main,
            TextureBinding(rt_.irradianceTexture,
                SamplerParams(GL_LINEAR, GL_LINEAR)));
        irrCube2equirectFilter_->material()->params().setUniform(UniformName::MipLevel, 0.0f);
        irrCube2equirectFilter_->material()->params().setUniform(UniformName::TLayer, static_cast<float>(rt_.index));
        irrCube2equirectFilter_->camera()->setOrder(camOrderLightProbe + 2);
        irrCube2equirectFilter_->camera()->setRenderTarget(AttachmentPoint::Color0, RenderTarget(textureManager.createRenderTexture(TextureType2D,
            equirectSz.x(), equirectSz.y(), 0, GL_RGB16F, GL_RGB, GL_FLOAT)));
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

        auto equirectSz0 = cubeSize2equirect(settings.lightProbe.specularResolution);

        for (std::uint32_t mip = 0; mip < settings.lightProbe.specularMipLevels; ++mip) {
            float roughness = (float)mip / (float)(settings.lightProbe.specularMipLevels - 1);

            auto mesh = baseMesh->clone();
            mesh->subMeshes()[0]->material()->params().setUniform(UniformName::Roughness, roughness);

            for (size_t i = 0; i < 6; ++i) {
                auto face = static_cast<TextureCubeFace>(i);
                auto filter = std::make_shared<RenderFilterComponent>(mesh);
                filter->camera()->setOrder(camOrderLightProbe);
                filter->camera()->setFov(btRadians(90.0f));
                filter->camera()->setAspect(1.0f);
                filter->camera()->setTransform(btTransform(textureCubeFaceBasis(face)));
                filter->camera()->setRenderTarget(AttachmentPoint::Color0, RenderTarget(rt_.specularTexture, mip, face, rt_.index));
                parent()->addComponent(filter);
                specularGenFilters_.push_back(filter);
            }

            auto filter = std::make_shared<RenderFilterComponent>(MaterialTypeFilterCube2Equirect);
            filter->material()->setTextureBinding(SamplerName::Main,
                TextureBinding(rt_.specularTexture,
                    SamplerParams(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR)));
            filter->material()->params().setUniform(UniformName::MipLevel, static_cast<float>(mip));
            filter->material()->params().setUniform(UniformName::TLayer, static_cast<float>(rt_.index));
            filter->camera()->setOrder(camOrderLightProbe + 1);
            filter->camera()->setRenderTarget(AttachmentPoint::Color0, RenderTarget(textureManager.createRenderTexture(TextureType2D,
                textureMipSize(equirectSz0.x(), mip), textureMipSize(equirectSz0.y(), mip), 0, GL_RGB16F, GL_RGB, GL_FLOAT)));
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
        return "lp_" + scene()->name() + "_" + (isGlobal() ? "global" : parent()->name()) + "_irr.hdr";
    }

    std::string LightProbeComponent::getSpecularTexName()
    {
        return "lp_" + scene()->name() + "_" + (isGlobal() ? "global" : parent()->name()) + "_spec.hdr";
    }

    std::string LightProbeComponent::getSpecularLUTTexName()
    {
        return "lp_spec_lut.hdr";
    }

    void LightProbeComponent::renderBounds(RenderList& rl)
    {
        auto w = scene()->workspace();
        if (!w->emObject()->active()) {
            return;
        }

        PhysicsDebugDraw dd;
        dd.setRenderList(&rl);
        if (w->emObject()->isSelected(parent()->sharedThis())) {
            dd.setAlpha(0.8f);
            dd.drawBox(bounds_->lowerBound, bounds_->upperBound,
                parent()->smoothTransform(), btVector3(1.0f, 1.0f, 1.0f));
            dd.flushLines(false);
        } else {
            dd.setAlpha(0.1f);
            dd.drawBox(bounds_->lowerBound, bounds_->upperBound,
                parent()->smoothTransform(), btVector3(1.0f, 1.0f, 1.0f));
            dd.flushLines(true);
        }
    }
}

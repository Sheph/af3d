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

#include "CameraRenderer.h"
#include "LightProbeComponent.h"
#include "Settings.h"

namespace af3d
{
    CameraRenderer::CameraRenderer()
    {
        clearColors_[static_cast<int>(AttachmentPoint::Color0)] = Color(0.23f, 0.23f, 0.23f, 1.0f);
    }

    const AABB2i& CameraRenderer::viewport() const
    {
        if (const auto& ct = renderTarget(AttachmentPoint::Color0)) {
            viewport_ = AABB2i(Vector2i_zero, Vector2i(ct.width(), ct.height()));
        } else if (const auto& dt = renderTarget(AttachmentPoint::Depth)) {
            viewport_ = AABB2i(Vector2i_zero, Vector2i(dt.width(), dt.height()));
        } else if (const auto& st = renderTarget(AttachmentPoint::Stencil)) {
            viewport_ = AABB2i(Vector2i_zero, Vector2i(st.width(), st.height()));
        }
        return viewport_;
    }

    void CameraRenderer::setViewport(const AABB2i& value)
    {
        viewport_ = value;
    }

    void CameraRenderer::addRenderPass(const RenderPassPtr& pass, bool run)
    {
        passes_.emplace_back(pass, run);
    }

    void CameraRenderer::setAutoParams(const RenderList& rl, const RenderList::Geometry& geom, std::uint32_t outputMask,
        std::vector<HardwareTextureBinding>& textures,
        std::vector<StorageBufferBinding>& storageBuffers, MaterialParams& params) const
    {
        setAutoParams(rl, geom.material, outputMask, textures, storageBuffers, params, geom.modelMat, geom.prevModelMat);
    }

    void CameraRenderer::setAutoParams(const RenderList& rl, const MaterialPtr& material, std::uint32_t outputMask,
        std::vector<HardwareTextureBinding>& textures,
        std::vector<StorageBufferBinding>& storageBuffers, MaterialParams& params,
        const Matrix4f& modelMat, const Matrix4f& prevModelMat) const
    {
        const CameraPtr& camera = rl.camera();
        const SceneEnvironmentPtr& env = rl.env();

        const Matrix4f& viewProjMat = camera->frustum().jitteredViewProjMat();
        const Matrix4f& stableViewProjMat = camera->frustum().viewProjMat();
        const Matrix4f& stableProjMat = camera->frustum().projMat();
        const Matrix4f& stableViewMat = camera->frustum().viewMat();

        const auto& activeUniforms = material->type()->prog()->activeUniforms();
        const auto& samplers = material->type()->prog()->samplers();

        for (int i = 0; i <= static_cast<int>(SamplerName::Max); ++i) {
            SamplerName sName = static_cast<SamplerName>(i);
            if (samplers[sName]) {
                const auto& tb = material->textureBinding(sName);
                textures.emplace_back(tb.tex ? tb.tex->hwTex() : HardwareTexturePtr(), tb.params);
                if ((sName == SamplerName::Irradiance) && !textures.back().tex) {
                    textures.back() = HardwareTextureBinding(env->irradianceTexture()->hwTex(),
                        SamplerParams(GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR));
                } else if ((sName == SamplerName::SpecularCM) && !textures.back().tex) {
                    textures.back() = HardwareTextureBinding(env->specularTexture()->hwTex(),
                        SamplerParams(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR));
                } else if ((sName == SamplerName::SpecularLUT) && !textures.back().tex) {
                    auto probe = env->globalLightProbe();
                    if (probe) {
                        textures.back() = HardwareTextureBinding(probe->specularLUTTexture()->hwTex(),
                            SamplerParams(GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR));
                    }
                }
            }
        }

        if (activeUniforms.count(UniformName::AmbientColor) > 0) {
            auto ac = camera->ambientColor();
            ac = gammaToLinear(ac);
            params.setUniform(UniformName::AmbientColor, Vector3f(ac.x(), ac.y(), ac.z()) * ac.w());
        }

        if (activeUniforms.count(UniformName::ViewProjMatrix) > 0) {
            params.setUniform(UniformName::ViewProjMatrix, viewProjMat);
        }

        if (activeUniforms.count(UniformName::StableProjMatrix) > 0) {
            params.setUniform(UniformName::StableProjMatrix, stableProjMat);
        }

        if (activeUniforms.count(UniformName::StableViewMatrix) > 0) {
            params.setUniform(UniformName::StableViewMatrix, stableViewMat);
        }

        bool prevStableMatSet = false;
        bool curStableMatSet = false;

        if (activeUniforms.count(UniformName::ModelViewProjMatrix) > 0) {
            params.setUniform(UniformName::ModelViewProjMatrix, viewProjMat * modelMat);
            if (!prevStableMatSet && activeUniforms.count(UniformName::PrevStableMatrix) > 0) {
                prevStableMatSet = true;
                params.setUniform(UniformName::PrevStableMatrix, camera->prevViewProjMat() * prevModelMat);
            }
            if (!curStableMatSet && activeUniforms.count(UniformName::CurStableMatrix) > 0) {
                curStableMatSet = true;
                params.setUniform(UniformName::CurStableMatrix, stableViewProjMat * modelMat);
            }
        }
        if (activeUniforms.count(UniformName::ModelMatrix) > 0) {
            params.setUniform(UniformName::ModelMatrix, modelMat);
            if (!prevStableMatSet && activeUniforms.count(UniformName::PrevStableMatrix) > 0) {
                prevStableMatSet = true;
                params.setUniform(UniformName::PrevStableMatrix, camera->prevViewProjMat() * prevModelMat);
            }
            if (!curStableMatSet && activeUniforms.count(UniformName::CurStableMatrix) > 0) {
                curStableMatSet = true;
                params.setUniform(UniformName::CurStableMatrix, stableViewProjMat * modelMat);
            }
        }
        if (!prevStableMatSet && activeUniforms.count(UniformName::PrevStableMatrix) > 0) {
            prevStableMatSet = true;
            params.setUniform(UniformName::PrevStableMatrix, camera->prevViewProjMat());
        }
        if (!curStableMatSet && activeUniforms.count(UniformName::CurStableMatrix) > 0) {
            curStableMatSet = true;
            params.setUniform(UniformName::CurStableMatrix, stableViewProjMat);
        }
        if (activeUniforms.count(UniformName::EyePos) > 0) {
            params.setUniform(UniformName::EyePos, camera->frustum().transform().getOrigin());
        }
        if (activeUniforms.count(UniformName::ViewportSize) > 0) {
            params.setUniform(UniformName::ViewportSize, Vector2f::fromVector2i(viewport().getSize()));
        }
        if (activeUniforms.count(UniformName::Time) > 0) {
            params.setUniform(UniformName::Time, env->time() + material->timeOffset());
        }
        if (activeUniforms.count(UniformName::Dt) > 0) {
            params.setUniform(UniformName::Dt, env->dt());
        }
        if (activeUniforms.count(UniformName::RealDt) > 0) {
            params.setUniform(UniformName::RealDt, env->realDt());
        }
        if (activeUniforms.count(UniformName::ClusterCfg) > 0) {
            float zNear = camera->frustum().nearDist();
            float zFar = camera->frustum().farDist();
            float scalingFactor = (float)settings.cluster.gridSize.z() / std::log2f(zFar / zNear);
            float biasFactor = -((float)settings.cluster.gridSize.z() * std::log2f(zNear) / std::log2f(zFar / zNear));
            params.setUniform(UniformName::ClusterCfg, Vector4f(zNear, zFar, scalingFactor, biasFactor));
        }
        if (activeUniforms.count(UniformName::OutputMask) > 0) {
            params.setUniform(UniformName::OutputMask, static_cast<int>(outputMask));
        }

        const auto& ssboNames = material->type()->prog()->storageBuffers();

        if (ssboNames[StorageBufferName::ClusterLights]) {
            storageBuffers.emplace_back(StorageBufferName::ClusterLights, env->lightsSSBO());
        }

        if (ssboNames[StorageBufferName::ClusterProbes]) {
            storageBuffers.emplace_back(StorageBufferName::ClusterProbes, env->probesSSBO());
        }

        for (const auto& pass : passes_) {
            pass.first->fillParams(material, storageBuffers, params);
        }
    }

    RenderNodePtr CameraRenderer::compile(const RenderList& rl) const
    {
        auto mrt = getHardwareMRT();
        auto rn = std::make_shared<RenderNode>(viewport(), clearMask(), clearColors(), mrt);
        int passIdx = 0;
        for (const auto& pass : passes_) {
            if (pass.second) {
                passIdx = pass.first->compile(*this, rl, passIdx, rn);
            }
        }
        return rn;
    }

    HardwareMRT CameraRenderer::getHardwareMRT() const
    {
        HardwareMRT mrt;
        for (int i = 0; i <= static_cast<int>(AttachmentPoint::Max); ++i) {
            AttachmentPoint p = static_cast<AttachmentPoint>(i);
            mrt.attachment(p) = renderTarget(p).toHardware();
        }
        return mrt;
    }
}

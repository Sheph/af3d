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

#include "RenderList.h"
#include "LightProbeComponent.h"
#include "HardwareResourceManager.h"
#include "MaterialManager.h"
#include "Renderer.h"
#include "Light.h"
#include "Logger.h"
#include "ShaderDataTypes.h"
#include "Settings.h"

namespace af3d
{
    RenderImm::RenderImm(const MaterialPtr& material,
        GLenum primitiveMode,
        float depthValue,
        const ScissorParams& scissorParams,
        RenderList& rl)
    : material_(material),
      primitiveMode_(primitiveMode),
      depthValue_(depthValue),
      scissorParams_(scissorParams),
      rl_(rl),
      startVertices_(rl_.env_->defaultVa().data().vertices.size())
    {
    }

    RenderImm::~RenderImm()
    {
        VertexArraySlice vaSlice(rl_.env_->defaultVa().vaNoEbo(),
            startVertices_,
            rl_.env_->defaultVa().data().vertices.size() - startVertices_,
            0);
        rl_.addGeometry(material_, vaSlice, primitiveMode_, depthValue_, scissorParams_);
    }

    std::vector<VertexImm>& RenderImm::vertices()
    {
        return rl_.env_->defaultVa().data().vertices;
    }

    void RenderImm::addLine(const btVector3& pos, const btVector3& dir, const btVector3& up, const Color& c, bool withCovers)
    {
        auto pos2 = pos + dir;

        auto dirN = dir.normalized();

        auto up2 = up.rotate(dirN, SIMD_HALF_PI);
        auto up3 = -up;
        auto up4 = -up2;

        if (withCovers) {
            // Back cover
            addVertex(pos, Vector2f_zero, c);
            addVertex(pos + up2, Vector2f_zero, c);
            addVertex(pos + up, Vector2f_zero, c);

            addVertex(pos, Vector2f_zero, c);
            addVertex(pos + up3, Vector2f_zero, c);
            addVertex(pos + up2, Vector2f_zero, c);

            addVertex(pos + up4, Vector2f_zero, c);
            addVertex(pos + up3, Vector2f_zero, c);
            addVertex(pos, Vector2f_zero, c);

            addVertex(pos + up4, Vector2f_zero, c);
            addVertex(pos, Vector2f_zero, c);
            addVertex(pos + up, Vector2f_zero, c);

            // Front cover
            addVertex(pos2 + up, Vector2f_zero, c);
            addVertex(pos2 + up2, Vector2f_zero, c);
            addVertex(pos2, Vector2f_zero, c);

            addVertex(pos2 + up2, Vector2f_zero, c);
            addVertex(pos2 + up3, Vector2f_zero, c);
            addVertex(pos2, Vector2f_zero, c);

            addVertex(pos2, Vector2f_zero, c);
            addVertex(pos2 + up3, Vector2f_zero, c);
            addVertex(pos2 + up4, Vector2f_zero, c);

            addVertex(pos2 + up, Vector2f_zero, c);
            addVertex(pos2, Vector2f_zero, c);
            addVertex(pos2 + up4, Vector2f_zero, c);
        }

        // Side 1
        addVertex(pos + up2, Vector2f_zero, c);
        addVertex(pos2 + up, Vector2f_zero, c);
        addVertex(pos + up, Vector2f_zero, c);

        addVertex(pos + up2, Vector2f_zero, c);
        addVertex(pos2 + up2, Vector2f_zero, c);
        addVertex(pos2 + up, Vector2f_zero, c);

        // Side 2
        addVertex(pos + up3, Vector2f_zero, c);
        addVertex(pos2 + up2, Vector2f_zero, c);
        addVertex(pos + up2, Vector2f_zero, c);

        addVertex(pos + up3, Vector2f_zero, c);
        addVertex(pos2 + up3, Vector2f_zero, c);
        addVertex(pos2 + up2, Vector2f_zero, c);

        // Side 3
        addVertex(pos + up4, Vector2f_zero, c);
        addVertex(pos2 + up4, Vector2f_zero, c);
        addVertex(pos + up3, Vector2f_zero, c);

        addVertex(pos + up3, Vector2f_zero, c);
        addVertex(pos2 + up4, Vector2f_zero, c);
        addVertex(pos2 + up3, Vector2f_zero, c);

        // Side 4
        addVertex(pos + up, Vector2f_zero, c);
        addVertex(pos2 + up, Vector2f_zero, c);
        addVertex(pos + up4, Vector2f_zero, c);

        addVertex(pos + up4, Vector2f_zero, c);
        addVertex(pos2 + up, Vector2f_zero, c);
        addVertex(pos2 + up4, Vector2f_zero, c);
    }

    void RenderImm::addArrow(const btVector3& pos, const btVector3& dir, const btVector3& up, const Color& c)
    {
        auto pos2 = pos + dir;

        auto dirN = dir.normalized();

        auto up2 = up.rotate(dirN, SIMD_2_PI / 3);
        auto up3 = up.rotate(dirN, -SIMD_2_PI / 3);

        addVertex(pos + up3, Vector2f_zero, c);
        addVertex(pos + up2, Vector2f_zero, c);
        addVertex(pos + up, Vector2f_zero, c);

        addVertex(pos + up2, Vector2f_zero, c);
        addVertex(pos2, Vector2f_zero, c);
        addVertex(pos + up, Vector2f_zero, c);

        addVertex(pos2, Vector2f_zero, c);
        addVertex(pos + up2, Vector2f_zero, c);
        addVertex(pos + up3, Vector2f_zero, c);

        addVertex(pos2, Vector2f_zero, c);
        addVertex(pos + up3, Vector2f_zero, c);
        addVertex(pos + up, Vector2f_zero, c);
    }

    void RenderImm::addQuadArrow(const btVector3& pos, const btVector3& dir, const btVector3& up, const Color& c)
    {
        auto pos2 = pos + dir;

        auto dirN = dir.normalized();

        auto up2 = up.rotate(dirN, SIMD_HALF_PI);
        auto up3 = -up;
        auto up4 = -up2;

        // Cover
        addVertex(pos, Vector2f_zero, c);
        addVertex(pos + up2, Vector2f_zero, c);
        addVertex(pos + up, Vector2f_zero, c);

        addVertex(pos, Vector2f_zero, c);
        addVertex(pos + up3, Vector2f_zero, c);
        addVertex(pos + up2, Vector2f_zero, c);

        addVertex(pos + up4, Vector2f_zero, c);
        addVertex(pos + up3, Vector2f_zero, c);
        addVertex(pos, Vector2f_zero, c);

        addVertex(pos + up4, Vector2f_zero, c);
        addVertex(pos, Vector2f_zero, c);
        addVertex(pos + up, Vector2f_zero, c);

        // Side
        addVertex(pos + up3, Vector2f_zero, c);
        addVertex(pos2, Vector2f_zero, c);
        addVertex(pos + up2, Vector2f_zero, c);

        addVertex(pos + up2, Vector2f_zero, c);
        addVertex(pos2, Vector2f_zero, c);
        addVertex(pos + up, Vector2f_zero, c);

        addVertex(pos + up4, Vector2f_zero, c);
        addVertex(pos2, Vector2f_zero, c);
        addVertex(pos + up3, Vector2f_zero, c);

        addVertex(pos + up, Vector2f_zero, c);
        addVertex(pos2, Vector2f_zero, c);
        addVertex(pos + up4, Vector2f_zero, c);
    }

    void RenderImm::addLineArrow(const btVector3& pos, const btVector3& dir, const btVector3& up, const Vector2f& arrowSize, const Color& c)
    {
        addLine(pos, dir, up, c);
        addArrow(pos + dir, dir.normalized() * arrowSize.x(), up.normalized() * arrowSize.y(), c);
    }

    void RenderImm::addLineBox(const btVector3& pos, const btVector3& dir, const btVector3& up, const btVector3& boxSize, const Color& c)
    {
        addLine(pos, dir, up, c);

        auto dirN = dir.normalized();
        auto upN = up.normalized();
        auto rightN = dirN.cross(upN);
        addBox(pos + dir - upN * boxSize.y() * 0.5f - rightN * boxSize.x() * 0.5f, {rightN * boxSize.x(), dirN * boxSize.z(), upN * boxSize.y()}, c);
    }

    void RenderImm::addQuad(const btVector3& pos, const std::array<btVector3, 2>& dirs, const Color& c)
    {
        auto p2 = pos + dirs[0];
        auto p3 = pos + dirs[1];
        auto p4 = p2 + dirs[1];

        addVertex(pos, Vector2f_zero, c);
        addVertex(p2, Vector2f_zero, c);
        addVertex(p4, Vector2f_zero, c);

        addVertex(pos, Vector2f_zero, c);
        addVertex(p4, Vector2f_zero, c);
        addVertex(p3, Vector2f_zero, c);
    }

    void RenderImm::addBox(const btVector3& pos, const std::array<btVector3, 3>& dirs, const Color& c)
    {
        auto p2 = pos + dirs[0];
        auto p3 = pos + dirs[1];
        auto p4 = p2 + dirs[1];

        auto t = pos + dirs[2];
        auto t2 = p2 + dirs[2];
        auto t3 = p3 + dirs[2];
        auto t4 = p4 + dirs[2];

        // Top
        addVertex(t, Vector2f_zero, c);
        addVertex(t2, Vector2f_zero, c);
        addVertex(t4, Vector2f_zero, c);

        addVertex(t, Vector2f_zero, c);
        addVertex(t4, Vector2f_zero, c);
        addVertex(t3, Vector2f_zero, c);

        // Bottom
        addVertex(p4, Vector2f_zero, c);
        addVertex(p2, Vector2f_zero, c);
        addVertex(pos, Vector2f_zero, c);

        addVertex(p3, Vector2f_zero, c);
        addVertex(p4, Vector2f_zero, c);
        addVertex(pos, Vector2f_zero, c);

        // Right
        addVertex(p2, Vector2f_zero, c);
        addVertex(p4, Vector2f_zero, c);
        addVertex(t4, Vector2f_zero, c);

        addVertex(p2, Vector2f_zero, c);
        addVertex(t4, Vector2f_zero, c);
        addVertex(t2, Vector2f_zero, c);

        // Left
        addVertex(pos, Vector2f_zero, c);
        addVertex(t3, Vector2f_zero, c);
        addVertex(p3, Vector2f_zero, c);

        addVertex(pos, Vector2f_zero, c);
        addVertex(t, Vector2f_zero, c);
        addVertex(t3, Vector2f_zero, c);

        // Front
        addVertex(pos, Vector2f_zero, c);
        addVertex(p2, Vector2f_zero, c);
        addVertex(t2, Vector2f_zero, c);

        addVertex(pos, Vector2f_zero, c);
        addVertex(t2, Vector2f_zero, c);
        addVertex(t, Vector2f_zero, c);

        // Back
        addVertex(p3, Vector2f_zero, c);
        addVertex(t3, Vector2f_zero, c);
        addVertex(t4, Vector2f_zero, c);

        addVertex(p3, Vector2f_zero, c);
        addVertex(t4, Vector2f_zero, c);
        addVertex(p4, Vector2f_zero, c);
    }

    void RenderImm::addRing(const btVector3& pos, const btVector3& up, float radius, const Color& c, int numSegments)
    {
        btQuaternion rot(up, SIMD_2_PI / numSegments);

        auto p = btZeroNormalized(btPerpendicular(up)) * radius;

        auto dirN = btZeroNormalized(p.cross(up));

        auto pUp1 = pos + p + up;
        auto pUp2 = pos + p + up.rotate(dirN, SIMD_2_PI / 3);
        auto pUp3 = pos + p + up.rotate(dirN, -SIMD_2_PI / 3);

        for (int i = 0; i < numSegments; ++i) {
            p = quatRotate(rot, p);
            dirN = quatRotate(rot, dirN);

            auto up1 = pos + p + up;
            auto up2 = pos + p + up.rotate(dirN, SIMD_2_PI / 3);
            auto up3 = pos + p + up.rotate(dirN, -SIMD_2_PI / 3);

            // Side 1
            addVertex(pUp3, Vector2f_zero, c);
            addVertex(up1, Vector2f_zero, c);
            addVertex(pUp1, Vector2f_zero, c);

            addVertex(pUp3, Vector2f_zero, c);
            addVertex(up3, Vector2f_zero, c);
            addVertex(up1, Vector2f_zero, c);

            // Side 2
            addVertex(pUp1, Vector2f_zero, c);
            addVertex(up1, Vector2f_zero, c);
            addVertex(pUp2, Vector2f_zero, c);

            addVertex(up1, Vector2f_zero, c);
            addVertex(up2, Vector2f_zero, c);
            addVertex(pUp2, Vector2f_zero, c);

            // Side 3
            addVertex(pUp2, Vector2f_zero, c);
            addVertex(up2, Vector2f_zero, c);
            addVertex(up3, Vector2f_zero, c);

            addVertex(pUp2, Vector2f_zero, c);
            addVertex(up3, Vector2f_zero, c);
            addVertex(pUp3, Vector2f_zero, c);

            pUp1 = up1;
            pUp2 = up2;
            pUp3 = up3;
        }
    }

    void RenderImm::addCircle(const btVector3& pos, const btVector3& up, const Color& c, int numSegments)
    {
        btQuaternion rot(up, SIMD_2_PI / numSegments);

        auto p = btZeroNormalized(btPerpendicular(up)) * up.length();

        auto p1 = pos + p;

        for (int i = 0; i < numSegments; ++i) {
            p = quatRotate(rot, p);

            auto p2 = pos + p;

            addVertex(p1, Vector2f_zero, c);
            addVertex(p2, Vector2f_zero, c);
            addVertex(pos, Vector2f_zero, c);

            p1 = p2;
        }
    }

    RenderList::RenderList(const CameraPtr& camera, const SceneEnvironmentPtr& env)
    : camera_(camera),
      env_(env)
    {
    }

    void RenderList::addGeometry(const Matrix4f& modelMat, const Matrix4f& prevModelMat,
        const AABB& aabb, const MaterialPtr& material,
        const VertexArraySlice& vaSlice, GLenum primitiveMode, float depthValue,
        const ScissorParams& scissorParams)
    {
        geomList_.emplace_back(modelMat, prevModelMat, aabb, material, vaSlice, primitiveMode, depthValue, scissorParams);
    }

    void RenderList::addGeometry(const MaterialPtr& material,
        const VertexArraySlice& vaSlice, GLenum primitiveMode, float depthValue,
        const ScissorParams& scissorParams)
    {
        geomList_.emplace_back(material, vaSlice, primitiveMode, depthValue, scissorParams);
    }

    RenderImm RenderList::addGeometry(const MaterialPtr& material,
        GLenum primitiveMode,
        float depthValue, const ScissorParams& scissorParams)
    {
        return RenderImm(material, primitiveMode, depthValue, scissorParams, *this);
    }

    VertexArraySlice RenderList::createGeometry(const VertexImm* vertices, std::uint32_t numVertices,
        const std::uint16_t* indices, std::uint32_t numIndices)
    {
        auto startVertices = env_->defaultVa().data().vertices.size();

        env_->defaultVa().data().vertices.insert(env_->defaultVa().data().vertices.end(), vertices, vertices + numVertices);
        if (indices) {
            auto startIndices = env_->defaultVa().data().indices.size();

            env_->defaultVa().data().indices.insert(env_->defaultVa().data().indices.end(), indices, indices + numIndices);

            return VertexArraySlice(env_->defaultVa().va(),
                startIndices,
                env_->defaultVa().data().indices.size() - startIndices,
                startVertices);
        } else {
            return VertexArraySlice(env_->defaultVa().vaNoEbo(),
                startVertices,
                env_->defaultVa().data().vertices.size() - startVertices,
                0);
        }
    }

    void RenderList::addLight(const LightPtr& light)
    {
        lightList_.push_back(light);
    }

    RenderNodePtr RenderList::compile() const
    {
        auto mrt = camera_->getHardwareMRT();
        auto rn = std::make_shared<RenderNode>(camera_->viewport(), camera_->clearMask(), camera_->clearColors(), mrt);
        auto drawBuffers = mrt.getDrawBuffers();

        RenderNode tmpNode;

        bool needClusterData = false;
        for (const auto& geom : geomList_) {
            const auto& ssbos = geom.material->type()->prog()->storageBuffers();
            if (ssbos[StorageBufferName::ClusterTileData]) {
                needClusterData = true;
                break;
            }
        }

        if (needClusterData) {
            auto& clusterData = camera_->clusterData();
            if (!clusterData.va) {
                clusterData.va = std::make_shared<VertexArray>(hwManager.createVertexArray(), VertexArrayLayout(), VBOList());
            }
            if (!clusterData.tilesSSBO) {
                clusterData.tilesSSBO = hwManager.createDataBuffer(HardwareBuffer::Usage::StaticCopy, sizeof(ShaderClusterTile));
            }
            if (clusterData.tilesSSBO->setValid()) {
                auto ssbo = clusterData.tilesSSBO;
                renderer.scheduleHwOp([ssbo](HardwareContext& ctx) {
                    ssbo->resize(settings.cluster.numClusters, ctx);
                });
            }
            if (clusterData.prevProjMat != camera_->frustum().projMat()) {
                // Projection changed, recalc cluster tile grid.
                clusterData.prevProjMat = camera_->frustum().projMat();
                auto material = materialManager.createMaterial(MaterialTypeClusterBuild);
                std::vector<HardwareTextureBinding> textures;
                std::vector<StorageBufferBinding> storageBuffers;
                MaterialParams params(material->type(), true);
                setAutoParams(material, textures, storageBuffers, params);
                rn->add(std::move(tmpNode), -1, AttachmentPoints(), material, clusterData.va,
                    std::move(storageBuffers), settings.cluster.gridSize, std::move(params));
            }
        }

        for (const auto& geom : geomList_) {
            std::vector<HardwareTextureBinding> textures;
            std::vector<StorageBufferBinding> storageBuffers;
            MaterialParams params(geom.material->type(), true);
            setAutoParams(geom, textures, storageBuffers, params);
            const auto& activeUniforms = geom.material->type()->prog()->activeUniforms();
            if (activeUniforms.count(UniformName::LightPos) > 0) {
                params.setUniform(UniformName::LightPos, Vector4f_zero);
            }
            if (activeUniforms.count(UniformName::LightColor) > 0) {
                auto ac = camera_->ambientColor();
                ac = gammaToLinear(ac);
                params.setUniform(UniformName::LightColor, Vector3f(ac.x(), ac.y(), ac.z()) * ac.w());
            }
            rn->add(std::move(tmpNode), 0, drawBuffers, geom.material,
                GL_LEQUAL, geom.depthValue,
                geom.material->blendingParams(), geom.flipCull,
                std::move(textures), std::move(storageBuffers),
                geom.vaSlice, geom.primitiveMode, geom.scissorParams,
                std::move(params));
        }

        int pass = 1;
        BlendingParams lightBp(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
        for (int i = static_cast<int>(AttachmentPoint::Color1); i <= static_cast<int>(AttachmentPoint::Max); ++i) {
            drawBuffers.reset(static_cast<AttachmentPoint>(i));
        }

        for (const auto& light : lightList_) {
            auto lightAABB = light->getWorldAABB();
            for (const auto& geom : geomList_) {
                if (!geom.material->type()->usesLight() || !lightAABB.overlaps(geom.aabb)) {
                    continue;
                }
                std::vector<HardwareTextureBinding> textures;
                std::vector<StorageBufferBinding> storageBuffers;
                MaterialParams params(geom.material->type(), true);
                setAutoParams(geom, textures, storageBuffers, params);
                light->setupMaterial(camera_->frustum().transform().getOrigin(), params);
                rn->add(std::move(tmpNode), pass, drawBuffers, geom.material,
                    GL_EQUAL, geom.depthValue,
                    lightBp, geom.flipCull, std::move(textures), std::move(storageBuffers),
                    geom.vaSlice, geom.primitiveMode, geom.scissorParams,
                    std::move(params));
            }
        }

        return rn;
    }

    void RenderList::setAutoParams(const Geometry& geom, std::vector<HardwareTextureBinding>& textures,
        std::vector<StorageBufferBinding>& storageBuffers, MaterialParams& params) const
    {
        setAutoParams(geom.material, textures, storageBuffers, params, geom.modelMat, geom.prevModelMat);
    }

    void RenderList::setAutoParams(const MaterialPtr& material, std::vector<HardwareTextureBinding>& textures,
        std::vector<StorageBufferBinding>& storageBuffers, MaterialParams& params,
        const Matrix4f& modelMat, const Matrix4f& prevModelMat) const
    {
        const Matrix4f& viewProjMat = camera_->frustum().jitteredViewProjMat();
        const Matrix4f& stableViewProjMat = camera_->frustum().viewProjMat();
        const Matrix4f& stableProjMat = camera_->frustum().projMat();

        const auto& activeUniforms = material->type()->prog()->activeUniforms();
        const auto& samplers = material->type()->prog()->samplers();

        for (int i = 0; i <= static_cast<int>(SamplerName::Max); ++i) {
            SamplerName sName = static_cast<SamplerName>(i);
            if (samplers[sName]) {
                const auto& tb = material->textureBinding(sName);
                textures.emplace_back(tb.tex ? tb.tex->hwTex() : HardwareTexturePtr(), tb.params);
                if ((sName == SamplerName::Irradiance) && !textures.back().tex) {
                    auto probe = env_->getLightProbeFor(btVector3_zero);
                    if (probe) {
                        textures.back() = HardwareTextureBinding(probe->irradianceTexture()->hwTex(),
                            SamplerParams(GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR));
                    }
                } else if ((sName == SamplerName::SpecularCM) && !textures.back().tex) {
                    auto probe = env_->getLightProbeFor(btVector3_zero);
                    if (probe) {
                        textures.back() = HardwareTextureBinding(probe->specularTexture()->hwTex(),
                            SamplerParams(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR));
                    }
                } else if ((sName == SamplerName::SpecularLUT) && !textures.back().tex) {
                    auto probe = env_->getLightProbeFor(btVector3_zero);
                    if (probe) {
                        textures.back() = HardwareTextureBinding(probe->specularLUTTexture()->hwTex(),
                            SamplerParams(GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR));
                    }
                }
            }
        }

        if (activeUniforms.count(UniformName::ViewProjMatrix) > 0) {
            params.setUniform(UniformName::ViewProjMatrix, viewProjMat);
        }

        if (activeUniforms.count(UniformName::StableProjMatrix) > 0) {
            params.setUniform(UniformName::StableProjMatrix, stableProjMat);
        }

        bool prevStableMatSet = false;
        bool curStableMatSet = false;

        if (activeUniforms.count(UniformName::ModelViewProjMatrix) > 0) {
            params.setUniform(UniformName::ModelViewProjMatrix, viewProjMat * modelMat);
            if (!prevStableMatSet && activeUniforms.count(UniformName::PrevStableMatrix) > 0) {
                prevStableMatSet = true;
                params.setUniform(UniformName::PrevStableMatrix, camera_->prevViewProjMat() * prevModelMat);
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
                params.setUniform(UniformName::PrevStableMatrix, camera_->prevViewProjMat() * prevModelMat);
            }
            if (!curStableMatSet && activeUniforms.count(UniformName::CurStableMatrix) > 0) {
                curStableMatSet = true;
                params.setUniform(UniformName::CurStableMatrix, stableViewProjMat * modelMat);
            }
        }
        if (!prevStableMatSet && activeUniforms.count(UniformName::PrevStableMatrix) > 0) {
            prevStableMatSet = true;
            params.setUniform(UniformName::PrevStableMatrix, camera_->prevViewProjMat());
        }
        if (!curStableMatSet && activeUniforms.count(UniformName::CurStableMatrix) > 0) {
            curStableMatSet = true;
            params.setUniform(UniformName::CurStableMatrix, stableViewProjMat);
        }
        if (activeUniforms.count(UniformName::EyePos) > 0) {
            params.setUniform(UniformName::EyePos, camera_->frustum().transform().getOrigin());
        }
        if (activeUniforms.count(UniformName::ViewportSize) > 0) {
            params.setUniform(UniformName::ViewportSize, Vector2f::fromVector2i(camera_->viewport().getSize()));
        }
        if (activeUniforms.count(UniformName::Time) > 0) {
            params.setUniform(UniformName::Time, env_->time() + material->timeOffset());
        }
        if (activeUniforms.count(UniformName::Dt) > 0) {
            params.setUniform(UniformName::Dt, env_->dt());
        }
        if (activeUniforms.count(UniformName::RealDt) > 0) {
            params.setUniform(UniformName::RealDt, env_->realDt());
        }
        if (activeUniforms.count(UniformName::SpecularCMLevels) > 0) {
            auto probe = env_->getLightProbeFor(btVector3_zero);
            params.setUniform(UniformName::SpecularCMLevels, static_cast<int>(probe ? probe->specularTextureLevels() : 0));
        }
        if (activeUniforms.count(UniformName::LightProbeInvMatrix) > 0) {
            auto probe = env_->getLightProbeFor(btVector3_zero);
            if (probe) {
                const auto& bounds = probe->bounds();
                auto mat = Matrix4f(probe->parent()->transform() * toTransform(bounds.getCenter())).scaled(bounds.getExtents());
                params.setUniform(UniformName::LightProbeInvMatrix, mat.inverse());
            }
        }
        if (activeUniforms.count(UniformName::LightProbePos) > 0) {
            auto probe = env_->getLightProbeFor(btVector3_zero);
            if (probe) {
                params.setUniform(UniformName::LightProbePos, probe->parent()->pos());
            }
        }
        if (activeUniforms.count(UniformName::LightProbeType) > 0) {
            auto probe = env_->getLightProbeFor(btVector3_zero);
            if (probe) {
                params.setUniform(UniformName::LightProbeType, probe->isGlobal() ? 0 : 1);
            }
        }

        const auto& ssboNames = material->type()->prog()->storageBuffers();
        if (ssboNames[StorageBufferName::ClusterTiles]) {
            storageBuffers.emplace_back(StorageBufferName::ClusterTiles, camera_->clusterData().tilesSSBO);
        }
    }
}

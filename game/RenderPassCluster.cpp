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

#include "RenderPassCluster.h"
#include "CameraRenderer.h"
#include "HardwareResourceManager.h"
#include "MaterialManager.h"
#include "ShaderDataTypes.h"
#include "Settings.h"
#include "Renderer.h"

namespace af3d
{
    int RenderPassCluster::compile(const CameraRenderer& cr, const RenderList& rl, int pass, const RenderNodePtr& rn)
    {
        bool needClusterData = false;
        for (const auto& geom : rl.geomList()) {
            if (!needClusterData) {
                const auto& ssbos = geom.material->type()->prog()->storageBuffers();
                if (ssbos[StorageBufferName::ClusterTileData]) {
                    needClusterData = true;
                    break;
                }
            }
        }

        if (!needClusterData) {
            return pass;
        }

        if (!va_) {
            va_ = std::make_shared<VertexArray>(hwManager.createVertexArray(), VertexArrayLayout(), VBOList());
        }
        if (!tilesSSBO_) {
            tilesSSBO_ = hwManager.createDataBuffer(HardwareBuffer::Usage::StaticCopy, sizeof(ShaderClusterTile));
        }
        if (!tileDataSSBO_) {
            tileDataSSBO_ = hwManager.createDataBuffer(HardwareBuffer::Usage::StaticCopy, sizeof(ShaderClusterTileData));
        }
        if (!lightIndicesSSBO_) {
            lightIndicesSSBO_ = hwManager.createDataBuffer(HardwareBuffer::Usage::StaticCopy, sizeof(std::uint32_t));
        }
        if (!probeIndicesSSBO_) {
            probeIndicesSSBO_ = hwManager.createDataBuffer(HardwareBuffer::Usage::StaticCopy, sizeof(std::uint32_t));
        }
        if (tilesSSBO_->setValid()) {
            auto ssbo = tilesSSBO_;
            renderer.scheduleHwOp([ssbo](HardwareContext& ctx) {
                ssbo->resize(settings.cluster.numTiles, ctx);
            });
        }
        if (tileDataSSBO_->setValid()) {
            auto ssbo = tileDataSSBO_;
            renderer.scheduleHwOp([ssbo](HardwareContext& ctx) {
                ssbo->resize(settings.cluster.numTiles, ctx);
            });
        }
        if (lightIndicesSSBO_->setValid()) {
            auto ssbo = lightIndicesSSBO_;
            renderer.scheduleHwOp([ssbo](HardwareContext& ctx) {
                ssbo->resize(settings.cluster.numTiles * settings.cluster.maxLightsPerTile, ctx);
            });
        }
        if (probeIndicesSSBO_->setValid()) {
            auto ssbo = probeIndicesSSBO_;
            renderer.scheduleHwOp([ssbo](HardwareContext& ctx) {
                ssbo->resize(settings.cluster.numTiles * settings.cluster.maxProbesPerTile, ctx);
            });
        }

        RenderNode tmpNode;

        std::vector<HardwareTextureBinding> textures;
        std::vector<StorageBufferBinding> storageBuffers;

        if (prevProjMat_ != rl.camera()->frustum().projMat()) {
            // Projection changed, recalc cluster tile grid.
            prevProjMat_ = rl.camera()->frustum().projMat();
            auto material = materialManager.createMaterial(MaterialTypeClusterBuild);
            MaterialParams params(material->type(), true);
            cr.setAutoParams(rl, material, 0, textures, storageBuffers, params);
            rn->add(std::move(tmpNode), pass, material, va_,
                std::move(storageBuffers), settings.cluster.gridSize, std::move(params));
        }

        auto material = materialManager.matClusterCull();
        MaterialParams params(material->type(), true);
        cr.setAutoParams(rl, material, 0, textures, storageBuffers, params);
        rn->add(std::move(tmpNode), pass + 1, material, va_,
            std::move(storageBuffers), settings.cluster.cullNumGroups, std::move(params));

        return pass + 2;
    }

    void RenderPassCluster::fillParams(const MaterialPtr& material, std::vector<StorageBufferBinding>& storageBuffers, MaterialParams& params) const
    {
        const auto& ssboNames = material->type()->prog()->storageBuffers();

        if (ssboNames[StorageBufferName::ClusterTiles]) {
            btAssert(tilesSSBO_);
            storageBuffers.emplace_back(StorageBufferName::ClusterTiles, tilesSSBO_);
        }

        if (ssboNames[StorageBufferName::ClusterTileData]) {
            btAssert(tileDataSSBO_);
            storageBuffers.emplace_back(StorageBufferName::ClusterTileData, tileDataSSBO_);
        }

        if (ssboNames[StorageBufferName::ClusterLightIndices]) {
            btAssert(lightIndicesSSBO_);
            storageBuffers.emplace_back(StorageBufferName::ClusterLightIndices, lightIndicesSSBO_);
        }

        if (ssboNames[StorageBufferName::ClusterProbeIndices]) {
            btAssert(probeIndicesSSBO_);
            storageBuffers.emplace_back(StorageBufferName::ClusterProbeIndices, probeIndicesSSBO_);
        }
    }
}

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

#include "MeshManager.h"
#include "BoxMeshGenerator.h"
#include "AssimpMeshLoader.h"
#include "HardwareResourceManager.h"
#include "Logger.h"
#include "Platform.h"
#include "AssimpIOSystem.h"
#include "af3d/Assert.h"
#include <cstring>

namespace af3d
{
    MeshManager meshManager;

    template <>
    Single<MeshManager>* Single<MeshManager>::single = nullptr;

    MeshManager::~MeshManager()
    {
        runtime_assert(cachedMeshes_.empty());
        runtime_assert(immediateMeshes_.empty());
    }

    bool MeshManager::init()
    {
        LOG4CPLUS_DEBUG(logger(), "meshManager: init...");
        importer_.SetIOHandler(new AssimpIOSystem());
        return true;
    }

    void MeshManager::shutdown()
    {
        LOG4CPLUS_DEBUG(logger(), "meshManager: shutdown...");
        runtime_assert(immediateMeshes_.empty());
        cachedMeshes_.clear();
    }

    void MeshManager::reload()
    {
        LOG4CPLUS_DEBUG(logger(), "meshManager: reload...");
        for (const auto& kv : cachedMeshes_) {
            kv.second->invalidate();
            kv.second->load();
        }
        for (auto mesh : immediateMeshes_) {
            mesh->invalidate();
            mesh->load();
        }
    }

    bool MeshManager::renderReload(HardwareContext& ctx)
    {
        LOG4CPLUS_DEBUG(logger(), "meshManager: render reload...");
        return true;
    }

    MeshPtr MeshManager::loadMesh(const std::string& path)
    {
        auto it = cachedMeshes_.find(path);
        if (it != cachedMeshes_.end()) {
            return it->second;
        }

        std::string actualPath;
        boost::optional<MaterialTypeName> convertToMatTypeName;

        auto pos = path.find_first_of('@');
        if (pos != std::string::npos) {
            actualPath = path.substr(0, pos);
            int matTypeNameInt = std::atoi(path.substr(pos + 1).c_str());
            if ((matTypeNameInt < MaterialTypeFirst) || (matTypeNameInt > MaterialTypeMax)) {
                LOG4CPLUS_ERROR(logger(), "meshManager: bad material type int");
                return MeshPtr();
            }
            convertToMatTypeName = static_cast<MaterialTypeName>(matTypeNameInt);
        } else {
            actualPath = path;
        }

        it = cachedMeshes_.find(actualPath);
        if (it == cachedMeshes_.end()) {
            auto loader = std::make_shared<AssimpMeshLoader>(actualPath);

            AABB aabb;
            std::vector<SubMeshPtr> subMeshes;

            if (!loader->init(importer_, aabb, subMeshes)) {
                return MeshPtr();
            }

            auto mesh = std::make_shared<Mesh>(this, actualPath, aabb, subMeshes, loader);
            mesh->aflagsSet(AObjectEditable);
            mesh->load();
            it = cachedMeshes_.emplace(actualPath, mesh).first;
        }

        if (convertToMatTypeName) {
            std::vector<SubMeshPtr> subMeshes;
            for (const auto& subMesh : it->second->subMeshes()) {
                subMeshes.push_back(std::make_shared<SubMesh>(
                    subMesh->material()->convert(*convertToMatTypeName), subMesh->vaSlice()));
            }

            auto mesh = std::make_shared<Mesh>(this, path, it->second->aabb(), subMeshes);
            mesh->aflagsSet(AObjectEditable);
            it = cachedMeshes_.emplace(path, mesh).first;
        }

        return it->second;
    }

    MeshPtr MeshManager::loadConvertedMesh(const std::string& path, MaterialTypeName matTypeName)
    {
        return loadMesh(path + "@" + std::to_string(static_cast<int>(matTypeName)));
    }

    MeshPtr MeshManager::createMesh(const AABB& aabb,
        const std::vector<SubMeshPtr>& subMeshes,
        const ResourceLoaderPtr& loader)
    {
        auto mesh = std::make_shared<Mesh>(this, "", aabb, subMeshes, loader);
        mesh->load();
        immediateMeshes_.insert(mesh.get());
        return mesh;
    }

    MeshPtr MeshManager::createMesh(const AABB& aabb,
        const std::vector<SubMeshPtr>& subMeshes,
        const std::vector<SubMeshDataPtr>& subMeshesData,
        const ResourceLoaderPtr& loader)
    {
        auto mesh = std::make_shared<Mesh>(this, "", aabb, subMeshes, subMeshesData, loader);
        mesh->load();
        immediateMeshes_.insert(mesh.get());
        return mesh;
    }

    MeshPtr MeshManager::createBoxMesh(const btVector3& size,
        const MaterialPtr& material, const std::array<Color, 6>& colors)
    {
        VertexArrayLayout vaLayout;

        vaLayout.addEntry(VertexArrayEntry(VertexAttribName::Pos, GL_FLOAT_VEC3, 0, 0));
        vaLayout.addEntry(VertexArrayEntry(VertexAttribName::UV, GL_FLOAT_VEC2, 12, 0));
        vaLayout.addEntry(VertexArrayEntry(VertexAttribName::Color, GL_UNSIGNED_INT8_VEC4_NV, 20, 0, true));

        auto vbo = hwManager.createDataBuffer(HardwareBuffer::Usage::StaticDraw, 24);
        auto ebo = hwManager.createIndexBuffer(HardwareBuffer::Usage::StaticDraw, HardwareIndexBuffer::UInt16);

        auto va = std::make_shared<VertexArray>(hwManager.createVertexArray(), vaLayout, VBOList{vbo}, ebo);

        auto subMesh = std::make_shared<SubMesh>(material, VertexArraySlice(va));

        return createMesh(AABB(btVector3_zero - (size / 2), btVector3_zero + (size / 2)),
            std::vector<SubMeshPtr>{subMesh}, std::make_shared<BoxMeshGenerator>(size, colors));
    }

    MeshPtr MeshManager::createBoxMesh(const btVector3& size, const MaterialPtr& material)
    {
        VertexArrayLayout vaLayout;

        vaLayout.addEntry(VertexArrayEntry(VertexAttribName::Pos, GL_FLOAT_VEC3, 0, 0));
        vaLayout.addEntry(VertexArrayEntry(VertexAttribName::UV, GL_FLOAT_VEC2, 12, 0));
        vaLayout.addEntry(VertexArrayEntry(VertexAttribName::Normal, GL_FLOAT_VEC3, 20, 0));

        auto vbo = hwManager.createDataBuffer(HardwareBuffer::Usage::StaticDraw, 32);
        auto ebo = hwManager.createIndexBuffer(HardwareBuffer::Usage::StaticDraw, HardwareIndexBuffer::UInt16);

        auto va = std::make_shared<VertexArray>(hwManager.createVertexArray(), vaLayout, VBOList{vbo}, ebo);

        auto subMesh = std::make_shared<SubMesh>(material, VertexArraySlice(va));

        return createMesh(AABB(btVector3_zero - (size / 2), btVector3_zero + (size / 2)),
            std::vector<SubMeshPtr>{subMesh}, std::make_shared<BoxMeshGenerator>(size));
    }

    void MeshManager::onMeshDestroy(Mesh* mesh)
    {
        immediateMeshes_.erase(mesh);
    }
}

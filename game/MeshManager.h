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

#ifndef _MESH_MANAGER_H_
#define _MESH_MANAGER_H_

#include "ResourceManager.h"
#include "Mesh.h"
#include "AssimpMeshLoader.h"
#include "af3d/Single.h"
#include <unordered_map>
#include <unordered_set>

namespace af3d
{
    class MeshManager : public ResourceManager,
                        public Single<MeshManager>
    {
    public:
        MeshManager() = default;
        ~MeshManager();

        bool init() override;

        void shutdown() override;

        void reload() override;

        bool renderReload(HardwareContext& ctx) override;

        MeshPtr loadMesh(const std::string& path);

        MeshPtr loadConvertedMesh(const std::string& path, MaterialTypeName matTypeName);

        MeshPtr createMesh(const AABB& aabb,
            const std::vector<SubMeshPtr>& subMeshes,
            const ResourceLoaderPtr& loader = ResourceLoaderPtr());

        MeshPtr createMesh(const AABB& aabb,
            const std::vector<SubMeshPtr>& subMeshes,
            const std::vector<SubMeshDataPtr>& subMeshesData,
            const ResourceLoaderPtr& loader = ResourceLoaderPtr());

        MeshPtr createBoxMesh(const btVector3& size,
            const MaterialPtr& material, const std::array<Color, 6>& colors);

        MeshPtr createBoxMesh(const btVector3& size, const MaterialPtr& material);

        void onMeshDestroy(Mesh* mesh);

    private:
        using CachedMeshes = std::unordered_map<std::string, MeshPtr>;
        using ImmediateMeshes = std::unordered_set<Mesh*>;

        void processAssimpNode(const AssimpNodePtr& node, const std::string& parentPath);

        CachedMeshes cachedMeshes_;
        ImmediateMeshes immediateMeshes_;

        Assimp::Importer importer_;
    };

    extern MeshManager meshManager;
}

#endif

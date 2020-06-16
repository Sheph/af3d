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

#ifndef _ASSIMPMESHLOADER_H_
#define _ASSIMPMESHLOADER_H_

#include "Resource.h"
#include "SubMesh.h"
#include "Utils.h"
#include "af3d/AABB.h"

namespace af3d
{
    struct AssimpNode;
    using AssimpNodePtr = std::shared_ptr<AssimpNode>;

    struct AssimpNode
    {
        std::string name;
        AABB aabb = AABB_empty;
        std::vector<SubMeshPtr> subMeshes;
        std::vector<AssimpNodePtr> children;
    };

    class AssimpMeshLoader : public ResourceLoader
    {
    public:
        explicit AssimpMeshLoader(const std::string& path);

        AssimpNodePtr init(Assimp::Importer& importer);

        void load(Resource& res, HardwareContext& ctx) override;

    private:
        struct InitContext
        {
            std::vector<MaterialPtr> mats;
            std::map<std::uint32_t, VertexArraySlice> slices;
        };

        struct LoadContext
        {
            std::map<std::uint32_t, SubMeshPtr> slices;
            std::uint32_t numVertices[2];
            float *allVerts[2];
            std::map<std::uint32_t, GLvoid*> allIndices;
        };

        AssimpNodePtr createNode(const aiNode* aiN, const aiMatrix4x4& parentXf, InitContext& ctx);

        void loadNode(const aiNode* aiN, const aiMatrix4x4& parentXf, LoadContext& ctx);

        AssimpScenePtr loadScene(Assimp::Importer& importer);

        MaterialPtr createMaterialBasic(const std::string& matName, aiMaterial* matData);

        MaterialPtr createMaterialPBR(const std::string& matName, aiMaterial* matData);

        MaterialPtr createMaterialFastPBR(const std::string& matName, aiMaterial* matData);

        std::string path_;
        AssimpScenePtr scene_;
        bool ignoreTransforms_;
    };
}

#endif

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

#ifndef _MESH_H_
#define _MESH_H_

#include "Resource.h"
#include "SubMesh.h"
#include "af3d/AABB.h"

namespace af3d
{
    class Mesh;
    class MeshManager;

    using MeshPtr = std::shared_ptr<Mesh>;

    class SubMeshData : boost::noncopyable
    {
    public:
        explicit SubMeshData();
        ~SubMeshData() = default;

        void invalidate();

        void load(const VertexArraySlice& vaSlice);

        inline const std::vector<Vector3f>& vertices() const { return vertices_; }

        inline const std::vector<TriFace>& faces() const { return faces_; }

    private:
        std::atomic<bool> loaded_;
        std::vector<Vector3f> vertices_;
        std::vector<TriFace> faces_;
    };

    using SubMeshDataPtr = std::shared_ptr<SubMeshData>;

    class Mesh : public std::enable_shared_from_this<Mesh>,
        public Resource
    {
    public:
        using ConvertFn = std::function<MaterialPtr(int, const MaterialPtr&)>;

        Mesh(MeshManager* mgr,
            const std::string& name,
            const AABB& aabb,
            const std::vector<SubMeshPtr>& subMeshes,
            const ResourceLoaderPtr& loader = ResourceLoaderPtr());
        Mesh(MeshManager* mgr,
            const std::string& name,
            const AABB& aabb,
            const std::vector<SubMeshPtr>& subMeshes,
            const std::vector<SubMeshDataPtr>& subMeshesData,
            const ResourceLoaderPtr& loader = ResourceLoaderPtr());
        ~Mesh();

        static const AClass& staticKlass();

        static AObjectPtr create(const APropertyValueMap& propVals);

        virtual AObjectPtr sharedThis() override { return shared_from_this(); }

        // Create new unnamed mesh with unnamed cloned materials.
        MeshPtr clone(const ConvertFn& convertFn = ConvertFn()) const;

        inline const AABB& aabb() const { return aabb_; }

        const std::vector<SubMeshPtr>& subMeshes() const { return subMeshes_; }

        SubMeshDataPtr getSubMeshData(int idx);

    private:
        MeshManager* mgr_;
        AABB aabb_;
        std::vector<SubMeshPtr> subMeshes_;
        std::vector<SubMeshDataPtr> subMeshesData_;
    };

    extern const APropertyTypeObject APropertyType_Mesh;

    ACLASS_DECLARE(Mesh)
}

#endif

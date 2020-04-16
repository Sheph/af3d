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

#include "CollisionShapeStaticMesh.h"
#include "Logger.h"
#include "PhysicsDebugDraw.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(CollisionShapeStaticMesh, CollisionShape)
    COLLISIONSHAPE_PARAM(CollisionShapeStaticMesh, "mesh", "Mesh", Mesh, MeshPtr())
    COLLISIONSHAPE_PARAM(CollisionShapeStaticMesh, "submesh index", "SubMesh index (< 0 - use all)", Int, -1)
    ACLASS_DEFINE_END(CollisionShapeStaticMesh)

    CollisionShapeStaticMesh::CollisionShapeStaticMesh(const MeshPtr& mesh, int subMeshIndex)
    : CollisionShape(AClass_CollisionShapeStaticMesh),
      shape_(initMesh(mesh, subMeshIndex), true, true)
    {
    }

    const AClass& CollisionShapeStaticMesh::staticKlass()
    {
        return AClass_CollisionShapeStaticMesh;
    }

    AObjectPtr CollisionShapeStaticMesh::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<CollisionShapeStaticMesh>(propVals.get("mesh").toObject<Mesh>(), propVals.get("submesh index").toInt());
        obj->afterCreate(propVals);
        return obj;
    }

    void CollisionShapeStaticMesh::render(PhysicsDebugDraw& dd, const btVector3& c)
    {
        dd.drawMesh(&shape_, worldTransform(), c);
    }

    btTriangleMesh* CollisionShapeStaticMesh::initMesh(const MeshPtr& mesh, int subMeshIndex)
    {
        // TODO: cache btBvhTriangleMeshShape, use from cache.

        if (subMeshIndex >= static_cast<int>(mesh->subMeshes().size())) {
            LOG4CPLUS_WARN(logger(), "subMeshIndex " << subMeshIndex << " too high, resetting to 0");
            subMeshIndex = 0;
        }

        if (subMeshIndex < 0) {
            int base = 0;
            for (size_t i = 0; i < mesh->subMeshes().size(); ++i) {
                auto data = mesh->getSubMeshData(i);
                for (const auto& v : data->vertices()) {
                    mesh_.findOrAddVertex(fromVector3f(v), false);
                }
                for (const auto& f : data->faces()) {
                    mesh_.addTriangleIndices(base + f[0], base + f[1], base + f[2]);
                }
                base += data->vertices().size();
            }
        } else {
            auto data = mesh->getSubMeshData(subMeshIndex);
            for (const auto& v : data->vertices()) {
                mesh_.findOrAddVertex(fromVector3f(v), false);
            }
            for (const auto& f : data->faces()) {
                mesh_.addTriangleIndices(f[0], f[1], f[2]);
            }
        }

        return &mesh_;
    }
}

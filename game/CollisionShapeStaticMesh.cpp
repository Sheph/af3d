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
#include "MeshManager.h"
#include "Logger.h"
#include "PhysicsDebugDraw.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(CollisionShapeStaticMesh, CollisionShape)
    COLLISIONSHAPE_PARAM(CollisionShapeStaticMesh, "mesh", "Mesh", StringMesh, "")
    COLLISIONSHAPE_PARAM(CollisionShapeStaticMesh, "submesh index", "SubMesh index (< 0 - use all)", Int, -1)
    COLLISIONSHAPE_PARAM(CollisionShapeStaticMesh, "recreate", "Recreate", Bool, false)
    COLLISIONSHAPE_PARAM_HIDDEN(CollisionShapeStaticMesh, "vertices", "Vertices", ArrayVec3f, std::vector<APropertyValue>{})
    COLLISIONSHAPE_PARAM_HIDDEN(CollisionShapeStaticMesh, "faces", "Faces", ArrayVec3i, std::vector<APropertyValue>{})
    ACLASS_DEFINE_END(CollisionShapeStaticMesh)

    CollisionShapeStaticMesh::CollisionShapeStaticMesh(const std::vector<APropertyValue>& vertices,
        const std::vector<APropertyValue>& faces)
    : CollisionShape(AClass_CollisionShapeStaticMesh),
      shape_(initMesh(vertices, faces), true, true)
    {
    }

    const AClass& CollisionShapeStaticMesh::staticKlass()
    {
        return AClass_CollisionShapeStaticMesh;
    }

    AObjectPtr CollisionShapeStaticMesh::create(const APropertyValueMap& propVals)
    {
        auto verts = propVals.get("vertices").toArray();
        auto faces = propVals.get("faces").toArray();
        bool recreate = propVals.get("recreate").toBool() || verts.empty() || faces.empty();

        if (recreate) {
            verts.clear();
            faces.clear();

            MeshPtr mesh = meshManager.loadMesh(propVals.get("mesh").toString());
            if (!mesh) {
                mesh = meshManager.loadMesh("cube.fbx");
            }
            int subMeshIndex = propVals.get("submesh index").toInt();

            if (subMeshIndex >= static_cast<int>(mesh->subMeshes().size())) {
                LOG4CPLUS_WARN(logger(), "subMeshIndex " << subMeshIndex << " too high, resetting to 0");
                subMeshIndex = 0;
            }

            if (subMeshIndex < 0) {
                int base = 0;
                for (size_t i = 0; i < mesh->subMeshes().size(); ++i) {
                    auto data = mesh->getSubMeshData(i);
                    for (const auto& v : data->vertices()) {
                        verts.emplace_back(v);
                    }
                    for (const auto& f : data->faces()) {
                        faces.emplace_back(TriFace(base + f[0], base + f[1], base + f[2]));
                    }
                    base += data->vertices().size();
                }
            } else {
                auto data = mesh->getSubMeshData(subMeshIndex);
                for (const auto& v : data->vertices()) {
                    verts.emplace_back(v);
                }
                for (const auto& f : data->faces()) {
                    faces.emplace_back(f);
                }
            }
        }

        auto obj = std::make_shared<CollisionShapeStaticMesh>(verts, faces);
        if (recreate) {
            APropertyValueMap propVals2 = propVals;
            propVals2.set("recreate", false);
            propVals2.set("vertices", verts);
            propVals2.set("faces", faces);
            obj->afterCreate(propVals2);
        } else {
            obj->afterCreate(propVals);
        }
        return obj;
    }

    void CollisionShapeStaticMesh::render(PhysicsDebugDraw& dd, const btVector3& c)
    {
        dd.drawMesh(&shape_, worldTransform(), c);
    }

    btTriangleMesh* CollisionShapeStaticMesh::initMesh(const std::vector<APropertyValue>& vertices,
        const std::vector<APropertyValue>& faces)
    {
        // TODO: cache btBvhTriangleMeshShape, use from cache.

        for (const auto& v : vertices) {
            mesh_.findOrAddVertex(v.toVec3(), false);
        }
        for (const auto& f : faces) {
            const auto& fv = f.toVec3i();
            mesh_.addTriangleIndices(fv.x(), fv.y(), fv.z());
        }

        return &mesh_;
    }
}

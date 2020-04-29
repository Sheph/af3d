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

#include "CollisionShapeConvexMesh.h"
#include "MeshManager.h"
#include "Logger.h"
#include "PhysicsDebugDraw.h"
#include "bullet/BulletCollision/CollisionShapes/btConvexPolyhedron.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(CollisionShapeConvexMesh, CollisionShape)
    COLLISIONSHAPE_PARAM(CollisionShapeConvexMesh, "mesh", "Mesh", StringMesh, "")
    COLLISIONSHAPE_PARAM(CollisionShapeConvexMesh, "submesh index", "SubMesh index (< 0 - use all)", Int, -1)
    COLLISIONSHAPE_PARAM(CollisionShapeConvexMesh, "polyhedral", "Generate polyhedral features", Bool, false)
    COLLISIONSHAPE_PARAM(CollisionShapeConvexMesh, "offset", "Shape offset", Vec3f, btVector3(0.0f, 0.0f, 0.0f))
    COLLISIONSHAPE_PARAM(CollisionShapeStaticMesh, "recreate", "Recreate", Bool, false)
    COLLISIONSHAPE_PARAM_HIDDEN(CollisionShapeConvexMesh, "vertices", "Vertices", ArrayVec3f, std::vector<APropertyValue>{})
    COLLISIONSHAPE_PARAM_HIDDEN(CollisionShapeConvexMesh, "faces", "Faces", ArrayArrayInt, std::vector<APropertyValue>{})
    COLLISIONSHAPE_PARAM_HIDDEN(CollisionShapeConvexMesh, "planes", "Planes", ArrayVec4f, std::vector<APropertyValue>{})
    ACLASS_DEFINE_END(CollisionShapeConvexMesh)

    CollisionShapeConvexMesh::CollisionShapeConvexMesh(const std::vector<APropertyValue>& vertices,
        const std::vector<APropertyValue>& faces, const std::vector<APropertyValue>& planes)
    : CollisionShape(AClass_CollisionShapeConvexMesh)
    {
        if (faces.empty()) {
            for (size_t i = 0; i < vertices.size(); ++i) {
                shape_.addPoint(vertices[i].toVec3(), i == (vertices.size() - 1));
            }
        } else {
            runtime_assert(faces.size() == planes.size());
            btConvexPolyhedron polyhedron;
            for (size_t i = 0; i < vertices.size(); ++i) {
                btVector3& v = polyhedron.m_vertices.expand();
                v = vertices[i].toVec3();
                shape_.addPoint(v, i == (vertices.size() - 1));
            }
            for (size_t i = 0; i < faces.size(); ++i) {
                btFace& f = polyhedron.m_faces.expand();
                auto indices = faces[i].toArray();
                for (size_t j = 0; j < indices.size(); ++j) {
                    int& idx = f.m_indices.expand();
                    idx = indices[j].toInt();
                }
                auto plane = planes[i].toVec4f();
                f.m_plane[0] = plane.x();
                f.m_plane[1] = plane.y();
                f.m_plane[2] = plane.z();
                f.m_plane[3] = plane.w();
            }
            polyhedron.initialize();
            shape_.setPolyhedralFeatures(polyhedron);
        }
    }

    const AClass& CollisionShapeConvexMesh::staticKlass()
    {
        return AClass_CollisionShapeConvexMesh;
    }

    AObjectPtr CollisionShapeConvexMesh::create(const APropertyValueMap& propVals)
    {
        auto verts = propVals.get("vertices").toArray();
        auto faces = propVals.get("faces").toArray();
        auto planes = propVals.get("planes").toArray();
        bool recreate = propVals.get("recreate").toBool() || verts.empty();

        if (recreate) {
            verts.clear();
            faces.clear();
            planes.clear();

            MeshPtr mesh = meshManager.loadMesh(propVals.get("mesh").toString());
            if (!mesh) {
                mesh = meshManager.loadMesh("cube.fbx");
            }
            int subMeshIndex = propVals.get("submesh index").toInt();
            bool polyhedral = propVals.get("polyhedral").toBool();
            btVector3 offset = propVals.get("offset").toVec3();

            if (subMeshIndex >= static_cast<int>(mesh->subMeshes().size())) {
                LOG4CPLUS_WARN(logger(), "subMeshIndex " << subMeshIndex << " too high, resetting to 0");
                subMeshIndex = 0;
            }

            btConvexHullShape shape;

            if (subMeshIndex < 0) {
                for (size_t j = 0; j < mesh->subMeshes().size(); ++j) {
                    auto data = mesh->getSubMeshData(j);
                    for (size_t i = 0; i < data->vertices().size(); ++i) {
                        shape.addPoint(fromVector3f(data->vertices()[i]) + offset, i == (data->vertices().size() - 1));
                    }
                }
            } else {
                auto data = mesh->getSubMeshData(subMeshIndex);
                for (size_t i = 0; i < data->vertices().size(); ++i) {
                    shape.addPoint(fromVector3f(data->vertices()[i]) + offset, i == (data->vertices().size() - 1));
                }
            }

            shape.optimizeConvexHull();
            if (polyhedral) {
                shape.setLocalScaling(propVals.get(AProperty_Scale).toVec3());
                shape.initializePolyhedralFeatures();
                auto polyhedron = shape.getConvexPolyhedron();
                for (int i = 0; i < polyhedron->m_vertices.size(); ++i) {
                    verts.emplace_back(polyhedron->m_vertices[i]);
                }
                for (int i = 0; i < polyhedron->m_faces.size(); ++i) {
                    std::vector<APropertyValue> indices;
                    for (int j = 0; j < polyhedron->m_faces[i].m_indices.size(); ++j) {
                        indices.emplace_back(polyhedron->m_faces[i].m_indices[j]);
                    }
                    faces.emplace_back(indices);
                    planes.emplace_back(Vector4f(polyhedron->m_faces[i].m_plane[0],
                        polyhedron->m_faces[i].m_plane[1],
                        polyhedron->m_faces[i].m_plane[2],
                        polyhedron->m_faces[i].m_plane[3]));
                }
            } else {
                for (int i = 0; i < shape.getNumPoints(); ++i) {
                    verts.emplace_back(shape.getPoints()[i]);
                }
            }
        }

        auto obj = std::make_shared<CollisionShapeConvexMesh>(verts, faces, planes);
        if (recreate) {
            APropertyValueMap propVals2 = propVals;
            propVals2.set("recreate", false);
            propVals2.set("vertices", verts);
            propVals2.set("faces", faces);
            propVals2.set("planes", planes);
            obj->afterCreate(propVals2);
        } else {
            obj->afterCreate(propVals);
        }
        return obj;
    }

    void CollisionShapeConvexMesh::render(PhysicsDebugDraw& dd, const btVector3& c)
    {
        dd.drawMesh(&shape_, worldTransform(), c);
    }
}

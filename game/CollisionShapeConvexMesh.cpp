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
#include "Logger.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(CollisionShapeConvexMesh, CollisionShape)
    COLLISIONSHAPE_PARAM(CollisionShapeConvexMesh, "mesh", "Mesh", Mesh, MeshPtr())
    COLLISIONSHAPE_PARAM(CollisionShapeConvexMesh, "submesh index", "SubMesh index (< 0 - use all)", Int, -1)
    COLLISIONSHAPE_PARAM(CollisionShapeConvexMesh, "polyhedral", "Generate polyhedral features", Bool, false)
    ACLASS_DEFINE_END(CollisionShapeConvexMesh)

    CollisionShapeConvexMesh::CollisionShapeConvexMesh(const MeshPtr& mesh, int subMeshIndex, bool polyhedral)
    : CollisionShape(AClass_CollisionShapeConvexMesh),
      polyhedral_(polyhedral)
    {
        if (subMeshIndex >= static_cast<int>(mesh->subMeshes().size())) {
            LOG4CPLUS_WARN(logger(), "subMeshIndex " << subMeshIndex << " too high, resetting to 0");
            subMeshIndex = 0;
        }

        if (subMeshIndex < 0) {
            for (size_t j = 0; j < mesh->subMeshes().size(); ++j) {
                auto data = mesh->getSubMeshData(j);
                for (size_t i = 0; i < data->vertices().size(); ++i) {
                    shape_.addPoint(fromVector3f(data->vertices()[i]), i == (data->vertices().size() - 1));
                }
            }
        } else {
            auto data = mesh->getSubMeshData(subMeshIndex);
            for (size_t i = 0; i < data->vertices().size(); ++i) {
                shape_.addPoint(fromVector3f(data->vertices()[i]), i == (data->vertices().size() - 1));
            }
        }

        shape_.optimizeConvexHull();
        doSetScale(shape_.getLocalScaling());
    }

    const AClass& CollisionShapeConvexMesh::staticKlass()
    {
        return AClass_CollisionShapeConvexMesh;
    }

    AObjectPtr CollisionShapeConvexMesh::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<CollisionShapeConvexMesh>(propVals.get("mesh").toObject<Mesh>(),
            propVals.get("submesh index").toInt(),
            propVals.get("polyhedral").toBool());
        obj->afterCreate(propVals);
        return obj;
    }

    void CollisionShapeConvexMesh::doSetScale(const btVector3& value)
    {
        if (polyhedral_) {
            shape_.initializePolyhedralFeatures();
        }
    }

    void CollisionShapeConvexMesh::render(PhysicsDebugDraw& dd, const btVector3& c)
    {
    }
}

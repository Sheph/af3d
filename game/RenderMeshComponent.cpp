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

#include "RenderMeshComponent.h"
#include "SceneObject.h"

namespace af3d
{
    RenderMeshComponent::RenderMeshComponent(const MeshPtr& mesh)
    : mesh_(mesh)
    {
    }

    void RenderMeshComponent::update(float dt)
    {
        if (testRotate) {
            setTransform(btTransform(btQuaternion(btVector3(1.0f, 1.0f, 1.0f), btRadians(dt * 90.0f))) * xf_);
        }

        if ((parent()->transform() == prevParentXf_) && !dirty_) {
            return;
        }

        dirty_ = false;

        AABB aabb = calcAABB();

        btVector3 displacement = parent()->transform().getOrigin() - prevParentXf_.getOrigin();

        manager()->moveAABB(cookie_, prevAABB_, aabb, displacement);

        prevParentXf_ = parent()->transform();
        prevAABB_ = aabb;
    }

    void RenderMeshComponent::render(RenderList& rl, void* const* parts, size_t numParts)
    {
        auto modelMat = Matrix4f(parent()->transform() * xf_).scaled(scale_);
        for (const auto& subMesh : mesh_->subMeshes()) {
            rl.addGeometry(modelMat, prevAABB_, subMesh->material(), subMesh->vaSlice(), GL_TRIANGLES);
        }
    }

    void RenderMeshComponent::debugDraw()
    {
    }

    void RenderMeshComponent::setTransform(const btTransform& value)
    {
        xf_ = value;
        dirty_ = true;
    }

    void RenderMeshComponent::setScale(const btVector3& value)
    {
        scale_ = value;
        dirty_ = true;
    }

    void RenderMeshComponent::onRegister()
    {
        prevParentXf_ = parent()->transform();
        prevAABB_ = calcAABB();
        cookie_ = manager()->addAABB(this, prevAABB_, nullptr);
    }

    void RenderMeshComponent::onUnregister()
    {
        manager()->removeAABB(cookie_);
    }

    AABB RenderMeshComponent::calcAABB() const
    {
        return mesh_->aabb().scaled(scale_).getTransformed(parent()->transform() * xf_);
    }
}

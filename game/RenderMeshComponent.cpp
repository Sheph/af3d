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
#include "MaterialManager.h"
#include "Scene.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(RenderMeshComponent, RenderComponent)
    ACLASS_PROPERTY(RenderMeshComponent, Mesh, "mesh", "Mesh", Mesh, APropertyValue(MeshPtr()), General)
    ACLASS_PROPERTY(RenderMeshComponent, LocalTransform, AProperty_LocalTransform, "Local transform", Transform, btTransform::getIdentity(), Position)
    ACLASS_PROPERTY(RenderMeshComponent, WorldTransform, AProperty_WorldTransform, "World transform", Transform, btTransform::getIdentity(), Position)
    ACLASS_PROPERTY(RenderMeshComponent, Scale, AProperty_Scale, "Mesh scale", Vec3f, btVector3(1.0f, 1.0f, 1.0f), Position)
    ACLASS_DEFINE_END(RenderMeshComponent)

    RenderMeshComponent::RenderMeshComponent()
    : RenderComponent(AClass_RenderMeshComponent)
    {
    }

    const AClass& RenderMeshComponent::staticKlass()
    {
        return AClass_RenderMeshComponent;
    }

    AObjectPtr RenderMeshComponent::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<RenderMeshComponent>();
        obj->propertiesSet(propVals);
        return obj;
    }

    void RenderMeshComponent::update(float dt)
    {
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

        render(rl, modelMat, MaterialPtr());

        auto w = scene()->workspace();
        if (w) {
            auto em = w->emVisual();
            if (em->active()) {
                if (em->isSelected(shared_from_this())) {
                    render(rl, modelMat, materialManager.matOutlineSelected());
                } else if (em->isHovered(shared_from_this())) {
                    render(rl, modelMat, materialManager.matOutlineHovered());
                } else {
                    render(rl, modelMat, materialManager.matOutlineInactive());
                }
            }
        }
    }

    std::pair<AObjectPtr, float> RenderMeshComponent::testRay(const Frustum& frustum, const Ray& ray, void* part)
    {
        auto res = ray.testAABB(prevAABB_);
        if (res.first) {
            return std::make_pair(sharedThis(), res.second);
        } else {
            return std::make_pair(AObjectPtr(), 0.0f);
        }
    }

    void RenderMeshComponent::debugDraw()
    {
    }

    void RenderMeshComponent::setMesh(const MeshPtr& value)
    {
        mesh_ = value;
        dirty_ = true;
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

    void RenderMeshComponent::render(RenderList& rl, const Matrix4f& modelMat, const MaterialPtr& material)
    {
        for (const auto& subMesh : mesh_->subMeshes()) {
            rl.addGeometry(modelMat, prevAABB_,
                (material ? material : subMesh->material()), subMesh->vaSlice(),
                GL_TRIANGLES);
        }
    }
}

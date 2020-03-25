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

#ifndef _RENDERMESHCOMPONENT_H_
#define _RENDERMESHCOMPONENT_H_

#include "RenderComponent.h"
#include "Mesh.h"
#include "SceneObject.h"

namespace af3d
{
    class RenderMeshComponent : public std::enable_shared_from_this<RenderMeshComponent>,
        public RenderComponent
    {
    public:
        RenderMeshComponent();
        ~RenderMeshComponent() = default;

        static const AClass& staticKlass();

        static AObjectPtr create(const APropertyValueMap& propVals);

        ComponentPtr sharedThis() override { return shared_from_this(); }

        void update(float dt) override;

        void render(RenderList& rl, void* const* parts, size_t numParts) override;

        void debugDraw() override;

        inline const MeshPtr& mesh() const { return mesh_; }
        void setMesh(const MeshPtr& value);

        inline const btTransform& transform() const { return xf_; }
        void setTransform(const btTransform& value);

        inline const btVector3& scale() const { return scale_; }
        void setScale(const btVector3& value);

        APropertyValue propertyMeshGet() const { return APropertyValue(mesh()); }
        void propertyMeshSet(const APropertyValue& value) { setMesh(value.toObject<Mesh>()); }

        APropertyValue propertyLocalTransformGet() const { return transform(); }
        void propertyLocalTransformSet(const APropertyValue& value) { setTransform(value.toTransform()); }

        APropertyValue propertyWorldTransformGet() const { return parent()->transform() * transform(); }
        void propertyWorldTransformSet(const APropertyValue& value) { setTransform(parent()->transform().inverse() * value.toTransform()); }

        APropertyValue propertyScaleGet() const { return scale(); }
        void propertyScaleSet(const APropertyValue& value) { setScale(value.toVec3()); }

    private:
        void onRegister() override;

        void onUnregister() override;

        AABB calcAABB() const;

        MeshPtr mesh_;
        btTransform xf_ = btTransform::getIdentity();
        btVector3 scale_ = btVector3_one;
        bool dirty_ = false;

        btTransform prevParentXf_;
        AABB prevAABB_;
        RenderCookie* cookie_ = nullptr;
    };

    using RenderMeshComponentPtr = std::shared_ptr<RenderMeshComponent>;

    ACLASS_DECLARE(RenderMeshComponent)
}

#endif

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

#include "CollisionShape.h"
#include "PhysicsBodyComponent.h"
#include "SceneObject.h"
#include "Settings.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN_ABSTRACT(CollisionShape, AObject)
    ACLASS_PROPERTY_RO(CollisionShape, Parent, AProperty_Parent, "Parent", AObject, Hierarchy, APropertyTransient)
    ACLASS_PROPERTY(CollisionShape, LocalTransform, AProperty_LocalTransform, "Local transform", Transform, btTransform::getIdentity(), Position, APropertyEditable)
    ACLASS_PROPERTY(CollisionShape, WorldTransform, AProperty_WorldTransform, "World transform", Transform, btTransform::getIdentity(), Position, APropertyEditable|APropertyTransient)
    ACLASS_DEFINE_END(CollisionShape)

    CollisionShape::CollisionShape(const AClass& klass, btCollisionShape* shape)
    : AObject(klass),
      shape_(shape)
    {
    }

    CollisionShape::~CollisionShape()
    {
        btAssert(parent_ == nullptr);
    }

    CollisionShape* CollisionShape::fromShape(btCollisionShape* shape)
    {
        auto ptr = shape->getUserPointer();
        if (!ptr) {
            return nullptr;
        }
        return static_cast<CollisionShapePtr*>(ptr)->get();
    }

    const AClass& CollisionShape::staticKlass()
    {
        return AClass_CollisionShape;
    }

    void CollisionShape::afterCreate(const APropertyValueMap& propVals)
    {
        propertiesSet(propVals);
        if (settings.editor.enabled) {
            APropertyValueMap params;
            for (const auto& prop : klass().thisProperties()) {
                if (prop.category() == APropertyCategory::Params) {
                    params.set(prop.name(), propVals.get(prop.name()));
                }
            }
            setParams(params);
        }
    }

    SceneObject* CollisionShape::parentObject() const
    {
        return parent() ? parent()->parent() : nullptr;
    }

    void CollisionShape::setTransform(const btTransform& value)
    {
        xf_ = value;
        if (parent_) {
            parent_->updateBodyCollision(false);
        }
    }

    void CollisionShape::setMass(float value)
    {
        mass_ = value;
        if (parent_) {
            parent_->updateBodyCollision(false);
        }
    }

    void CollisionShape::removeFromParent()
    {
        if (parent()) {
            parent()->removeShape(std::static_pointer_cast<CollisionShape>(sharedThis()));
        }
    }

    void CollisionShape::adopt(PhysicsBodyComponent* parent)
    {
        parent_ = parent;
    }

    void CollisionShape::abandon()
    {
        auto obj = parentObject();
        if (obj) {
            abandonedParentXf_ = obj->transform();
        }
        parent_ = nullptr;
    }

    APropertyValue CollisionShape::propertyParentGet(const std::string&) const
    {
        return APropertyValue(parent_ ? parent_->sharedThis() : AObjectPtr());
    }

    APropertyValue CollisionShape::propertyWorldTransformGet(const std::string&) const
    {
        auto obj = parentObject();
        if (obj) {
            return obj->transform() * xf_;
        } else {
            return abandonedParentXf_ * xf_;
        }
    }

    void CollisionShape::propertyWorldTransformSet(const std::string&, const APropertyValue& value)
    {
        auto obj = parentObject();
        if (obj) {
            setTransform(obj->transform().inverse() * value.toTransform());
        } else {
            setTransform(abandonedParentXf_.inverse() * value.toTransform());
        }
    }

    void CollisionShape::assignUserPointer()
    {
        runtime_assert(shape_->getUserPointer() == nullptr);
        CollisionShapePtr s = std::static_pointer_cast<CollisionShape>(sharedThis());
        CollisionShapePtr* ptr = new CollisionShapePtr(s);
        shape_->setUserPointer(ptr);
    }

    void CollisionShape::resetUserPointer()
    {
        CollisionShapePtr* ptr = static_cast<CollisionShapePtr*>(shape_->getUserPointer());
        if (!ptr) {
            return;
        }
        shape_->setUserPointer(nullptr);
        delete ptr;
    }
}

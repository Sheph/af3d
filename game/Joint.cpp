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

#include "Joint.h"
#include "Settings.h"
#include "Scene.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN_ABSTRACT(Joint, AObject)
    ACLASS_PROPERTY_RO(Joint, Parent, AProperty_Parent, "Parent", AObject, Hierarchy, APropertyTransient)
    ACLASS_DEFINE_END(Joint)

    Joint::Joint(const AClass& klass,
        const SceneObjectPtr& objectA, const SceneObjectPtr& objectB,
        bool collideConnected)
    : AObject(klass),
      objectA_(objectA),
      objectB_(objectB),
      collideConnected_(collideConnected),
      hasBodyB_(objectB)
    {
    }

    Joint::~Joint()
    {
        btAssert(parent_ == nullptr);
    }

    const AClass& Joint::staticKlass()
    {
        return AClass_Joint;
    }

    Joint* Joint::fromConstraint(btTypedConstraint* constraint)
    {
        auto ptr = constraint->getUserConstraintPtr();
        if (!ptr) {
            return nullptr;
        }
        return static_cast<Joint*>(ptr);
    }

    void Joint::refresh(bool forceDelete)
    {
        doRefresh(forceDelete);
        auto c = constraint();
        btAssert(!(forceDelete && c));
        if (c) {
            c->setUserConstraintPtr(this);
        }
    }

    void Joint::afterCreate(const APropertyValueMap& propVals)
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

    void Joint::removeFromParent()
    {
        if (parent_) {
            parent_->removeJoint(std::static_pointer_cast<Joint>(sharedThis()));
        }
    }

    SceneObjectPtr Joint::objectA()
    {
        return objectA_.lock();
    }

    SceneObjectPtr Joint::objectB()
    {
        return objectB_.lock();
    }

    bool Joint::enabled() const
    {
        auto c = constraint();
        return c ? c->isEnabled() : false;
    }

    void Joint::adopt(Scene* parent)
    {
        parent_ = parent;
        refresh(false);
    }

    void Joint::abandon()
    {
        refresh(true);
        parent_ = nullptr;
    }

    APropertyValue Joint::propertyParentGet(const std::string&) const
    {
        return APropertyValue(parent_ ? parent_->sharedThis() : AObjectPtr());
    }
}

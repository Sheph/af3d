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

#ifndef _COLLISIONSHAPE_H_
#define _COLLISIONSHAPE_H_

#include "AObject.h"
#include "af3d/Types.h"
#include "bullet/btBulletCollisionCommon.h"

namespace af3d
{
    class PhysicsBodyComponent;
    class Scene;
    class SceneObject;
    class PhysicsDebugDraw;

    class CollisionShape : public AObject
    {
    public:
        explicit CollisionShape(const AClass& klass);
        ~CollisionShape();

        // btCollisionShape's userPointer is CollisionShapePtr* unless
        static CollisionShape* fromShape(btCollisionShape* shape);

        static const AClass& staticKlass();

        virtual btCollisionShape* shape() = 0;

        virtual void render(PhysicsDebugDraw& dd, const btVector3& c) = 0;

        void onActivate();

        void onDeactivate();

        inline const btCollisionShape* shape() const { return const_cast<CollisionShape*>(this)->shape(); }

        inline bool active() const { return active_; }

        void afterCreate(const APropertyValueMap& propVals);

        Scene* scene() const;

        inline PhysicsBodyComponent* parent() const { return parent_; }
        SceneObject* parentObject() const;

        inline const btTransform& transform() const { return xf_; }
        void setTransform(const btTransform& value);

        btTransform worldTransform() const;

        inline const btVector3& scale() const { return shape()->getLocalScaling(); }
        void setScale(const btVector3& value);

        inline float mass() const { return mass_; }
        void setMass(float value);

        void removeFromParent();

        inline const APropertyValueMap& params() const { return params_; }
        inline void setParams(const APropertyValueMap& value) { params_ = value; }

        /*
         * Internal, do not call.
         * @{
         */

        void adopt(PhysicsBodyComponent* parent);
        void abandon();

        void assignUserPointer();
        void resetUserPointer();

        /*
         * @}
         */

        APropertyValue propertyParentGet(const std::string&) const;

        APropertyValue propertyLocalTransformGet(const std::string&) const { return transform(); }
        void propertyLocalTransformSet(const std::string&, const APropertyValue& value) { setTransform(value.toTransform()); }

        APropertyValue propertyWorldTransformGet(const std::string&) const;
        void propertyWorldTransformSet(const std::string&, const APropertyValue& value);

        APropertyValue propertyScaleGet(const std::string&) const { return scale(); }
        void propertyScaleSet(const std::string&, const APropertyValue& value) { setScale(value.toVec3()); }

        APropertyValue propertyMassGet(const std::string&) const { return mass(); }
        void propertyMassSet(const std::string&, const APropertyValue& value) { setMass(value.toFloat()); }

        APropertyValue propertyParamGet(const std::string& key) const { return params_.get(key); }

    private:
        virtual void doSetScale(const btVector3& value);

        btTransform xf_ = btTransform::getIdentity();
        float mass_ = 1.0f;

        PhysicsBodyComponent* parent_ = nullptr;
        btTransform abandonedParentXf_ = btTransform::getIdentity();

        APropertyValueMap params_;
        bool active_ = false;
    };

    using CollisionShapePtr = std::shared_ptr<CollisionShape>;

    using CollisionShapes = std::vector<CollisionShapePtr>;

    ACLASS_DECLARE(CollisionShape)

    #define COLLISIONSHAPE_PARAM(SName, Name, Tooltip, Type, Def) \
        {Name, Tooltip, APropertyType_##Type, APropertyValue(Def), APropertyCategory::Params, APropertyReadable|APropertyEditable, (APropertyGetter)&CollisionShape::propertyParamGet, nullptr},
}

#endif

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

#ifndef _JOINT_H_
#define _JOINT_H_

#include "AObject.h"
#include "AParameterized.h"
#include "SceneObjectManager.h"
#include "af3d/Types.h"
#include "af3d/Utils.h"
#include "bullet/btBulletDynamicsCommon.h"

namespace af3d
{
    class Joint : public AObject, public AParameterized
    {
    public:
        Joint(const AClass& klass,
            const SceneObjectPtr& objectA, const SceneObjectPtr& objectB,
            bool collideConnected);
        ~Joint();

        static const AClass& staticKlass();

        static Joint* fromConstraint(btTypedConstraint* constraint);

        inline bool hasBodyB() const { return hasBodyB_; }

        virtual btTypedConstraint* constraint() = 0;

        void refresh(bool forceDelete);

        inline const btTypedConstraint* constraint() const { return const_cast<Joint*>(this)->constraint(); }

        void afterCreate(const APropertyValueMap& propVals);

        void removeFromParent();

        SceneObjectPtr objectA() const;
        SceneObjectPtr objectB() const;
        inline bool collideConnected() const { return collideConnected_; }

        bool enabled() const;

        inline Scene* parent() { return parent_; }
        inline const Scene* parent() const { return parent_; }

        /*
         * Internal, do not call.
         * @{
         */

        void adopt(Scene* parent);
        void abandon();

        /*
         * @}
         */

        APropertyValue propertyParentGet(const std::string&) const;

        APropertyValue propertyWorldPositionGet(const std::string&) const { return pos(); }
        void propertyWorldPositionSet(const std::string&, const APropertyValue& value) { setPos(value.toVec3()); }

        APropertyValue propertyParamGet(const std::string& key) const { return params().get(key); }

    protected:
        void setDirty();

        inline const btVector3& pos() const { return pos_; }
        inline void setPos(const btVector3& value) { pos_ = value; }

        SceneObjectPtr createPointEdit(const std::string& xfPropName, bool isDefault = false);

    private:
        virtual void doRefresh(bool forceDelete) = 0;

        virtual void doAdopt(bool withEdit) = 0;

        virtual void doAbandon() = 0;

        mutable btVector3 pos_ = btVector3_zero;

        std::weak_ptr<SceneObject> objectA_;
        std::weak_ptr<SceneObject> objectB_;
        bool collideConnected_ = false;
        bool hasBodyB_ = false;

        Scene* parent_ = nullptr;
    };

    using JointPtr = std::shared_ptr<Joint>;

    using Joints = std::vector<JointPtr>;

    ACLASS_DECLARE(Joint)

    #define JOINT_PARAM(SName, Name, Tooltip, Type, Def) \
        {Name, Tooltip, APropertyType_##Type, APropertyValue(Def), APropertyCategory::Params, APropertyReadable|APropertyEditable, (APropertyGetter)&Joint::propertyParamGet, nullptr},
}

#endif

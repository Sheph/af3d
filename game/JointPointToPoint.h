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

#ifndef _JOINTPOINTTOPOINT_H_
#define _JOINTPOINTTOPOINT_H_

#include "Joint.h"

namespace af3d
{
    class JointPointToPoint : public std::enable_shared_from_this<JointPointToPoint>,
        public Joint
    {
    public:
        explicit JointPointToPoint(const SceneObjectPtr& objectA, const SceneObjectPtr& objectB = SceneObjectPtr(),
            bool collideConnected = false);
        ~JointPointToPoint() = default;

        static const AClass& staticKlass();

        static AObjectPtr create(const APropertyValueMap& propVals);

        AObjectPtr sharedThis() override { return shared_from_this(); }

        btPoint2PointConstraint* constraint() override { return constraint_; }

        inline const btVector3& pivotA() const { return pivotA_; }
        void setPivotA(const btVector3& value);

        inline const btVector3& pivotB() const { return pivotB_; }
        void setPivotB(const btVector3& value);

        btVector3 worldPivotA() const;
        void setWorldPivotA(const btVector3& value);

        btVector3 worldPivotB() const;
        void setWorldPivotB(const btVector3& value);

        APropertyValue propertyLocalPivotAGet(const std::string&) const { return pivotA(); }
        void propertyLocalPivotASet(const std::string&, const APropertyValue& value) { setPivotA(value.toVec3()); }

        APropertyValue propertyLocalPivotBGet(const std::string&) const { return pivotB(); }
        void propertyLocalPivotBSet(const std::string&, const APropertyValue& value) { setPivotB(value.toVec3()); }

        APropertyValue propertyWorldPivotAGet(const std::string&) const { return worldPivotA(); }
        void propertyWorldPivotASet(const std::string&, const APropertyValue& value) { setWorldPivotA(value.toVec3()); }

        APropertyValue propertyWorldPivotBGet(const std::string&) const { return worldPivotB(); }
        void propertyWorldPivotBSet(const std::string&, const APropertyValue& value) { setWorldPivotB(value.toVec3()); }

        APropertyValue propertyWorldPivotATransformGet(const std::string&) const;
        void propertyWorldPivotATransformSet(const std::string&, const APropertyValue& value);

        APropertyValue propertyWorldPivotBTransformGet(const std::string&) const;
        void propertyWorldPivotBTransformSet(const std::string&, const APropertyValue& value);

    private:
        void doRefresh(bool forceDelete) override;

        void doAdopt(bool withEdit) override;

        void doAbandon() override;

        btVector3 pivotA_ = btVector3_zero;
        btVector3 pivotB_ = btVector3_zero;

        btPoint2PointConstraint* constraint_ = nullptr;

        SceneObjectPtr editA_;
        SceneObjectPtr editB_;
    };

    using JointPointToPointPtr = std::shared_ptr<JointPointToPoint>;

    ACLASS_DECLARE(JointPointToPoint)
}

#endif

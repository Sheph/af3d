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

#ifndef _PHYSICSJOINTCOMPONENT_H_
#define _PHYSICSJOINTCOMPONENT_H_

#include "PhysicsComponent.h"
#include "Joint.h"

namespace af3d
{
    class PhysicsJointComponent : public std::enable_shared_from_this<PhysicsJointComponent>,
         public PhysicsComponent
    {
    public:
        explicit PhysicsJointComponent(Joints&& joints);
        ~PhysicsJointComponent() = default;

        static const AClass& staticKlass();

        static AObjectPtr create(const APropertyValueMap& propVals);

        AObjectPtr sharedThis() override { return shared_from_this(); }

        template <class T>
        inline std::shared_ptr<T> joint(const std::string& name) const
        {
            for (const auto& j : joints_) {
                const auto& jt = aobjectCast<T>(j);
                if (jt && (jt->name() == name)) {
                    return jt;
                }
            }
            return std::shared_ptr<T>();
        }

        template <class T>
        std::vector<std::shared_ptr<T>> joints(const std::string& name) const
        {
            std::vector<std::shared_ptr<T>> res;

            for (const auto& j : joints_) {
                const auto& jt = aobjectCast<T>(j);
                if (jt && (jt->name() == name)) {
                    res.push_back(jt);
                }
            }

            return res;
        }

        inline const Joints& joints() const { return joints_; }

        inline Joints script_getJoints() const { return joints_; }

    private:
        void onRegister() override;

        void onUnregister() override;

        Joints joints_;
    };

    using PhysicsJointComponentPtr = std::shared_ptr<PhysicsJointComponent>;

    ACLASS_DECLARE(PhysicsJointComponent)
}

#endif

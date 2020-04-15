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

#ifndef _PHYSICSBODYCOMPONENT_H_
#define _PHYSICSBODYCOMPONENT_H_

#include "PhysicsComponent.h"
#include "CollisionShapeCompound.h"
#include "Logger.h"

namespace af3d
{
    class PhysicsBodyComponent : public std::enable_shared_from_this<PhysicsBodyComponent>,
         public PhysicsComponent
    {
    public:
        PhysicsBodyComponent();
        ~PhysicsBodyComponent();

        static const AClass& staticKlass();

        static AObjectPtr create(const APropertyValueMap& propVals);

        AObjectPtr sharedThis() override { return shared_from_this(); }

        void addShape(const CollisionShapePtr& cs);

        void removeShape(const CollisionShapePtr& cs);

        int numShapes() const;
        CollisionShape* shape(int index);
        const CollisionShape* shape(int index) const;

        CollisionShapes getShapes();

        CollisionShapes getShapes(const std::string& name);

        template <class T>
        std::shared_ptr<T> getShape(const std::string& name)
        {
            for (int i = 0; i < numShapes(); ++i) {
                auto s = aobjectCast<T>(shape(i));
                if (s && (s->name() == name)) {
                    return s->sharedThis();
                }
            }
            return std::shared_ptr<T>();
        }

        void onFreeze() override;

        void onThaw() override;

        APropertyValue propertyChildrenGet(const std::string&) const
        {
            std::vector<APropertyValue> res;
            res.reserve(numShapes());
            for (int i = 0; i < numShapes(); ++i) {
                res.push_back(APropertyValue(
                    CollisionShape::fromShape(compound_->shape()->getChildShape(i))->sharedThis()));
            }
            return res;
        }
        void propertyChildrenSet(const std::string&, const APropertyValue& value)
        {
            {
                auto shapes = getShapes();
                for (const auto& s : shapes) {
                    if ((s->aflags() & AObjectEditable) != 0) {
                        s->removeFromParent();
                    }
                }
            }

            auto arr = value.toArray();
            for (const auto& el : arr) {
                auto obj = el.toObject();
                if (auto shape = aobjectCast<CollisionShape>(obj)) {
                    addShape(shape);
                } else {
                    LOG4CPLUS_ERROR(logger(), "Bad child object \"" << obj->name() << "\", class - \"" << obj->klass().name() << "\"");
                }
            }
        }

        void updateBodyCollision(bool addRemove);

    private:
        void onRegister() override;

        void onUnregister() override;

        CollisionShapeCompoundPtr compound_;
    };

    using PhysicsBodyComponentPtr = std::shared_ptr<PhysicsBodyComponent>;

    ACLASS_DECLARE(PhysicsBodyComponent)
}

#endif

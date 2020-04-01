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

#ifndef _SCENEASSET_H_
#define _SCENEASSET_H_

#include "SceneObject.h"
#include "AObject.h"
#include "Logger.h"

namespace af3d
{
    class SceneAsset : public std::enable_shared_from_this<SceneAsset>,
        public AObject
    {
    public:
        SceneAsset();
        ~SceneAsset() = default;

        static const AClass& staticKlass();

        static AObjectPtr create(const APropertyValueMap& propVals);

        AObjectPtr sharedThis() override { return shared_from_this(); }

        inline const std::vector<SceneObjectPtr> objects() const { return objects_; }

        APropertyValue propertyChildrenGet(const std::string&) const
        {
            std::vector<APropertyValue> res;
            res.reserve(objects_.size());
            for (const auto& obj : objects_) {
                res.emplace_back(obj);
            }
            return res;
        }
        void propertyChildrenSet(const std::string&, const APropertyValue& value)
        {
            objects_.clear();
            auto children = value.toArray();
            objects_.reserve(children.size());
            for (const auto& c : children) {
                auto obj = c.toObject();
                if (auto sObj = aobjectCast<SceneObject>(obj)) {
                    objects_.emplace_back(sObj);
                } else {
                    LOG4CPLUS_ERROR(logger(), "Bad child object \"" << obj->name() << "\", class - \"" << obj->klass().name() << "\"");
                }
            }
        }

    private:
        std::vector<SceneObjectPtr> objects_;
    };

    using SceneAssetPtr = std::shared_ptr<SceneAsset>;

    ACLASS_DECLARE(SceneAsset)
}

#endif

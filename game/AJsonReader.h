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

#ifndef _AJSONREADER_H_
#define _AJSONREADER_H_

#include "AJsonSerializer.h"
#include <boost/optional.hpp>
#include <unordered_set>
#include <list>

namespace af3d
{
    class AJsonReader : boost::noncopyable
    {
    public:
        AJsonReader(AJsonSerializer& serializer, bool editor);
        ~AJsonReader() = default;

        inline AJsonSerializer& serializer() { return serializer_; }

        std::vector<AObjectPtr> read(const Json::Value& jsonValue);

        boost::optional<AObjectPtr> getObject(std::uint32_t id, bool nested);

    private:
        struct DelayedProperty
        {
            DelayedProperty(const AProperty& prop, std::unordered_set<std::uint32_t>& otherDeps)
            : prop(prop)
            {
                deps.swap(otherDeps);
            }

            AProperty prop;
            std::unordered_set<std::uint32_t> deps;
        };

        struct ObjectState
        {
            ObjectState(const Json::Value& jsonValue, const AClass& klass)
            : jsonValue(jsonValue),
              klass(klass) {}

            const Json::Value& jsonValue;
            const AClass& klass;
            boost::optional<AObjectPtr> obj;
            bool reading = false;
            bool referred = false;

            std::unordered_set<std::uint32_t> objectsToNotify;
            std::list<DelayedProperty> delayedProps;
        };

        using ObjectStateMap = std::unordered_map<std::uint32_t, ObjectState>;

        AJsonSerializer& serializer_;
        bool editor_ = false;

        ObjectStateMap objectStateMap_;
    };
}

#endif

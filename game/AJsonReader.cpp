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

#include "AJsonReader.h"
#include "AClassRegistry.h"
#include "Logger.h"

namespace af3d
{
    AJsonReader::AJsonReader(AJsonSerializer& serializer)
    : serializer_(serializer)
    {
    }

    std::vector<AObjectPtr> AJsonReader::read(const Json::Value& jsonValue)
    {
        std::vector<AObjectPtr> res;

        objectStateMap_.clear();

        if (!jsonValue.isArray()) {
            LOG4CPLUS_ERROR(logger(), "Root Json value is not an array");
            return res;
        }

        for (std::uint32_t i = 0; i < jsonValue.size(); ++i) {
            const auto& jv = jsonValue[i];
            if (!jv["id"].isInt()) {
                LOG4CPLUS_ERROR(logger(), "Bad \"id\" field, skipping");
                continue;
            }
            std::uint32_t id = jv["id"].asUInt();

            if (!jv["class"].isString()) {
                LOG4CPLUS_ERROR(logger(), "Bad \"class\" field, skipping");
                continue;
            }
            std::string klassName = jv["class"].asString();

            const AClass* klass = AClassRegistry::instance().classFind(klassName);
            if (!klass) {
                LOG4CPLUS_ERROR(logger(), "Unknown class \"" << klassName << "\", skipping");
                continue;
            }

            objectStateMap_.emplace(id, ObjectState(jv, *klass));
        }

        for (const auto& kv : objectStateMap_) {
            getObject(kv.first);
        }

        return std::vector<AObjectPtr>();
    }

    AObjectPtr AJsonReader::getObject(std::uint32_t id)
    {
        return AObjectPtr();
    }
}

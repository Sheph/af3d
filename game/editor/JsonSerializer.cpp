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

#include "editor/JsonSerializer.h"
#include "Logger.h"

namespace af3d { namespace editor
{
    JsonSerializer::JsonSerializer(const AObjectPtr& parent)
    : parent_(parent)
    {
    }

    Json::Value JsonSerializer::toJsonValue(const AObjectPtr& obj)
    {
        if (!obj) {
            return Json::Value::null;
        }
        AObjectPtr tmp = obj;
        while (tmp && parent_) {
            if (tmp == parent_) {
                return Json::Value::null;
            }
            tmp = tmp->propertyGet(AProperty_Parent).toObject();
        }
        Json::Value v(Json::arrayValue);
        v.append(static_cast<Json::UInt64>(obj->cookie()));
        return v;
    }

    AObjectPtr JsonSerializer::fromJsonValue(const Json::Value& value)
    {
        if (!value.isArray() || (value.size() != 1) || (!value[0].isInt() && !value[0].isUInt())) {
            return AObjectPtr();
        }
        std::uint64_t cookie = value[0].asUInt64();
        auto obj = AObject::getByCookie(cookie);
        if (obj) {
            return obj->sharedThis();
        } else {
            LOG4CPLUS_ERROR(logger(), "Object with cookie " << cookie << " not found");
            return AObjectPtr();
        }
    }
} }

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

#include "MeshImportSettings.h"
#include "Logger.h"
#include "json/json.h"

namespace af3d
{
    const APropertyTypeObject APropertyType_MeshImportSettings{"MeshImportSettings", AClass_MeshImportSettings};

    ACLASS_DEFINE_BEGIN(MeshImportSettings, AObject)
    ACLASS_PROPERTY(MeshImportSettings, Scale, "scale", "Scale", Float, 1.0f, General, APropertyEditable)
    ACLASS_PROPERTY(MeshImportSettings, Root, "root", "Root", String, "", General, APropertyEditable)
    ACLASS_DEFINE_END(MeshImportSettings)

    MeshImportSettings::MeshImportSettings()
    : AObject(AClass_MeshImportSettings)
    {
        aflagsSet(AObjectEditable);
    }

    const AClass& MeshImportSettings::staticKlass()
    {
        return AClass_MeshImportSettings;
    }

    AObjectPtr MeshImportSettings::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<MeshImportSettings>();
        obj->propertiesSet(propVals);
        return obj;
    }

    MeshImportSettingsPtr MeshImportSettings::clone() const
    {
        auto cloned = std::make_shared<MeshImportSettings>();
        cloned->setName(name());
        cloned->scale_ = scale_;
        cloned->root_ = root_;
        return cloned;
    }

    APropertyValue MeshImportSettings::propertyRootGet(const std::string&) const
    {
        return Json::FastWriter().write(toJson(root_));
    }

    void MeshImportSettings::propertyRootSet(const std::string&, const APropertyValue& value)
    {
        auto str = value.toString();
        if (str.empty()) {
            root_ = ObjectEntry();
            return;
        }

        Json::Value jsonValue;
        Json::Reader reader;
        if (!reader.parse(str, jsonValue)) {
            LOG4CPLUS_ERROR(logger(), "Failed to parse JSON: " << reader.getFormattedErrorMessages());
            root_ = ObjectEntry();
            return;
        }
        root_ = fromJson(jsonValue);
    }

    Json::Value MeshImportSettings::toJson(const ObjectEntry& entry)
    {
        Json::Value jsonValue(Json::objectValue);

        jsonValue["name"] = entry.name;
        for (const auto& kv : entry.subObjs) {
            jsonValue["subObjs"][kv.first] = toJson(kv.second);
        }
        for (const auto& kv : entry.meshes) {
            jsonValue["meshes"][kv.first] = kv.second.name;
        }

        return jsonValue;
    }

    MeshImportSettings::ObjectEntry MeshImportSettings::fromJson(const Json::Value& jsonValue)
    {
        ObjectEntry entry;

        entry.name = jsonValue["name"].asString();
        const auto& subObjs = jsonValue["subObjs"];
        for (auto it = subObjs.begin(); it != subObjs.end(); ++it) {
            entry.subObjs[it.key().asString()] = fromJson(*it);
        }
        const auto& meshes = jsonValue["meshes"];
        for (auto it = meshes.begin(); it != meshes.end(); ++it) {
            entry.meshes[it.key().asString()].name = (*it).asString();
        }

        return entry;
    }
}

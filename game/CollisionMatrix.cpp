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

#include "CollisionMatrix.h"
#include "AssetManager.h"
#include "Logger.h"
#include "json/json.h"
#include "log4cplus/ndc.h"

namespace af3d
{
    const APropertyTypeObject APropertyType_CollisionMatrix{"CollisionMatrix", AClass_CollisionMatrix};

    ACLASS_DEFINE_BEGIN(CollisionMatrix, AObject)
    ACLASS_DEFINE_END(CollisionMatrix)

    CollisionMatrix::CollisionMatrix()
    : AObject(AClass_CollisionMatrix)
    {
        aflagsSet(AObjectEditable);
    }

    CollisionMatrixPtr CollisionMatrix::fromStream(const std::string& fileName, std::istream& is)
    {
        std::string json;

        {
            log4cplus::NDCContextCreator ndc(fileName);

            if (!is) {
                LOG4CPLUS_ERROR(logger(), "Cannot open file");
                return CollisionMatrixPtr();
            }

            if (!readStream(is, json)) {
                LOG4CPLUS_ERROR(logger(), "Error reading file");
                return CollisionMatrixPtr();
            }
        }

        return fromString(fileName, json);
    }

    CollisionMatrixPtr CollisionMatrix::fromString(const std::string& fileName, const std::string& json)
    {
        Json::Value jsonValue;
        Json::Reader reader;

        log4cplus::NDCContextCreator ndc(fileName);

        if (!reader.parse(json, jsonValue)) {
            LOG4CPLUS_ERROR(logger(),
                "Failed to parse JSON: " << reader.getFormattedErrorMessages());
            return CollisionMatrixPtr();
        }

        LOG4CPLUS_DEBUG(logger(), "Processing...");

        return fromJsonValue(jsonValue);
    }

    CollisionMatrixPtr CollisionMatrix::fromJsonValue(const Json::Value& jsonValue)
    {
        CollisionMatrixPtr cm = std::make_shared<CollisionMatrix>();

        if (!jsonValue.isArray()) {
            LOG4CPLUS_ERROR(logger(), "Not a json array");
            return CollisionMatrixPtr();
        }

        for (std::uint32_t i = 0; i < jsonValue.size(); ++i) {
            if (i > static_cast<std::uint32_t>(Layer::Max)) {
                LOG4CPLUS_WARN(logger(), "Too high row " << i);
                break;
            }
            if (!jsonValue[i].isArray()) {
                LOG4CPLUS_ERROR(logger(), "Not a json array");
                return CollisionMatrixPtr();
            }
            Layer layer = static_cast<Layer>(i);
            for (std::uint32_t j = 0; j < jsonValue[i].size(); ++j) {
                if (!jsonValue[i][j].isInt() && !jsonValue[i][j].isUInt()) {
                    LOG4CPLUS_ERROR(logger(), "Not an int");
                    return CollisionMatrixPtr();
                }
                int val = jsonValue[i][j].asInt();
                if (val > static_cast<int>(Layer::Max)) {
                    LOG4CPLUS_WARN(logger(), "Too high value " << val);
                    break;
                }
                cm->row(layer).set(static_cast<Layer>(val));
            }
        }

        return cm;
    }

    const AClass& CollisionMatrix::staticKlass()
    {
        return AClass_CollisionMatrix;
    }

    Json::Value CollisionMatrix::toJsonValue() const
    {
        Json::Value rows(Json::arrayValue);
        for (int i = 0; i <= static_cast<int>(Layer::Max); ++i) {
            const auto& layers = row(static_cast<Layer>(i));
            Json::Value values(Json::arrayValue);
            for (int j = 0; j <= static_cast<int>(Layer::Max); ++j) {
                if (layers[static_cast<Layer>(j)]) {
                    values.append(j);
                }
            }
            rows.append(values);
        }
        return rows;
    }

    void CollisionMatrix::save()
    {
        assetManager.saveCollisionMatrix(shared_from_this());
    }

    AObjectPtr CollisionMatrix::create(const APropertyValueMap& propVals)
    {
        return assetManager.getCollisionMatrix(propVals.get("name").toString());
    }
}

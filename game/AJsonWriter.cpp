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

#include "AJsonWriter.h"
#include "Logger.h"

namespace af3d
{
    class AJsonWriteVisitor : public APropertyTypeVisitor
    {
    public:
        AJsonWriteVisitor(AJsonWriter& writer,
            const APropertyValue& value,
            Json::Value& jsonValue)
        : writer_(writer),
          value_(value),
          jsonValue_(jsonValue)
        {
        }

        ~AJsonWriteVisitor() = default;

        void visitBool(const APropertyTypeBool& type) override
        {
            jsonValue_ = value_.toBool();
        }

        void visitInt(const APropertyTypeInt& type) override
        {
            jsonValue_ = value_.toInt();
        }

        void visitFloat(const APropertyTypeFloat& type) override
        {
            jsonValue_ = value_.toFloat();
        }

        void visitString(const APropertyTypeString& type) override
        {
            jsonValue_ = value_.toString();
        }

        void visitVec2f(const APropertyTypeVec2f& type) override
        {
            auto v = value_.toVec2f();
            jsonValue_ = Json::arrayValue;
            jsonValue_.append(v.x());
            jsonValue_.append(v.y());
        }

        void visitVec3f(const APropertyTypeVec3f& type) override
        {
            auto v = value_.toVec3f();
            jsonValue_ = Json::arrayValue;
            jsonValue_.append(v.x());
            jsonValue_.append(v.y());
            jsonValue_.append(v.z());
        }

        void visitVec4f(const APropertyTypeVec4f& type) override
        {
            auto v = value_.toVec4f();
            jsonValue_ = Json::arrayValue;
            jsonValue_.append(v.x());
            jsonValue_.append(v.y());
            jsonValue_.append(v.z());
            jsonValue_.append(v.w());
        }

        void visitColor(const APropertyTypeColor& type) override
        {
            auto v = value_.toColor();
            jsonValue_ = Json::arrayValue;
            jsonValue_.append(v.x());
            jsonValue_.append(v.y());
            jsonValue_.append(v.z());
            jsonValue_.append(v.w());
        }

        void visitEnum(const APropertyTypeEnum& type) override
        {
            jsonValue_ = value_.toInt();
        }

        void visitObject(const APropertyTypeObject& type) override
        {
            auto obj = value_.toObject();
            jsonValue_ = writer_.serializer().toJsonValue(obj);
            if (jsonValue_.isNull()) {
                if (!obj) {
                    jsonValue_ = 0U;
                } else if ((obj->aflags() & AObjectEditable) != 0) {
                    jsonValue_ = writer_.registerObject(obj);
                }
            }
        }

        void visitTransform(const APropertyTypeTransform& type) override
        {
            auto v = value_.toTransform();
            jsonValue_ = Json::arrayValue;
            jsonValue_.append(v.getOrigin().x());
            jsonValue_.append(v.getOrigin().y());
            jsonValue_.append(v.getOrigin().z());
            auto q = v.getRotation();
            jsonValue_.append(q.x());
            jsonValue_.append(q.y());
            jsonValue_.append(q.z());
            jsonValue_.append(q.w());
        }

        void visitQuaternion(const APropertyTypeQuaternion& type) override
        {
            auto v = value_.toQuaternion();
            jsonValue_ = Json::arrayValue;
            jsonValue_.append(v.x());
            jsonValue_.append(v.y());
            jsonValue_.append(v.z());
            jsonValue_.append(v.w());
        }

        void visitArray(const APropertyTypeArray& type) override
        {
            jsonValue_ = Json::arrayValue;
            auto arr = value_.toArray();
            for (const auto& val : arr) {
                Json::Value jsonElValue(Json::nullValue);
                AJsonWriteVisitor visitor(writer_, val, jsonElValue);
                type.type().accept(visitor);
                if (!jsonElValue.isNull()) {
                    jsonValue_.append(jsonElValue);
                }
            }
        }

    private:
        AJsonWriter& writer_;
        const APropertyValue& value_;
        Json::Value& jsonValue_;
    };

    AJsonWriter::AJsonWriter(Json::Value& jsonValue, AJsonSerializer& serializer, bool withCookie)
    : jsonValue_(jsonValue),
      serializer_(serializer),
      withCookie_(withCookie)
    {
        runtime_assert(jsonValue.isArray());
    }

    void AJsonWriter::write(const AObjectPtr& obj)
    {
        registerObject(obj);
        while (!objectsToProcess_.empty()) {
            AObjectPtr nextObj = objectsToProcess_.front();
            objectsToProcess_.pop_front();

            auto& value = jsonValue_.append(Json::objectValue);
            value["id"] = registerObject(nextObj);
            value["class"] = (nextObj->klass().name() == "Scene") ? "SceneAsset" : nextObj->klass().name();
            if (withCookie_) {
                value["cookie"] = static_cast<Json::UInt64>(nextObj->cookie());
            }
            auto props = nextObj->klass().getProperties();
            for (const auto& prop : props) {
                if ((prop.flags() & APropertyTransient) == 0) {
                    Json::Value jsonPropValue(Json::nullValue);
                    auto v = nextObj->propertyGet(prop.name());
                    AJsonWriteVisitor visitor(*this, v, jsonPropValue);
                    prop.type().accept(visitor);
                    if (jsonPropValue.isNull()) {
                        LOG4CPLUS_WARN(logger(), "Object \"" << obj->name() << "\", prop \"" << prop.name() << "\" set to non-editable value");
                    } else {
                        value[prop.name()] = jsonPropValue;
                    }
                }
            }
        }
    }

    std::uint32_t AJsonWriter::registerObject(const AObjectPtr& obj)
    {
        if (!obj) {
            return 0;
        }
        auto it = objectMap_.find(obj);
        if (it != objectMap_.end()) {
            return it->second;
        } else {
            objectsToProcess_.push_back(obj);
            objectMap_.emplace(obj, nextId_);
            return nextId_++;
        }
    }
}

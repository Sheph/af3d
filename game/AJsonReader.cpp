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
#include "SceneObject.h"
#include "Logger.h"
#include "editor/ObjectComponent.h"

namespace af3d
{
    class AJsonReadVisitor : public APropertyTypeVisitor
    {
    public:
        AJsonReadVisitor(AJsonReader& reader,
            const Json::Value& jsonValue,
            std::unordered_set<std::uint32_t>& deps)
        : reader_(reader),
          jsonValue_(jsonValue),
          deps_(deps)
        {
        }

        ~AJsonReadVisitor() = default;

        inline const APropertyValue& value() const { return value_; }

        void visitBool(const APropertyTypeBool& type) override
        {
            if (!jsonValue_.isBool()) {
                LOG4CPLUS_ERROR(logger(), "Bool not a json bool");
                value_ = APropertyValue(false);
                return;
            }
            value_ = APropertyValue(jsonValue_.asBool());
        }

        void visitInt(const APropertyTypeInt& type) override
        {
            if (!jsonValue_.isInt()) {
                LOG4CPLUS_ERROR(logger(), "Int not a json int");
                value_ = APropertyValue(0);
                return;
            }
            value_ = APropertyValue(jsonValue_.asInt());
        }

        void visitFloat(const APropertyTypeFloat& type) override
        {
            if (!jsonValue_.isDouble()) {
                LOG4CPLUS_ERROR(logger(), "Float not a json float");
                value_ = APropertyValue(0.0f);
                return;
            }
            value_ = APropertyValue(jsonValue_.asFloat());
        }

        void visitString(const APropertyTypeString& type) override
        {
            if (!jsonValue_.isString()) {
                LOG4CPLUS_ERROR(logger(), "String not a json string");
                value_ = APropertyValue("");
                return;
            }
            value_ = APropertyValue(jsonValue_.asString());
        }

        void visitVec2f(const APropertyTypeVec2f& type) override
        {
            if (!jsonValue_.isArray() || (jsonValue_.size() != 2)) {
                LOG4CPLUS_ERROR(logger(), "Vec2f not a json array(2)");
                value_ = APropertyValue(Vector2f_zero);
                return;
            }
            if (!jsonValue_[0].isDouble() || !jsonValue_[1].isDouble()) {
                LOG4CPLUS_ERROR(logger(), "Vec2f not a json array(float, float)");
                value_ = APropertyValue(Vector2f_zero);
                return;
            }
            value_ = APropertyValue(Vector2f(jsonValue_[0].asFloat(),
                jsonValue_[1].asFloat()));
        }

        void visitVec3f(const APropertyTypeVec3f& type) override
        {
            if (!jsonValue_.isArray() || (jsonValue_.size() != 3)) {
                LOG4CPLUS_ERROR(logger(), "Vec3f not a json array(3)");
                value_ = APropertyValue(Vector3f_zero);
                return;
            }
            if (!jsonValue_[0].isDouble() || !jsonValue_[1].isDouble() || !jsonValue_[2].isDouble()) {
                LOG4CPLUS_ERROR(logger(), "Vec3f not a json array(float, float, float)");
                value_ = APropertyValue(Vector3f_zero);
                return;
            }
            value_ = APropertyValue(Vector3f(jsonValue_[0].asFloat(),
                jsonValue_[1].asFloat(), jsonValue_[2].asFloat()));
        }

        void visitVec4f(const APropertyTypeVec4f& type) override
        {
            if (!jsonValue_.isArray() || (jsonValue_.size() != 4)) {
                LOG4CPLUS_ERROR(logger(), "Vec4f not a json array(4)");
                value_ = APropertyValue(Vector4f_zero);
                return;
            }
            if (!jsonValue_[0].isDouble() || !jsonValue_[1].isDouble() || !jsonValue_[2].isDouble() || !jsonValue_[3].isDouble()) {
                LOG4CPLUS_ERROR(logger(), "Vec4f not a json array(float, float, float, float)");
                value_ = APropertyValue(Vector4f_zero);
                return;
            }
            value_ = APropertyValue(Vector4f(jsonValue_[0].asFloat(),
                jsonValue_[1].asFloat(), jsonValue_[2].asFloat(), jsonValue_[3].asFloat()));
        }

        void visitColor(const APropertyTypeColor& type) override
        {
            if (!jsonValue_.isArray() || (jsonValue_.size() != 4)) {
                LOG4CPLUS_ERROR(logger(), "Color not a json array(4)");
                value_ = APropertyValue(Color_zero);
                return;
            }
            if (!jsonValue_[0].isDouble() || !jsonValue_[1].isDouble() || !jsonValue_[2].isDouble() || !jsonValue_[3].isDouble()) {
                LOG4CPLUS_ERROR(logger(), "Color not a json array(float, float, float, float)");
                value_ = APropertyValue(Color_zero);
                return;
            }
            value_ = APropertyValue(Color(jsonValue_[0].asFloat(),
                jsonValue_[1].asFloat(), jsonValue_[2].asFloat(), jsonValue_[3].asFloat()));
        }

        void visitEnum(const APropertyTypeEnum& type) override
        {
            if (!jsonValue_.isInt()) {
                LOG4CPLUS_ERROR(logger(), "Enum not a json int");
                value_ = APropertyValue(0);
                return;
            }
            int v = jsonValue_.asInt();
            if (v >= static_cast<int>(type.enumerators().size())) {
                LOG4CPLUS_ERROR(logger(), "Enum int too large");
                value_ = APropertyValue(0);
                return;
            }
            value_ = APropertyValue(v);
        }

        void visitObject(const APropertyTypeObject& type) override
        {
            auto obj = reader_.serializer().fromJsonValue(jsonValue_);
            if (obj) {
                value_ = APropertyValue(obj);
                return;
            }
            if (!jsonValue_.isInt() && !jsonValue_.isUInt()) {
                LOG4CPLUS_ERROR(logger(), "Object id not an int");
                value_ = APropertyValue(AObjectPtr());
                return;
            }
            std::uint32_t id = jsonValue_.asUInt();
            if (id == 0) {
                value_ = APropertyValue(AObjectPtr());
                return;
            }
            auto optObj = reader_.getObject(id, true);
            if (!optObj) {
                // Delayed processing...
                deps_.insert(id);
                return;
            }
            value_ = APropertyValue(*optObj);
        }

        void visitTransform(const APropertyTypeTransform& type) override
        {
            if (!jsonValue_.isArray() || (jsonValue_.size() != 7)) {
                LOG4CPLUS_ERROR(logger(), "Transform not a json array(7)");
                value_ = APropertyValue(btTransform::getIdentity());
                return;
            }
            if (!jsonValue_[0].isDouble() || !jsonValue_[1].isDouble() || !jsonValue_[2].isDouble() || !jsonValue_[3].isDouble() ||
                !jsonValue_[4].isDouble() || !jsonValue_[5].isDouble() || !jsonValue_[6].isDouble()) {
                LOG4CPLUS_ERROR(logger(), "Transform not a json array(float, float, float, float, float, float, float)");
                value_ = APropertyValue(btTransform::getIdentity());
                return;
            }

            value_ = APropertyValue(btTransform(
                btQuaternion(jsonValue_[3].asFloat(), jsonValue_[4].asFloat(), jsonValue_[5].asFloat(), jsonValue_[6].asFloat()),
                btVector3(jsonValue_[0].asFloat(), jsonValue_[1].asFloat(), jsonValue_[2].asFloat())));
        }

        void visitArray(const APropertyTypeArray& type) override
        {
            if (!jsonValue_.isArray()) {
                LOG4CPLUS_ERROR(logger(), "Array not a json array");
                value_ = APropertyValue(std::vector<APropertyValue>());
                return;
            }

            std::vector<APropertyValue> res;
            res.reserve(jsonValue_.size());
            for (std::uint32_t i = 0; i < jsonValue_.size(); ++i) {
                AJsonReadVisitor visitor(reader_, jsonValue_[i], deps_);
                type.type().accept(visitor);
                res.push_back(visitor.value());
            }
            value_ = APropertyValue(res);
        }

    private:
        AJsonReader& reader_;
        const Json::Value& jsonValue_;
        std::unordered_set<std::uint32_t>& deps_;
        APropertyValue value_;
    };

    AJsonReader::AJsonReader(AJsonSerializer& serializer, bool editor, bool withCookie)
    : serializer_(serializer),
      editor_(editor),
      withCookie_(withCookie)
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

        std::uint32_t firstId = 0;

        for (std::uint32_t i = 0; i < jsonValue.size(); ++i) {
            const auto& jv = jsonValue[i];
            if (!jv["id"].isInt() && !jv["id"].isUInt()) {
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

            if (!firstId) {
                firstId = id;
            }

            objectStateMap_.emplace(id, ObjectState(jv, *klass));
        }

        for (const auto& kv : objectStateMap_) {
            getObject(kv.first, false);
        }

        bool allReferred = true;

        for (const auto& kv : objectStateMap_) {
            runtime_assert(kv.second.obj);
            runtime_assert(kv.second.objectsToNotify.empty());
            runtime_assert(kv.second.delayedProps.empty());
            if (!kv.second.referred) {
                allReferred = false;
                if (*kv.second.obj) {
                    res.push_back(*kv.second.obj);
                }
            }
        }

        if (allReferred && firstId) {
            // No top-level objects, probably not a scene asset, just pick
            // first object.
            runtime_assert(res.empty());
            auto it = objectStateMap_.find(firstId);
            runtime_assert(it != objectStateMap_.end());
            if (*it->second.obj) {
                res.push_back(*it->second.obj);
            }
        }

        return res;
    }

    boost::optional<AObjectPtr> AJsonReader::getObject(std::uint32_t id, bool nested)
    {
        auto it = objectStateMap_.find(id);
        if (it == objectStateMap_.end()) {
            LOG4CPLUS_ERROR(logger(), "Bad object id - " << id);
            return AObjectPtr();
        }

        if (nested) {
            it->second.referred = true;
        }

        if (it->second.obj) {
            return *it->second.obj;
        }

        if (it->second.reading) {
            // Read pending, process later...
            return boost::optional<AObjectPtr>();
        }

        it->second.reading = true;

        std::unordered_set<std::uint32_t> deps;

        APropertyValueMap propVals;

        auto props = it->second.klass.getProperties();
        for (const auto& prop : props) {
            if ((prop.flags() & APropertyTransient) != 0) {
                continue;
            }
            const auto& jv = it->second.jsonValue[prop.name()];
            if (!jv.isNull()) {
                AJsonReadVisitor visitor(*this, jv, deps);
                prop.type().accept(visitor);
                if (deps.empty()) {
                    // No dependencies on other objects, insert value.
                    runtime_assert(!visitor.value().empty());
                    propVals.set(prop.name(), visitor.value());
                } else {
                    for (const auto& dep : deps) {
                        auto jt = objectStateMap_.find(dep);
                        runtime_assert(jt != objectStateMap_.end());
                        // When dependent object is done notify us, so
                        // we could set the property.
                        jt->second.objectsToNotify.insert(id);
                    }
                    // Save delayed property, set it later.
                    it->second.delayedProps.emplace_back(prop, deps);
                    // And set default value for now.
                    propVals.set(prop.name(), prop.def());
                }
            } else {
                // No value, insert default.
                propVals.set(prop.name(), prop.def());
            }
        }

        it->second.reading = false;

        it->second.obj = it->second.klass.create(propVals);
        if (*it->second.obj) {
            if (withCookie_) {
                const auto& val = it->second.jsonValue["cookie"];
                if (!val.isInt() && !val.isUInt()) {
                    LOG4CPLUS_ERROR(logger(), "withCookie specified, but no cookie found for object " << id);
                } else {
                    (*it->second.obj)->setCookie(val.asUInt64());
                }
            }
            if (editor_) {
                (*it->second.obj)->aflagsSet(AObjectEditable);
                if (auto sObj = aobjectCast<SceneObject>(*it->second.obj)) {
                    sObj->addComponent(std::make_shared<editor::ObjectComponent>());
                }
            }
        }

        for (auto objId : it->second.objectsToNotify) {
            auto jt = objectStateMap_.find(objId);
            runtime_assert(jt != objectStateMap_.end());
            runtime_assert(jt->second.obj);
            for (auto dpIt = jt->second.delayedProps.begin(); dpIt != jt->second.delayedProps.end();) {
                dpIt->deps.erase(id);
                if (dpIt->deps.empty()) {
                    // This delayed property can now be set!
                    if (*jt->second.obj) {
                        // If an object was constructed, that is.
                        const auto& jv = jt->second.jsonValue[dpIt->prop.name()];
                        runtime_assert(!jv.isNull());
                        AJsonReadVisitor visitor(*this, jv, deps);
                        dpIt->prop.type().accept(visitor);
                        runtime_assert(deps.empty());
                        runtime_assert(!visitor.value().empty());
                        (*jt->second.obj)->propertySet(dpIt->prop.name(), visitor.value());
                    }
                    jt->second.delayedProps.erase(dpIt++);
                } else {
                    ++dpIt;
                }
            }
        }

        it->second.objectsToNotify.clear();

        return *it->second.obj;
    }
}

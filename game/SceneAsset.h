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
#include "Joint.h"
#include "CollisionMatrix.h"
#include "Logger.h"

namespace af3d
{
    class Scene;

    class SceneAsset : public std::enable_shared_from_this<SceneAsset>,
        public AObject
    {
    public:
        SceneAsset();
        ~SceneAsset() = default;

        static const AClass& staticKlass();

        static AObjectPtr create(const APropertyValueMap& propVals);

        AObjectPtr sharedThis() override { return shared_from_this(); }

        inline void setRoot(const SceneObjectPtr& value) { root_ = value; }
        inline const SceneObjectPtr& root() const { return root_; }

        inline const std::string& scriptPath() const { return scriptPath_; }

        inline void setCollisionMatrix(const CollisionMatrixPtr& value) { collisionMatrix_ = value; }
        inline const CollisionMatrixPtr& collisionMatrix() const { return collisionMatrix_; }

        void apply(Scene* scene);

        void apply(const SceneObjectPtr& parent);

        APropertyValue propertyChildrenGet(const std::string&) const
        {
            std::vector<APropertyValue> res;

            res.reserve(objects_.size() + joints_.size());
            for (const auto& obj : objects_) {
                res.emplace_back(obj);
            }
            for (const auto& j : joints_) {
                res.emplace_back(j);
            }

            return res;
        }
        void propertyChildrenSet(const std::string&, const APropertyValue& value)
        {
            objects_.clear();
            joints_.clear();
            auto children = value.toArray();
            objects_.reserve(children.size());
            joints_.reserve(children.size());
            for (const auto& c : children) {
                auto obj = c.toObject();
                if (auto sObj = aobjectCast<SceneObject>(obj)) {
                    objects_.emplace_back(sObj);
                } else if (auto j = aobjectCast<Joint>(obj)) {
                    joints_.emplace_back(j);
                } else {
                    LOG4CPLUS_ERROR(logger(), "Bad child object \"" << obj->name() << "\", class - \"" << obj->klass().name() << "\"");
                }
            }
        }

        APropertyValue propertyGravityGet(const std::string&) const { return gravity_; }
        void propertyGravitySet(const std::string&, const APropertyValue& value) { gravity_ = value.toVec3(); }

        APropertyValue propertyClearColorGet(const std::string&) const { return clearColor_; }
        void propertyClearColorSet(const std::string&, const APropertyValue& value) { clearColor_ = value.toColor(); }

        APropertyValue propertyAmbientColorGet(const std::string&) const { return ambientColor_; }
        void propertyAmbientColorSet(const std::string&, const APropertyValue& value) { ambientColor_ = value.toColor(); }

        APropertyValue propertyScriptGet(const std::string&) const { return scriptPath_; }
        void propertyScriptSet(const std::string&, const APropertyValue& value) { scriptPath_ = value.toString(); }

        APropertyValue propertyRootGet(const std::string&) const { return APropertyValue(root()); }
        void propertyRootSet(const std::string&, const APropertyValue& value) { setRoot(value.toObject<SceneObject>()); }

        APropertyValue propertyCollisionMatrixGet(const std::string&) const { return APropertyValue(collisionMatrix()); }
        void propertyCollisionMatrixSet(const std::string&, const APropertyValue& value) { setCollisionMatrix(value.toObject<CollisionMatrix>()); }

        APropertyValue propertyCameraTransformGet(const std::string&) const { return cameraXf_; }

        APropertyValue propertyUpdateLightProbesGet(const std::string&) const { return false; }
        void propertyUpdateLightProbesSet(const std::string&, const APropertyValue& value) { }

    private:
        SceneObjectPtr root_;
        std::vector<SceneObjectPtr> objects_;
        Joints joints_;

        btVector3 gravity_;
        Color clearColor_ = Color_zero;
        Color ambientColor_ = Color_zero;
        std::string scriptPath_;
        btTransform cameraXf_;
        CollisionMatrixPtr collisionMatrix_;
    };

    using SceneAssetPtr = std::shared_ptr<SceneAsset>;

    ACLASS_DECLARE(SceneAsset)

    #define SCENE_PROPS(Class) \
        ACLASS_PROPERTY(Class, Gravity, "gravity", "World gravity", Vec3f, btVector3(0.0f, -10.0f, 0.0f), Physics, APropertyEditable) \
        ACLASS_PROPERTY(Class, ClearColor, "clear color", "Clear Color", ColorRGB, Color(0.23f, 0.23f, 0.23f, 1.0f), General, APropertyEditable) \
        ACLASS_PROPERTY(Class, AmbientColor, "ambient color", "Ambient Color", ColorRGB, Color(0.2f, 0.2f, 0.2f, 1.0f), General, APropertyEditable) \
        ACLASS_PROPERTY(Class, Script, "script", "Scene script asset path", String, "", General, APropertyEditable) \
        ACLASS_PROPERTY(Class, Root, "root", "Scene root body", SceneObject, SceneObjectPtr(), General, APropertyEditable) \
        ACLASS_PROPERTY(Class, CollisionMatrix, "collision matrix", "Scene collision matrix", CollisionMatrix, CollisionMatrixPtr(), General, APropertyEditable) \
        ACLASS_PROPERTY_RO(Class, CameraTransform, AProperty_CameraTransform, "Camera transform", Transform, Position, APropertyEditable) \
        ACLASS_PROPERTY(Class, UpdateLightProbes, "update light probes", "Update light probes", Bool, false, General, APropertyEditable|APropertyTransient)
}

#endif

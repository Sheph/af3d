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

#include "editor/CommandDup.h"
#include "editor/CommandSelect.h"
#include "editor/JsonSerializer.h"
#include "SceneObject.h"
#include "Scene.h"
#include "PhysicsBodyComponent.h"
#include "Logger.h"
#include "AJsonWriter.h"
#include "AJsonReader.h"

namespace af3d { namespace editor
{
    CommandDup::CommandDup(Scene* scene, const AObjectPtr& obj)
    : Command(scene),
      wobj_(obj)
    {
    }

    bool CommandDup::redo()
    {
        auto obj = wobj_.lock();
        if (!obj) {
            LOG4CPLUS_ERROR(logger(), "redo: Cannot get obj by cookie: " << description());
            return false;
        }

        AObjectPtr dupObj;

        if (dupWobj_.empty()) {
            {
                JsonSerializer ser(obj);
                data_.clear();
                AJsonWriter writer(data_, ser);
                writer.write(obj);
            }

            {
                JsonSerializer ser;
                AJsonReader reader(ser, true);
                auto objs = reader.read(data_);

                if (objs.size() != 1) {
                    LOG4CPLUS_ERROR(logger(), "redo: Read count != 1: " << description());
                    return false;
                }

                dupObj = objs.back();
            }
        } else {
            JsonSerializer ser;
            AJsonReader reader(ser, true, true);
            auto objs = reader.read(data_);

            if (objs.size() != 1) {
                LOG4CPLUS_ERROR(logger(), "redo: Read count != 1: " << description());
                return false;
            }

            dupObj = objs.back();
            if (dupObj->cookie() != dupWobj_.cookie()) {
                LOG4CPLUS_ERROR(logger(), "redo: Object with bad cookie read: " << description());
                return false;
            }
        }

        if (auto sceneObj = aobjectCast<SceneObject>(obj)) {
            if (!sceneObj->parent()) {
                LOG4CPLUS_ERROR(logger(), "redo: Scene object not parented: " << description());
                return false;
            }
            auto dupSObj = aobjectCast<SceneObject>(dupObj);
            if (!dupSObj) {
                LOG4CPLUS_ERROR(logger(), "redo: Duped object not scene object: " << description());
                return false;
            }
            setDescription("Duplicate object");
            sceneObj->parent()->addObject(dupSObj);
        } else if (auto c = aobjectCast<Component>(obj)) {
            if (!c->parent()) {
                LOG4CPLUS_ERROR(logger(), "redo: Component not parented: " << description());
                return false;
            }
            auto dupC = aobjectCast<Component>(dupObj);
            if (!dupC) {
                LOG4CPLUS_ERROR(logger(), "redo: Duped object not a component: " << description());
                return false;
            }
            setDescription("Duplicate component");
            c->parent()->addComponent(dupC);
        } else if (auto shape = aobjectCast<CollisionShape>(obj)) {
            if (!shape->parent()) {
                LOG4CPLUS_ERROR(logger(), "redo: Collision shape not parented: " << description());
                return false;
            }
            auto dupShape = aobjectCast<CollisionShape>(dupObj);
            if (!dupShape) {
                LOG4CPLUS_ERROR(logger(), "redo: Duped object not a collision shape: " << description());
                return false;
            }
            setDescription("Duplicate collision");
            shape->parent()->addShape(dupShape);
        } else if (auto j = aobjectCast<Joint>(obj)) {
            if (!j->parent()) {
                LOG4CPLUS_ERROR(logger(), "redo: Joint not parented: " << description());
                return false;
            }
            auto dupJoint = aobjectCast<Joint>(dupObj);
            if (!dupJoint) {
                LOG4CPLUS_ERROR(logger(), "redo: Duped object not a joint: " << description());
                return false;
            }
            setDescription("Duplicate joint");
            j->parent()->addJoint(dupJoint);
        } else {
            LOG4CPLUS_ERROR(logger(), "redo: Bad object type: " << description());
            return false;
        }

        if (dupWobj_.empty()) {
            JsonSerializer ser(dupObj);
            data_ = Json::Value::null;
            AJsonWriter writer(data_, ser, true);
            writer.write(dupObj);

            const auto& ems = scene()->workspace()->ems();
            for (auto em : ems) {
                if (em->isSelected(obj)) {
                    nested_.push_back(std::make_shared<CommandSelect>(scene(),
                        reinterpret_cast<EditModeImpl*>(em), EditMode::AList{EditMode::Item(dupObj)}));
                }
            }
        }

        auto xfProp = dupObj->propertyGet(AProperty_WorldTransform);
        if (xfProp.type() == APropertyValue::Transform) {
            auto xf = xfProp.toTransform();
            xf.getOrigin() += xf.getBasis() * btVector3_forward * 2.0f;
            dupObj->propertySetOneOf(AProperty_WorldTransformRecursive, AProperty_WorldTransform, xf);
        }

        dupWobj_.reset(dupObj);

        for (const auto& c : nested_) {
            c->redo();
        }

        return true;
    }

    bool CommandDup::undo()
    {
        auto dupObj = dupWobj_.lock();
        if (!dupObj) {
            LOG4CPLUS_ERROR(logger(), "undo: Cannot get dup obj by cookie: " << description());
            return false;
        }

        if (auto sceneObj = aobjectCast<SceneObject>(dupObj)) {
            runtime_assert(sceneObj->parent());
            sceneObj->removeFromParent();
        } else if (auto c = aobjectCast<Component>(dupObj)) {
            runtime_assert(c->parent());
            c->removeFromParent();
        } else if (auto shape = aobjectCast<CollisionShape>(dupObj)) {
            runtime_assert(shape->parent());
            shape->removeFromParent();
        } else if (auto j = aobjectCast<Joint>(dupObj)) {
            runtime_assert(j->parent());
            j->removeFromParent();
        } else {
            LOG4CPLUS_ERROR(logger(), "undo: Bad object type: " << description());
            return false;
        }

        for (const auto& c : nested_) {
            c->undo();
        }

        return true;
    }
} }

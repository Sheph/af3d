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

#include "editor/CommandDelete.h"
#include "editor/CommandSelect.h"
#include "editor/CommandSetProperty.h"
#include "editor/JsonSerializer.h"
#include "SceneObject.h"
#include "Scene.h"
#include "PhysicsBodyComponent.h"
#include "Logger.h"
#include "AJsonWriter.h"
#include "AJsonReader.h"

namespace af3d { namespace editor
{
    CommandDelete::CommandDelete(Scene* scene, const AObjectPtr& obj)
    : Command(scene),
      wobj_(obj)
    {
    }

    bool CommandDelete::redo()
    {
        auto obj = wobj_.lock();
        if (!obj) {
            LOG4CPLUS_ERROR(logger(), "redo: Cannot get obj by cookie: " << description());
            return false;
        }

        std::unordered_set<ACookie> serializedObjs;

        if (auto sceneObj = aobjectCast<SceneObject>(obj)) {
            if (!sceneObj->parent()) {
                LOG4CPLUS_ERROR(logger(), "redo: Scene object not parented: " << description());
                return false;
            }
            setDescription("Delete object");
            preDelete(obj, serializedObjs);
            parentWobj_ = AWeakObject(sceneObj->parent()->sharedThis());
            sceneObj->removeFromParent();
        } else if (auto c = aobjectCast<Component>(obj)) {
            if (!c->parent()) {
                LOG4CPLUS_ERROR(logger(), "redo: Component not parented: " << description());
                return false;
            }
            setDescription("Delete component");
            preDelete(obj, serializedObjs);
            parentWobj_ = AWeakObject(c->parent()->sharedThis());
            c->removeFromParent();
        } else if (auto shape = aobjectCast<CollisionShape>(obj)) {
            if (!shape->parent()) {
                LOG4CPLUS_ERROR(logger(), "redo: Collision shape not parented: " << description());
                return false;
            }
            setDescription("Delete collision");
            preDelete(obj, serializedObjs);
            parentWobj_ = AWeakObject(shape->parent()->sharedThis());
            shape->removeFromParent();
        } else {
            LOG4CPLUS_ERROR(logger(), "redo: Bad object type: " << description());
            return false;
        }

        redoNested(serializedObjs);

        first_ = false;

        return true;
    }

    bool CommandDelete::undo()
    {
        if (data_.isNull()) {
            return true;
        }

        JsonSerializer ser;

        AJsonReader reader(ser, true, true);
        auto objs = reader.read(data_);

        if (objs.size() != 1) {
            LOG4CPLUS_ERROR(logger(), "undo: Read count != 1: " << description());
            return false;
        }

        auto obj = objs.back();

        if (auto sceneObj = aobjectCast<SceneObject>(obj)) {
            runtime_assert(!sceneObj->parent());

            auto parentObj = aweakObjectCast<SceneObjectManager>(parentWobj_);
            if (!parentObj) {
                LOG4CPLUS_ERROR(logger(), "undo: Cannot get parent obj by cookie: " << description());
                return false;
            }

            parentObj->addObject(sceneObj);
        } else if (auto c = aobjectCast<Component>(obj)) {
            runtime_assert(!c->parent());

            auto parentObj = aweakObjectCast<SceneObject>(parentWobj_);
            if (!parentObj) {
                LOG4CPLUS_ERROR(logger(), "undo: Cannot get parent obj by cookie: " << description());
                return false;
            }

            parentObj->addComponent(c);
        } else if (auto shape = aobjectCast<CollisionShape>(obj)) {
            runtime_assert(!shape->parent());

            auto parentPc = aweakObjectCast<PhysicsBodyComponent>(parentWobj_);
            if (!parentPc) {
                LOG4CPLUS_ERROR(logger(), "undo: Cannot get parent physics body component by cookie: " << description());
                return false;
            }

            parentPc->addShape(shape);
        } else {
            LOG4CPLUS_ERROR(logger(), "undo: Bad object type: " << description());
            return false;
        }

        for (const auto& c : nested_) {
            c->undo();
        }

        return true;
    }

    void CommandDelete::preDelete(const AObjectPtr& obj, std::unordered_set<ACookie>& serializedObjs)
    {
        if (!first_) {
            return;
        }

        serializedObjs.insert(obj->cookie());
        JsonSerializer ser(obj, &serializedObjs);

        AJsonWriter writer(data_, ser, true);
        writer.write(obj);

        const auto& ems = scene()->workspace()->ems();
        for (auto em : ems) {
            std::list<AObjectPtr> objs;
            const auto& sel = em->selected();
            bool needSelect = false;
            for (const auto& wobj : sel) {
                if (serializedObjs.count(wobj.cookie()) == 0) {
                    objs.push_back(wobj.lock());
                } else {
                    needSelect = true;
                }
            }
            if (needSelect) {
                //LOG4CPLUS_DEBUG(logger(), "nested: select " << em->name() << " updated");
                nested_.push_back(std::make_shared<CommandSelect>(scene(), reinterpret_cast<EditModeImpl*>(em), objs));
            }
        }
    }

    void CommandDelete::redoNested(const std::unordered_set<ACookie>& serializedObjs)
    {
        if (first_) {
            // 'cut out' all refs to 'serializedObjs'.
            std::unordered_set<ACookie> visitedObjs;
            buildNested(scene(), serializedObjs, visitedObjs);
        }

        for (const auto& c : nested_) {
            c->redo();
        }
    }

    void CommandDelete::buildNested(AObject* obj, const std::unordered_set<ACookie>& serializedObjs,
        std::unordered_set<ACookie>& visitedObjs)
    {
        if (!visitedObjs.insert(obj->cookie()).second) {
            return;
        }

        auto props = obj->klass().getProperties();
        for (const auto& prop : props) {
            auto val = obj->propertyGet(prop.name());
            if (buildNested(val, serializedObjs, visitedObjs)) {
                LOG4CPLUS_DEBUG(logger(), "nested: set " << obj->name() << "|" << prop.name() << " = " << val.toString());
                nested_.push_back(
                    std::make_shared<CommandSetProperty>(scene(), obj->sharedThis(),
                        prop.name(), val, (prop.category() == APropertyCategory::Params)));
            }
        }
    }

    bool CommandDelete::buildNested(APropertyValue& value, const std::unordered_set<ACookie>& serializedObjs,
        std::unordered_set<ACookie>& visitedObjs)
    {
        switch (value.type()) {
        case APropertyValue::Object: {
            if (serializedObjs.count(value.toWeakObject().cookie()) > 0) {
                value = APropertyValue(AObjectPtr());
                return true;
            } else if (auto obj = value.toObject()) {
                buildNested(obj.get(), serializedObjs, visitedObjs);
            }
            break;
        }
        case APropertyValue::WeakObject: {
            if (serializedObjs.count(value.toWeakObject().cookie()) > 0) {
                value = APropertyValue(AWeakObject());
                return true;
            } else if (auto obj = value.toObject()) {
                buildNested(obj.get(), serializedObjs, visitedObjs);
            }
            break;
        }
        case APropertyValue::Array: {
            auto arr = value.toArray();
            bool res = false;
            arr.erase(std::remove_if(arr.begin(), arr.end(), [this, &res, &serializedObjs, &visitedObjs](APropertyValue& v) {
                bool r = buildNested(v, serializedObjs, visitedObjs);
                res |= r;
                return r && (v.type() != APropertyValue::Array);
            }), arr.end());
            if (res) {
                value = APropertyValue(arr);
            }
            return res;
        }
        default:
            break;
        }
        return false;
    }
} }

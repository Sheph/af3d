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

#include "editor/CommandAdd.h"
#include "editor/ObjectComponent.h"
#include "Logger.h"
#include "SceneObject.h"
#include "PhysicsBodyComponent.h"

namespace af3d { namespace editor
{
    CommandAdd::CommandAdd(Scene* scene,
        const AObjectPtr& parent,
        const AClass& klass, const std::string& kind,
        const APropertyValueMap& initVals)
    : Command(scene),
      parentWobj_(parent),
      klass_(klass),
      initVals_(initVals)
    {
        setDescription("Add " + kind);
    }

    bool CommandAdd::redo()
    {
        auto parentObj = parentWobj_.lock();
        if (!parentObj) {
            LOG4CPLUS_ERROR(logger(), "redo: Cannot get parent obj by cookie: " << description());
            return false;
        }

        APropertyValueMap propVals;

        auto props = klass_.getProperties();
        for (const auto& prop : props) {
            if ((prop.flags() & APropertyTransient) == 0) {
                propVals.set(prop.name(), prop.def());
            }
        }
        for (const auto& kv : initVals_.items()) {
            propVals.set(kv.first, kv.second);
        }

        auto aobj = klass_.create(propVals);
        if (!aobj) {
            LOG4CPLUS_ERROR(logger(), "redo: Cannot create class obj: " << description());
            return false;
        }

        if (!wobj_.empty()) {
            aobj->setCookie(wobj_.cookie());
        } else {
            wobj_.reset(aobj);
        }

        aobj->aflagsSet(AObjectEditable);

        if (auto sObj = aobjectCast<SceneObject>(aobj)) {
            auto parentSObj = aobjectCast<SceneObjectManager>(parentObj);
            if (!parentSObj) {
                LOG4CPLUS_ERROR(logger(), "redo: Parent obj not a scene object manager: " << description());
                return false;
            }
            sObj->addComponent(std::make_shared<ObjectComponent>());
            parentSObj->addObject(sObj);
        } else if (auto c = aobjectCast<Component>(aobj)) {
            auto parentSObj = aobjectCast<SceneObject>(parentObj);
            if (!parentSObj) {
                LOG4CPLUS_ERROR(logger(), "redo: Parent obj not a scene object: " << description());
                return false;
            }
            parentSObj->addComponent(c);
        } else if (auto shape = aobjectCast<CollisionShape>(aobj)) {
            auto parentPc = aobjectCast<PhysicsBodyComponent>(parentObj);
            if (!parentPc) {
                LOG4CPLUS_ERROR(logger(), "redo: Parent obj not a physics object component: " << description());
                return false;
            }
            parentPc->addShape(shape);
        } else {
            LOG4CPLUS_ERROR(logger(), "redo: Class obj not supported: " << description());
            return false;
        }

        return true;
    }

    bool CommandAdd::undo()
    {
        auto aobj = wobj_.lock();
        if (!aobj) {
            LOG4CPLUS_ERROR(logger(), "undo: Cannot find obj: " << description());
            return false;
        }

        if (auto sObj = aobjectCast<SceneObject>(aobj)) {
            sObj->removeFromParent();
        } else if (auto c = aobjectCast<Component>(aobj)) {
            c->removeFromParent();
        } else if (auto shape = aobjectCast<CollisionShape>(aobj)) {
            shape->removeFromParent();
        } else {
            LOG4CPLUS_ERROR(logger(), "undo: obj not supported: " << description());
            return false;
        }

        return true;
    }
} }

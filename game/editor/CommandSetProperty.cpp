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

#include "editor/CommandSetProperty.h"
#include "editor/CommandDelete.h"
#include "SceneObject.h"
#include "CollisionShape.h"
#include "Logger.h"

namespace af3d { namespace editor
{
    CommandSetProperty::CommandSetProperty(Scene* scene,
        const AObjectPtr& obj,
        const std::string& propName, const APropertyValue& propValue,
        bool isParam)
    : Command(scene),
      wobj_(obj),
      name_(propName),
      prevValue_(obj->propertyGet(propName)),
      value_(propValue),
      isParam_(isParam)
    {
        setDescription("Set \"" + propName + "\" property");
    }

    bool CommandSetProperty::redo()
    {
        auto obj = wobj_.lock();
        if (!obj) {
            LOG4CPLUS_ERROR(logger(), "redo: Cannot get obj by cookie: " << description());
            return false;
        }

        setValue(obj, value_);

        return true;
    }

    bool CommandSetProperty::undo()
    {
        auto obj = wobj_.lock();
        if (!obj) {
            LOG4CPLUS_ERROR(logger(), "undo: Cannot get obj by cookie: " << description());
            return false;
        }

        setValue(obj, prevValue_);

        return true;
    }

    void CommandSetProperty::setValue(const AObjectPtr& obj, APropertyValue& value)
    {
        value.convertFromWeak();

        if (isParam_) {
            if (auto sObj = aobjectCast<SceneObject>(obj)) {
                auto origPvm = sObj->params();
                auto pvm = origPvm;
                pvm.set(name_, value);
                sObj->setParams(pvm);
                auto cmd = std::make_shared<CommandDelete>(scene(), obj);
                if (cmd->redo()) {
                    cmd->undo();
                } else {
                    sObj->setParams(origPvm);
                }
            } else if (auto shape = aobjectCast<CollisionShape>(obj)) {
                auto origPvm = shape->params();
                auto pvm = origPvm;
                pvm.set(name_, value);
                shape->setParams(pvm);
                auto cmd = std::make_shared<CommandDelete>(scene(), obj);
                if (cmd->redo()) {
                    cmd->undo();
                } else {
                    shape->setParams(origPvm);
                }
            } else {
                LOG4CPLUS_ERROR(logger(), "Unknown param handling object: " << description());
            }
        } else {
            obj->propertySet(name_, value);
        }
    }
} }

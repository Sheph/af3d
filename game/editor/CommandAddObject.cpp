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

#include "editor/CommandAddObject.h"
#include "Logger.h"
#include "SceneObject.h"
#include "Scene.h"

namespace af3d { namespace editor
{
    CommandAddObject::CommandAddObject(Scene* scene,
        const AClass& klass, const std::string& klassName,
        const btTransform& xf)
    : Command(scene, "Add \"" + klassName + "\" object"),
      klass_(klass),
      xf_(xf)
    {
    }

    bool CommandAddObject::redo()
    {
        APropertyValueMap propVals;

        for (const auto& param : klass_.thisProperties()) {
            propVals.set(param.name(), param.def());
        }

        propVals.set(AProperty_WorldTransform, xf_);

        auto obj = klass_.create(propVals);
        if (!obj) {
            LOG4CPLUS_ERROR(logger(), "redo: Cannot create class obj: " << description());
            return false;
        }

        auto sObj = std::dynamic_pointer_cast<SceneObject>(obj);
        if (!sObj) {
            LOG4CPLUS_ERROR(logger(), "redo: Class obj not a scene object: " << description());
            return false;
        }

        if (cookie_) {
            sObj->setCookie(cookie_);
        } else {
            cookie_ = sObj->cookie();
        }

        scene()->addObject(sObj);

        return true;
    }

    bool CommandAddObject::undo()
    {
        auto obj = scene()->getObjectByCookieRecursive(cookie_);
        if (!obj) {
            LOG4CPLUS_ERROR(logger(), "undo: Cannot find obj: " << description());
            return false;
        }

        obj->removeFromParent();

        return true;
    }
} }

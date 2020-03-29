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

#include "editor/CommandAddComponent.h"
#include "Logger.h"
#include "SceneObject.h"

namespace af3d { namespace editor
{
    CommandAddComponent::CommandAddComponent(Scene* scene,
        const AObjectPtr& parent,
        const AClass& klass, const std::string& kind,
        const APropertyValueMap& initVals)
    : Command(scene),
      parentCookie_(parent->cookie()),
      klass_(klass),
      initVals_(initVals)
    {
        setDescription("Add " + kind);
    }

    bool CommandAddComponent::redo()
    {
        auto parentObj = AObject::getByCookie(parentCookie_);
        if (!parentObj) {
            LOG4CPLUS_ERROR(logger(), "redo: Cannot get parent obj by cookie: " << description());
            return false;
        }

        auto parentSObj = aobjectCast<SceneObject>(parentObj);
        if (!parentSObj) {
            LOG4CPLUS_ERROR(logger(), "redo: Parent obj not a scene object: " << description());
            return false;
        }

        APropertyValueMap propVals;

        auto props = klass_.getProperties();
        for (const auto& prop : props) {
            propVals.set(prop.name(), prop.def());
        }
        for (const auto& kv : initVals_.items()) {
            propVals.set(kv.first, kv.second);
        }

        auto aobj = klass_.create(propVals);
        if (!aobj) {
            LOG4CPLUS_ERROR(logger(), "redo: Cannot create class obj: " << description());
            return false;
        }

        auto c = aobjectCast<Component>(aobj);
        if (!c) {
            LOG4CPLUS_ERROR(logger(), "redo: Class obj not a component: " << description());
            return false;
        }

        if (cookie_) {
            c->setCookie(cookie_);
        } else {
            cookie_ = c->cookie();
        }

        c->aflagsSet(AObjectEditable);
        parentSObj->addComponent(c);

        return true;
    }

    bool CommandAddComponent::undo()
    {
        auto aobj = AObject::getByCookie(cookie_);
        if (!aobj) {
            LOG4CPLUS_ERROR(logger(), "undo: Cannot find obj: " << description());
            return false;
        }

        auto c = aobjectCast<Component>(aobj);
        if (!c) {
            LOG4CPLUS_ERROR(logger(), "undo: obj not a component: " << description());
            return false;
        }

        c->removeFromParent();

        return true;
    }
} }

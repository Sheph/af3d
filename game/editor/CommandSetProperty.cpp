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
#include "editor/EditPart.h"
#include "AParameterized.h"
#include "Logger.h"

namespace af3d { namespace editor
{
    CommandSetProperty::CommandSetProperty(Scene* scene,
        const AObjectPtr& obj,
        const std::string& propName, const APropertyValue& propValue)
    : Command(scene),
      wobj_(obj),
      name_(propName),
      prevValue_(obj->propertyGet(propName)),
      value_(propValue),
      isParam_(obj->propertyFind(propName)->category() == APropertyCategory::Params)
    {
        if ((obj->aflags() & AObjectEditable) == 0) {
            if (auto ep = aobjectCast<EditPart>(obj)) {
                const auto& mappedPropName = ep->targetPropertyName(propName);
                if (mappedPropName.empty()) {
                    LOG4CPLUS_ERROR(logger(), "CommandSetProperty for EditPart: cannot map property " << name_);
                } else {
                    wobj_.reset(ep->target());
                    name_ = mappedPropName;
                }
            } else {
                LOG4CPLUS_ERROR(logger(), "CommandSetProperty for non-editable obj " << obj->name() << ", delete undo won't work!");
            }
        }

        setDescription("Set \"" + name_ + "\" property");
    }

    bool CommandSetProperty::redo()
    {
        auto obj = wobj_.lock();
        if (!obj) {
            LOG4CPLUS_ERROR(logger(), "redo: Cannot get obj by cookie: " << description());
            return false;
        }

        value_.refreshObjects();

        if (!cmd_) {
            cmd_ = setValue(obj, value_);
            first_ = false;
            return true;
        } else {
            return cmd_->redo();
        }
    }

    bool CommandSetProperty::undo()
    {
        auto obj = wobj_.lock();
        if (!obj) {
            LOG4CPLUS_ERROR(logger(), "undo: Cannot get obj by cookie: " << description());
            return false;
        }

        prevValue_.refreshObjects();

        if (!cmd_) {
            setValue(obj, prevValue_);
            return true;
        } else {
            return cmd_->undo();
        }
    }

    ACommandPtr CommandSetProperty::setValue(const AObjectPtr& obj, APropertyValue& value)
    {
        if (isParam_) {
            if (first_) {
                // If 'value' contains objects that have param properties that refer to this
                // object then it's an impossible configuration, so we just 'cut out' these
                // objects from 'value'. Leaving them would result in an infinite loop of
                // sets and deletes.
                fixForParam(value);
            }

            if (auto ap = dynamic_cast<AParameterized*>(obj.get())) {
                auto origPvm = ap->params();
                auto pvm = origPvm;
                pvm.set(name_, value);
                ap->setParams(pvm);
                auto cmd = std::make_shared<CommandDelete>(scene(), obj, true);
                if (cmd->redo()) {
                    cmd->undo();
                } else {
                    ap->setParams(origPvm);
                }
            } else {
                LOG4CPLUS_ERROR(logger(), "Unknown param handling object: " << description());
            }

            return ACommandPtr();
        } else {
            return obj->propertySet(name_, value);
        }
    }

    bool CommandSetProperty::fixForParam(APropertyValue& value)
    {
        switch (value.type()) {
        case APropertyValue::Object: {
            std::unordered_set<ACookie> visitedObjs;
            if (reachableViaParams(value.toWeakObject(), wobj_, visitedObjs)) {
                value = APropertyValue(AObjectPtr());
                return true;
            }
            break;
        }
        case APropertyValue::WeakObject: {
            std::unordered_set<ACookie> visitedObjs;
            if (reachableViaParams(value.toWeakObject(), wobj_, visitedObjs)) {
                value = APropertyValue(AWeakObject());
                return true;
            }
            break;
        }
        case APropertyValue::Array: {
            auto arr = value.toArray();
            bool res = false;
            arr.erase(std::remove_if(arr.begin(), arr.end(), [this, &res](APropertyValue& v) {
                bool r = fixForParam(v);
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

    bool CommandSetProperty::reachableViaParams(const AWeakObject& from, const AWeakObject& to,
        std::unordered_set<ACookie>& visitedObjs)
    {
        if (!visitedObjs.insert(from.cookie()).second) {
            return false;
        }

        AObjectPtr fromObj = from.lock();
        AObjectPtr toObj = to.lock();

        if (!fromObj || !toObj) {
            return false;
        }

        auto props = fromObj->klass().getProperties();
        for (const auto& prop : props) {
            if (prop.category() == APropertyCategory::Params) {
                if (reachableViaParams(fromObj->propertyGet(prop.name()), to, visitedObjs)) {
                    return true;
                }
            }
        }

        return false;
    }

    bool CommandSetProperty::reachableViaParams(const APropertyValue& value, const AWeakObject& to,
        std::unordered_set<ACookie>& visitedObjs)
    {
        switch (value.type()) {
        case APropertyValue::Object:
        case APropertyValue::WeakObject: {
            auto w = value.toWeakObject();
            if ((w == to) || reachableViaParams(w, to, visitedObjs)) {
                return true;
            }
            break;
        }
        case APropertyValue::Array: {
            auto arr = value.toArray();
            for (const auto& el : arr) {
                if (reachableViaParams(el, to, visitedObjs)) {
                    return true;
                }
            }
            return false;
        }
        default:
            break;
        }
        return false;
    }
} }

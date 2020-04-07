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
#include "editor/JsonSerializer.h"
#include "SceneObject.h"
#include "Scene.h"
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

        if (auto sceneObj = aobjectCast<SceneObject>(obj)) {
            if (!sceneObj->parent()) {
                LOG4CPLUS_ERROR(logger(), "redo: Scene object not parented: " << description());
                return false;
            }
            setDescription("Delete object");
            preDelete(obj);
            parentWobj_ = AWeakObject(sceneObj->parent()->sharedThis());
            sceneObj->removeFromParent();
        } else if (auto c = aobjectCast<Component>(obj)) {
            if (!c->parent()) {
                LOG4CPLUS_ERROR(logger(), "redo: Component not parented: " << description());
                return false;
            }
            setDescription("Delete component");
            preDelete(obj);
            parentWobj_ = AWeakObject(c->parent()->sharedThis());
            c->removeFromParent();
        }

        redoNested();

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
        } else {
            LOG4CPLUS_ERROR(logger(), "undo: Bad object type: " << description());
            return false;
        }

        for (const auto& c : nested_) {
            c->undo();
        }

        return true;
    }

    void CommandDelete::preDelete(const AObjectPtr& obj)
    {
        if (!first_) {
            return;
        }

        JsonSerializer ser(obj);

        AJsonWriter writer(data_, ser, true);
        writer.write(obj);

        const auto& ems = scene()->workspace()->ems();
        for (auto em : ems) {
            std::list<AObjectPtr> objs;
            const auto& sel = em->selected();
            bool needSelect = false;
            for (const auto& wobj : sel) {
                auto sObj = wobj.lock();
                if (sObj != obj) {
                    objs.push_back(sObj);
                } else {
                    needSelect = true;
                }
            }
            if (needSelect) {
                nested_.push_back(std::make_shared<CommandSelect>(scene(), reinterpret_cast<EditModeImpl*>(em), objs));
            }
        }
    }

    void CommandDelete::redoNested()
    {
        if (first_) {
            // TODO: fill nested_ with CommandSetProperty that set/unset dependent
            // props.
        }

        for (const auto& c : nested_) {
            c->redo();
        }
    }
} }

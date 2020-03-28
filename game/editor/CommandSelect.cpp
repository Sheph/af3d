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

#include "editor/CommandSelect.h"
#include "editor/EditModeImpl.h"
#include "Logger.h"

namespace af3d { namespace editor
{
    CommandSelect::CommandSelect(Scene* scene, EditModeImpl* em,
        const std::list<AObjectPtr>& objs)
    : Command(scene),
      em_(em)
    {
        setDescription("Select/deselect " + em->name());

        const auto& prev = em->selected();
        for (const auto& obj : prev) {
            prevCookies_.push_back(obj->cookie());
        }
        for (const auto& obj : objs) {
            cookies_.push_back(obj->cookie());
        }
    }

    bool CommandSelect::redo()
    {
        EditMode::AList objs;

        for (auto cookie : cookies_) {
            auto obj = AObject::getByCookie(cookie);
            if (!obj) {
                LOG4CPLUS_ERROR(logger(), "redo: Cannot get obj by cookie: " << description());
                return false;
            }
            objs.push_back(obj->sharedThis());
        }

        em_->setSelected(std::move(objs));

        return true;
    }

    bool CommandSelect::undo()
    {
        EditMode::AList objs;

        for (auto cookie : prevCookies_) {
            auto obj = AObject::getByCookie(cookie);
            if (!obj) {
                LOG4CPLUS_ERROR(logger(), "undo: Cannot get obj by cookie: " << description());
                return false;
            }
            objs.push_back(obj->sharedThis());
        }

        em_->setSelected(std::move(objs));

        return true;
    }
} }

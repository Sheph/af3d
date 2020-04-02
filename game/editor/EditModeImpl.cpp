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

#include "editor/EditModeImpl.h"
#include "editor/Workspace.h"
#include "editor/CommandSelect.h"

namespace af3d { namespace editor
{
    EditModeImpl::EditModeImpl(Workspace* workspace, const std::string& name)
    : workspace_(workspace),
      name_(name)
    {
    }

    bool EditModeImpl::active() const
    {
        return active_;
    }

    void EditModeImpl::activate()
    {
        workspace_->setEditMode(this);
    }

    const EditMode::AWeakList& EditModeImpl::hovered() const
    {
        for (auto it = hovered_.begin(); it != hovered_.end();) {
            auto obj = it->lock();
            if (obj && isAlive(obj)) {
                ++it;
            } else {
                hovered_.erase(it++);
            }
        }
        return hovered_;
    }

    const EditMode::AWeakList& EditModeImpl::selected() const
    {
        for (auto it = selected_.begin(); it != selected_.end();) {
            auto obj = it->lock();
            if (obj && isAlive(obj)) {
                ++it;
            } else {
                selected_.erase(it++);
            }
        }
        return selected_;
    }

    bool EditModeImpl::isHovered(const AObjectPtr& obj) const
    {
        for (const auto& wh : hovered_) {
            auto h = wh.lock();
            if (h == obj) {
                return isAlive(h);
            }
        }
        return false;
    }

    bool EditModeImpl::isSelected(const AObjectPtr& obj) const
    {
        for (const auto& ws : selected_) {
            auto s = ws.lock();
            if (s == obj) {
                return isAlive(s);
            }
        }
        return false;
    }

    void EditModeImpl::select(std::list<AObjectPtr>&& objs)
    {
        workspace_->cmdHistory().add(
            std::make_shared<CommandSelect>(workspace_->scene(), this, std::move(objs)));
    }

    void EditModeImpl::enter()
    {
        active_ = true;
    }

    void EditModeImpl::leave()
    {
        hovered_.clear();
        active_ = false;
    }

    void EditModeImpl::setHovered(const AWeakList& wobjs)
    {
        hovered_ = wobjs;
    }

    void EditModeImpl::setSelected(const AWeakList& wobjs)
    {
        selected_.clear();
        for (const auto& wobj : wobjs) {
            auto obj = wobj.lock();
            if (obj && isValid(obj) && isAlive(obj)) {
                selected_.push_back(wobj);
            }
        }
    }

    Scene* EditModeImpl::scene()
    {
        return workspace_->scene();
    }

    const Scene* EditModeImpl::scene() const
    {
        return workspace_->scene();
    }
} }

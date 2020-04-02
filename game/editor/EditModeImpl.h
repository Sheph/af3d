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

#ifndef _EDITOR_EDITMODE_IMPL_H_
#define _EDITOR_EDITMODE_IMPL_H_

#include "editor/EditMode.h"

namespace af3d {
    class Scene;

namespace editor
{
    class Workspace;

    class EditModeImpl : public virtual EditMode
    {
    public:
        EditModeImpl(Workspace* workspace, const std::string& name);
        ~EditModeImpl() = default;

        const std::string& name() const override { return name_; }

        bool active() const override;

        void activate() override;

        const AWeakList& hovered() const override;

        const AWeakList& selected() const override;

        bool isHovered(const AObjectPtr& obj) const override;

        bool isSelected(const AObjectPtr& obj) const override;

        void select(std::list<AObjectPtr>&& objs) override;

        void enter();

        void leave();

        void setHovered(const AWeakList& wobjs);

        void setSelected(const AWeakList& wobjs);

    protected:
        inline Workspace& workspace() { return *workspace_; }
        inline const Workspace& workspace() const { return *workspace_; }
        Scene* scene();
        const Scene* scene() const;

    private:
        Workspace* workspace_;
        std::string name_;

        bool active_ = false;
        mutable AWeakList hovered_;
        mutable AWeakList selected_;
    };
} }

#endif

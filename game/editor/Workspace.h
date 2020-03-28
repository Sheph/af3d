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

#ifndef _EDITOR_WORKSPACE_H_
#define _EDITOR_WORKSPACE_H_

#include "UIComponent.h"
#include "editor/CommandHistory.h"
#include "editor/EditModeObjectImpl.h"

namespace af3d { namespace editor
{
    class Workspace : public std::enable_shared_from_this<Workspace>,
        public UIComponent
    {
    public:
        Workspace();
        ~Workspace() = default;

        static const AClass& staticKlass();

        static AObjectPtr create(const APropertyValueMap& propVals);

        AObjectPtr sharedThis() override { return shared_from_this(); }

        void update(float dt) override;

        void render(RenderList& rl) override;

        inline const std::vector<std::string>& objectKinds() const { return objectKinds_; }

        inline const EditModeObject* emObject() { return emObject_.get(); }

        inline const EditMode* em() { return em_; }

        inline CommandHistory& cmdHistory() { return cmdHistory_; }

        void openMainPopup();

        void addObject(const std::string& kind);

    private:
        void onRegister() override;

        void onUnregister() override;

        std::vector<std::string> objectKinds_;

        std::unique_ptr<EditModeObjectImpl> emObject_;

        EditModeImpl* em_;

        CommandHistory cmdHistory_;
    };

    using WorkspacePtr = std::shared_ptr<Workspace>;
}
    ACLASS_NS_DECLARE(editor, Workspace)
}

#endif

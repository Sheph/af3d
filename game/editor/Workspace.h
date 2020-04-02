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

#include "editor/CommandHistory.h"
#include "editor/EditModeObjectImpl.h"
#include "editor/EditModeVisualImpl.h"
#include "UIComponent.h"
#include "Action.h"

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

        inline EditModeObject* emObject() { return emObject_.get(); }
        inline EditModeVisual* emVisual() { return emVisual_.get(); }

        inline EditMode* em() { return em_; }

        inline const std::vector<EditMode*>& ems() { return ems_; }

        inline CommandHistory& cmdHistory() { return cmdHistory_; }

        inline Action& actionSceneNew() { return actionSceneNew_; }
        inline Action& actionSceneOpen() { return actionSceneOpen_; }
        inline Action& actionSceneSave() { return actionSceneSave_; }
        inline Action& actionModeScene() { return actionModeScene_; }
        inline Action& actionModeObject() { return actionModeObject_; }
        inline Action& actionModeVisual() { return actionModeVisual_; }
        inline Action& actionModeLight() { return actionModeLight_; }
        inline Action& actionUndo() { return actionUndo_; }
        inline Action& actionRedo() { return actionRedo_; }

        inline Action& actionOpMenu() { return actionOpMenu_; }
        inline Action& actionOpMenuAdd() { return actionOpMenuAdd_; }
        inline Action& actionOpMenuAddObject() { return actionOpMenuAddObject_; }
        inline Action& actionOpMenuAddMesh() { return actionOpMenuAddMesh_; }

        inline Action& actionMainPopup() { return actionMainPopup_; }
        inline Action& actionCommandHistory() { return actionCommandHistory_; }
        inline Action& actionPropertyEditor() { return actionPropertyEditor_; }

        void addObject(const std::string& kind);

        void setProperty(const AObjectPtr& obj,
            const std::string& name, const APropertyValue& value, bool isParam);

        void setEditMode(EditModeImpl* value);

        void deleteObject(const AObjectPtr& obj);

    private:
        void onRegister() override;

        void onUnregister() override;

        void setupActions();

        float mainMenu();

        void mainMenuContents();

        void mainToolbar(float offsetY);

        void mainToolbarContents();

        static void toolbarButton(Action& action);

        static void toolbarSep();

        std::vector<std::string> objectKinds_;

        std::unique_ptr<EditModeObjectImpl> emObject_;
        std::unique_ptr<EditModeVisualImpl> emVisual_;

        std::vector<EditMode*> ems_;

        EditModeImpl* em_;

        CommandHistory cmdHistory_;

        Action actionSceneNew_;
        Action actionSceneOpen_;
        Action actionSceneSave_;
        Action actionModeScene_;
        Action actionModeObject_;
        Action actionModeVisual_;
        Action actionModeLight_;
        Action actionUndo_;
        Action actionRedo_;

        Action actionOpMenu_;
        Action actionOpMenuAdd_;
        Action actionOpMenuAddObject_;
        Action actionOpMenuAddMesh_;

        Action actionMainPopup_;
        Action actionCommandHistory_;
        Action actionPropertyEditor_;
    };

    using WorkspacePtr = std::shared_ptr<Workspace>;
}
    ACLASS_NS_DECLARE(editor, Workspace)
}

#endif

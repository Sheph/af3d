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
#include "editor/EditModeLightImpl.h"
#include "editor/EditModeSceneImpl.h"
#include "editor/EditModeCollisionImpl.h"
#include "editor/EditModeJointImpl.h"
#include "editor/ToolSelect.h"
#include "editor/ToolMove.h"
#include "editor/ToolRotate.h"
#include "editor/ToolScale.h"
#include "UIComponent.h"
#include "Action.h"
#include "SceneObjectManager.h"

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

        inline bool locked() const { return locked_; }

        inline bool lock()
        {
            if (!locked_) {
                locked_ = true;
                return true;
            } else {
                return false;
            }
        }

        inline void unlock()
        {
            runtime_assert(locked_);
            locked_ = false;
        }

        inline const std::vector<std::string>& objectKinds() const { return objectKinds_; }

        inline EditModeObject* emObject() { return emObject_.get(); }
        inline EditModeVisual* emVisual() { return emVisual_.get(); }
        inline EditModeLight* emLight() { return emLight_.get(); }
        inline EditModeCollision* emCollision() { return emCollision_.get(); }
        inline EditModeJoint* emJoint() { return emJoint_.get(); }
        inline EditModeScene* emScene() { return emScene_.get(); }

        inline EditMode* em() { return em_; }
        inline EditMode* overriddenEm() { return overriddenEm_ ? overriddenEm_ : em_; }

        inline const std::vector<EditMode*>& ems() { return ems_; }

        inline CommandHistory& cmdHistory() { return cmdHistory_; }

        inline Action& actionSceneNew() { return actionSceneNew_; }
        inline Action& actionSceneOpen() { return actionSceneOpen_; }
        inline Action& actionSceneSave() { return actionSceneSave_; }
        inline Action& actionModeScene() { return actionModeScene_; }
        inline Action& actionModeObject() { return actionModeObject_; }
        inline Action& actionModeVisual() { return actionModeVisual_; }
        inline Action& actionModeLight() { return actionModeLight_; }
        inline Action& actionModeCollision() { return actionModeCollision_; }
        inline Action& actionModeJoint() { return actionModeJoint_; }
        inline Action& actionUndo() { return actionUndo_; }
        inline Action& actionRedo() { return actionRedo_; }
        inline Action& actionDelete() { return actionDelete_; }
        inline Action& actionDup() { return actionDup_; }
        inline Action& actionPlay() { return actionPlay_; }

        inline Action& actionOpMenu() { return actionOpMenu_; }
        inline Action& actionOpMenuAdd() { return actionOpMenuAdd_; }
        inline Action& actionOpMenuAddObject() { return actionOpMenuAddObject_; }
        inline Action& actionOpMenuAddMesh() { return actionOpMenuAddMesh_; }
        inline Action& actionOpMenuAddLight() { return actionOpMenuAddLight_; }
        inline Action& actionOpMenuAddLightDirectional() { return actionOpMenuAddLightDirectional_; }
        inline Action& actionOpMenuAddLightPoint() { return actionOpMenuAddLightPoint_; }
        inline Action& actionOpMenuAddCollision() { return actionOpMenuAddCollision_; }
        inline Action& actionOpMenuAddCollisionBox() { return actionOpMenuAddCollisionBox_; }
        inline Action& actionOpMenuAddCollisionCapsule() { return actionOpMenuAddCollisionCapsule_; }
        inline Action& actionOpMenuAddCollisionSphere() { return actionOpMenuAddCollisionSphere_; }
        inline Action& actionOpMenuAddCollisionCylinder() { return actionOpMenuAddCollisionCylinder_; }
        inline Action& actionOpMenuAddCollisionCone() { return actionOpMenuAddCollisionCone_; }
        inline Action& actionOpMenuAddCollisionPlane() { return actionOpMenuAddCollisionPlane_; }
        inline Action& actionOpMenuAddCollisionStaticMesh() { return actionOpMenuAddCollisionStaticMesh_; }
        inline Action& actionOpMenuAddCollisionConvexMesh() { return actionOpMenuAddCollisionConvexMesh_; }
        inline Action& actionOpMenuAddJoint() { return actionOpMenuAddJoint_; }
        inline Action& actionOpMenuAddJointP2P() { return actionOpMenuAddJointP2P_; }
        inline Action& actionOpMenuAddPhysicsBody() { return actionOpMenuAddPhysicsBody_; }
        inline Action& actionOpMenuRemove() { return actionOpMenuRemove_; }
        inline Action& actionOpMenuRemovePhysicsBody() { return actionOpMenuRemovePhysicsBody_; }

        inline Action& actionMainPopup() { return actionMainPopup_; }
        inline Action& actionCommandHistory() { return actionCommandHistory_; }
        inline Action& actionPropertyEditor() { return actionPropertyEditor_; }
        inline Action& actionToolbox() { return actionToolbox_; }

        inline bool toolsActive() const { return toolsActive_; }
        void setToolsActive(bool value);

        inline Tool* currentTool() { return currentTool_; }
        inline void setCurrentTool(Tool* value) { runtime_assert(value); currentTool_ = value; }

        inline const std::vector<Tool*>& tools() { return tools_; }

        void addObject(const std::string& kind);

        void setProperty(const AObjectPtr& obj,
            const std::string& name, const APropertyValue& value);

        void setEditMode(EditModeImpl* value);

        void setOverrideEditMode(EditMode* value);

        void deleteObject(const AObjectPtr& obj);

        void duplicateObject(const AObjectPtr& obj);

    private:
        void onRegister() override;

        void onUnregister() override;

        void setupActions();

        float mainMenu();

        void mainMenuContents();

        void mainToolbar(float offsetY);

        void mainToolbarContents();

        void saveAs(const std::string& path);

        bool objectWithPhysicsBodySelected() const;

        static void toolbarButton(Action& action);

        static void toolbarSep();

        std::vector<std::string> objectKinds_;

        std::unique_ptr<EditModeObjectImpl> emObject_;
        std::unique_ptr<EditModeVisualImpl> emVisual_;
        std::unique_ptr<EditModeLightImpl> emLight_;
        std::unique_ptr<EditModeCollisionImpl> emCollision_;
        std::unique_ptr<EditModeJointImpl> emJoint_;
        std::unique_ptr<EditModeSceneImpl> emScene_;

        std::vector<EditMode*> ems_;

        EditModeImpl* em_;
        EditModeImpl* overriddenEm_ = nullptr;

        CommandHistory cmdHistory_;

        Action actionSceneNew_;
        Action actionSceneOpen_;
        Action actionSceneSave_;
        Action actionModeScene_;
        Action actionModeObject_;
        Action actionModeVisual_;
        Action actionModeLight_;
        Action actionModeCollision_;
        Action actionModeJoint_;
        Action actionUndo_;
        Action actionRedo_;
        Action actionDelete_;
        Action actionDup_;
        Action actionPlay_;

        Action actionOpMenu_;
        Action actionOpMenuAdd_;
        Action actionOpMenuAddObject_;
        Action actionOpMenuAddMesh_;
        Action actionOpMenuAddLight_;
        Action actionOpMenuAddLightDirectional_;
        Action actionOpMenuAddLightPoint_;
        Action actionOpMenuAddCollision_;
        Action actionOpMenuAddCollisionBox_;
        Action actionOpMenuAddCollisionCapsule_;
        Action actionOpMenuAddCollisionSphere_;
        Action actionOpMenuAddCollisionCylinder_;
        Action actionOpMenuAddCollisionCone_;
        Action actionOpMenuAddCollisionPlane_;
        Action actionOpMenuAddCollisionStaticMesh_;
        Action actionOpMenuAddCollisionConvexMesh_;
        Action actionOpMenuAddJoint_;
        Action actionOpMenuAddJointP2P_;
        Action actionOpMenuAddPhysicsBody_;
        Action actionOpMenuRemove_;
        Action actionOpMenuRemovePhysicsBody_;

        Action actionMainPopup_;
        Action actionCommandHistory_;
        Action actionPropertyEditor_;
        Action actionToolbox_;

        std::vector<Action*> actions_;

        std::unique_ptr<ToolSelect> toolSelect_;
        std::unique_ptr<ToolMove> toolMove_;
        std::unique_ptr<ToolRotate> toolRotate_;
        std::unique_ptr<ToolScale> toolScale_;

        std::vector<Tool*> tools_;

        bool toolsActive_ = true;
        Tool* currentTool_;

        SceneObjectPtr grid_;

        bool locked_ = false;
        bool needNewSceneDlg_ = false;
        bool needOpenSceneDlg_ = false;
    };

    using WorkspacePtr = std::shared_ptr<Workspace>;
}
    ACLASS_NS_DECLARE(editor, Workspace)
}

#endif

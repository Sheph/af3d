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

#include "editor/JointComponent.h"
#include "Scene.h"
#include "SceneObject.h"
#include "AssetManager.h"
#include "Settings.h"

namespace af3d {
    ACLASS_NS_DEFINE_BEGIN(editor, JointComponent, PhasedComponent)
    ACLASS_NS_DEFINE_END(editor, JointComponent)

namespace editor {
    JointComponent::JointComponent(const JointPtr& joint)
    : PhasedComponent(AClass_editorJointComponent, phasePreRender),
      joint_(joint)
    {
    }

    const AClass& JointComponent::staticKlass()
    {
        return AClass_editorJointComponent;
    }

    AObjectPtr JointComponent::create(const APropertyValueMap& propVals)
    {
        return AObjectPtr();
    }

    void JointComponent::preRender(float dt)
    {
        parent()->setPos(joint_->pos());

        auto emJoint = scene()->workspace()->emJoint();

        bool showMarker = emJoint->active();

        if (!showMarker) {
            showMarker = true;
        }

        markerRc_->setVisible(showMarker);

        if (showMarker) {
            if (emJoint->active()) {
                if (emJoint->isSelected(joint_)) {
                    markerRc_->setColor(settings.editor.objMarkerColorSelected);
                } else if (emJoint->isHovered(joint_)) {
                    markerRc_->setColor(settings.editor.objMarkerColorHovered);
                } else {
                    markerRc_->setColor(settings.editor.objMarkerColorInactive);
                }
            } else {
                markerRc_->setColor(Color(0.6f, 0.6f, 0.6f, 0.35f));
            }
        }
    }

    void JointComponent::onRegister()
    {
        markerRc_ = std::make_shared<RenderQuadComponent>();
        markerRc_->setDrawable(assetManager.getDrawable("common1/mode_joint.png"));
        markerRc_->setDepthTest(false);
        markerRc_->setHeight(settings.editor.objMarkerSizeWorld);
        markerRc_->setViewportHeight((float)settings.editor.objMarkerSizePixels / settings.viewHeight);
        markerRc_->setColor(settings.editor.objMarkerColorInactive);
        markerRc_->aflagsSet(AObjectMarkerJoint);
        markerRc_->setVisible(false);
        parent()->addComponent(markerRc_);
    }

    void JointComponent::onUnregister()
    {
        markerRc_->removeFromParent();
        markerRc_.reset();
    }
} }

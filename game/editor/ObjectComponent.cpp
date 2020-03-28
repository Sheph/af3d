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

#include "editor/ObjectComponent.h"
#include "Scene.h"
#include "SceneObject.h"
#include "ImageManager.h"
#include "Settings.h"

namespace af3d {
    ACLASS_NS_DEFINE_BEGIN(editor, ObjectComponent, PhasedComponent)
    ACLASS_NS_DEFINE_END(editor, ObjectComponent)

namespace editor {
    ObjectComponent::ObjectComponent()
    : PhasedComponent(AClass_editorObjectComponent, phasePreRender)
    {
    }

    const AClass& ObjectComponent::staticKlass()
    {
        return AClass_editorObjectComponent;
    }

    AObjectPtr ObjectComponent::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<ObjectComponent>();
        obj->propertiesSet(propVals);
        return obj;
    }

    void ObjectComponent::preRender(float dt)
    {
        auto emObject = scene()->workspace()->emObject();

        markerRc_->setVisible(emObject->active());

        if (emObject->active()) {
            if (emObject->isSelected(parent()->shared_from_this())) {
                markerRc_->setColor(settings.editor.objMarkerColorSelected);
            } else if (emObject->isHovered(parent()->shared_from_this())) {
                markerRc_->setColor(settings.editor.objMarkerColorHovered);
            } else {
                markerRc_->setColor(settings.editor.objMarkerColorInactive);
            }
        }
    }

    void ObjectComponent::onRegister()
    {
        markerRc_ = std::make_shared<RenderQuadComponent>();
        markerRc_->setDrawable(imageManager.getDrawable("common1/marker1.png"));
        markerRc_->setDepthTest(false);
        markerRc_->setHeight(settings.editor.objMarkerSizeWorld);
        markerRc_->setViewportHeight((float)settings.editor.objMarkerSizePixels / settings.viewHeight);
        markerRc_->setColor(settings.editor.objMarkerColorInactive);
        markerRc_->aflagsSet(AObjectMarkerObject);
        parent()->addComponent(markerRc_);
    }

    void ObjectComponent::onUnregister()
    {
        markerRc_->removeFromParent();
    }
} }

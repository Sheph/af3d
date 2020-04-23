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

#include "editor/EditPartTransform.h"
#include "af3d/Utils.h"

namespace af3d {
    ACLASS_NS_DEFINE_BEGIN(editor, EditPartTransform, editorEditPart)
    ACLASS_PROPERTY(editor::EditPartTransform, Transform, AProperty_WorldTransform, "World transform", Transform, btTransform::getIdentity(), Position, APropertyTransient)
    ACLASS_NS_DEFINE_END(editor, EditPartTransform)

namespace editor {
    EditPartTransform::EditPartTransform(const AObjectPtr& target, bool isDefault, const std::string& propName,
        const std::function<void()>& updateFn)
    : EditPart(AClass_editorEditPartTransform, target, isDefault),
      propName_(propName),
      updateFn_(updateFn)
    {
    }

    const AClass& EditPartTransform::staticKlass()
    {
        return AClass_editorEditPartTransform;
    }

    const std::string& EditPartTransform::targetPropertyName(const std::string& propName) const
    {
        if (propName == AProperty_WorldTransform) {
            return propName_;
        } else {
            return string_empty;
        }
    }

    AObjectPtr EditPartTransform::create(const APropertyValueMap& propVals)
    {
        return AObjectPtr();
    }

    void EditPartTransform::preRender(float dt)
    {
        updateFn_();
    }

    void EditPartTransform::onRegister()
    {
    }

    void EditPartTransform::onUnregister()
    {
    }
} }

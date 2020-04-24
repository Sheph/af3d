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

#ifndef _EDITOR_EDITPARTTRANSFORM_H_
#define _EDITOR_EDITPARTTRANSFORM_H_

#include "editor/EditPart.h"

namespace af3d { namespace editor
{
    class EditPartTransform : public std::enable_shared_from_this<EditPartTransform>,
        public EditPart
    {
    public:
        EditPartTransform(const AObjectPtr& target, bool isDefault, const std::string& propName,
            const std::function<void()>& updateFn);
        ~EditPartTransform() = default;

        static const AClass& staticKlass();

        const std::string& targetPropertyName(const std::string& propName) const override;

        static AObjectPtr create(const APropertyValueMap& propVals);

        AObjectPtr sharedThis() override { return shared_from_this(); }

        void preRender(float dt) override;

        APropertyValue propertyTransformGet(const std::string&) const { return target()->propertyGet(propName_); }
        void propertyTransformSet(const std::string&, const APropertyValue& value) { target()->propertySet(propName_, value); }

    private:
        void onRegister() override;

        void onUnregister() override;

        std::string propName_;
        std::function<void()> updateFn_;
    };

    using EditPartTransformPtr = std::shared_ptr<EditPartTransform>;
}
    ACLASS_NS_DECLARE(editor, EditPartTransform)
}

#endif
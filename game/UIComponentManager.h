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

#ifndef _UICOMPONENTMANAGER_H_
#define _UICOMPONENTMANAGER_H_

#include "ComponentManager.h"
#include "RenderNode.h"
#include "VertexArrayWriter.h"
#include <set>

namespace af3d
{
    class UIComponent;
    using UIComponentPtr = std::shared_ptr<UIComponent>;

    class UIComponentComparer : public std::binary_function<UIComponentPtr, UIComponentPtr, bool>
    {
    public:
        bool operator()(const UIComponentPtr& l, const UIComponentPtr& r) const;
    };

    class UIComponentManager : public ComponentManager
    {
    public:
        UIComponentManager() = default;
        ~UIComponentManager();

        virtual void cleanup() override;

        virtual void addComponent(const ComponentPtr& component) override;

        virtual void removeComponent(const ComponentPtr& component) override;

        virtual void freezeComponent(const ComponentPtr& component) override;

        virtual void thawComponent(const ComponentPtr& component) override;

        virtual bool update(float dt) override;

        virtual void debugDraw() override;

        RenderNodePtr render(VertexArrayWriter& defaultVa);

    private:
        std::set<UIComponentPtr, UIComponentComparer> components_;
    };
}

#endif

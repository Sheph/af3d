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

#ifndef _RENDERCOMPONENT_H_
#define _RENDERCOMPONENT_H_

#include "RenderComponentManager.h"
#include "RenderList.h"

namespace af3d
{
    class RenderComponent : public Component
    {
    public:
        explicit RenderComponent(bool renderAlways = false)
        : renderAlways_(renderAlways)
        {
        }

        ~RenderComponent() = default;

        RenderComponentManager* manager() override { return manager_; }
        inline void setManager(RenderComponentManager* value)
        {
            onSetManager(manager_, value);
        }

        inline bool renderAlways() const { return renderAlways_; }
        inline void setRenderAlways(bool value) { renderAlways_ = value; }

        inline bool visible() const { return visible_; }
        inline void setVisible(bool value) { visible_ = value; }

        virtual void update(float dt) = 0;

        virtual void render(RenderList& rl, void* const* parts, size_t numParts) = 0;

        inline const std::string& name() const { return name_; }
        inline void setName(const std::string& value) { name_ = value; }

    private:
        bool renderAlways_;
        bool visible_ = true;
        std::string name_;
        RenderComponentManager* manager_ = nullptr;
    };
}

#endif

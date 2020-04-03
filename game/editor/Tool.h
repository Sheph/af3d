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

#ifndef _EDITOR_TOOL_H_
#define _EDITOR_TOOL_H_

#include "Image.h"
#include "af3d/Types.h"
#include <boost/noncopyable.hpp>

namespace af3d {
    class Scene;

namespace editor
{
    class Workspace;

    class Tool : boost::noncopyable
    {
    public:
        explicit Tool(Workspace* workspace, const std::string& name,
            const Image& icon);
        virtual ~Tool() = default;

        inline const std::string& name() const { return name_; }

        inline const Image& icon() const { return icon_; }

        inline bool active() const { return active_; }

        virtual bool canWork() const { return true; }

        void activate(bool value);

        void update(float dt);

        void options();

    protected:
        inline Workspace& workspace() { return *workspace_; }
        inline const Workspace& workspace() const { return *workspace_; }
        Scene* scene();
        const Scene* scene() const;

    private:
        virtual void onActivate() = 0;

        virtual void onDeactivate() = 0;

        virtual void doUpdate(float dt) = 0;

        virtual void doOptions() = 0;

        Workspace* workspace_;
        std::string name_;
        Image icon_;

        bool active_ = false;
    };
} }

#endif

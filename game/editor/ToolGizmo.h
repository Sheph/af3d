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

#ifndef _EDITOR_TOOLGIZMO_H_
#define _EDITOR_TOOLGIZMO_H_

#include "editor/Tool.h"
#include "af3d/Ray.h"
#include "af3d/Frustum.h"

namespace af3d { namespace editor
{
    class ToolGizmo : public Tool
    {
    public:
        ToolGizmo(Workspace* workspace, const std::string& name,
            const Image& icon);
        ~ToolGizmo() = default;

    protected:
        inline bool captured() const { return !capturedRay_.empty(); }
        inline const Ray& capturedRay() const { return capturedRay_; }

        void onActivate() override;

        void onDeactivate() override;

        void doUpdate(float dt) override;

    private:
        void cleanup();

        virtual bool gizmoCreate(const AObjectPtr& obj) = 0;
        virtual void gizmoDestroy() = 0;

        virtual bool gizmoCapture(const Frustum& frustum, const Ray& ray) = 0;
        virtual void gizmoRelease(bool canceled) = 0;
        virtual void gizmoMove(const Frustum& frustum, const Ray& ray) = 0;

        AObjectPtr target_;
        Ray capturedRay_ = Ray_empty;
        Vector2f capturedMousePos_;
    };
} }

#endif

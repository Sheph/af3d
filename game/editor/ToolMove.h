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

#ifndef _EDITOR_TOOLMOVE_H_
#define _EDITOR_TOOLMOVE_H_

#include "editor/ToolSelect.h"
#include "editor/ToolGizmo.h"
#include "RenderGizmoAxesComponent.h"

namespace af3d { namespace editor
{
    class ToolMove : public ToolGizmo
    {
    public:
        explicit ToolMove(Workspace* workspace);
        ~ToolMove() = default;

    private:
        void onActivate() override;

        void onDeactivate() override;

        void doUpdate(float dt) override;

        void doOptions() override;

        bool gizmoCreate(const AObjectPtr& obj) override;
        void gizmoDestroy() override;

        bool gizmoCapture(const Frustum& frustum, const Ray& ray) override;
        void gizmoRelease(bool canceled) override;
        void gizmoMove(const Frustum& frustum, const Ray& ray) override;

        btPlane getMovePlane(const Frustum& frustum);

        btTransform capturedTargetXfOriented() const;

        ToolSelect selTool_;
        RenderGizmoAxesComponentPtr rc_;

        btTransform capturedTargetXf_ = btTransform::getIdentity();
    };
} }

#endif

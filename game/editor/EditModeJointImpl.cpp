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

#include "editor/EditModeJointImpl.h"
#include "Scene.h"
#include "SceneObject.h"

namespace af3d { namespace editor
{
    EditModeJointImpl::EditModeJointImpl(Workspace* workspace)
    : EditModeImpl(workspace, "joint")
    {
    }

    EditModeJoint::TList EditModeJointImpl::hoveredTyped() const
    {
        TList res;
        for (const auto& wobj : hovered()) {
            auto obj = wobj.lock();
            res.push_back(std::static_pointer_cast<Joint>(obj));
        }
        return res;
    }

    EditModeJoint::TList EditModeJointImpl::selectedTyped() const
    {
        TList res;
        for (const auto& wobj : selected()) {
            auto obj = wobj.lock();
            res.push_back(std::static_pointer_cast<Joint>(obj));
        }
        return res;
    }

    AObjectPtr EditModeJointImpl::rayCast(const Frustum& frustum, const Ray& ray) const
    {
        JointPtr res;

        return res;
    }

    bool EditModeJointImpl::isValid(const AObjectPtr& obj) const
    {
        auto r = aobjectCast<Joint>(obj);
        return r && ((r->aflags() & AObjectEditable) != 0);
    }

    bool EditModeJointImpl::isAlive(const AObjectPtr& obj) const
    {
        auto r = std::static_pointer_cast<Joint>(obj);
        return r->parent();
    }
} }

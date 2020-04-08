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

#include "editor/EditModeLightImpl.h"
#include "Scene.h"
#include "SceneObject.h"
#include "Light.h"

namespace af3d { namespace editor
{
    EditModeLightImpl::EditModeLightImpl(Workspace* workspace)
    : EditModeImpl(workspace, "light")
    {
    }

    EditModeLight::TList EditModeLightImpl::hoveredTyped() const
    {
        TList res;
        for (const auto& wobj : hovered()) {
            auto obj = wobj.lock();
            res.push_back(std::static_pointer_cast<Light>(obj));
        }
        return res;
    }

    EditModeLight::TList EditModeLightImpl::selectedTyped() const
    {
        TList res;
        for (const auto& wobj : selected()) {
            auto obj = wobj.lock();
            res.push_back(std::static_pointer_cast<Light>(obj));
        }
        return res;
    }

    AObjectPtr EditModeLightImpl::rayCast(const Frustum& frustum, const Ray& ray) const
    {
        LightPtr res;

        scene()->rayCastRender(frustum, ray, [&res](const RenderComponentPtr& r, const AObjectPtr&, const btVector3&, float dist) {
            if ((r->aflags() & AObjectMarkerLight) == 0) {
                return -1.0f;
            }
            auto lights = r->parent()->findComponents<Light>();
            for (const auto& l : lights) {
                if (l->markerRc() == r) {
                    res = l;
                    break;
                }
            }
            return dist;
        });

        return res;
    }

    bool EditModeLightImpl::isValid(const AObjectPtr& obj) const
    {
        return !!aobjectCast<Light>(obj);
    }

    bool EditModeLightImpl::isAlive(const AObjectPtr& obj) const
    {
        auto light = std::static_pointer_cast<Light>(obj);
        return light->parent();
    }
} }

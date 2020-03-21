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

#include "UIBoxComponent.h"
#include "SceneObject.h"
#include "imgui.h"

namespace af3d
{
    UIBoxComponent::UIBoxComponent(const AABB2f& aabb, const MaterialPtr& material, int zOrder)
    : UIComponent(zOrder),
      material_(material)
    {
        v_[0] = btVector3(aabb.lowerBound.x(), aabb.lowerBound.y(), 0.0f);
        v_[1] = btVector3(aabb.upperBound.x(), aabb.lowerBound.y(), 0.0f);
        v_[2] = btVector3(aabb.upperBound.x(), aabb.upperBound.y(), 0.0f);
        v_[3] = btVector3(aabb.lowerBound.x(), aabb.upperBound.y(), 0.0f);
    }

    void UIBoxComponent::update(float dt)
    {
        bool open = true;
        ImGui::ShowDemoWindow(&open);
    }

    void UIBoxComponent::render(RenderList& rl)
    {
        auto rop = rl.addGeometry(material_, GL_TRIANGLES, zOrder());

        Color c = Color(1.0f, 1.0f, 1.0f, 0.8f);

        btVector3 tmp[4];

        for (int i = 0; i < 4; ++i) {
            tmp[i] = parent()->transform() * v_[i];
        }

        rop.addVertex(tmp[0], Vector2f(0.0f, 1.0f), c);
        rop.addVertex(tmp[1], Vector2f(1.0f, 1.0f), c);
        rop.addVertex(tmp[2], Vector2f(1.0f, 0.0f), c);

        rop.addVertex(tmp[0], Vector2f(0.0f, 1.0f), c);
        rop.addVertex(tmp[2], Vector2f(1.0f, 0.0f), c);
        rop.addVertex(tmp[3], Vector2f(0.0f, 0.0f), c);
    }

    void UIBoxComponent::onRegister()
    {
    }

    void UIBoxComponent::onUnregister()
    {
    }
}

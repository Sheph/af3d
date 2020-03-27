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

#include "RenderQuadComponent.h"
#include "MaterialManager.h"
#include "SceneObject.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(RenderQuadComponent, RenderComponent)
    ACLASS_PROPERTY(RenderQuadComponent, Position, AProperty_Position, "Position", Vec3f, btVector3(0.0f, 0.0f, 0.0f), Position)
    ACLASS_PROPERTY(RenderQuadComponent, Angle, AProperty_Angle, "Angle", Float, 0.0f, Position)
    ACLASS_PROPERTY(RenderQuadComponent, Size, AProperty_Size, "Size", Vec2f, Vector2f(0.0f, 0.0f), Position)
    ACLASS_PROPERTY(RenderQuadComponent, Width, AProperty_Width, "Width", Float, 0.0f, Position)
    ACLASS_PROPERTY(RenderQuadComponent, Height, AProperty_Height, "Height", Float, 0.0f, Position)
    ACLASS_DEFINE_END(RenderQuadComponent)

    RenderQuadComponent::RenderQuadComponent()
    : RenderComponent(AClass_RenderQuadComponent)
    {
    }

    const AClass& RenderQuadComponent::staticKlass()
    {
        return AClass_RenderQuadComponent;
    }

    AObjectPtr RenderQuadComponent::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<RenderQuadComponent>();
        obj->propertiesSet(propVals);
        return obj;
    }

    void RenderQuadComponent::update(float dt)
    {
        if ((parent()->transform() == prevParentXf_) && !dirty_) {
            return;
        }

        AABB aabb = updateAABB();

        btVector3 displacement = parent()->transform().getOrigin() - prevParentXf_.getOrigin();

        manager()->moveAABB(cookie_, prevAABB_, aabb, displacement);

        prevParentXf_ = parent()->transform();
        prevAABB_ = aabb;
        dirty_ = false;
    }

    void RenderQuadComponent::render(RenderList& rl, void* const* parts, size_t numParts)
    {
        if (!drawable_ || !drawable_->image()) {
            return;
        }

        const auto& image = drawable_->image();

        if (!material_ ||
            (material_->textureBinding(SamplerName::Main).tex != image.texture())) {
            material_ = materialManager.loadImmMaterial(image.texture(), SamplerParams(), depthTest_);
        }

        auto vRight = rl.frustum().transform().getBasis().getColumn(0);
        auto vUp = rl.frustum().transform().getBasis().getColumn(1);

        if (viewportHeight_ > 0.0f) {
            float scale = viewportHeight_ * rl.frustum().getExtents(worldCenter_).y();
            vRight *= scale;
            vUp *= scale;
        }

        auto p0 = toVector3f(worldCenter_ + vRight * points_[0].x() + vUp * points_[0].y());
        auto p1 = toVector3f(worldCenter_ + vRight * points_[1].x() + vUp * points_[1].y());
        auto p2 = toVector3f(worldCenter_ + vRight * points_[2].x() + vUp * points_[2].y());
        auto p3 = toVector3f(worldCenter_ + vRight * points_[3].x() + vUp * points_[3].y());

        auto rop = rl.addGeometry(material_, GL_TRIANGLES);

        if (flip_) {
            rop.addVertex(p0, image.texCoords()[1], color());
            rop.addVertex(p1, image.texCoords()[0], color());
            rop.addVertex(p2, image.texCoords()[3], color());

            rop.addVertex(p0, image.texCoords()[1], color());
            rop.addVertex(p2, image.texCoords()[3], color());
            rop.addVertex(p3, image.texCoords()[2], color());
        } else {
            rop.addVertex(p0, image.texCoords()[0], color());
            rop.addVertex(p1, image.texCoords()[1], color());
            rop.addVertex(p2, image.texCoords()[2], color());

            rop.addVertex(p0, image.texCoords()[0], color());
            rop.addVertex(p2, image.texCoords()[2], color());
            rop.addVertex(p3, image.texCoords()[3], color());
        }
    }

    void RenderQuadComponent::debugDraw()
    {
    }

    void RenderQuadComponent::setDrawable(const DrawablePtr& value)
    {
        drawable_ = value;
    }

    void RenderQuadComponent::setPos(const btVector3& value)
    {
        if (pos_ != value) {
            pos_ = value;
            dirty_ = true;
        }
    }

    void RenderQuadComponent::setAngle(float value)
    {
        if (angle_ != value) {
            angle_ = value;
            dirty_ = true;
            updatePoints();
        }
    }

    void RenderQuadComponent::setSize(const Vector2f& value)
    {
        if (size_ != value) {
            size_ = value;
            dirty_ = true;
            updatePoints();
        }
    }

    void RenderQuadComponent::setWidth(float value, bool keepAspect)
    {
        if (keepAspect && drawable_ && drawable_->image()) {
            setSize(Vector2f(value, value / drawable_->image().aspect()));
        } else {
            setSize(Vector2f(value, size().y()));
        }
    }

    void RenderQuadComponent::setHeight(float value, bool keepAspect)
    {
        if (keepAspect && drawable_ && drawable_->image()) {
            setSize(Vector2f(value * drawable_->image().aspect(), value));
        } else {
            setSize(Vector2f(size().x(), value));
        }
    }

    void RenderQuadComponent::setFixedPos(bool value)
    {
        if (fixedPos_ != value) {
            fixedPos_ = value;
            dirty_ = true;
        }
    }

    void RenderQuadComponent::setFlip(bool value)
    {
        flip_ = value;
    }

    void RenderQuadComponent::setDepthTest(bool value)
    {
        if (depthTest_ != value) {
            depthTest_ = value;
            material_.reset();
        }
    }

    void RenderQuadComponent::setViewportHeight(float value)
    {
        if (viewportHeight_ != value) {
            viewportHeight_ = value;
            dirty_ = true;
            updatePoints();
        }
    }

    void RenderQuadComponent::onRegister()
    {
        prevParentXf_ = parent()->transform();
        prevAABB_ = updateAABB();
        cookie_ = manager()->addAABB(this, prevAABB_, nullptr);
        dirty_ = false;
    }

    void RenderQuadComponent::onUnregister()
    {
        manager()->removeAABB(cookie_);
    }

    void RenderQuadComponent::updatePoints()
    {
        btQuaternion rot(btVector3_back, angle_);

        const Vector2f& sz = (viewportHeight_ > 0.0f) ? Vector2f_one : size_;

        points_[0] = quatRotate(rot, btVector3(-sz.x() / 2.0f, -sz.y() / 2.0f, 0.0f));
        points_[1] = quatRotate(rot, btVector3(sz.x() / 2.0f, -sz.y() / 2.0f, 0.0f));
        points_[2] = quatRotate(rot, btVector3(sz.x() / 2.0f, sz.y() / 2.0f, 0.0f));
        points_[3] = quatRotate(rot, btVector3(-sz.x() / 2.0f, sz.y() / 2.0f, 0.0f));
    }

    AABB RenderQuadComponent::updateAABB()
    {
        auto xf = parent()->transform();

        if (fixedPos_) {
            xf.getBasis().setIdentity();
        }

        worldCenter_ = xf * pos_;

        // Multiply by 1.5 in order to account for rotations, i.e. >= sqrt(2)
        float mx = (btMax(size_.x(), size_.y()) / 2.0f) * 1.5f;

        auto msz = btVector3(mx, mx, mx);

        return AABB(worldCenter_ - msz, worldCenter_ + msz);
    }
}

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
#include "CameraFilter.h"
#include "af3d/Ray.h"

namespace af3d
{
    class RenderComponent : public Component
    {
    public:
        explicit RenderComponent(const AClass& klass, bool renderAlways = false);
        ~RenderComponent() = default;

        static const AClass& staticKlass();

        RenderComponentManager* manager() override { return manager_; }
        inline void setManager(RenderComponentManager* value)
        {
            onSetManager(manager_, value);
        }

        inline bool renderAlways() const { return renderAlways_; }
        inline void setRenderAlways(bool value) { renderAlways_ = value; }

        inline bool visible() const { return visible_; }
        inline void setVisible(bool value) { visible_ = value; }

        inline const CameraFilter& cameraFilter() const { return camFilter_; }
        inline CameraFilter& cameraFilter() { return camFilter_; }

        virtual void update(float dt) = 0;

        virtual void render(RenderList& rl, void* const* parts, size_t numParts) = 0;

        virtual std::pair<AObjectPtr, float> testRay(const Frustum& frustum, const Ray& ray, void* part) = 0;

        APropertyValue propertyVisibleGet(const std::string&) const { return visible(); }
        void propertyVisibleSet(const std::string&, const APropertyValue& value) { setVisible(value.toBool()); }

    private:
        bool renderAlways_;
        bool visible_ = true;
        CameraFilter camFilter_;
        RenderComponentManager* manager_ = nullptr;
    };

    ACLASS_DECLARE(RenderComponent)
}

#endif

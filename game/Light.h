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

#ifndef _LIGHT_H_
#define _LIGHT_H_

#include "Material.h"
#include "af3d/Types.h"
#include "af3d/AABB.h"
#include "af3d/Vector4.h"

namespace af3d
{
    class Light : boost::noncopyable
    {
    public:
        explicit Light(int typeId)
        : typeId_(typeId)
        {
            btAssert(typeId > 0);
        }
        virtual ~Light() = default;

        inline const btTransform& transform() const { return xf_; }
        inline void setTransform(const btTransform& value) { xf_ = value; }

        inline const Color& color() const { return color_; }
        inline void setColor(const Color& value) { color_ = value; }

        inline float intensity() const { return color_.w(); }
        inline void setIntensity(float value) { color_.setW(value); }

        inline bool hasAABB() const { return !localAABB_.empty(); }

        inline const AABB& localAABB() const { return localAABB_; }
        AABB getWorldAABB() const;

        void setupMaterial(const btVector3& eyePos, MaterialParams& params) const;

    protected:
        inline void setLocalAABB(const AABB& value) { localAABB_ = value; }

    private:
        virtual void doSetupMaterial(const btVector3& eyePos, MaterialParams& params) const = 0;

        int typeId_ = 0; // 0 - ambient light.
        btTransform xf_ = btTransform::getIdentity();
        Color color_ = Color_one; // Color in rgb, alpha = intensity.
        AABB localAABB_ = AABB_empty;
    };

    using LightPtr = std::shared_ptr<Light>;
}

#endif

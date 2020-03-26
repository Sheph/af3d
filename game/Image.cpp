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

#include "Image.h"
#include "af3d/AABB2.h"

namespace af3d
{
    Image::Image(const TexturePtr& texture,
        std::uint32_t x, std::uint32_t y,
        std::uint32_t width, std::uint32_t height)
    : texture_(texture),
      x_(x),
      y_(y),
      width_(width),
      height_(height)
    {
        if (!texture) {
            aspect_ = 0.0f;
            return;
        }

        float xf = static_cast<float>(x);
        float yf = static_cast<float>(y);

        AABB2f aabb;

        aabb.lowerBound.setX((xf - 0.5f) / texture->width());
        aabb.lowerBound.setY(1.0f - (yf + height + 0.5f) / texture->height());
        aabb.upperBound.setX((xf + width + 0.5f) / texture->width());
        aabb.upperBound.setY(1.0f - (yf - 0.5f) / texture->height());

        texCoords_[0][0] = texCoords_[3][0] = aabb.lowerBound.x();
        texCoords_[0][1] = texCoords_[3][1] = aabb.lowerBound.y();
        texCoords_[1][0] = aabb.upperBound.x();
        texCoords_[1][1] = aabb.lowerBound.y();
        texCoords_[2][0] = texCoords_[4][0] = aabb.upperBound.x();
        texCoords_[2][1] = texCoords_[4][1] = aabb.upperBound.y();
        texCoords_[5][0] = aabb.lowerBound.x();
        texCoords_[5][1] = aabb.upperBound.y();

        aspect_ = static_cast<float>(width) / height;
    }
}

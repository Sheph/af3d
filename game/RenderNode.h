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

#ifndef _RENDER_NODE_H_
#define _RENDER_NODE_H_

#include "Material.h"
#include "VertexArray.h"
#include <set>

namespace af3d
{
    class RenderNode
    {
    public:
        RenderNode() = default;
        ~RenderNode() = default;

        bool operator<(const RenderNode& other);

    private:
        enum class Type
        {
            DepthTest = 0,
            Depth,
            BlendingParams,
            MaterialType,
            Textures,
            VertexArray,
            MaterialParams,
            Draw
        };

        struct HardwareTextureBinding
        {
            HardwareTextureBinding() = default;
            HardwareTextureBinding(const HardwareTexturePtr& tex,
                const SamplerParams& params)
            : tex(tex),
              params(params) {}

            HardwareTexturePtr tex;
            SamplerParams params;
        };

        Type type_;

        // Type::DepthTest
        bool depthTest_;

        // Type::Depth
        float depth_;

        // Type::BlendingParams
        BlendingParams blendingParams_;

        // Type::MaterialType
        MaterialTypePtr materialType_;

        // Type::Textures
        std::vector<HardwareTextureBinding> textures_;

        // Type::VertexArray and Type::Draw
        VertexArrayPtr va_;

        // Type::MaterialParams
        MaterialParams materialParamsAuto_;
        MaterialParams materialParams_;

        // Type::Draw
        GLenum drawPrimitiveMode_;
        std::uint32_t drawStart_;
        std::uint32_t drawCount_;
        std::uint32_t drawBaseVertex_;

        std::set<RenderNode> children_;
    };

    using RenderNodePtr = std::shared_ptr<RenderNode>;
}

#endif

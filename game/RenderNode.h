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
#include "VertexArraySlice.h"
#include "af3d/AABB2.h"
#include <set>

namespace af3d
{
    struct ScissorParams
    {
        ScissorParams() = default;
        ScissorParams(GLint x,
            GLint y,
            GLsizei width,
            GLsizei height)
        : enabled(true),
          x(x),
          y(y),
          width(width),
          height(height) {}

        bool enabled = false;
        GLint x;
        GLint y;
        GLsizei width;
        GLsizei height;
    };

    class RenderNode
    {
    public:
        RenderNode(GLenum clearMask, const Color& clearColor);
        RenderNode() = default;
        ~RenderNode() = default;

        inline const AABB2i& viewport() const { btAssert(type_ == Type::Root); return viewport_; }
        inline void setViewport(const AABB2i& value) { btAssert(type_ == Type::Root); viewport_ = value; }

        // Returns empty material auto params, these should be filled in by the caller.
        MaterialParams& add(RenderNode&& tmpNode, int pass, const MaterialPtr& material,
            GLenum depthFunc, float depthValue, GLenum cullFaceMode, const BlendingParams& blendingParams,
            const VertexArraySlice& vaSlice, GLenum primitiveMode,
            const ScissorParams& scissorParams);

        bool operator<(const RenderNode& other) const;

        void apply(HardwareContext& ctx) const;

    private:
        enum class Type
        {
            Root = 0,
            Pass,
            DepthTest,
            Depth,
            CullFace,
            BlendingParams,
            MaterialType,
            Textures,
            VertexArray,
            Draw
        };

        struct HardwareTextureBinding
        {
            HardwareTextureBinding() = default;
            HardwareTextureBinding(const HardwareTexturePtr& tex,
                const SamplerParams& params)
            : tex(tex),
              params(params) {}

            inline bool operator<(const HardwareTextureBinding& other) const
            {
                if (tex != other.tex) {
                    return tex < other.tex;
                }
                return params < other.params;
            }

            HardwareTexturePtr tex;
            SamplerParams params;
        };

        using Children = std::set<RenderNode>;

        bool comparePass(const RenderNode& other) const;
        bool compareDepthTest(const RenderNode& other) const;
        bool compareDepth(const RenderNode& other) const;
        bool compareCullFace(const RenderNode& other) const;
        bool compareBlendingParams(const RenderNode& other) const;
        bool compareMaterialType(const RenderNode& other) const;
        bool compareTextures(const RenderNode& other) const;
        bool compareVertexArray(const RenderNode& other) const;
        bool compareDraw(const RenderNode& other) const;

        RenderNode* insertPass(RenderNode&& tmpNode, int pass);
        RenderNode* insertDepthTest(RenderNode&& tmpNode, bool depthTest, GLenum depthFunc);
        RenderNode* insertDepth(RenderNode&& tmpNode, float depth);
        RenderNode* insertCullFace(RenderNode&& tmpNode, GLenum cullFaceMode);
        RenderNode* insertBlendingParams(RenderNode&& tmpNode, const BlendingParams& blendingParams);
        RenderNode* insertMaterialType(RenderNode&& tmpNode, const MaterialTypePtr& materialType);
        RenderNode* insertTextures(RenderNode&& tmpNode, std::vector<HardwareTextureBinding>&& textures);
        RenderNode* insertVertexArray(RenderNode&& tmpNode, const VertexArrayPtr& va);
        RenderNode* insertDraw(RenderNode&& tmpNode, int drawIdx);
        RenderNode* insertImpl(RenderNode&& tmpNode);

        void applyRoot(HardwareContext& ctx) const;
        void applyPass(HardwareContext& ctx) const;
        void applyDepthTest(HardwareContext& ctx) const;
        void applyDepth(HardwareContext& ctx) const;
        void applyCullFace(HardwareContext& ctx) const;
        void applyBlendingParams(HardwareContext& ctx) const;
        void applyMaterialType(HardwareContext& ctx) const;
        void applyTextures(HardwareContext& ctx) const;
        void applyVertexArray(HardwareContext& ctx) const;
        void applyVertexArrayDone(HardwareContext& ctx) const;
        void applyDraw(HardwareContext& ctx) const;

        Type type_;

        // Type::Root
        AABB2i viewport_;
        GLenum clearMask_;
        Color clearColor_;
        int numDraws_;

        union
        {
            struct
            {
                // Type::Pass
                int pass_;
            };
            struct
            {
                // Type::DepthTest
                bool depthTest_;
                GLenum depthFunc_;
            };
            struct
            {
                // Type::Depth
                float depth_;
            };
            struct
            {
                // Type::CullFace
                GLenum cullFaceMode_; // 0 - disabled.
            };
        };

        // Type::BlendingParams
        BlendingParams blendingParams_;

        // Type::MaterialType
        MaterialTypePtr materialType_;

        // Type::Textures
        std::vector<HardwareTextureBinding> textures_;

        // Type::VertexArray and Type::Draw
        VertexArrayPtr va_;

        // Type::Draw
        int drawIdx_;
        ScissorParams scissorParams_;
        MaterialParams materialParams_;
        MaterialParams materialParamsAuto_;
        GLenum drawPrimitiveMode_;
        std::uint32_t drawStart_;
        std::uint32_t drawCount_;
        std::uint32_t drawBaseVertex_;

        Children children_;
    };

    using RenderNodePtr = std::shared_ptr<RenderNode>;

    using RenderNodeList = std::vector<RenderNodePtr>;
}

#endif

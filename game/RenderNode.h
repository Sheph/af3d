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
#include "HardwareMRT.h"
#include "af3d/AABB2.h"
#include <set>
#include <boost/optional.hpp>

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

    using StorageBufferBinding = std::pair<StorageBufferName, HardwareDataBufferPtr>;

    class RenderNode
    {
    public:
        RenderNode(const AABB2i& viewport, const AttachmentPoints& clearMask, const AttachmentColors& clearColors, const HardwareMRT& mrt);
        RenderNode() = default;
        ~RenderNode() = default;

        inline const AABB2i& viewport() const { btAssert(type_ == Type::Root); return viewport_; }

        void add(RenderNode&& tmpNode, int pass, const AttachmentPoints& drawBuffers, const HardwareProgram::Outputs& outputs,
            const MaterialTypePtr& matType,
            const MaterialParams& matParams,
            const BlendingParams& matBlendingParams,
            bool matDepthTest,
            bool matDepthWrite,
            GLenum matCullFaceMode,
            GLenum depthFunc, float depthValue, bool flipCull,
            std::vector<HardwareTextureBinding>&& textures, std::vector<StorageBufferBinding>&& storageBuffers,
            const VertexArraySlice& vaSlice, GLenum primitiveMode,
            const ScissorParams& scissorParams, MaterialParams&& materialParamsAuto);

        void add(RenderNode&& tmpNode, int pass, const MaterialPtr& material,
            const VertexArrayPtr& va,
            std::vector<StorageBufferBinding>&& storageBuffers,
            const Vector3i& computeNumGroups,
            MaterialParams&& materialParamsAuto);

        bool operator<(const RenderNode& other) const;

        void apply(HardwareContext& ctx) const;

    private:
        enum class Type
        {
            Root = 0,
            Pass,
            DepthTest,
            Depth,
            BlendingParams,
            CullFace,
            MaterialType,
            Textures,
            VertexArray,
            Draw
        };

        struct DrawBufferBinding
        {
            void setup(const AttachmentPoints& drawBuffers, const HardwareProgram::Outputs& outputs);

            GLenum buffers[static_cast<int>(AttachmentPoint::Max) - static_cast<int>(AttachmentPoint::Color0) + 1];
            int numBuffers; // -1 - skip.
        };

        using Children = std::set<RenderNode>;

        bool comparePass(const RenderNode& other) const;
        bool compareDepthTest(const RenderNode& other) const;
        bool compareDepth(const RenderNode& other) const;
        bool compareBlendingParams(const RenderNode& other) const;
        bool compareCullFace(const RenderNode& other) const;
        bool compareMaterialType(const RenderNode& other) const;
        bool compareTextures(const RenderNode& other) const;
        bool compareVertexArray(const RenderNode& other) const;
        bool compareDraw(const RenderNode& other) const;

        RenderNode* insertPass(RenderNode&& tmpNode, int pass);
        RenderNode* insertDepthTest(RenderNode&& tmpNode, bool depthTest, GLenum depthFunc);
        RenderNode* insertDepth(RenderNode&& tmpNode, float depth);
        RenderNode* insertBlendingParams(RenderNode&& tmpNode, const BlendingParams& blendingParams);
        RenderNode* insertCullFace(RenderNode&& tmpNode, GLenum cullFaceMode);
        RenderNode* insertMaterialType(RenderNode&& tmpNode, const MaterialTypePtr& materialType);
        RenderNode* insertTextures(RenderNode&& tmpNode, std::vector<HardwareTextureBinding>&& textures);
        RenderNode* insertVertexArray(RenderNode&& tmpNode, const VertexArrayPtr& va, std::vector<StorageBufferBinding>&& storageBuffers);
        RenderNode* insertDraw(RenderNode&& tmpNode, int drawIdx);
        RenderNode* insertImpl(RenderNode&& tmpNode);

        void applyRoot(HardwareContext& ctx) const;
        void applyPass(HardwareContext& ctx) const;
        void applyDepthTest(HardwareContext& ctx) const;
        void applyDepth(HardwareContext& ctx) const;
        void applyBlendingParams(HardwareContext& ctx) const;
        void applyCullFace(HardwareContext& ctx) const;
        void applyMaterialType(HardwareContext& ctx) const;
        void applyTextures(HardwareContext& ctx) const;
        void applyVertexArray(HardwareContext& ctx) const;
        void applyVertexArrayDone(HardwareContext& ctx) const;
        void applyDraw(HardwareContext& ctx) const;

        Type type_;

        // Type::Root
        AABB2i viewport_;
        AttachmentPoints clearMask_;
        AttachmentColors clearColors_;
        HardwareMRT mrt_;
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
            struct
            {
                // Type::Draw
                int drawIdx_;
                DrawBufferBinding drawBufferBinding_;
                GLenum drawPrimitiveMode_;
                std::uint32_t drawStart_;
                std::uint32_t drawCount_;
                std::uint32_t drawBaseVertex_;
                bool depthWrite_;
            };
        };

        // Type::BlendingParams
        BlendingParams blendingParams_;

        // Type::MaterialType
        MaterialTypePtr materialType_;

        // Type::Textures
        std::vector<HardwareTextureBinding> textures_;

        // Type::VertexArray
        std::vector<StorageBufferBinding> storageBuffers_;

        // Type::VertexArray and Type::Draw
        VertexArrayPtr va_;

        // Type::Draw
        ScissorParams scissorParams_;
        MaterialParams materialParams_;
        MaterialParams materialParamsAuto_;
        boost::optional<Vector3i> computeNumGroups_;

        Children children_;
    };

    using RenderNodePtr = std::shared_ptr<RenderNode>;

    using RenderNodeList = std::vector<RenderNodePtr>;
}

#endif

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

#include "RenderNode.h"
#include "TextureManager.h"
#include "Logger.h"

namespace af3d
{
    RenderNode::RenderNode(const AABB2i& viewport, GLenum clearMask, const Color& clearColor)
    : type_(Type::Root),
      viewport_(viewport),
      clearMask_(clearMask),
      clearColor_(clearColor),
      numDraws_(0)
    {
    }

    MaterialParams& RenderNode::add(RenderNode&& tmpNode, int pass, const MaterialPtr& material,
        GLenum depthFunc, float depthValue, GLenum cullFaceMode, const BlendingParams& blendingParams,
        const VertexArraySlice& vaSlice, GLenum primitiveMode, const ScissorParams& scissorParams)
    {
        btAssert(type_ == Type::Root);

        RenderNode* node = this;

        node = node->insertPass(std::move(tmpNode), pass);
        node = node->insertDepthTest(std::move(tmpNode), material->depthTest(), depthFunc);
        node = node->insertDepth(std::move(tmpNode), depthValue);
        node = node->insertCullFace(std::move(tmpNode), cullFaceMode);
        node = node->insertBlendingParams(std::move(tmpNode), blendingParams);
        node = node->insertMaterialType(std::move(tmpNode), material->type());

        const auto& samplers = material->type()->prog()->samplers();
        std::vector<HardwareTextureBinding> textures;
        for (int i = 0; i <= static_cast<int>(SamplerName::Max); ++i) {
            SamplerName sName = static_cast<SamplerName>(i);
            if (samplers[sName]) {
                const auto& tb = material->textureBinding(sName);
                textures.emplace_back(tb.tex ? tb.tex->hwTex() : HardwareTexturePtr(), tb.params);
            }
        }

        node = node->insertTextures(std::move(tmpNode), std::move(textures));
        node = node->insertVertexArray(std::move(tmpNode), vaSlice.va());
        node = node->insertDraw(std::move(tmpNode), numDraws_++);

        node->va_ = vaSlice.va();
        node->scissorParams_ = scissorParams;
        node->materialParams_ = material->params();
        node->materialParamsAuto_ = MaterialParams(material->type(), true);
        node->drawPrimitiveMode_ = primitiveMode;
        node->drawStart_ = vaSlice.start();
        node->drawCount_ = vaSlice.count();
        node->drawBaseVertex_ = vaSlice.baseVertex();

        return node->materialParamsAuto_;
    }

    bool RenderNode::operator<(const RenderNode& other) const
    {
        switch (type_) {
        case Type::Pass: return comparePass(other);
        case Type::DepthTest: return compareDepthTest(other);
        case Type::Depth: return compareDepth(other);
        case Type::CullFace: return compareCullFace(other);
        case Type::BlendingParams: return compareBlendingParams(other);
        case Type::MaterialType: return compareMaterialType(other);
        case Type::Textures: return compareTextures(other);
        case Type::VertexArray: return compareVertexArray(other);
        case Type::Draw: return compareDraw(other);
        case Type::Root:
        default:
            runtime_assert(false);
        }
        return false;
    }

    void RenderNode::apply(HardwareContext& ctx) const
    {
        switch (type_) {
        case Type::Root:
            applyRoot(ctx);
            break;
        case Type::Pass:
            applyPass(ctx);
            break;
        case Type::DepthTest:
            applyDepthTest(ctx);
            break;
        case Type::Depth:
            applyDepth(ctx);
            break;
        case Type::CullFace:
            applyCullFace(ctx);
            break;
        case Type::BlendingParams:
            applyBlendingParams(ctx);
            break;
        case Type::MaterialType:
            applyMaterialType(ctx);
            break;
        case Type::Textures:
            applyTextures(ctx);
            break;
        case Type::VertexArray:
            applyVertexArray(ctx);
            break;
        case Type::Draw:
            applyDraw(ctx);
            break;
        default:
            runtime_assert(false);
        }
        for (const auto& node : children_) {
            node.apply(ctx);
        }
        if (type_ == Type::VertexArray) {
            applyVertexArrayDone(ctx);
        }
    }

    bool RenderNode::comparePass(const RenderNode& other) const
    {
        return pass_ < other.pass_;
    }

    bool RenderNode::compareDepthTest(const RenderNode& other) const
    {
        if (depthTest_ != other.depthTest_) {
            return (int)depthTest_ > (int)other.depthTest_;
        }
        return depthFunc_ < other.depthFunc_;
    }

    bool RenderNode::compareDepth(const RenderNode& other) const
    {
        return depth_ < other.depth_;
    }

    bool RenderNode::compareCullFace(const RenderNode& other) const
    {
        return cullFaceMode_ < other.cullFaceMode_;
    }

    bool RenderNode::compareBlendingParams(const RenderNode& other) const
    {
        return blendingParams_ < other.blendingParams_;
    }

    bool RenderNode::compareMaterialType(const RenderNode& other) const
    {
        return materialType_ < other.materialType_;
    }

    bool RenderNode::compareTextures(const RenderNode& other) const
    {
        return textures_ < other.textures_;
    }

    bool RenderNode::compareVertexArray(const RenderNode& other) const
    {
        return va_ < other.va_;
    }

    bool RenderNode::compareDraw(const RenderNode& other) const
    {
        return drawIdx_ < other.drawIdx_;
    }

    RenderNode* RenderNode::insertPass(RenderNode&& tmpNode, int pass)
    {
        tmpNode.type_ = Type::Pass;
        tmpNode.pass_ = pass;
        return insertImpl(std::move(tmpNode));
    }

    RenderNode* RenderNode::insertDepthTest(RenderNode&& tmpNode, bool depthTest, GLenum depthFunc)
    {
        tmpNode.type_ = Type::DepthTest;
        tmpNode.depthTest_ = depthTest;
        tmpNode.depthFunc_ = depthFunc;
        return insertImpl(std::move(tmpNode));
    }

    RenderNode* RenderNode::insertDepth(RenderNode&& tmpNode, float depth)
    {
        tmpNode.type_ = Type::Depth;
        tmpNode.depth_ = depth;
        return insertImpl(std::move(tmpNode));
    }

    RenderNode* RenderNode::insertCullFace(RenderNode&& tmpNode, GLenum cullFaceMode)
    {
        tmpNode.type_ = Type::CullFace;
        tmpNode.cullFaceMode_ = cullFaceMode;
        return insertImpl(std::move(tmpNode));
    }

    RenderNode* RenderNode::insertBlendingParams(RenderNode&& tmpNode, const BlendingParams& blendingParams)
    {
        tmpNode.type_ = Type::BlendingParams;
        tmpNode.blendingParams_ = blendingParams;
        return insertImpl(std::move(tmpNode));
    }

    RenderNode* RenderNode::insertMaterialType(RenderNode&& tmpNode, const MaterialTypePtr& materialType)
    {
        tmpNode.type_ = Type::MaterialType;
        tmpNode.materialType_ = materialType;
        return insertImpl(std::move(tmpNode));
    }

    RenderNode* RenderNode::insertTextures(RenderNode&& tmpNode, std::vector<HardwareTextureBinding>&& textures)
    {
        tmpNode.type_ = Type::Textures;
        tmpNode.textures_ = std::move(textures);
        return insertImpl(std::move(tmpNode));
    }

    RenderNode* RenderNode::insertVertexArray(RenderNode&& tmpNode, const VertexArrayPtr& va)
    {
        tmpNode.type_ = Type::VertexArray;
        tmpNode.va_ = va;
        return insertImpl(std::move(tmpNode));
    }

    RenderNode* RenderNode::insertDraw(RenderNode&& tmpNode, int drawIdx)
    {
        tmpNode.type_ = Type::Draw;
        tmpNode.drawIdx_ = drawIdx;
        return insertImpl(std::move(tmpNode));
    }

    RenderNode* RenderNode::insertImpl(RenderNode&& tmpNode)
    {
        auto res = children_.insert(std::move(tmpNode));
        return const_cast<RenderNode*>(&*res.first);
    }

    void RenderNode::applyRoot(HardwareContext& ctx) const
    {
        //LOG4CPLUS_DEBUG(logger(), "draw(" << numDraws_ << ")");
        ogl.Viewport(viewport_.lowerBound[0], viewport_.lowerBound[1],
            viewport_.upperBound[0] - viewport_.lowerBound[0],
            viewport_.upperBound[1] - viewport_.lowerBound[1]);
        if (clearMask_ != 0) {
            ogl.ClearColor(clearColor_[0], clearColor_[1], clearColor_[2], clearColor_[3]);
            ogl.Clear(clearMask_);
        }
    }

    void RenderNode::applyPass(HardwareContext& ctx) const
    {
    }

    void RenderNode::applyDepthTest(HardwareContext& ctx) const
    {
        if (depthTest_) {
            ogl.DepthFunc(depthFunc_);
            ogl.Enable(GL_DEPTH_TEST);
        } else {
            ogl.Disable(GL_DEPTH_TEST);
        }
    }

    void RenderNode::applyDepth(HardwareContext& ctx) const
    {
    }

    void RenderNode::applyCullFace(HardwareContext& ctx) const
    {
        if (cullFaceMode_) {
            ogl.CullFace(cullFaceMode_);
            ogl.Enable(GL_CULL_FACE);
        } else {
            ogl.Disable(GL_CULL_FACE);
        }
    }

    void RenderNode::applyBlendingParams(HardwareContext& ctx) const
    {
        if (blendingParams_.isEnabled()) {
            ogl.BlendFuncSeparate(blendingParams_.blendSfactor, blendingParams_.blendDfactor,
                blendingParams_.blendSfactorAlpha, blendingParams_.blendDfactorAlpha);
            ogl.Enable(GL_BLEND);
        } else {
            ogl.Disable(GL_BLEND);
        }
    }

    void RenderNode::applyMaterialType(HardwareContext& ctx) const
    {
        ogl.UseProgram(materialType_->prog()->id(ctx));
    }

    void RenderNode::applyTextures(HardwareContext& ctx) const
    {
        // TODO: Handle multitexturing and samplers.
        for (int i = 0; i < static_cast<int>(textures_.size()); ++i) {
            ogl.ActiveTexture(GL_TEXTURE0 + i);
            GLuint id = textures_[i].tex ? textures_[i].tex->id(ctx) : 0;
            if (id == 0) {
                id = textureManager.white1x1()->hwTex()->id(ctx);
            }
            ogl.BindTexture(GL_TEXTURE_2D, id);
        }
    }

    void RenderNode::applyVertexArray(HardwareContext& ctx) const
    {
        ogl.BindVertexArray(va_->vao(ctx)->id(ctx));
    }

    void RenderNode::applyVertexArrayDone(HardwareContext& ctx) const
    {
        ogl.BindVertexArray(0);
    }

    void RenderNode::applyDraw(HardwareContext& ctx) const
    {
        if (scissorParams_.enabled) {
            ogl.Scissor(scissorParams_.x, scissorParams_.y, scissorParams_.width, scissorParams_.height);
            ogl.Enable(GL_SCISSOR_TEST);
        }

        // TODO: Handle other draw modes.
        materialParamsAuto_.apply(ctx);
        materialParams_.apply(ctx);
        if (drawCount_ == 0) {
            if (va_->ebo()) {
                ogl.DrawElements(drawPrimitiveMode_, va_->ebo()->count(ctx),
                    va_->ebo()->glDataType(),
                    (const void*)(va_->ebo()->elementSize() * drawStart_));
            }
        } else {
            if (va_->ebo()) {
                ogl.DrawElementsBaseVertex(drawPrimitiveMode_, drawCount_,
                    va_->ebo()->glDataType(),
                    (void*)(va_->ebo()->elementSize() * drawStart_), drawBaseVertex_);
            } else {
                ogl.DrawArrays(drawPrimitiveMode_, drawStart_, drawCount_);
            }
        }

        if (scissorParams_.enabled) {
            ogl.Disable(GL_SCISSOR_TEST);
        }
    }
}

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

#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "Resource.h"
#include "MaterialType.h"
#include "Texture.h"
#include "af3d/Utils.h"

namespace af3d
{
    class MaterialParams
    {
    public:
        MaterialParams() = default;
        MaterialParams(const MaterialTypePtr& materialType, bool isAuto);
        ~MaterialParams() = default;

        void setUniform(UniformName name, float value);
        void setUniform(UniformName name, std::int32_t value);
        void setUniform(UniformName name, std::uint32_t value);
        void setUniform(UniformName name, const Vector2f& value);
        void setUniform(UniformName name, const Vector3f& value);
        void setUniform(UniformName name, const btVector3& value);
        void setUniform(UniformName name, const Vector4f& value);
        void setUniform(UniformName name, const Matrix4f& value);

        bool getUniform(UniformName name, float& value, bool withDefault = false) const;
        bool getUniform(UniformName name, std::int32_t& value, bool withDefault = false) const;
        bool getUniform(UniformName name, std::uint32_t& value, bool withDefault = false) const;
        bool getUniform(UniformName name, Vector2f& value, bool withDefault = false) const;
        bool getUniform(UniformName name, Vector3f& value, bool withDefault = false) const;
        bool getUniform(UniformName name, btVector3& value, bool withDefault = false) const;
        bool getUniform(UniformName name, Vector4f& value, bool withDefault = false) const;
        bool getUniform(UniformName name, Matrix4f& value, bool withDefault = false) const;

        void apply(HardwareContext& ctx) const;

        void convert(MaterialParams& other) const;

    private:
        using UniformMap = EnumUnorderedMap<UniformName, GLsizei>; // uniform -> actual count
        using ParamList = std::vector<Byte>;

        bool checkName(UniformName name, size_t& offset, VariableInfo& info, bool quiet) const;

        void setUniformImpl(UniformName name, const Byte* data, GLenum baseType, GLint numComponents, GLsizei count, bool quiet = false);

        bool getUniformImpl(UniformName name, Byte* data, GLenum baseType, GLint numComponents, GLsizei count) const;

        MaterialTypePtr materialType_;
        bool isAuto_ = false;
        ParamList paramList_;
        UniformMap uniforms_;
    };

    struct BlendingParams
    {
        BlendingParams()
        : blendSfactor(GL_ONE),
          blendDfactor(GL_ZERO),
          blendSfactorAlpha(GL_ONE),
          blendDfactorAlpha(GL_ZERO) {}
        BlendingParams(GLenum blendSfactor,
            GLenum blendDfactor,
            GLenum blendSfactorAlpha,
            GLenum blendDfactorAlpha)
        : blendSfactor(blendSfactor),
          blendDfactor(blendDfactor),
          blendSfactorAlpha(blendSfactorAlpha),
          blendDfactorAlpha(blendDfactorAlpha)
        {
        }

        inline bool isEnabled() const
        {
            return !(blendSfactor == GL_ONE && blendDfactor == GL_ZERO &&
                blendSfactorAlpha == GL_ONE && blendDfactorAlpha == GL_ZERO);
        }

        inline bool operator<(const BlendingParams& other) const
        {
            int enabled = isEnabled();
            int otherEnabled = other.isEnabled();

            if (enabled != otherEnabled) {
                return enabled < otherEnabled;
            }
            if (blendSfactor != other.blendSfactor) {
                return blendSfactor < other.blendSfactor;
            }
            if (blendDfactor != other.blendDfactor) {
                return blendDfactor < other.blendDfactor;
            }
            if (blendSfactorAlpha != other.blendSfactorAlpha) {
                return blendSfactorAlpha < other.blendSfactorAlpha;
            }
            return blendDfactorAlpha < other.blendDfactorAlpha;
        }

        GLenum blendSfactor;
        GLenum blendDfactor;
        GLenum blendSfactorAlpha;
        GLenum blendDfactorAlpha;
    };

    struct TextureBinding
    {
        TextureBinding() = default;
        explicit TextureBinding(const TexturePtr& tex,
            const SamplerParams& params = SamplerParams())
        : tex(tex),
          params(params) {}

        TexturePtr tex;
        SamplerParams params;
    };

    class Material;
    using MaterialPtr = std::shared_ptr<Material>;

    class MaterialManager;

    class Material : public std::enable_shared_from_this<Material>,
        public Resource
    {
    public:
        Material(MaterialManager* mgr, const std::string& name, const MaterialTypePtr& type);
        ~Material();

        static const AClass& staticKlass();

        static AObjectPtr create(const APropertyValueMap& propVals);

        virtual AObjectPtr sharedThis() override { return shared_from_this(); }

        inline const MaterialTypePtr& type() const { return type_; }

        MaterialPtr clone(const std::string& newName = "") const;

        MaterialPtr convert(MaterialTypeName matTypeName, const std::string& newName = "") const;

        const TextureBinding& textureBinding(SamplerName samplerName) const;
        void setTextureBinding(SamplerName samplerName, const TextureBinding& value);

        inline MaterialParams& params() { return params_; }

        const BlendingParams& blendingParams() const;
        void setBlendingParams(const BlendingParams& value);

        bool depthTest() const;
        void setDepthTest(bool value);

        bool depthWrite() const;
        void setDepthWrite(bool value);

        GLenum cullFaceMode() const;
        void setCullFaceMode(GLenum value);

    private:
        bool cloneImpl(const MaterialPtr& cloned) const;

        MaterialManager* mgr_;
        MaterialTypePtr type_;
        std::array<TextureBinding, static_cast<int>(SamplerName::Max) + 1> tbs_;
        MaterialParams params_;
        BlendingParams blendingParams_;
        bool depthTest_ = true;
        bool depthWrite_ = true;
        GLenum cullFaceMode_ = GL_BACK;
    };

    ACLASS_DECLARE(Material)
}

#endif

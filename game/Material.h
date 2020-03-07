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
#include "af3d/Vector2.h"
#include "af3d/Vector3.h"
#include "af3d/Vector4.h"

namespace af3d
{
    class MaterialParams
    {
    public:
        MaterialParams() = default;
        explicit MaterialParams(const MaterialTypePtr& materialType);
        ~MaterialParams() = default;

        void setUniform(UniformName name, float value);
        void setUniform(UniformName name, std::int32_t value);
        void setUniform(UniformName name, std::uint32_t value);
        void setUniform(UniformName name, const Vector2f& value);
        void setUniform(UniformName name, const Vector3f& value);
        void setUniform(UniformName name, const btVector3& value);
        void setUniform(UniformName name, const Vector4f& value);
        void setUniform(UniformName name, const float* value, GLsizei count);
        void setUniform(UniformName name, const std::int32_t* value, GLsizei count);
        void setUniform(UniformName name, const std::uint32_t* value, GLsizei count);

    private:
        using FloatParamList = std::vector<float>;
        using IntParamList = std::vector<std::int32_t>;
        using UIntParamList = std::vector<std::uint32_t>;

        MaterialTypePtr materialType_;
        FloatParamList floatParams_;
        IntParamList intParams_;
        UIntParamList uintParams_;
    };

    class Material;
    using MaterialPtr = std::shared_ptr<Material>;

    class Material : public Resource
    {
    public:
        Material(const MaterialTypePtr& type, const std::string& name);
        ~Material() = default;

        inline const MaterialTypePtr& type() const { return type_; }

        MaterialPtr clone() const;

    private:
        void doInvalidate(HardwareContext& ctx) override;

        MaterialTypePtr type_;
        std::vector<TexturePtr> textures_;
        MaterialParams params_;
    };
}

#endif

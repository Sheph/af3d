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

#ifndef _MATERIAL_TYPE_H_
#define _MATERIAL_TYPE_H_

#include "HardwareProgram.h"
#include "af3d/EnumSet.h"
#include "af3d/Utils.h"
#include "af3d/Vector2.h"
#include "af3d/Vector3.h"
#include "af3d/Vector4.h"
#include "af3d/Matrix4.h"

namespace af3d
{
    enum MaterialTypeName
    {
        MaterialTypeBasic = 0, // Basic lit shader.
        MaterialTypeUnlit,     // Unlit shader.
        MaterialTypeUnlitVC,   // Unlit shader with per-vertex color.
        MaterialTypeImm,       // Immediate draw shader. pos(3d), color(4d) and one texture(2d)
        MaterialTypeOutline,   // Selection outline.
        MaterialTypeGrid,
        MaterialTypeFirst = MaterialTypeBasic,
        MaterialTypeMax = MaterialTypeGrid
    };

    class MaterialType : boost::noncopyable
    {
    public:
        struct ParamListInfo
        {
            EnumUnorderedMap<UniformName, size_t> offsets;
            size_t totalSize = 0;

            std::vector<Byte> defaultParamList;
            EnumUnorderedMap<UniformName, GLsizei> defaultUniforms;
        };

        MaterialType(MaterialTypeName name, const HardwareProgramPtr& prog, bool usesLight);
        ~MaterialType() = default;

        inline MaterialTypeName name() const { return name_; }

        inline const HardwareProgramPtr& prog() const { return prog_; }

        inline bool usesLight() const { return usesLight_; }

        const ParamListInfo& paramListInfo(bool isAuto) const { return isAuto ? autoParamListInfo_ : paramListInfo_; }

        bool reload(const std::string& vertSource, const std::string& fragSource, HardwareContext& ctx);

        void setDefaultUniform(UniformName name, float value);
        void setDefaultUniform(UniformName name, std::int32_t value);
        void setDefaultUniform(UniformName name, std::uint32_t value);
        void setDefaultUniform(UniformName name, const Vector2f& value);
        void setDefaultUniform(UniformName name, const Vector3f& value);
        void setDefaultUniform(UniformName name, const btVector3& value);
        void setDefaultUniform(UniformName name, const Vector4f& value);
        void setDefaultUniform(UniformName name, const Matrix4f& value);

    private:
        bool checkName(UniformName uName, size_t& offset, VariableInfo& info);

        void setDefaultUniformImpl(UniformName uName, const Byte* data, GLenum baseType, GLint numComponents, GLsizei count);

        MaterialTypeName name_;
        HardwareProgramPtr prog_;
        bool usesLight_;
        ParamListInfo autoParamListInfo_;
        ParamListInfo paramListInfo_;
    };

    using MaterialTypePtr = std::shared_ptr<MaterialType>;
}

#endif

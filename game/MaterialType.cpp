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

#include "MaterialType.h"
#include "HardwareResourceManager.h"
#include "Logger.h"

namespace af3d
{
    const APropertyTypeEnumImpl<MaterialTypeName, MaterialTypeMax + 1> APropertyType_MaterialTypeName{"MaterialTypeName",
        {
            "Basic",
            "BasicNM",
            "Unlit",
            "UnlitVC",
            "Imm",
            "Outline",
            "Grid",
            "FilterVHS",
            "FilterIrradianceConv",
            "PBR",
            "PBRNM",
            "FilterCube2Equirect",
            "FilterEquirect2Cube",
            "FilterSpecularCM",
            "FilterSpecularLUT",
            "FilterFXAA",
            "FilterToneMapping",
            "FilterGaussianBlur",
            "FilterBloomPass1",
            "FilterBloomPass2",
            "FilterTAA",
            "FilterDownscale",
            "SkyBox",
            "FastPBR",
            "FastPBRNM",
            "ClusterBuild",
            "ClusterCull",
            "Prepass1",
            "Prepass2",
            "PrepassWS"
        }
    };

    MaterialTypeName materialTypeWithNM(MaterialTypeName matTypeName)
    {
        switch (matTypeName) {
        case MaterialTypeBasic:
            return MaterialTypeBasicNM;
        case MaterialTypePBR:
            return MaterialTypePBRNM;
        case MaterialTypeFastPBR:
            return MaterialTypeFastPBRNM;
        default:
            return matTypeName;
        }
    }

    bool materialTypeHasNM(MaterialTypeName matTypeName)
    {
        switch (matTypeName) {
        case MaterialTypeBasicNM:
        case MaterialTypePBRNM:
        case MaterialTypeFastPBRNM:
            return true;
        default:
            return false;
        }
    }

    MaterialType::MaterialType(MaterialTypeName name, const HardwareProgramPtr& prog, bool isCompute)
    : name_(name),
      prog_(prog),
      isCompute_(isCompute)
    {
    }

    bool MaterialType::reload(const std::vector<HardwareShaderPtr>& shaders, HardwareContext& ctx)
    {
        for (const auto& shader : shaders) {
            prog_->attachShader(shader, ctx);
        }

        if (!prog_->link(ctx)) {
            return false;
        }

        autoParamListInfo_ = ParamListInfo();
        paramListInfo_ = ParamListInfo();

        for (int i = static_cast<int>(UniformName::FirstAuto); i <= static_cast<int>(UniformName::MaxAuto); ++i) {
            UniformName name = static_cast<UniformName>(i);
            auto it = prog_->activeUniforms().find(name);
            if (it != prog_->activeUniforms().end()) {
                autoParamListInfo_.offsets[name] = autoParamListInfo_.totalSize;
                autoParamListInfo_.totalSize += it->second.sizeInBytes();
            }
        }
        autoParamListInfo_.defaultParamList.resize(autoParamListInfo_.totalSize);

        for (int i = static_cast<int>(UniformName::MaxAuto) + 1; i <= static_cast<int>(UniformName::Max); ++i) {
            UniformName name = static_cast<UniformName>(i);
            auto it = prog_->activeUniforms().find(name);
            if (it != prog_->activeUniforms().end()) {
                paramListInfo_.offsets[name] = paramListInfo_.totalSize;
                paramListInfo_.totalSize += it->second.sizeInBytes();
            }
        }

        paramListInfo_.defaultParamList.resize(paramListInfo_.totalSize);

        return true;
    }

    void MaterialType::setDefaultUniform(UniformName name, float value)
    {
        setDefaultUniformImpl(name, reinterpret_cast<const Byte*>(&value), GL_FLOAT, 1, 1);
    }

    void MaterialType::setDefaultUniform(UniformName name, std::int32_t value)
    {
        setDefaultUniformImpl(name, reinterpret_cast<const Byte*>(&value), GL_INT, 1, 1);
    }

    void MaterialType::setDefaultUniform(UniformName name, std::uint32_t value)
    {
        setDefaultUniformImpl(name, reinterpret_cast<const Byte*>(&value), GL_UNSIGNED_INT, 1, 1);
    }

    void MaterialType::setDefaultUniform(UniformName name, const Vector2f& value)
    {
        setDefaultUniformImpl(name, reinterpret_cast<const Byte*>(value.v), GL_FLOAT, 2, 1);
    }

    void MaterialType::setDefaultUniform(UniformName name, const Vector3f& value)
    {
        setDefaultUniformImpl(name, reinterpret_cast<const Byte*>(value.v), GL_FLOAT, 3, 1);
    }

    void MaterialType::setDefaultUniform(UniformName name, const btVector3& value)
    {
        setDefaultUniform(name, toVector3f(value));
    }

    void MaterialType::setDefaultUniform(UniformName name, const Vector4f& value)
    {
        setDefaultUniformImpl(name, reinterpret_cast<const Byte*>(value.v), GL_FLOAT, 4, 1);
    }

    void MaterialType::setDefaultUniform(UniformName name, const Matrix4f& value)
    {
        setDefaultUniformImpl(name, reinterpret_cast<const Byte*>(value.v), GL_FLOAT, 16, 1);
    }

    bool MaterialType::getDefaultUniform(UniformName name, float& value) const
    {
        auto it = paramListInfo_.defaultUniforms.find(name);
        if (it != paramListInfo_.defaultUniforms.end()) {
            return getDefaultUniformImpl(name, reinterpret_cast<Byte*>(&value), GL_FLOAT, 1, 1);
        } else {
            return false;
        }
    }

    bool MaterialType::getDefaultUniform(UniformName name, std::int32_t& value) const
    {
        auto it = paramListInfo_.defaultUniforms.find(name);
        if (it != paramListInfo_.defaultUniforms.end()) {
            return getDefaultUniformImpl(name, reinterpret_cast<Byte*>(&value), GL_INT, 1, 1);
        } else {
            return false;
        }
    }

    bool MaterialType::getDefaultUniform(UniformName name, std::uint32_t& value) const
    {
        auto it = paramListInfo_.defaultUniforms.find(name);
        if (it != paramListInfo_.defaultUniforms.end()) {
            return getDefaultUniformImpl(name, reinterpret_cast<Byte*>(&value), GL_UNSIGNED_INT, 1, 1);
        } else {
            return false;
        }
    }

    bool MaterialType::getDefaultUniform(UniformName name, Vector2f& value) const
    {
        auto it = paramListInfo_.defaultUniforms.find(name);
        if (it != paramListInfo_.defaultUniforms.end()) {
            return getDefaultUniformImpl(name, reinterpret_cast<Byte*>(value.v), GL_FLOAT, 2, 1);
        } else {
            return false;
        }
    }

    bool MaterialType::getDefaultUniform(UniformName name, Vector3f& value) const
    {
        auto it = paramListInfo_.defaultUniforms.find(name);
        if (it != paramListInfo_.defaultUniforms.end()) {
            return getDefaultUniformImpl(name, reinterpret_cast<Byte*>(value.v), GL_FLOAT, 3, 1);
        } else {
            return false;
        }
    }

    bool MaterialType::getDefaultUniform(UniformName name, btVector3& value) const
    {
        auto it = paramListInfo_.defaultUniforms.find(name);
        if (it != paramListInfo_.defaultUniforms.end()) {
            return getDefaultUniformImpl(name, reinterpret_cast<Byte*>(value.m_floats), GL_FLOAT, 3, 1);
        } else {
            return false;
        }
    }

    bool MaterialType::getDefaultUniform(UniformName name, Vector4f& value) const
    {
        auto it = paramListInfo_.defaultUniforms.find(name);
        if (it != paramListInfo_.defaultUniforms.end()) {
            return getDefaultUniformImpl(name, reinterpret_cast<Byte*>(value.v), GL_FLOAT, 4, 1);
        } else {
            return false;
        }
    }

    bool MaterialType::getDefaultUniform(UniformName name, Matrix4f& value) const
    {
        auto it = paramListInfo_.defaultUniforms.find(name);
        if (it != paramListInfo_.defaultUniforms.end()) {
            return getDefaultUniformImpl(name, reinterpret_cast<Byte*>(value.v), GL_FLOAT, 16, 1);
        } else {
            return false;
        }
    }

    bool MaterialType::checkName(UniformName uName, size_t& offset, VariableInfo& info) const
    {
        if (HardwareProgram::isAuto(uName)) {
            LOG4CPLUS_WARN(logger(), "Material type " << name() << ", bad param " << uName << ": auto");
            return false;
        }

        auto it = paramListInfo_.offsets.find(uName);
        if (it == paramListInfo_.offsets.end()) {
            return false;
        }

        offset = it->second;
        auto jt = prog()->activeUniforms().find(uName);
        runtime_assert(jt != prog()->activeUniforms().end());
        info = jt->second;

        return true;
    }

    void MaterialType::setDefaultUniformImpl(UniformName uName, const Byte* data, GLenum baseType, GLint numComponents, GLsizei count)
    {
        size_t offset = 0;
        VariableInfo info;

        if (!checkName(uName, offset, info)) {
            return;
        }

        const auto& ti = HardwareProgram::getTypeInfo(info.type);

        if (ti.baseType != baseType) {
            LOG4CPLUS_WARN(logger(), "Material type " << name() << " bad param " << uName << ": base type mismatch");
            return;
        }

        if (ti.numComponents != numComponents) {
            LOG4CPLUS_WARN(logger(), "Material type " << name() << " bad param " << uName << ": num components mismatch");
            return;
        }

        if (count > info.count) {
            LOG4CPLUS_WARN(logger(), "Material type " << name() << " bad param " << uName << ": count too large");
            return;
        }

        std::memcpy(&paramListInfo_.defaultParamList[offset], data, ti.sizeInBytes * count);
        paramListInfo_.defaultUniforms[uName] = count;
    }

    bool MaterialType::getDefaultUniformImpl(UniformName uName, Byte* data, GLenum baseType, GLint numComponents, GLsizei count) const
    {
        size_t offset = 0;
        VariableInfo info;

        if (!checkName(uName, offset, info)) {
            return false;
        }

        const auto& ti = HardwareProgram::getTypeInfo(info.type);

        if (ti.baseType != baseType) {
            LOG4CPLUS_WARN(logger(), "Material type " << name() << " bad param " << uName << ": base type mismatch");
            return false;
        }

        if (ti.numComponents != numComponents) {
            LOG4CPLUS_WARN(logger(), "Material type " << name() << " bad param " << uName << ": num components mismatch");
            return false;
        }

        std::memcpy(data, &paramListInfo_.defaultParamList[offset], ti.sizeInBytes * count);

        return true;
    }
}

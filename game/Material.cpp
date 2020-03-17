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

#include "Material.h"
#include "MaterialManager.h"
#include "Logger.h"
#include <cstring>

namespace af3d
{
    MaterialParams::MaterialParams(const MaterialTypePtr& materialType, bool isAuto)
    : materialType_(materialType),
      isAuto_(isAuto),
      paramList_(materialType_->paramListInfo(isAuto_).totalSize)
    {
    }

    void MaterialParams::setUniform(UniformName name, float value)
    {
        setUniformImpl(name, reinterpret_cast<const Byte*>(&value), GL_FLOAT, 1, 1);
    }

    void MaterialParams::setUniform(UniformName name, std::int32_t value)
    {
        setUniformImpl(name, reinterpret_cast<const Byte*>(&value), GL_INT, 1, 1);
    }

    void MaterialParams::setUniform(UniformName name, std::uint32_t value)
    {
        setUniformImpl(name, reinterpret_cast<const Byte*>(&value), GL_UNSIGNED_INT, 1, 1);
    }

    void MaterialParams::setUniform(UniformName name, const Vector2f& value)
    {
        setUniformImpl(name, reinterpret_cast<const Byte*>(value.v), GL_FLOAT, 2, 1);
    }

    void MaterialParams::setUniform(UniformName name, const Vector3f& value)
    {
        setUniformImpl(name, reinterpret_cast<const Byte*>(value.v), GL_FLOAT, 3, 1);
    }

    void MaterialParams::setUniform(UniformName name, const btVector3& value)
    {
        setUniform(name, toVector3f(value));
    }

    void MaterialParams::setUniform(UniformName name, const Vector4f& value)
    {
        setUniformImpl(name, reinterpret_cast<const Byte*>(value.v), GL_FLOAT, 4, 1);
    }

    void MaterialParams::setUniform(UniformName name, const Matrix4f& value)
    {
        setUniformImpl(name, reinterpret_cast<const Byte*>(value.v), GL_FLOAT, 16, 1);
    }

    void MaterialParams::apply(HardwareContext& ctx) const
    {
        const auto& activeUniforms = materialType_->prog()->activeUniforms();
        const auto& paramListInfo = materialType_->paramListInfo(isAuto_);
        for (const auto& kv : paramListInfo.offsets) {
            auto jt = activeUniforms.find(kv.first);
            runtime_assert(jt != activeUniforms.end());

            const Byte* data = nullptr;
            GLsizei count = 0;
            auto it = uniforms_.find(kv.first);
            if (it == uniforms_.end()) {
                data = &paramListInfo.defaultParamList[0];
                it = paramListInfo.defaultUniforms.find(kv.first);
                if (it == paramListInfo.defaultUniforms.end()) {
                    // No default, just skip, possibly keeping an old value bound.
                    continue;
                } else {
                    count = it->second;
                }
            } else {
                data = &paramList_[0];
                count = it->second;
            }

            switch (jt->second.type) {
            case GL_FLOAT:
                ogl.Uniform1fv(jt->second.location, count, (const GLfloat*)&data[kv.second]);
                break;
            case GL_FLOAT_VEC4:
                ogl.Uniform4fv(jt->second.location, count, (const GLfloat*)&data[kv.second]);
                break;
            case GL_FLOAT_MAT4:
                ogl.UniformMatrix4fv(jt->second.location, count, GL_FALSE, (const GLfloat*)&data[kv.second]);
                break;
            default:
                runtime_assert(false);
                break;
            }
        }
    }

    bool MaterialParams::checkName(UniformName name, size_t& offset, VariableInfo& info)
    {
        if (HardwareProgram::isAuto(name) ^ isAuto_) {
            LOG4CPLUS_WARN(logger(), "Material type " << materialType_->name() << ", auto = " << isAuto_ << " bad param " << name << ": group mismatch");
            return false;
        }

        const auto& offsets = materialType_->paramListInfo(isAuto_).offsets;

        auto it = offsets.find(name);
        if (it == offsets.end()) {
            LOG4CPLUS_WARN(logger(), "Material type " << materialType_->name() << ", auto = " << isAuto_ << " bad param " << name << ": not used");
            return false;
        }

        offset = it->second;
        auto jt = materialType_->prog()->activeUniforms().find(name);
        runtime_assert(jt != materialType_->prog()->activeUniforms().end());
        info = jt->second;

        return true;
    }

    void MaterialParams::setUniformImpl(UniformName name, const Byte* data, GLenum baseType, GLint numComponents, GLsizei count)
    {
        size_t offset = 0;
        VariableInfo info;

        if (!checkName(name, offset, info)) {
            return;
        }

        const auto& ti = HardwareProgram::getTypeInfo(info.type);

        if (ti.baseType != baseType) {
            LOG4CPLUS_WARN(logger(), "Material type " << materialType_->name() << " bad param " << name << ": base type mismatch");
            return;
        }

        if (ti.numComponents != numComponents) {
            LOG4CPLUS_WARN(logger(), "Material type " << materialType_->name() << " bad param " << name << ": num components mismatch");
            return;
        }

        if (count > info.count) {
            LOG4CPLUS_WARN(logger(), "Material type " << materialType_->name() << " bad param " << name << ": count too large");
            return;
        }

        std::memcpy(&paramList_[offset], data, ti.sizeInBytes * count);
        uniforms_[name] = count;
    }

    Material::Material(MaterialManager* mgr, const std::string& name, const MaterialTypePtr& type)
    : Resource(name),
      mgr_(mgr),
      type_(type),
      params_(type, false)
    {
        btAssert(type);
    }

    Material::~Material()
    {
        mgr_->onMaterialDestroy(this);
    }

    MaterialPtr Material::clone(const std::string& newName) const
    {
        auto cloned = std::make_shared<Material>(mgr_, newName, type_);
        cloned->tbs_ = tbs_;
        cloned->params_ = params_;
        cloned->blendingParams_ = blendingParams_;
        cloned->depthTest_ = depthTest_;
        cloned->depthValue_ = depthValue_;
        if (!mgr_->onMaterialClone(cloned)) {
            return MaterialPtr();
        }
        return cloned;
    }

    const TextureBinding& Material::textureBinding(SamplerName samplerName) const
    {
        return tbs_[static_cast<int>(samplerName)];
    }

    void Material::setTextureBinding(SamplerName samplerName, const TextureBinding& value)
    {
        if (!type_->prog()->samplers()[samplerName]) {
            LOG4CPLUS_WARN(logger(), "Material type " << type_->name() << " bad sampler " << samplerName << ": not used");
            return;
        }

        tbs_[static_cast<int>(samplerName)] = value;
    }

    const BlendingParams& Material::blendingParams() const
    {
        return blendingParams_;
    }

    void Material::setBlendingParams(const BlendingParams& value)
    {
        blendingParams_ = value;
    }

    bool Material::depthTest() const
    {
        return depthTest_;
    }

    void Material::setDepthTest(bool value)
    {
        depthTest_ = value;
    }

    float Material::depthValue() const
    {
        return depthValue_;
    }

    void Material::setDepthValue(float value)
    {
        depthValue_ = value;
    }
}

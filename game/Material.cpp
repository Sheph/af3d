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
    ACLASS_DEFINE_BEGIN(Material, Resource)
    ACLASS_DEFINE_END(Material)

    MaterialParams::MaterialParams(const MaterialTypePtr& materialType, bool isAuto)
    : materialType_(materialType),
      isAuto_(isAuto),
      paramList_(materialType_->paramListInfo(isAuto_).totalSize)
    {
    }

    void MaterialParams::setUniform(UniformName name, float value, bool quiet)
    {
        setUniformImpl(name, reinterpret_cast<const Byte*>(&value), GL_FLOAT, 1, 1, quiet);
    }

    void MaterialParams::setUniform(UniformName name, const std::vector<float>& value, bool quiet)
    {
        setUniformImpl(name, reinterpret_cast<const Byte*>(&value[0]), GL_FLOAT, 1, value.size(), quiet);
    }

    void MaterialParams::setUniform(UniformName name, std::int32_t value, bool quiet)
    {
        setUniformImpl(name, reinterpret_cast<const Byte*>(&value), GL_INT, 1, 1, quiet);
    }

    void MaterialParams::setUniform(UniformName name, std::uint32_t value, bool quiet)
    {
        setUniformImpl(name, reinterpret_cast<const Byte*>(&value), GL_UNSIGNED_INT, 1, 1, quiet);
    }

    void MaterialParams::setUniform(UniformName name, const Vector2f& value, bool quiet)
    {
        setUniformImpl(name, reinterpret_cast<const Byte*>(value.v), GL_FLOAT, 2, 1, quiet);
    }

    void MaterialParams::setUniform(UniformName name, const Vector3f& value, bool quiet)
    {
        setUniformImpl(name, reinterpret_cast<const Byte*>(value.v), GL_FLOAT, 3, 1, quiet);
    }

    void MaterialParams::setUniform(UniformName name, const btVector3& value, bool quiet)
    {
        setUniform(name, toVector3f(value), quiet);
    }

    void MaterialParams::setUniform(UniformName name, const Vector4f& value, bool quiet)
    {
        setUniformImpl(name, reinterpret_cast<const Byte*>(value.v), GL_FLOAT, 4, 1, quiet);
    }

    void MaterialParams::setUniform(UniformName name, const Matrix3f& value, bool quiet)
    {
        setUniformImpl(name, reinterpret_cast<const Byte*>(value.v), GL_FLOAT, 9, 1, quiet);
    }

    void MaterialParams::setUniform(UniformName name, const Matrix4f& value, bool quiet)
    {
        setUniformImpl(name, reinterpret_cast<const Byte*>(value.v), GL_FLOAT, 16, 1, quiet);
    }

    bool MaterialParams::getUniform(UniformName name, float& value, bool withDefault) const
    {
        auto it = uniforms_.find(name);
        if (it != uniforms_.end()) {
            return getUniformImpl(name, reinterpret_cast<Byte*>(&value), GL_FLOAT, 1, 1);
        } else {
            return !withDefault || materialType_->getDefaultUniform(name, value);
        }
    }

    bool MaterialParams::getUniform(UniformName name, std::int32_t& value, bool withDefault) const
    {
        auto it = uniforms_.find(name);
        if (it != uniforms_.end()) {
            return getUniformImpl(name, reinterpret_cast<Byte*>(&value), GL_INT, 1, 1);
        } else {
            return !withDefault || materialType_->getDefaultUniform(name, value);
        }
    }

    bool MaterialParams::getUniform(UniformName name, std::uint32_t& value, bool withDefault) const
    {
        auto it = uniforms_.find(name);
        if (it != uniforms_.end()) {
            return getUniformImpl(name, reinterpret_cast<Byte*>(&value), GL_UNSIGNED_INT, 1, 1);
        } else {
            return !withDefault || materialType_->getDefaultUniform(name, value);
        }
    }

    bool MaterialParams::getUniform(UniformName name, Vector2f& value, bool withDefault) const
    {
        auto it = uniforms_.find(name);
        if (it != uniforms_.end()) {
            return getUniformImpl(name, reinterpret_cast<Byte*>(value.v), GL_FLOAT, 2, 1);
        } else {
            return !withDefault || materialType_->getDefaultUniform(name, value);
        }
    }

    bool MaterialParams::getUniform(UniformName name, Vector3f& value, bool withDefault) const
    {
        auto it = uniforms_.find(name);
        if (it != uniforms_.end()) {
            return getUniformImpl(name, reinterpret_cast<Byte*>(value.v), GL_FLOAT, 3, 1);
        } else {
            return !withDefault || materialType_->getDefaultUniform(name, value);
        }
    }

    bool MaterialParams::getUniform(UniformName name, btVector3& value, bool withDefault) const
    {
        auto it = uniforms_.find(name);
        if (it != uniforms_.end()) {
            return getUniformImpl(name, reinterpret_cast<Byte*>(value.m_floats), GL_FLOAT, 3, 1);
        } else {
            return !withDefault || materialType_->getDefaultUniform(name, value);
        }
    }

    bool MaterialParams::getUniform(UniformName name, Vector4f& value, bool withDefault) const
    {
        auto it = uniforms_.find(name);
        if (it != uniforms_.end()) {
            return getUniformImpl(name, reinterpret_cast<Byte*>(value.v), GL_FLOAT, 4, 1);
        } else {
            return !withDefault || materialType_->getDefaultUniform(name, value);
        }
    }

    bool MaterialParams::getUniform(UniformName name, Matrix4f& value, bool withDefault) const
    {
        auto it = uniforms_.find(name);
        if (it != uniforms_.end()) {
            return getUniformImpl(name, reinterpret_cast<Byte*>(value.v), GL_FLOAT, 16, 1);
        } else {
            return !withDefault || materialType_->getDefaultUniform(name, value);
        }
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
            case GL_INT:
                ogl.Uniform1iv(jt->second.location, count, (const GLint*)&data[kv.second]);
                break;
            case GL_FLOAT_VEC2:
                ogl.Uniform2fv(jt->second.location, count, (const GLfloat*)&data[kv.second]);
                break;
            case GL_FLOAT_VEC3:
                ogl.Uniform3fv(jt->second.location, count, (const GLfloat*)&data[kv.second]);
                break;
            case GL_FLOAT_VEC4:
                ogl.Uniform4fv(jt->second.location, count, (const GLfloat*)&data[kv.second]);
                break;
            case GL_FLOAT_MAT3:
                ogl.UniformMatrix3fv(jt->second.location, count, GL_FALSE, (const GLfloat*)&data[kv.second]);
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

    void MaterialParams::convert(MaterialParams& other) const
    {
        if (isAuto_ ^ other.isAuto_) {
            LOG4CPLUS_WARN(logger(), "Converting incompatible params (auto / non-auto)");
            return;
        }

        const auto& activeUniforms = materialType_->prog()->activeUniforms();
        const auto& offsets = materialType_->paramListInfo(isAuto_).offsets;

        for (const auto& kv : uniforms_) {
            auto jt = offsets.find(kv.first);
            runtime_assert(jt != offsets.end());
            auto kt = activeUniforms.find(kv.first);
            runtime_assert(kt != activeUniforms.end());
            const auto& ti = HardwareProgram::getTypeInfo(kt->second.type);

            other.setUniformImpl(kv.first, &paramList_[jt->second], ti.baseType, ti.numComponents, kv.second, true);
        }
    }

    bool MaterialParams::checkName(UniformName name, size_t& offset, VariableInfo& info, bool quiet) const
    {
        if (HardwareProgram::isAuto(name) ^ isAuto_) {
            LOG4CPLUS_WARN(logger(), "Material type " << materialType_->name() << ", auto = " << isAuto_ << " bad param " << name << ": group mismatch");
            return false;
        }

        const auto& offsets = materialType_->paramListInfo(isAuto_).offsets;

        auto it = offsets.find(name);
        if (it == offsets.end()) {
            if (!quiet) {
                LOG4CPLUS_WARN(logger(), "Material type " << materialType_->name() << ", auto = " << isAuto_ << " bad param " << name << ": not used");
            }
            return false;
        }

        offset = it->second;
        auto jt = materialType_->prog()->activeUniforms().find(name);
        runtime_assert(jt != materialType_->prog()->activeUniforms().end());
        info = jt->second;

        return true;
    }

    void MaterialParams::setUniformImpl(UniformName name, const Byte* data, GLenum baseType, GLint numComponents, GLsizei count, bool quiet)
    {
        size_t offset = 0;
        VariableInfo info;

        if (!checkName(name, offset, info, quiet)) {
            return;
        }

        const auto& ti = HardwareProgram::getTypeInfo(info.type);

        if (ti.baseType != baseType) {
            if (!quiet) {
                LOG4CPLUS_WARN(logger(), "Material type " << materialType_->name() << " bad param " << name << ": base type mismatch");
            }
            return;
        }

        if (ti.numComponents != numComponents) {
            if (!quiet) {
                LOG4CPLUS_WARN(logger(), "Material type " << materialType_->name() << " bad param " << name << ": num components mismatch");
            }
            return;
        }

        if (count > info.count) {
            if (!quiet) {
                LOG4CPLUS_WARN(logger(), "Material type " << materialType_->name() << " bad param " << name << ": count too large");
            }
            return;
        }

        std::memcpy(&paramList_[offset], data, ti.sizeInBytes * count);
        uniforms_[name] = count;
    }

    bool MaterialParams::getUniformImpl(UniformName name, Byte* data, GLenum baseType, GLint numComponents, GLsizei count) const
    {
        size_t offset = 0;
        VariableInfo info;

        if (!checkName(name, offset, info, false)) {
            return false;
        }

        const auto& ti = HardwareProgram::getTypeInfo(info.type);

        if (ti.baseType != baseType) {
            LOG4CPLUS_WARN(logger(), "Material type " << materialType_->name() << " bad param " << name << ": base type mismatch");
            return false;
        }

        if (ti.numComponents != numComponents) {
            LOG4CPLUS_WARN(logger(), "Material type " << materialType_->name() << " bad param " << name << ": num components mismatch");
            return false;
        }

        std::memcpy(data, &paramList_[offset], ti.sizeInBytes * count);

        return true;
    }

    Material::Material(MaterialManager* mgr, const std::string& name, const MaterialTypePtr& type)
    : Resource(AClass_Material, name),
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

    const AClass& Material::staticKlass()
    {
        return AClass_Material;
    }

    AObjectPtr Material::create(const APropertyValueMap& propVals)
    {
        return MaterialPtr();
    }

    MaterialPtr Material::clone(const std::string& newName) const
    {
        auto cloned = std::make_shared<Material>(mgr_, newName, type_);
        cloned->tbs_ = tbs_;
        cloned->params_ = params_;
        if (!cloneImpl(cloned)) {
            return MaterialPtr();
        }
        return cloned;
    }

    MaterialPtr Material::convert(MaterialTypeName matTypeName, const std::string& newName) const
    {
        auto cloned = std::make_shared<Material>(mgr_, newName, mgr_->getMaterialType(matTypeName));

        const auto& samplers = cloned->type()->prog()->samplers();
        for (int i = 0; i <= static_cast<int>(SamplerName::Max); ++i) {
            SamplerName sName = static_cast<SamplerName>(i);
            if (samplers[sName]) {
                cloned->setTextureBinding(sName, textureBinding(sName));
            }
        }

        params_.convert(cloned->params_);

        if (!cloneImpl(cloned)) {
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

    bool Material::depthWrite() const
    {
        return depthWrite_;
    }

    void Material::setDepthWrite(bool value)
    {
        depthWrite_ = value;
    }

    GLenum Material::cullFaceMode() const
    {
        return cullFaceMode_;
    }

    void Material::setCullFaceMode(GLenum value)
    {
        cullFaceMode_ = value;
    }

    float Material::timeOffset() const
    {
        return timeOffset_;
    }

    void Material::setTimeOffset(float value)
    {
        timeOffset_ = value;
    }

    bool Material::cloneImpl(const MaterialPtr& cloned) const
    {
        cloned->blendingParams_ = blendingParams_;
        cloned->depthTest_ = depthTest_;
        cloned->depthWrite_ = depthWrite_;
        cloned->cullFaceMode_ = cullFaceMode_;
        cloned->timeOffset_ = timeOffset_;
        return mgr_->onMaterialClone(cloned);
    }
}

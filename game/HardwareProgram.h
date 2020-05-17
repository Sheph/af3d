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

#ifndef _HARDWARE_PROGRAM_H_
#define _HARDWARE_PROGRAM_H_

#include "HardwareShader.h"
#include "af3d/Utils.h"
#include "af3d/EnumSet.h"
#include <type_traits>

namespace af3d
{
    enum class VertexAttribName
    {
        Pos = 0,
        UV,
        Normal,
        Color, // Unlit shader only!
        Tangent,
        Bitangent,
        Max = Bitangent
    };

    enum class UniformName
    {
        ViewProjMatrix = 0,
        ModelViewProjMatrix,
        ModelMatrix,
        PrevStableMatrix,
        CurStableMatrix,
        EyePos,
        LightPos, // LightPos = vec4(x,y,z,lightType)
        LightColor, // LightColor = vec3(r,g,b)
        LightDir, // length = distance
        LightCutoffCos,
        LightCutoffInnerCos,
        LightPower,
        ViewportSize,
        Time,
        SpecularCMLevels,
        Dt,
        RealDt,
        FirstAuto = ViewProjMatrix,
        MaxAuto = RealDt,
        MainColor, // Light only!
        SpecularColor, // Light only!
        Shininess, // Light only!
        GridPos,
        GridRight,
        GridUp,
        GridStep,
        GridXColor,
        GridYColor,
        Roughness,
        MipLevel,
        GaussianKernel,
        GaussianOffset,
        GaussianMSize,
        GaussianDir,
        Threshold,
        Strength,
        ArgJitter,
        ArgPrevViewProjMatrix,
        ArgViewProjMatrix,
        SampleWeights,
        LowpassWeights,
        PlusWeights,
        RandomOffset,
        VelocityScaling,
        Max = VelocityScaling
    };

    enum class SamplerName
    {
        Main = 0,
        Normal,
        Specular,
        Noise,
        Roughness,
        Metalness,
        AO,
        Irradiance,
        SpecularCM,
        SpecularLUT,
        Prev,
        Depth,
        Max = Depth
    };

    struct VariableTypeInfo
    {
        VariableTypeInfo() = default;
        VariableTypeInfo(GLenum baseType, GLint numComponents, GLint sizeInBytes)
        : baseType(baseType),
          numComponents(numComponents),
          sizeInBytes(sizeInBytes) {}

        GLenum baseType;
        GLint numComponents;
        GLint sizeInBytes;
    };

    static_assert(std::is_pod<VariableTypeInfo>::value, "VariableTypeInfo must be POD type");

    struct VariableInfo
    {
        VariableInfo() = default;
        VariableInfo(GLenum type, GLint count, GLint location)
        : type(type),
          count(count),
          location(location) {}

        GLint sizeInBytes() const;

        GLenum type;
        GLint count;
        GLint location;
    };

    static_assert(std::is_pod<VariableInfo>::value, "VariableInfo must be POD type");

    class HardwareProgram : public HardwareResource
    {
    public:
        using ActiveAttribs = EnumUnorderedMap<VertexAttribName, VariableInfo>;
        using ActiveUniforms = EnumUnorderedMap<UniformName, VariableInfo>;
        using Samplers = EnumSet<SamplerName>;

        explicit HardwareProgram(HardwareResourceManager* mgr);
        ~HardwareProgram();

        static inline bool isAuto(UniformName name) { return name <= UniformName::MaxAuto; }

        static GLint getVertexAttribLocation(VertexAttribName name);

        static const VariableTypeInfo& getTypeInfo(GLenum type);

        void invalidate(HardwareContext& ctx) override;

        GLuint id(HardwareContext& ctx) const override;

        void attachShader(const HardwareShaderPtr& shader, HardwareContext& ctx);

        bool link(HardwareContext& ctx);

        inline const ActiveAttribs& activeAttribs() const { return activeAttribs_; }
        inline const ActiveUniforms& activeUniforms() const { return activeUniforms_; }
        inline const Samplers& samplers() const { return samplers_; }

    private:
        bool fillAttribs(HardwareContext& ctx);

        bool fillUniforms(HardwareContext& ctx);

        std::vector<std::pair<GLuint, HardwareShaderPtr>> shaders_;
        GLuint id_ = 0;
        ActiveAttribs activeAttribs_;
        ActiveUniforms activeUniforms_;
        Samplers samplers_;
    };

    using HardwareProgramPtr = std::shared_ptr<HardwareProgram>;
}

inline std::ostream& operator <<(std::ostream& os, af3d::UniformName name)
{
    return (os << static_cast<int>(name));
}

inline std::ostream& operator <<(std::ostream& os, af3d::SamplerName name)
{
    return (os << static_cast<int>(name));
}

#endif

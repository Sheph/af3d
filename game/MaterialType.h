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

namespace af3d
{
    struct SamplerParam
    {
        texFilter;
        texWrapU;
        texWrapV;
    };

    class MaterialType : boost::noncopyable
    {
    public:
        MaterialType(const std::string& name, const HardwareProgramPtr& prog);
        ~MaterialType() = default;

        //void attachShader(const HardwareShaderPtr& shader);

        //void addVarying(VertexAttribName name);

        //void addUniform(const String& name, GpuConstantType constType, size_t arraySize = 1);

        //void addAutoUniform(AutoUniformName name);

        static HardwareShader::VariableInfo getUniformVariableInfo(UniformName name);

        static HardwareShader::VariableInfo getVaryingVariableInfo(UniformName name);

    private:
        std::string name_;
        HardwareProgramPtr prog_;
        EnumSet<VertexAttribName> attribs_;
        EnumSet<UniformName> uniforms_;
        std::vector<SamplerParam> samplers_;
    };

    using MaterialTypePtr = std::shared_ptr<MaterialType>;
}

#endif

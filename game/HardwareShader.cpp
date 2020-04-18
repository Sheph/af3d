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

#include "HardwareShader.h"
#include "Logger.h"

namespace af3d
{
    HardwareShader::HardwareShader(HardwareResourceManager* mgr, Type type)
    : HardwareResource(mgr),
      type_(type)
    {
    }

    HardwareShader::~HardwareShader()
    {
        GLuint id = id_;
        if (id != 0) {
            cleanup([id](HardwareContext& ctx) {
                ogl.DeleteShader(id);
            });
        } else {
            cleanup();
        }
    }

    GLenum HardwareShader::glShaderType(Type type)
    {
        switch (type) {
        case Type::Fragment:
            return GL_FRAGMENT_SHADER;
        default:
            btAssert(false);
        case Type::Vertex:
            return GL_VERTEX_SHADER;
        }
    }

    void HardwareShader::invalidate(HardwareContext& ctx)
    {
        id_ = 0;
    }

    GLuint HardwareShader::id(HardwareContext& ctx) const
    {
        return id_;
    }

    bool HardwareShader::compile(const std::string& source, HardwareContext& ctx)
    {
        if (id_ == 0) {
            id_ = ogl.CreateShader(glShaderType(type_));
            btAssert(id_ != 0);
        }

        const char* str = source.c_str();

        ogl.ShaderSource(id_, 1, &str, nullptr);
        ogl.CompileShader(id_);

        GLint tmp = 0;

        ogl.GetShaderiv(id_, GL_COMPILE_STATUS, &tmp);

        if (!tmp) {
            tmp = 0;
            ogl.GetShaderiv(id_, GL_INFO_LOG_LENGTH, &tmp);

            std::string buff(tmp, 0);

            ogl.GetShaderInfoLog(id_, buff.size(), nullptr, &buff[0]);

            LOG4CPLUS_ERROR(logger(), "Unable to compile shader (type = " << static_cast<int>(type_) << ") - " << buff);

            return false;
        }

        return true;
    }
}

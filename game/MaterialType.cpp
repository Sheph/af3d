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

namespace af3d
{
    MaterialType::MaterialType(MaterialTypeName name, const HardwareProgramPtr& prog)
    : name_(name),
      prog_(prog)
    {
    }

    bool MaterialType::reload(const std::string& vertSource, const std::string& fragSource, HardwareContext& ctx)
    {
        auto vertexShader = hwManager.createShader(HardwareShader::Type::Vertex);

        if (!vertexShader->compile(vertSource, ctx)) {
            return false;
        }

        auto fragmentShader = hwManager.createShader(HardwareShader::Type::Fragment);

        if (!fragmentShader->compile(fragSource, ctx)) {
            return false;
        }

        prog_->attachShader(vertexShader, ctx);
        prog_->attachShader(fragmentShader, ctx);

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

        for (int i = static_cast<int>(UniformName::MaxAuto) + 1; i <= static_cast<int>(UniformName::Max); ++i) {
            UniformName name = static_cast<UniformName>(i);
            auto it = prog_->activeUniforms().find(name);
            if (it != prog_->activeUniforms().end()) {
                paramListInfo_.offsets[name] = paramListInfo_.totalSize;
                paramListInfo_.totalSize += it->second.sizeInBytes();
            }
        }

        return true;
    }
}

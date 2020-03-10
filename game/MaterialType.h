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

namespace af3d
{
    enum MaterialTypeName
    {
        MaterialType2DDefault = 0, // 2D with pos, color and one texture
        MaterialType2DColor,       // 2D with pos and color
        MaterialTypeUnlitDefault,  // Unlit shader, pos, color and one texture
        MaterialTypeUnlitColor,    // Unlit shader, pos and color
        MaterialTypeBasicDefault,  // Basic lighting, pos, normal and one texture
        MaterialTypeBasicColor,    // Basic lighting, pos and normal
        MaterialTypeFirst = MaterialType2DDefault,
        MaterialTypeMax = MaterialTypeBasicColor
    };

    class MaterialType : boost::noncopyable
    {
    public:
        MaterialType(MaterialTypeName name, const HardwareProgramPtr& prog);
        ~MaterialType() = default;

        inline MaterialTypeName name() const { return name_; }

        bool reload(const std::string& vertSource, const std::string& fragSource);

    private:
        MaterialTypeName name_;
        HardwareProgramPtr prog_;
        EnumSet<VertexAttribName> attribs_;
        EnumUnorderedMap<UniformName, size_t> uniforms_; // uniform -> param array offset
        int numTextures_;
    };

    using MaterialTypePtr = std::shared_ptr<MaterialType>;
}

#endif

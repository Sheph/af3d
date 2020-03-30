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

#ifndef _UTILS_H_
#define _UTILS_H_

#include "af3d/Types.h"
#include "af3d/Vector2.h"
#include "af3d/Vector4.h"
#include "af3d/Utils.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "imgui.h"
#include <memory>

namespace af3d
{
    using AssimpScenePtr = std::unique_ptr<aiScene>;

    AssimpScenePtr assimpImport(Assimp::Importer& importer,
        const std::string& path,
        std::uint32_t flags);

    inline btVector3 fromAssimp(const aiVector3D& v)
    {
        return btVector3(v.x, v.y, v.z);
    }

    inline Color fromAssimp(const aiColor4D& c)
    {
        return Color(c.r, c.g, c.b, c.a);
    }

    inline Vector2f fromImVec2(const ImVec2& v)
    {
        return Vector2f(v.x, v.y);
    }

    inline ImVec2 toImVec2(const Vector2f& v)
    {
        return ImVec2(v.x(), v.y());
    }
}

#endif

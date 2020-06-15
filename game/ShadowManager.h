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

#ifndef _SHADOW_MANAGER_H_
#define _SHADOW_MANAGER_H_

#include "ShadowMapCSM.h"
#include "Texture.h"
#include "HardwareDataBuffer.h"
#include <set>

namespace af3d
{
    class ShadowManager : boost::noncopyable
    {
    public:
        ShadowManager();
        ~ShadowManager();

        bool addShadowMap(ShadowMapCSM* csm);

        void removeShadowMap(ShadowMapCSM* csm);

        void preSwap();

        inline const TexturePtr& csmTexture() const { return csmTexture_; }
        inline const HardwareDataBufferPtr& csmSSBO() const { return csmSSBO_; }

    private:
        using IndexSet = std::set<int>;

        TexturePtr csmTexture_;
        HardwareDataBufferPtr csmSSBO_;

        std::unordered_set<ShadowMapCSM*> csms_;
        IndexSet csmFreeIndices_;
    };
}

#endif

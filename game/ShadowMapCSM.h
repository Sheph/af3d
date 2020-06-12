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

#ifndef _SHADOW_MAP_CSM_H_
#define _SHADOW_MAP_CSM_H_

#include "ShadowMap.h"
#include "Camera.h"

namespace af3d
{
    class ShadowManager;

    struct CSMRenderTarget
    {
        CSMRenderTarget() = default;
        CSMRenderTarget(const TexturePtr& tex,
            const std::pair<int, int>& layers)
        : tex(tex),
          layers(std::move(layers)) {}

        TexturePtr tex;
        std::pair<int, int> layers;
    };

    class ShadowMapCSM : public ShadowMap
    {
    public:
        explicit ShadowMapCSM(Scene* scene);
        ~ShadowMapCSM();

        inline bool active() const { return mgr_ != nullptr; }
        inline int index() const { return index_; }

        void remove() override;

        void update(const CameraPtr& viewCam);

        // TODO: void setupSSBO(ShaderCSM& sCSM) const;

        /*
         * Internal, do not call.
         * @{
         */

        void adopt(ShadowManager* mgr, int index, const CSMRenderTarget& rt);
        void abandon();

        /*
         * @}
         */

    private:
        struct Split
        {
            CameraPtr cam;
        };

        using Splits = std::vector<Split>;

        ShadowManager* mgr_ = nullptr;
        int index_ = -1;

        Splits splits_;
    };

    using ShadowMapCSMPtr = std::shared_ptr<ShadowMapCSM>;
}

#endif

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

#include "ShadowManager.h"
#include "TextureManager.h"
#include "Settings.h"
#include "Logger.h"

namespace af3d
{
    ShadowManager::ShadowManager()
    : csmTexture_(textureManager.createRenderTexture(TextureType2DArray,
          settings.csm.resolution, settings.csm.resolution, (settings.csm.maxCount * settings.csm.numSplits),
              GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT))
    {
        for (int i = 0; i < static_cast<int>(settings.csm.maxCount); ++i) {
            csmFreeIndices_.insert(i);
        }
    }

    ShadowManager::~ShadowManager()
    {
        btAssert(csms_.empty());
    }

    bool ShadowManager::addShadowMap(ShadowMapCSM* csm)
    {
        if (csmFreeIndices_.empty()) {
            LOG4CPLUS_WARN(logger(), "Too many CSMs...");
            return false;
        }
        int idx = *csmFreeIndices_.begin();
        csmFreeIndices_.erase(csmFreeIndices_.begin());
        csms_.insert(csm);
        csmRemovedIndices_.erase(idx);

        csm->adopt(this, idx, CSMRenderTarget(csmTexture_,
            std::make_pair(idx * settings.csm.numSplits, (idx + 1) * settings.csm.numSplits - 1)));

        return true;
    }

    void ShadowManager::removeShadowMap(ShadowMapCSM* csm)
    {
        if (csms_.erase(csm) <= 0) {
            return;
        }

        btAssert(csm->index() >= 0);
        bool res = csmFreeIndices_.insert(csm->index()).second;
        btAssert(res);
        csmRemovedIndices_.insert(csm->index());

        csm->abandon();
    }
}

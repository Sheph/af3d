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
#include "ShaderDataTypes.h"
#include "HardwareResourceManager.h"
#include "Renderer.h"

namespace af3d
{
    namespace
    {
        struct CSMSSBOUpdate : boost::noncopyable
        {
            HardwareDataBufferPtr ssbo;
            std::vector<ShaderCSM> csms;
        };
    };

    ShadowManager::ShadowManager()
    : csmTexture_(textureManager.createRenderTexture(TextureType2DArray,
          settings.csm.resolution, settings.csm.resolution, (settings.csm.maxCount * settings.csm.numSplits),
              GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT)),
      csmSSBO_(hwManager.createDataBuffer(HardwareBuffer::Usage::DynamicDraw, sizeof(ShaderCSM)))
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

        csm->abandon();
    }

    void ShadowManager::preSwap()
    {
        bool recreate = csmSSBO_->setValid();
        if (csms_.empty() && !recreate) {
            return;
        }

        auto upd = std::make_shared<CSMSSBOUpdate>();
        upd->ssbo = csmSSBO_;
        upd->csms.resize(settings.csm.maxCount);

        for (auto csm : csms_) {
            csm->setupSSBO(upd->csms[csm->index()]);
        }

        renderer.scheduleHwOp([upd](HardwareContext& ctx) {
            upd->ssbo->reload(upd->csms.size(), &upd->csms[0], ctx);
        });
    }
}

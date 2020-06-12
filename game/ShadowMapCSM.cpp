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

#include "ShadowMapCSM.h"
#include "ShadowManager.h"

namespace af3d
{
    ShadowMapCSM::ShadowMapCSM(Scene* scene)
    : ShadowMap(scene)
    {
    }

    ShadowMapCSM::~ShadowMapCSM()
    {
        btAssert(mgr_ == nullptr);
    }

    void ShadowMapCSM::remove()
    {
        if (mgr_) {
            mgr_->removeShadowMap(this);
        }
    }

    void ShadowMapCSM::update(const CameraPtr& viewCam)
    {
        btAssert(!splits_.empty());
        // TODO: setup splits against this view cam.
    }

    void ShadowMapCSM::adopt(ShadowManager* mgr, int index, const CSMRenderTarget& rt)
    {
        btAssert(!mgr_);
        mgr_ = mgr;
        index_ = index;
        // TODO: setup splits.
    }

    void ShadowMapCSM::abandon()
    {
        btAssert(mgr_);
        // TODO: remove splits.
        mgr_ = nullptr;
        index_ = -1;
    }
}

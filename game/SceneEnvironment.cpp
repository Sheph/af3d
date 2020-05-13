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

#include "SceneEnvironment.h"
#include "LightProbeComponent.h"

namespace af3d
{
    SceneEnvironment::~SceneEnvironment()
    {
        btAssert(lightProbes_.empty());
    }

    void SceneEnvironment::update(float realDt, float dt)
    {
        realDt_ = realDt;
        dt_ = dt;
        time_ += dt;
    }

    void SceneEnvironment::preSwap()
    {
        defaultVa_.upload();
    }

    void SceneEnvironment::addLightProbe(LightProbeComponent* probe)
    {
        lightProbes_.insert(probe);
    }

    void SceneEnvironment::removeLightProbe(LightProbeComponent* probe)
    {
        lightProbes_.erase(probe);
    }

    void SceneEnvironment::updateLightProbes()
    {
        for (auto probe : lightProbes_) {
            probe->recreate();
        }
    }

    LightProbeComponent* SceneEnvironment::getLightProbeFor(const btVector3& pos)
    {
        return lightProbes_.empty() ? nullptr : *lightProbes_.begin();
    }
}

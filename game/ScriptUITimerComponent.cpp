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

#include "ScriptUITimerComponent.h"
#include "Utils.h"
#include "Logger.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(ScriptUITimerComponent, UITimerComponent)
    ACLASS_DEFINE_END(ScriptUITimerComponent)

    ScriptUITimerComponent::ScriptUITimerComponent(luabind::object self, float t, int zOrder)
    : UITimerComponent(AClass_ScriptUITimerComponent, t, zOrder),
      self_(self)
    {
    }

    const AClass& ScriptUITimerComponent::staticKlass()
    {
        return AClass_ScriptUITimerComponent;
    }

    AObjectPtr ScriptUITimerComponent::create(const APropertyValueMap& propVals)
    {
        return AObjectPtr();
    }

    void ScriptUITimerComponent::onRegister()
    {
        AF3D_SCRIPT_CALL("onRegister");
    }

    void ScriptUITimerComponent::onUnregister()
    {
        AF3D_SCRIPT_CALL("onUnregister");
        self_ = luabind::object();
    }

    void ScriptUITimerComponent::timeoutReached(float dt)
    {
        AF3D_SCRIPT_CALL("timeoutReached", dt);
    }
}

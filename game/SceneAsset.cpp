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

#include "SceneAsset.h"
#include "Scene.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(SceneAsset, AObject)
    ACLASS_PROPERTY(SceneAsset, Children, AProperty_Children, "Children", ArrayAObject, std::vector<APropertyValue>{}, Hierarchy, 0)
    SCENE_PROPS(SceneAsset)
    ACLASS_DEFINE_END(SceneAsset)

    SceneAsset::SceneAsset()
    : AObject(AClass_SceneAsset)
    {
    }

    const AClass& SceneAsset::staticKlass()
    {
        return AClass_SceneAsset;
    }

    AObjectPtr SceneAsset::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<SceneAsset>();
        obj->propertiesSet(propVals);
        return obj;
    }

    void SceneAsset::apply(Scene* scene)
    {
        scene->camera()->findComponent<CameraComponent>()->renderSettings().setClearColor(clearColor_);
        scene->camera()->findComponent<CameraComponent>()->renderSettings().setAmbientColor(ambientColor_);
        for (const auto& obj : objects_) {
            scene->addObject(obj);
        }
    }
}

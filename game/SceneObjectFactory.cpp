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

#include "SceneObjectFactory.h"
#include "MaterialManager.h"
#include "MeshManager.h"
#include "AssetManager.h"
#include "RenderMeshComponent.h"
#include "RenderQuadComponent.h"
#include "PhysicsBodyComponent.h"
#include "CollisionSensorComponent.h"
#include "Settings.h"
#include "Utils.h"
#include "Logger.h"
#include "af3d/Utils.h"

namespace af3d
{
    SceneObjectFactory sceneObjectFactory;

    template <>
    Single<SceneObjectFactory>* Single<SceneObjectFactory>::single = nullptr;

    bool SceneObjectFactory::init()
    {
        LOG4CPLUS_DEBUG(logger(), "sceneObjectFactory: init...");
        return true;
    }

    void SceneObjectFactory::shutdown()
    {
        LOG4CPLUS_DEBUG(logger(), "sceneObjectFactory: shutdown...");
    }

    SceneObjectPtr SceneObjectFactory::createDummy()
    {
        auto obj = std::make_shared<SceneObject>();
        obj->setBodyType(BodyType::Kinematic);

        obj->addComponent(std::make_shared<PhysicsBodyComponent>());

        return obj;
    }

    SceneObjectPtr SceneObjectFactory::createColoredBox(const btVector3& size, const Color& color1, const Color& color2)
    {
        auto mesh = meshManager.createBoxMesh(size,
            materialManager.matUnlitVCDefault(),
            {
                color2,
                color1,
                color2,
                color1,
                color1,
                color2
            });

        auto obj = std::make_shared<SceneObject>();
        auto rc = std::make_shared<RenderMeshComponent>();
        rc->setMesh(mesh);
        obj->addComponent(rc);

        return obj;
    }

    SceneObjectPtr SceneObjectFactory::createInstance(const std::string& assetPath)
    {
        auto sa = assetManager.getSceneAsset(assetPath);

        if (!sa) {
            return std::make_shared<SceneObject>();
        }

        if (sa->root()) {
            return sa->root();
        }

        auto obj = std::make_shared<SceneObject>();

        sa->apply(obj);

        return obj;
    }

    SceneObjectPtr SceneObjectFactory::createSensor(bool allowSensor)
    {
        auto obj = std::make_shared<SceneObject>();

        auto c = std::make_shared<CollisionSensorComponent>();
        c->setAllowSensor(allowSensor);

        obj->addComponent(c);

        return obj;
    }

    SceneObjectPtr SceneObjectFactory::createTestRef(const SceneObjectPtr& other1, const SceneObjectPtr& other2)
    {
        auto obj = std::make_shared<SceneObject>();

        return obj;
    }

    SCENEOBJECT_DEFINE_BEGIN(Dummy)
    {
        return sceneObjectFactory.createDummy();
    }
    SCENEOBJECT_DEFINE_PROPS(Dummy)
    SCENEOBJECT_DEFINE_END(Dummy)

    SCENEOBJECT_DEFINE_BEGIN(ColoredBox)
    {
        return sceneObjectFactory.createColoredBox(
            params.get("size").toVec3(),
            params.get("color1").toColor(),
            params.get("color2").toColor());
    }
    SCENEOBJECT_DEFINE_PROPS(ColoredBox)
    SCENEOBJECT_PARAM(ColoredBox, "size", "Box size", Vec3f, btVector3(1.0f, 2.0f, 3.0f))
    SCENEOBJECT_PARAM(ColoredBox, "color1", "Box color #1", ColorRGB, Color(1.0f, 0.0f, 0.0f, 1.0f))
    SCENEOBJECT_PARAM(ColoredBox, "color2", "Box color #2", ColorRGB, Color(0.0f, 1.0f, 0.0f, 1.0f))
    SCENEOBJECT_DEFINE_END(ColoredBox)

    SCENEOBJECT_DEFINE_BEGIN(Instance)
    {
        return sceneObjectFactory.createInstance(
            params.get("asset").toString());
    }
    SCENEOBJECT_DEFINE_PROPS(Instance)
    SCENEOBJECT_PARAM(Instance, "asset", "Asset path", String, "empty.af3")
    SCENEOBJECT_DEFINE_END(Instance)

    SCENEOBJECT_DEFINE_BEGIN(Sensor)
    {
        return sceneObjectFactory.createSensor(params.get("allow sensors").toBool());
    }
    SCENEOBJECT_DEFINE_PROPS_NO_RESTRICT(Sensor)
    SCENEOBJECT_PARAM(Sensor, "allow sensors", "React on other sensors", Bool, false)
    SCENEOBJECT_DEFINE_END(Sensor)

    SCENEOBJECT_DEFINE_BEGIN(TestRef)
    {
        return sceneObjectFactory.createTestRef(params.get("other1").toObject<SceneObject>(),
            params.get("other2").toObject<SceneObject>());
    }
    SCENEOBJECT_DEFINE_PROPS(TestRef)
    SCENEOBJECT_PARAM(TestRef, "other1", "Other1", SceneObject, SceneObjectPtr())
    SCENEOBJECT_PARAM(TestRef, "other2", "Other2", SceneObject, SceneObjectPtr())
    SCENEOBJECT_DEFINE_END(TestRef)
}

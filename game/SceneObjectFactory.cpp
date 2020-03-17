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
#include "TextureManager.h"
#include "RenderMeshComponent.h"
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
        /*b2BodyDef bodyDef;

        bodyDef.type = b2_kinematicBody;

        RUBEBodyPtr body = boost::make_shared<RUBEBody>("", bodyDef);

        ComponentPtr component =
            boost::make_shared<PhysicsBodyComponent>(body);*/

        auto obj = std::make_shared<SceneObject>();

        //obj->setBodyDef(body->bodyDef());

        //obj->addComponent(component);

        return obj;
    }

    SceneObjectPtr SceneObjectFactory::createColoredBox(const btVector3& size, const std::string& texPath)
    {
        MaterialPtr material;

        if (texPath.empty()) {
            material = materialManager.getMaterial(MaterialManager::materialUnlitDefault);
        } else {
            auto matName = "_coloredBox_" + texPath;
            material = materialManager.getMaterial(matName);
            if (!material) {
                material = materialManager.createMaterial(MaterialTypeUnlit, matName);
                runtime_assert(material);
                material->setTextureBinding(SamplerName::Main, TextureBinding(textureManager.loadTexture(texPath)));
            }
        }

        auto mesh = meshManager.createBoxMesh(size,
            material,
            {
                Color(1.0f, 0.0f, 0.0f, 1.0f),
                Color(0.0f, 1.0f, 0.0f, 1.0f),
                Color(0.0f, 0.0f, 1.0f, 1.0f),
                Color(1.0f, 1.0f, 0.0f, 1.0f),
                Color(1.0f, 0.0f, 1.0f, 1.0f),
                Color(0.0f, 1.0f, 1.0f, 1.0f)
            });

        auto obj = std::make_shared<SceneObject>();

        auto rc = std::make_shared<RenderMeshComponent>(mesh);

        obj->addComponent(rc);

        return obj;
    }

    SceneObjectPtr SceneObjectFactory::createLitBox(const btVector3& size, const std::string& texPath)
    {
        auto matName = "_litBox_" + texPath;
        auto material = materialManager.getMaterial(matName);
        if (!material) {
            material = materialManager.createMaterial(MaterialTypeBasic, matName);
            runtime_assert(material);
            if (!texPath.empty()) {
                material->setTextureBinding(SamplerName::Main, TextureBinding(textureManager.loadTexture(texPath)));
            }
            material->params().setUniform(UniformName::SpecularColor, Color_one);
            material->params().setUniform(UniformName::Shininess, 50.0f);
        }

        auto mesh = meshManager.createBoxMesh(size, material);

        auto obj = std::make_shared<SceneObject>();

        auto rc = std::make_shared<RenderMeshComponent>(mesh);

        obj->addComponent(rc);

        return obj;
    }
}

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

#include "SceneObjectManager.h"
#include "SceneObject.h"
#include "Scene.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN_ABSTRACT(SceneObjectManager, AObject)
    ACLASS_PROPERTY(SceneObjectManager, Children, AProperty_Children, "Children", ArrayAObject, std::vector<APropertyValue>{}, Hierarchy, 0)
    ACLASS_DEFINE_END(SceneObjectManager)

    const APropertyTypeEnumImpl<SceneObjectType> APropertyType_SceneObjectType{"SceneObjectType",
        {
            "Other",
            "Player",
            "Enemy",
            "PlayerMissile",
            "EnemyMissile",
            "Terrain",
            "Rock",
            "Blocker",
            "Ally",
            "AllyMissile",
            "EnemyBuilding",
            "Gizmo",
            "NeutralMissile",
            "Garbage",
            "Vehicle",
            "Deadbody",
            "Liquid",
        }
    };

    SceneObjectManager::SceneObjectManager(const AClass& klass)
    : AObject(klass)
    {
    }

    const AClass& SceneObjectManager::staticKlass()
    {
        return AClass_SceneObjectManager;
    }

    void SceneObjectManager::addObject(const SceneObjectPtr& obj)
    {
        btAssert(!obj->parent());

        objects_.insert(obj);
        obj->setParent(this);

        if (scene()) {
            registerObject(obj);
        }
    }

    void SceneObjectManager::removeObject(const SceneObjectPtr& obj)
    {
        /*
         * Hold on to this object while
         * removing.
         */
        SceneObjectPtr tmp = obj;

        if (objects_.erase(tmp)) {
            if (scene()) {
                unregisterObject(tmp);
            }

            tmp->setParent(nullptr);
        }
    }

    void SceneObjectManager::removeAllObjects()
    {
        auto tmp = objects_;

        for (const auto& obj : tmp) {
            removeObject(obj);
        }
    }

    std::vector<SceneObjectPtr> SceneObjectManager::getObjects(const std::string& name) const
    {
        std::vector<SceneObjectPtr> res;

        for (const auto& obj : objects_) {
            if (obj->name() == name) {
                res.push_back(obj);
            }
        }

        return res;
    }

    std::vector<SceneObjectPtr> SceneObjectManager::getObjects() const
    {
        std::vector<SceneObjectPtr> res;

        for (const auto& obj : objects_) {
            res.push_back(obj);
        }

        return res;
    }

    void SceneObjectManager::registerObject(const SceneObjectPtr& obj)
    {
        obj->setScene(scene());

        auto tmpObjs = obj->objects_;
        auto tmpComponents = obj->components();

        for (const auto& tmpObj : tmpObjs) {
            registerObject(tmpObj);
        }

        for (const auto& tmpComponent : tmpComponents) {
            scene()->registerComponent(tmpComponent);
        }
    }

    void SceneObjectManager::unregisterObject(const SceneObjectPtr& obj)
    {
        auto tmpObjs = obj->objects_;
        auto tmpComponents = obj->components();

        for (const auto& tmpComponent : tmpComponents) {
            scene()->unregisterComponent(tmpComponent);
        }

        for (const auto& tmpObj : tmpObjs) {
            unregisterObject(tmpObj);
        }

        obj->setScene(nullptr);
    }
}

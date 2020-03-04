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

#ifndef _SCENEOBJECTMANAGER_H_
#define _SCENEOBJECTMANAGER_H_

#include "af3d/Types.h"
#include "af3d/EnumSet.h"
#include <boost/noncopyable.hpp>
#include <unordered_set>
#include <memory>

namespace af3d
{
    class Scene;

    class SceneObject;
    using SceneObjectPtr = std::shared_ptr<SceneObject>;

    enum class SceneObjectType
    {
        Other = 0,
        Player = 1,
        Enemy = 2,
        PlayerMissile = 3,
        EnemyMissile = 4,
        Terrain = 5,
        Rock = 6,
        Blocker = 7,
        Ally = 8,
        AllyMissile = 9,
        EnemyBuilding = 10,
        Gizmo = 11,
        NeutralMissile = 12,
        Garbage = 13,
        Vehicle = 14,
        Deadbody = 15,
        Liquid = 16,
        Max = Liquid
    };

    using SceneObjectTypes = EnumSet<SceneObjectType, static_cast<int>(SceneObjectType::Max) + 1>;

    class SceneObjectManager : boost::noncopyable
    {
    public:
        SceneObjectManager() = default;
        virtual ~SceneObjectManager() = default;

        void addObject(const SceneObjectPtr& obj);

        void removeObject(const SceneObjectPtr& obj);

        void removeAllObjects();

        inline const std::unordered_set<SceneObjectPtr>& objects() const { return objects_; }

        std::vector<SceneObjectPtr> getObjects(const std::string& name) const;

        std::vector<SceneObjectPtr> getObjects() const;

        inline SceneObjectManager* parent() { return parent_; }
        inline void setParent(SceneObjectManager* value) { parent_ = value; }

        inline Scene* scene() { return scene_; }
        inline void setScene(Scene* value) { scene_ = value; }

    private:
        void registerObject(const SceneObjectPtr& obj);

        void unregisterObject(const SceneObjectPtr& obj);

        SceneObjectManager* parent_ = nullptr;

        Scene* scene_ = nullptr;

        std::unordered_set<SceneObjectPtr> objects_;
    };
}

#endif

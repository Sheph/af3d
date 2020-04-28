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

#ifndef _COLLISIONFILTER_H_
#define _COLLISIONFILTER_H_

#include "SceneObjectManager.h"
#include <boost/noncopyable.hpp>
#include <unordered_set>
#include "bullet/btBulletDynamicsCommon.h"

namespace af3d
{
    class SceneObject;

    class CollisionFilter : boost::noncopyable
    {
    public:
        CollisionFilter() = default;
        virtual ~CollisionFilter() = default;

        virtual bool shouldCollideWith(btCollisionObject* thisOne, btCollisionObject* other) const = 0;
    };

    using CollisionFilterPtr = std::shared_ptr<CollisionFilter>;

    class CollisionCookieFilter : public CollisionFilter
    {
    public:
        CollisionCookieFilter() = default;
        ~CollisionCookieFilter() = default;

        bool shouldCollideWith(btCollisionObject* thisOne, btCollisionObject* other) const override;

        void add(ACookie cookie);

    private:
        std::unordered_set<ACookie> cookies_;
    };
}

#endif

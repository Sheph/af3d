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

#ifndef _PHYSICS_DEBUG_DRAW_H_
#define _PHYSICS_DEBUG_DRAW_H_

#include "af3d/Types.h"
#include <boost/noncopyable.hpp>
#include "bullet/LinearMath/btIDebugDraw.h"

namespace af3d
{
    class RenderList;

    class PhysicsDebugDraw : public btIDebugDraw,
        boost::noncopyable
    {
    public:
        PhysicsDebugDraw() = default;
        ~PhysicsDebugDraw() = default;

        void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;

        void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override;

        void reportErrorWarning(const char* warningString) override;

        void draw3dText(const btVector3& location, const char* textString) override;

        void setDebugMode(int debugMode) override;

        int getDebugMode() const override;

        void clearLines() override;

        void flushLines() override;

        inline void setRenderList(RenderList* value) { rl_ = value; }

    private:
        struct Line
        {
            Line(const btVector3& from, const btVector3& to, const btVector3& color)
            : from(from),
              to(to),
              color(color) {}

            btVector3 from;
            btVector3 to;
            btVector3 color;
        };

        int debugMode_ = 0;
        std::vector<Line> lines_;
        RenderList* rl_ = nullptr;
    };
}

#endif

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

#include "PhysicsDebugDraw.h"
#include "MaterialManager.h"
#include "RenderList.h"
#include "Logger.h"

namespace af3d
{
    void PhysicsDebugDraw::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
    {
        lines_.emplace_back(from, to, color);
    }

    void PhysicsDebugDraw::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
    {
        drawLine(PointOnB, PointOnB + normalOnB * distance, color);
        btVector3 ncolor(0, 0, 0);
        drawLine(PointOnB, PointOnB + normalOnB * 0.01, ncolor);
    }

    void PhysicsDebugDraw::drawTransform(const btTransform& transform, btScalar orthoLen)
    {
        btVector3 start = transform.getOrigin();
        drawLine(start, start + transform.getBasis() * btVector3_right * orthoLen, btVector3(btScalar(1.), btScalar(0.3), btScalar(0.3)));
        drawLine(start, start + transform.getBasis() * btVector3_up * orthoLen, btVector3(btScalar(0.3), btScalar(1.), btScalar(0.3)));
        drawLine(start, start + transform.getBasis() * btVector3_forward * orthoLen, btVector3(btScalar(0.3), btScalar(0.3), btScalar(1.)));
    }

    void PhysicsDebugDraw::reportErrorWarning(const char* warningString)
    {
        LOG4CPLUS_WARN(logger(), "PhysicsDebugDraw: " << warningString);
    }

    void PhysicsDebugDraw::draw3dText(const btVector3& location, const char* textString)
    {
        LOG4CPLUS_DEBUG(logger(), "draw3dText: " << location << ", " << textString);
    }

    void PhysicsDebugDraw::setDebugMode(int debugMode)
    {
        debugMode_ = debugMode;
    }

    int PhysicsDebugDraw::getDebugMode() const
    {
        return debugMode_;
    }

    void PhysicsDebugDraw::clearLines()
    {
        lines_.clear();
    }

    void PhysicsDebugDraw::flushLines()
    {
        btAssert(rl_);

        auto rop = rl_->addGeometry(materialManager.matImmDefault(false, false), GL_LINES);

        for (const auto& line : lines_) {
            rop.addVertex(line.from, Vector2f_zero, Color(line.color, 1.0f));
            rop.addVertex(line.to, Vector2f_zero, Color(line.color, 1.0f));
        }

        lines_.clear();
    }
}

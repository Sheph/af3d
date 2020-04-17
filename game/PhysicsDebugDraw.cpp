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
#include "bullet/BulletCollision/CollisionShapes/btConvexPolyhedron.h"

namespace af3d
{
    namespace
    {
        class TriDrawCallback : public btTriangleCallback, public btInternalTriangleIndexCallback
        {
        public:
            TriDrawCallback(btIDebugDraw* debugDrawer, const btTransform& worldTrans, const btVector3& color)
            : debugDrawer_(debugDrawer),
              color_(color),
              worldTrans_(worldTrans)
            {
            }

            void internalProcessTriangleIndex(btVector3* triangle, int partId, int triangleIndex) override
            {
                processTriangle(triangle, partId, triangleIndex);
            }

            void processTriangle(btVector3* triangle, int partId, int triangleIndex) override
            {
                (void)partId;
                (void)triangleIndex;

                btVector3 wv0, wv1, wv2;
                wv0 = worldTrans_ * triangle[0];
                wv1 = worldTrans_ * triangle[1];
                wv2 = worldTrans_ * triangle[2];

                debugDrawer_->drawLine(wv0, wv1, color_);
                debugDrawer_->drawLine(wv1, wv2, color_);
                debugDrawer_->drawLine(wv2, wv0, color_);
            }

        private:
            btIDebugDraw* debugDrawer_;
            btVector3 color_;
            btTransform worldTrans_;
        };
    }

    void PhysicsDebugDraw::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
    {
        lines_.emplace_back(from, to, Color(color, alpha_));
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
            rop.addVertex(line.from, Vector2f_zero, line.color);
            rop.addVertex(line.to, Vector2f_zero, line.color);
        }

        lines_.clear();
    }

    void PhysicsDebugDraw::drawMesh(btCollisionShape* shape, const btTransform& worldTransform, const btVector3& color)
    {
        if (shape->isPolyhedral()) {
            btPolyhedralConvexShape* polyshape = (btPolyhedralConvexShape*)shape;
            if (polyshape->getConvexPolyhedron()) {
                const btConvexPolyhedron* poly = polyshape->getConvexPolyhedron();
                for (int i = 0; i < poly->m_faces.size(); i++) {
                    int numVerts = poly->m_faces[i].m_indices.size();
                    if (numVerts) {
                        int lastV = poly->m_faces[i].m_indices[numVerts - 1];
                        for (int v = 0; v < poly->m_faces[i].m_indices.size(); v++) {
                            int curVert = poly->m_faces[i].m_indices[v];
                            drawLine(worldTransform * poly->m_vertices[lastV], worldTransform * poly->m_vertices[curVert], color);
                            lastV = curVert;
                        }
                    }
                }
            } else {
                for (int i = 0; i < polyshape->getNumEdges(); i++) {
                    btVector3 a, b;
                    polyshape->getEdge(i, a, b);
                    btVector3 wa = worldTransform * a;
                    btVector3 wb = worldTransform * b;
                    drawLine(wa, wb, color);
                }
            }
        } else if (shape->isConcave()) {
            btConcaveShape* concaveMesh = (btConcaveShape*)shape;
            btVector3 aabbMax(BT_LARGE_FLOAT, BT_LARGE_FLOAT, BT_LARGE_FLOAT);
            btVector3 aabbMin(-BT_LARGE_FLOAT, -BT_LARGE_FLOAT, -BT_LARGE_FLOAT);
            TriDrawCallback drawCallback(this, worldTransform, color);
            concaveMesh->processAllTriangles(&drawCallback, aabbMin, aabbMax);
        } else if (shape->getShapeType() == CONVEX_TRIANGLEMESH_SHAPE_PROXYTYPE) {
            btConvexTriangleMeshShape* convexMesh = (btConvexTriangleMeshShape*)shape;
            btVector3 aabbMax(BT_LARGE_FLOAT, BT_LARGE_FLOAT, BT_LARGE_FLOAT);
            btVector3 aabbMin(-BT_LARGE_FLOAT, -BT_LARGE_FLOAT, -BT_LARGE_FLOAT);
            TriDrawCallback drawCallback(this, worldTransform, color);
            convexMesh->getMeshInterface()->InternalProcessAllTriangles(&drawCallback, aabbMin, aabbMax);
        }
    }
}

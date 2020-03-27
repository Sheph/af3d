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

#ifndef _RENDER_LIST_H_
#define _RENDER_LIST_H_

#include "Material.h"
#include "VertexArraySlice.h"
#include "VertexArrayWriter.h"
#include "Light.h"
#include "RenderNode.h"
#include "RenderSettings.h"
#include "af3d/Frustum.h"
#include "af3d/Vector2.h"

namespace af3d
{
    class RenderList;

    class RenderImmIndexed
    {
    public:
        RenderImmIndexed(const MaterialPtr& material,
            GLenum primitiveMode,
            float depthValue,
            const ScissorParams& scissorParams,
            RenderList& rl);
        ~RenderImmIndexed();

        std::vector<VertexImm>& vertices();

        std::vector<std::uint16_t>& indices();

    private:
        MaterialPtr material_;
        GLenum primitiveMode_;
        float depthValue_;
        ScissorParams scissorParams_;

        RenderList& rl_;
        size_t startVertices_;
        size_t startIndices_;
    };

    class RenderImm
    {
    public:
        RenderImm(const MaterialPtr& material,
            GLenum primitiveMode,
            float depthValue,
            const ScissorParams& scissorParams,
            RenderList& rl);
        ~RenderImm();

        std::vector<VertexImm>& vertices();

        inline void addVertex(const btVector3& pos, const Vector2f& uv, const Color& color)
        {
            vertices().emplace_back(pos, uv, toPackedColor(color));
        }

        inline void addVertex(const Vector3f& pos, const Vector2f& uv, const Color& color)
        {
            vertices().emplace_back(pos, uv, toPackedColor(color));
        }

        inline void addVertex(const Vector2f& pos, const Vector2f& uv, const Color& color)
        {
            vertices().emplace_back(pos, uv, toPackedColor(color));
        }

    private:
        MaterialPtr material_;
        GLenum primitiveMode_;
        float depthValue_;
        ScissorParams scissorParams_;

        RenderList& rl_;
        size_t startVertices_;
    };

    class RenderList : boost::noncopyable
    {
    public:
        RenderList(const Frustum& frustum, const RenderSettings& rs, VertexArrayWriter& defaultVa);
        ~RenderList() = default;

        inline const Frustum& frustum() const { return frustum_; }

        void addGeometry(const Matrix4f& modelMat, const AABB& aabb, const MaterialPtr& material,
            const VertexArraySlice& vaSlice, GLenum primitiveMode, float depthValue = 0.0f, const ScissorParams& scissorParams = ScissorParams());

        void addGeometry(const MaterialPtr& material,
            const VertexArraySlice& vaSlice, GLenum primitiveMode, float depthValue = 0.0f, const ScissorParams& scissorParams = ScissorParams());

        RenderImmIndexed addGeometryIndexed(const MaterialPtr& material,
            GLenum primitiveMode,
            float depthValue = 0.0f, const ScissorParams& scissorParams = ScissorParams());

        RenderImm addGeometry(const MaterialPtr& material,
            GLenum primitiveMode,
            float depthValue = 0.0f, const ScissorParams& scissorParams = ScissorParams());

        // Create immediate geometry by using default VAO, use only for small stuff like UI!
        VertexArraySlice createGeometry(const VertexImm* vertices, std::uint32_t numVertices,
            const std::uint16_t* indices = nullptr, std::uint32_t numIndices = 0);

        void addLight(const LightPtr& light);

        RenderNodePtr compile() const;

    private:
        friend class RenderImmIndexed;
        friend class RenderImm;

        struct Geometry
        {
            Geometry() = default;
            Geometry(const MaterialPtr& material,
                const VertexArraySlice& vaSlice,
                GLenum primitiveMode,
                float depthValue,
                const ScissorParams& scissorParams)
            : material(material),
              vaSlice(vaSlice),
              primitiveMode(primitiveMode),
              depthValue(depthValue),
              scissorParams(scissorParams)
            {
            }
            Geometry(const Matrix4f& modelMat,
                const AABB& aabb,
                const MaterialPtr& material,
                const VertexArraySlice& vaSlice,
                GLenum primitiveMode,
                float depthValue,
                const ScissorParams& scissorParams)
            : modelMat(modelMat),
              aabb(aabb),
              material(material),
              vaSlice(vaSlice),
              primitiveMode(primitiveMode),
              depthValue(depthValue),
              scissorParams(scissorParams)
            {
            }

            Matrix4f modelMat;
            AABB aabb;
            MaterialPtr material;
            VertexArraySlice vaSlice;
            GLenum primitiveMode;
            float depthValue;
            ScissorParams scissorParams;
        };

        using GeometryList = std::vector<Geometry>;
        using LightList = std::vector<LightPtr>;

        const Frustum& frustum_;
        const RenderSettings& rs_;
        VertexArrayWriter& defaultVa_;

        GeometryList geomList_;
        LightList lightList_;
    };
}

#endif

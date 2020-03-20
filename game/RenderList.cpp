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

#include "RenderList.h"
#include "Logger.h"

namespace af3d
{
    RenderList::RenderList(const Frustum& frustum, const RenderSettings& rs)
    : frustum_(frustum),
      rs_(rs)
    {
    }

    void RenderList::addGeometry(const Matrix4f& modelMat, const AABB& aabb, const MaterialPtr& material,
        const VertexArraySlice& vaSlice, GLenum primitiveMode, float depthValue, const ScissorParams& scissorParams)
    {
        geomList_.emplace_back(modelMat, aabb, material, vaSlice, primitiveMode, depthValue, scissorParams);
    }

    void RenderList::addGeometry(const MaterialPtr& material,
        const VertexArraySlice& vaSlice, GLenum primitiveMode, float depthValue, const ScissorParams& scissorParams)
    {
        geomList_.emplace_back(material, vaSlice, primitiveMode, depthValue, scissorParams);
    }

    void RenderList::addLight(const LightPtr& light)
    {
        lightList_.push_back(light);
    }

    RenderNodePtr RenderList::compile() const
    {
        const Matrix4f& viewProjMat = frustum_.viewProjMat();
        auto rn = std::make_shared<RenderNode>(rs_.clearMask(), rs_.clearColor());
        RenderNode tmpNode;
        for (const auto& geom : geomList_) {
            auto& params = rn->add(std::move(tmpNode), 0, geom.material,
                GL_LESS, geom.depthValue, rs_.cullFaceMode(), geom.material->blendingParams(),
                geom.vaSlice, geom.primitiveMode, geom.scissorParams);
            const auto& activeUniforms = geom.material->type()->prog()->activeUniforms();
            if (activeUniforms.count(UniformName::ViewProjMatrix) > 0) {
                params.setUniform(UniformName::ViewProjMatrix, viewProjMat);
            }
            if (activeUniforms.count(UniformName::ModelViewProjMatrix) > 0) {
                params.setUniform(UniformName::ModelViewProjMatrix, viewProjMat * geom.modelMat);
            }
            if (activeUniforms.count(UniformName::ModelMatrix) > 0) {
                params.setUniform(UniformName::ModelMatrix, geom.modelMat);
            }
            if (activeUniforms.count(UniformName::LightPos) > 0) {
                params.setUniform(UniformName::LightPos, Vector4f_zero);
            }
            if (activeUniforms.count(UniformName::LightColor) > 0) {
                auto ac = rs_.ambientColor();
                params.setUniform(UniformName::LightColor, Vector3f(ac.x(), ac.y(), ac.z()) * ac.w());
            }
        }

        int pass = 1;
        BlendingParams lightBp(GL_ONE, GL_ONE, GL_ONE, GL_ONE);

        for (const auto& light : lightList_) {
            auto lightAABB = light->getWorldAABB();
            for (const auto& geom : geomList_) {
                if (!geom.material->type()->usesLight() || !lightAABB.overlaps(geom.aabb)) {
                    continue;
                }
                auto& params = rn->add(std::move(tmpNode), pass, geom.material,
                    GL_EQUAL, geom.depthValue, rs_.cullFaceMode(), lightBp,
                    geom.vaSlice, geom.primitiveMode, geom.scissorParams);
                const auto& activeUniforms = geom.material->type()->prog()->activeUniforms();
                if (activeUniforms.count(UniformName::ViewProjMatrix) > 0) {
                    params.setUniform(UniformName::ViewProjMatrix, viewProjMat);
                }
                if (activeUniforms.count(UniformName::ModelViewProjMatrix) > 0) {
                    params.setUniform(UniformName::ModelViewProjMatrix, viewProjMat * geom.modelMat);
                }
                if (activeUniforms.count(UniformName::ModelMatrix) > 0) {
                    params.setUniform(UniformName::ModelMatrix, geom.modelMat);
                }
                light->setupMaterial(frustum_.transform().getOrigin(), params);
            }
            ++pass;
        }

        return rn;
    }
}

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
    RenderList::RenderList(const CameraComponentPtr& cc)
    : cc_(cc)
    {
    }

    void RenderList::addGeometry(const btTransform& xf, const MaterialPtr& material, const VertexArraySlice& vaSlice, GLenum primitiveMode)
    {
        geomList_.emplace_back(xf, material, vaSlice, primitiveMode);
    }

    void RenderList::addLight(const LightPtr& light)
    {
        //LOG4CPLUS_DEBUG(logger(), "light " << light->name());
        lightList_.push_back(light);
    }

    RenderNodePtr RenderList::compile() const
    {
        const Matrix4f& viewProjMat = cc_->getFrustum().viewProjMat();
        auto rn = std::make_shared<RenderNode>(cc_->clearColor(), cc_->viewport());
        RenderNode tmpNode;
        for (const auto& geom : geomList_) {
            auto& params = rn->add(std::move(tmpNode), geom.material, geom.vaSlice, geom.primitiveMode);
            const auto& activeUniforms = geom.material->type()->prog()->activeUniforms();
            if (activeUniforms.count(UniformName::ProjMatrix) > 0) {
                params.setUniform(UniformName::ProjMatrix, viewProjMat * Matrix4f(geom.xf));
            }
            if (activeUniforms.count(UniformName::Time) > 0) {
                params.setUniform(UniformName::Time, 0.0f);
            }
        }
        return rn;
    }
}

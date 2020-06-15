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

#include "RenderPassCSM.h"
#include "RenderList.h"
#include "MaterialManager.h"
#include "CameraRenderer.h"

namespace af3d
{
    int RenderPassCSM::compile(const CameraRenderer& cr, const RenderList& rl, int pass, const RenderNodePtr& rn)
    {
        auto drawBuffers = rn->mrt().getDrawBuffers();

        btAssert(drawBuffers[AttachmentPoint::Depth]);

        RenderNode tmpNode;

        std::vector<HardwareTextureBinding> textures;
        std::vector<StorageBufferBinding> storageBuffers;

        AttachmentPoints prepassDrawBuffers;
        prepassDrawBuffers.set(AttachmentPoint::Depth);

        for (const auto& geom : rl.geomList()) {
            if ((geom.material->type()->name() != MaterialTypeSkyBox) && !geom.material->blendingParams().isEnabled()) {
                const auto& activeUniforms = geom.material->type()->prog()->activeUniforms();
                const auto& mat = (activeUniforms.count(UniformName::ModelViewProjMatrix) != 0) ? materialManager.matPrepass(0) :
                    ((activeUniforms.count(UniformName::ModelMatrix) != 0) ? materialManager.matPrepass(1) : materialManager.matPrepassWS());
                DrawBufferBinding drawBufferBinding(prepassDrawBuffers, mat->type()->prog()->outputs());
                MaterialParams params(mat->type(), true);
                cr.setAutoParams(rl, mat, drawBufferBinding.mask, textures, storageBuffers, params, geom.modelMat, geom.prevModelMat);
                rn->add(std::move(tmpNode), pass, drawBufferBinding,
                    mat->type(),
                    mat->params(),
                    mat->blendingParams(),
                    geom.material->depthTest(),
                    geom.material->depthWrite(),
                    geom.material->cullFaceMode(),
                    GL_LESS, geom.depthValue, geom.flipCull,
                    std::move(textures), std::move(storageBuffers),
                    geom.vaSlice, geom.primitiveMode, geom.scissorParams,
                    std::move(params));
            }
        }

        return pass + 1;
    }
}

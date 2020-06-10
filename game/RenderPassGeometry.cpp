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

#include "RenderPassGeometry.h"
#include "CameraRenderer.h"

namespace af3d
{
    RenderPassGeometry::RenderPassGeometry(const AttachmentPoints& colorAttachments, bool withOpaque, bool withTransparent, bool zPrepassed)
    : colorAttachments_(colorAttachments),
      withOpaque_(withOpaque),
      withTransparent_(withTransparent),
      zPrepassed_(zPrepassed)
    {
    }

    int RenderPassGeometry::compile(const CameraRenderer& cr, const RenderList& rl, int basePass, const RenderNodePtr& rn)
    {
        auto drawBuffers = rn->mrt().getDrawBuffers();

        for (int i = static_cast<int>(AttachmentPoint::Color0); i <= static_cast<int>(AttachmentPoint::Max); ++i) {
            if (!colorAttachments_[static_cast<AttachmentPoint>(i)]) {
                drawBuffers.reset(static_cast<AttachmentPoint>(i));
            }
        }

        RenderNode tmpNode;

        std::vector<HardwareTextureBinding> textures;
        std::vector<StorageBufferBinding> storageBuffers;

        for (const auto& geom : rl.geomList()) {
            bool transparent = geom.material->blendingParams().isEnabled();
            if (transparent && !withTransparent_) {
                continue;
            }
            if (!transparent && !withOpaque_) {
                continue;
            }
            MaterialParams params(geom.material->type(), true);
            cr.setAutoParams(rl, geom, textures, storageBuffers, params);
            if (zPrepassed_) {
                int pass;
                GLenum depthFunc;
                if (geom.material->type()->name() == MaterialTypeSkyBox) {
                    pass = basePass + 1;
                    depthFunc = GL_LEQUAL;
                } else if (transparent) {
                    pass = basePass + 2;
                    depthFunc = GL_LEQUAL;
                } else {
                    pass = basePass;
                    depthFunc = GL_EQUAL;
                }
                rn->add(std::move(tmpNode), pass, drawBuffers, geom.material->type()->prog()->outputs(),
                    geom.material->type(),
                    geom.material->params(),
                    geom.material->blendingParams(),
                    geom.material->depthTest(),
                    false,
                    geom.material->cullFaceMode(),
                    depthFunc, geom.depthValue, geom.flipCull,
                    std::move(textures), std::move(storageBuffers),
                    geom.vaSlice, geom.primitiveMode, geom.scissorParams,
                    std::move(params));
            } else {
                int pass;
                if (geom.material->type()->name() == MaterialTypeSkyBox) {
                    pass = basePass + 1;
                } else if (transparent) {
                    pass = basePass + 2;
                } else {
                    pass = basePass;
                }
                rn->add(std::move(tmpNode), pass, drawBuffers, geom.material->type()->prog()->outputs(),
                    geom.material->type(),
                    geom.material->params(),
                    geom.material->blendingParams(),
                    geom.material->depthTest(),
                    geom.material->depthWrite(),
                    geom.material->cullFaceMode(),
                    GL_LEQUAL, geom.depthValue, geom.flipCull,
                    std::move(textures), std::move(storageBuffers),
                    geom.vaSlice, geom.primitiveMode, geom.scissorParams,
                    std::move(params));
            }
        }

        return basePass + 3;
    }
}

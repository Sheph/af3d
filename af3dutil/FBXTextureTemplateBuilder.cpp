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

#include "af3d/FBXTextureTemplateBuilder.h"
#include "Logger.h"

namespace af3d
{
    void FBXTextureTemplateBuilder::onProperty(const std::string& key, bool value)
    {
        if (key == "PremultiplyAlpha") {
            target_->setPremultiplyAlpha(value);
        } else {
            //LOG4CPLUS_WARN(af3dutil::logger(), "TextureTemplate: unknown key " << key);
        }
    }

    void FBXTextureTemplateBuilder::onProperty(const std::string& key, std::int32_t value)
    {
        if (key == "TextureTypeUse") {
            if (value >= 0 && value <= (int)FBXTextureUse::Max) {
                target_->setTextureUse((FBXTextureUse)value);
            } else {
                LOG4CPLUS_WARN(af3dutil::logger(), "TextureTemplate: bad value " << value << " for key " << key);
            }
        } else if (key == "WrapModeU") {
            if (value >= 0 && value <= (int)FBXWrapMode::Max) {
                target_->setWrapU((FBXWrapMode)value);
            } else {
                LOG4CPLUS_WARN(af3dutil::logger(), "TextureTemplate: bad value " << value << " for key " << key);
            }
        } else if (key == "WrapModeV") {
            if (value >= 0 && value <= (int)FBXWrapMode::Max) {
                target_->setWrapV((FBXWrapMode)value);
            } else {
                LOG4CPLUS_WARN(af3dutil::logger(), "TextureTemplate: bad value " << value << " for key " << key);
            }
        } else if (key == "CurrentTextureBlendMode") {
            if (value >= 0 && value <= (int)FBXBlendMode::Max) {
                target_->setBlendMode((FBXBlendMode)value);
            } else {
                LOG4CPLUS_WARN(af3dutil::logger(), "TextureTemplate: bad value " << value << " for key " << key);
            }
        } else {
            //LOG4CPLUS_WARN(af3dutil::logger(), "TextureTemplate: unknown key " << key);
        }
    }

    void FBXTextureTemplateBuilder::onProperty(const std::string& key, float value)
    {
        if (key == "Texture alpha") {
            target_->setAlpha(value);
        } else {
            //LOG4CPLUS_WARN(af3dutil::logger(), "TextureTemplate: unknown key " << key);
        }
    }

    void FBXTextureTemplateBuilder::onProperty(const std::string& key, const std::string& value)
    {
        //LOG4CPLUS_WARN(af3dutil::logger(), "TextureTemplate: unknown key " << key);
    }

    void FBXTextureTemplateBuilder::onProperty(const std::string& key, const btVector3& value)
    {
        //LOG4CPLUS_WARN(af3dutil::logger(), "TextureTemplate: unknown key " << key);
    }

    void FBXTextureTemplateBuilder::onProperty(const std::string& key, const Color& value)
    {
        //LOG4CPLUS_WARN(af3dutil::logger(), "TextureTemplate: unknown key " << key);
    }
}

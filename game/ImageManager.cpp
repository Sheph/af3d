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

#include "ImageManager.h"
#include "TextureManager.h"
#include "Logger.h"
#include "Platform.h"

namespace af3d
{
    ImageManager imageManager;

    template <>
    Single<ImageManager>* Single<ImageManager>::single = NULL;

    bool ImageManager::init()
    {
        LOG4CPLUS_DEBUG(logger(), "imageManager: init...");
        return true;
    }

    void ImageManager::shutdown()
    {
        LOG4CPLUS_DEBUG(logger(), "imageManager: shutdown...");
        tpsMap_.clear();
    }

    Image ImageManager::getImage(const std::string& name)
    {
        std::string::size_type pos = name.find('/');

        if (pos == std::string::npos) {
            auto texture = textureManager.loadTexture(name);

            return Image(texture, 0, 0, texture->width(), texture->height());
        } else {
            std::string tpsName = name.substr(0, pos) + ".json";
            std::string fileName = name.substr(pos + 1);

            auto it = tpsMap_.find(tpsName);

            if (it == tpsMap_.end()) {
                PlatformIFStream is(tpsName);

                if (!is) {
                    it = tpsMap_.insert(std::make_pair(tpsName, TPSPtr())).first;
                } else {
                    TPSPtr tps = TPS::fromStream(tpsName, is);

                    it = tpsMap_.insert(std::make_pair(tpsName, tps)).first;
                }
            }

            if (!it->second) {
                auto texture = textureManager.loadTexture(name);

                return Image(texture, 0, 0, texture->width(), texture->height());
            }

            auto texture = textureManager.loadTexture(it->second->imageFileName());

            auto entry = it->second->entry(fileName, false);

            if (entry.valid()) {
                return Image(texture, entry.x, entry.y, entry.width, entry.height);
            } else {
                return Image(texture, 0, 0, texture->width(), texture->height());
            }
        }
    }

    DrawablePtr ImageManager::getDrawable(const std::string& name)
    {
        return std::make_shared<Drawable>(getImage(name));
    }
}

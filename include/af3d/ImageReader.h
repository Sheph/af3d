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

#ifndef _AF3D_IMAGEREADER_H_
#define _AF3D_IMAGEREADER_H_

#include "af3d/Types.h"
#include <boost/noncopyable.hpp>
#include <vector>
#include <iostream>
#include <memory>
#if defined(ANDROID) || defined(__ANDROID__)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#ifndef GLAPIENTRY
#define GLAPIENTRY GL_APIENTRY
#endif

namespace af3d
{
    class ImageReader : boost::noncopyable
    {
    public:
        enum Flag
        {
            FlagHDR = (1 << 0),
            FlagSRGB = (1 << 1)
        };

        struct Info
        {
            std::uint32_t flags = 0;
            std::uint32_t width = 0;
            std::uint32_t height = 0;
            std::uint32_t numMipLevels = 1;
            GLenum format = 0;
        };

        ImageReader(const std::string& path, std::istream& is);
        ~ImageReader();

        static const char* glFormatStr(GLenum format);

        bool init(Info& info);

        bool read(std::uint32_t mip, std::vector<Byte>& data);

    private:
        class Impl;
        std::unique_ptr<Impl> impl_;
    };
}

#endif

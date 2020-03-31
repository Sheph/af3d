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

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include "af3d/Types.h"
#include "af3d/Single.h"
#include <iostream>
#include <memory>

namespace af3d
{
    struct VideoMode
    {
        VideoMode(std::uint32_t width, std::uint32_t height)
        : width(width), height(height) {}
        VideoMode() = default;

        inline bool operator<(const VideoMode& other) const
        {
            if (width != other.width) {
                return width > other.width;
            }

            return height > other.height;
        }

        inline bool operator==(const VideoMode& other) const
        {
            return (width == other.width) && (height == other.height);
        }

        std::uint32_t width = 0;
        std::uint32_t height = 0;
    };

    class PlatformIFStream : public std::istream
    {
    public:
        explicit PlatformIFStream(const std::string& fileName);
        ~PlatformIFStream();

        void close();

    private:
        std::streambuf* streamBuf_;
    };

    class Platform : public Single<Platform>
    {
    public:
        Platform() = default;
        virtual ~Platform() = default;

        inline const std::vector<VideoMode>& desktopVideoModes() const { return desktopVideoModes_; }
        inline void setDesktopVideoModes(const std::vector<VideoMode>& value) { desktopVideoModes_ = value; }

        inline const std::vector<VideoMode>& winVideoModes() const { return winVideoModes_; }
        inline void setWinVideoModes(const std::vector<VideoMode>& value) { winVideoModes_ = value; }

        inline int defaultVideoMode() const { return defaultVideoMode_; }
        inline void setDefaultVideoMode(int value) { defaultVideoMode_ = value; }

        inline int desktopVideoMode() const { return desktopVideoMode_; }
        inline void setDesktopVideoMode(int value) { desktopVideoMode_ = value; }

        inline const std::vector<std::uint32_t>& msaaModes() const { return msaaModes_; }
        inline void setMsaaModes(const std::vector<std::uint32_t>& value) { msaaModes_ = value; }

        inline bool vsyncSupported() const { return vsyncSupported_; }
        inline void setVSyncSupported(bool value) { vsyncSupported_ = value; }

        virtual const std::string& assetsPath() const = 0;

        virtual std::streambuf* openFile(const std::string& fileName) = 0;

        virtual bool changeVideoMode(bool fullscreen, int videoMode, int msaaMode, bool vsync, bool trilinearFilter) = 0;

        virtual std::string readUserConfig() const = 0;
        virtual bool writeUserConfig(const std::string& value) = 0;

        virtual std::string readUserSaveData() const = 0;
        virtual bool writeUserSaveData(const std::string& value) = 0;

    private:
        std::vector<VideoMode> desktopVideoModes_;
        std::vector<VideoMode> winVideoModes_;
        int defaultVideoMode_ = -1;
        int desktopVideoMode_ = -1;
        std::vector<std::uint32_t> msaaModes_;
        bool vsyncSupported_ = false;
    };

    using PlatformPtr = std::unique_ptr<Platform>;

    extern PlatformPtr platform;
}

#endif

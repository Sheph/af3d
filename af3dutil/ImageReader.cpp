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

#include "af3d/ImageReader.h"
#include "af3d/Utils.h"
#include "Logger.h"
#include <boost/optional.hpp>
#include "stb_image.h"
#include "dds-ktx.h"
#include <cstring>

namespace af3d
{
    struct DDSKTXFile
    {
        std::string data;
        ddsktx_texture_info info;
    };

    class ImageReader::Impl
    {
    public:
        Impl(const std::string& path, std::istream& is)
        : path_(path),
          is_(is)
        {
            callbacks_.read = &Impl::ioReadFn;
            callbacks_.skip = &Impl::ioSkipFn;
            callbacks_.eof = &Impl::ioEofFn;
        }

        static int ioReadFn(void* user, char* data, int size)
        {
            Impl* this_ = (Impl*)user;
            this_->is_.read(reinterpret_cast<char*>(data), size);
            return this_->is_.gcount();
        }

        static void ioSkipFn(void* user, int n)
        {
            Impl* this_ = (Impl*)user;
            this_->is_.seekg(n, std::ios_base::cur);
        }

        static int ioEofFn(void* user)
        {
            Impl* this_ = (Impl*)user;
            return this_->is_.eof();
        }

        std::string path_;
        std::istream& is_;
        stbi_io_callbacks callbacks_;
        boost::optional<DDSKTXFile> ddsktxFile_;
        Info info_;
    };

    ImageReader::ImageReader(const std::string& path, std::istream& is)
    : impl_(new Impl(path, is))
    {
    }

    ImageReader::~ImageReader()
    {
    }

    const char* ImageReader::glFormatStr(GLenum format)
    {
        switch (format) {
        case GL_RED: return "GL_RED";
        case GL_RGB: return "GL_RGB";
        case GL_RGBA: return "GL_RGBA";
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT: return "GL_COMPRESSED_RGB_S3TC_DXT1";
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT: return "GL_COMPRESSED_RGBA_S3TC_DXT1";
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT: return "GL_COMPRESSED_RGBA_S3TC_DXT5";
        case GL_COMPRESSED_RG_RGTC2: return "GL_COMPRESSED_RG_RGTC2";
        default: return "GL_UNKNOWN";
        }
    }

    bool ImageReader::init(Info& info)
    {
        if (impl_->info_.width != 0) {
            LOG4CPLUS_ERROR(af3dutil::logger(), "Image reader already initialized");
            return false;
        }

        if (!impl_->is_) {
            LOG4CPLUS_ERROR(af3dutil::logger(), "Cannot open " << impl_->path_);
            return false;
        }

        stbi_set_flip_vertically_on_load(true);

        impl_->info_.flags = stbi_is_hdr_from_callbacks(&impl_->callbacks_, impl_.get()) ? FlagHDR : 0;
        impl_->is_.seekg(0, std::ios_base::beg);

        int x = 0, y = 0, comp = 0;

        if (!stbi_info_from_callbacks(&impl_->callbacks_, impl_.get(), &x, &y, &comp)) {
            impl_->is_.seekg(0, std::ios_base::beg);
            impl_->ddsktxFile_ = DDSKTXFile();
            if (readStream(impl_->is_, impl_->ddsktxFile_->data)) {
                ddsktx_error err;
                if (!ddsktx_parse(&impl_->ddsktxFile_->info, &impl_->ddsktxFile_->data[0], impl_->ddsktxFile_->data.size(), &err)) {
                    LOG4CPLUS_ERROR(af3dutil::logger(), "Error reading " << impl_->path_ << ": " << err.msg);
                    impl_->ddsktxFile_.reset();
                    return false;
                }

                btAssert(impl_->ddsktxFile_->info.depth == 1);
                btAssert(impl_->ddsktxFile_->info.num_layers == 1);
                btAssert(!(impl_->ddsktxFile_->info.flags & DDSKTX_TEXTURE_FLAG_CUBEMAP));
                btAssert(impl_->ddsktxFile_->info.num_mips >= 1);

                impl_->info_.width = impl_->ddsktxFile_->info.width;
                impl_->info_.height = impl_->ddsktxFile_->info.height;

                switch (impl_->ddsktxFile_->info.format) {
                case DDSKTX_FORMAT_BC1:
                    if ((impl_->ddsktxFile_->info.flags & DDSKTX_TEXTURE_FLAG_ALPHA) != 0) {
                        impl_->info_.format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
                    } else {
                        impl_->info_.format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
                    }
                    break;
                case DDSKTX_FORMAT_BC3:
                    impl_->info_.format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
                    break;
                case DDSKTX_FORMAT_BC5:
                    btAssert((impl_->ddsktxFile_->info.flags & DDSKTX_TEXTURE_FLAG_ALPHA) == 0);
                    impl_->info_.format = GL_COMPRESSED_RG_RGTC2;
                    break;
                default:
                    LOG4CPLUS_ERROR(af3dutil::logger(), "Bad format:  " << impl_->ddsktxFile_->info.format);
                    btAssert(false);
                    break;
                }

                if ((impl_->ddsktxFile_->info.flags & DDSKTX_TEXTURE_FLAG_SRGB) != 0) {
                    impl_->info_.flags |= FlagSRGB;
                }

                impl_->info_.numMipLevels = impl_->ddsktxFile_->info.num_mips;

                info = impl_->info_;

                return true;
            }

            LOG4CPLUS_ERROR(af3dutil::logger(), "Error reading " << impl_->path_ << " header: " << stbi_failure_reason());
            return false;
        }

        impl_->is_.seekg(0, std::ios_base::beg);

        if ((x <= 0) || (y <= 0) || (comp <= 0)) {
            LOG4CPLUS_ERROR(af3dutil::logger(), "Error reading " << impl_->path_ << " header: bad width/height/comp");
            return false;
        }

        impl_->info_.width = x;
        impl_->info_.height = y;

        switch (comp) {
        case 3:
            impl_->info_.format = GL_RGB;
            break;
        case 4:
            impl_->info_.format = GL_RGBA;
            break;
        default:
            btAssert(false);
        case 1:
            impl_->info_.format = GL_RED;
            break;
        }

        info = impl_->info_;

        return true;
    }

    bool ImageReader::read(std::uint32_t mip, std::vector<Byte>& data)
    {
        if (impl_->info_.width == 0) {
            LOG4CPLUS_ERROR(af3dutil::logger(), "Image reader not initialized");
            return false;
        }

        if (impl_->ddsktxFile_) {
            ddsktx_sub_data subData;
            ddsktx_get_sub(&impl_->ddsktxFile_->info, &subData, &impl_->ddsktxFile_->data[0], impl_->ddsktxFile_->data.size(), 0, 0, mip);

            //btAssert(subData.row_pitch_bytes == ((int)textureMipSize(impl_->ddsktxFile_->info.width, mip) * impl_->ddsktxFile_->info.bpp / 8));

            data.resize(subData.size_bytes);
            std::memcpy(&data[0], subData.buff, data.size());

            return true;
        }

        btAssert(mip == 0);

        int x = 0, y = 0, comp = 0;
        Byte* ptr;
        size_t sz;

        if ((impl_->info_.flags & FlagHDR) != 0) {
            ptr = (Byte*)stbi_loadf_from_callbacks(&impl_->callbacks_, impl_.get(), &x, &y, &comp, 0);
            sz = x * y * comp * sizeof(float);
        } else {
            ptr = stbi_load_from_callbacks(&impl_->callbacks_, impl_.get(), &x, &y, &comp, 0);
            sz = x * y * comp;
        }

        if (!ptr) {
            LOG4CPLUS_ERROR(af3dutil::logger(), "Error reading " << impl_->path_ << ": " << stbi_failure_reason());
            return false;
        }
        if ((x != static_cast<int>(impl_->info_.width)) ||
            (y != static_cast<int>(impl_->info_.height))) {
            stbi_image_free(ptr);
            LOG4CPLUS_ERROR(af3dutil::logger(), "Error reading " << impl_->path_ << ": width/height changed");
            return false;
        }
        data.resize(sz);
        std::memcpy(&data[0], ptr, data.size());
        stbi_image_free(ptr);

        return true;
    }
}

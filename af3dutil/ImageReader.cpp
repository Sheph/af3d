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
#include "Logger.h"
#include "stb_image.h"
#include <cstring>

namespace af3d
{
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
        Info info_;
    };

    ImageReader::ImageReader(const std::string& path, std::istream& is)
    : impl_(new Impl(path, is))
    {
    }

    ImageReader::~ImageReader()
    {
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

        impl_->info_.isHDR = stbi_is_hdr_from_callbacks(&impl_->callbacks_, impl_.get());
        impl_->is_.seekg(0, std::ios_base::beg);

        int x = 0, y = 0, comp = 0;

        if (!stbi_info_from_callbacks(&impl_->callbacks_, impl_.get(), &x, &y, &comp)) {
            LOG4CPLUS_ERROR(af3dutil::logger(), "Error reading " << impl_->path_ << " header: " << stbi_failure_reason());
            impl_->is_.seekg(0, std::ios_base::beg);
            return false;
        }

        impl_->is_.seekg(0, std::ios_base::beg);

        if ((x <= 0) || (y <= 0) || (comp <= 0)) {
            LOG4CPLUS_ERROR(af3dutil::logger(), "Error reading " << impl_->path_ << " header: bad width/height/comp");
            return false;
        }

        impl_->info_.width = x;
        impl_->info_.height = y;
        impl_->info_.numComponents = comp;

        info = impl_->info_;

        return true;
    }

    bool ImageReader::read(std::vector<Byte>& data)
    {
        if (impl_->info_.width == 0) {
            LOG4CPLUS_ERROR(af3dutil::logger(), "Image reader not initialized");
            return false;
        }

        int x = 0, y = 0, comp = 0;
        Byte* ptr;
        size_t sz;

        if (impl_->info_.isHDR) {
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
            (y != static_cast<int>(impl_->info_.height)) ||
            (comp != impl_->info_.numComponents)) {
            stbi_image_free(ptr);
            LOG4CPLUS_ERROR(af3dutil::logger(), "Error reading " << impl_->path_ << ": width/height/comp changed");
            return false;
        }
        data.resize(sz);
        std::memcpy(&data[0], ptr, data.size());
        stbi_image_free(ptr);

        return true;
    }
}

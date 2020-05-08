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

#include "af3d/ImageWriter.h"
#include "Logger.h"
#include "stb_image_write.h"

namespace af3d
{
    class ImageWriter::Impl
    {
    public:
        Impl(const std::string& path, std::ostream& os)
        : path_(path),
          os_(os)
        {
        }

        static void ioWriteFn(void* context, void* data, int size)
        {
            Impl* this_ = (Impl*)context;
            this_->os_.write((const char*)data, size);
        }

        std::string path_;
        std::ostream& os_;
    };

    ImageWriter::ImageWriter(const std::string& path, std::ostream& os)
    : impl_(new Impl(path, os))
    {
    }

    ImageWriter::~ImageWriter()
    {
    }

    void ImageWriter::writeHDR(std::uint32_t width, std::uint32_t height, int numComponents, const std::vector<Byte>& data)
    {
        stbi_flip_vertically_on_write(true);

        if (!stbi_write_hdr_to_func(&Impl::ioWriteFn, impl_.get(), width, height, numComponents, (const float*)&data[0])) {
            LOG4CPLUS_ERROR(af3dutil::logger(), "Error writing " << impl_->path_);
        }
    }

    void ImageWriter::writePNG(std::uint32_t width, std::uint32_t height, int numComponents, const std::vector<Byte>& data)
    {
        stbi_flip_vertically_on_write(true);

        if (!stbi_write_png_to_func(&Impl::ioWriteFn, impl_.get(), width, height, numComponents, &data[0], width * numComponents)) {
            LOG4CPLUS_ERROR(af3dutil::logger(), "Error writing " << impl_->path_);
        }
    }
}

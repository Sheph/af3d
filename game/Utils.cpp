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

#include "Utils.h"
#include "Material.h"

namespace af3d
{
    AssimpScenePtr assimpImport(Assimp::Importer& importer,
        const std::string& path,
        std::uint32_t flags)
    {
        const aiScene* tmp = importer.ReadFile(path, flags);
        if (!tmp) {
            return AssimpScenePtr();
        }
        return AssimpScenePtr(importer.GetOrphanedScene());
    }

    static void createGaussianKernel(int mSize, float sigma, float kernel[])
    {
        int kSize = (mSize - 1) / 2;
        for (int j = 0; j <= kSize; ++j) {
            kernel[kSize + j] = kernel[kSize - j] = 0.39894 * std::exp(-0.5 * double(j) * double(j) / (sigma * sigma)) / sigma;
        }
    }

    static int createGaussianDirectionalKernel(int mSize, float kernel[], float offset[])
    {
        int kSize = (mSize - 1) / 2;
        int rSize = (kSize / 2) + 1;

        std::vector<float> tmp(rSize);

        tmp[0] = kernel[kSize];
        offset[0] = 0.0f;

        float total = tmp[0];

        for (int i = 1; i < rSize; ++i) {
            int t1 = (i * 2) - 1;
            int t2 = (i * 2) + 0;
            //if (i == rSize - 1) {
              //  tmp[i] = kernel[kSize + t1];
            //} else {
                tmp[i] = kernel[kSize + t1] + kernel[kSize + t2];
            //}
            offset[i] = (t1 * kernel[kSize + t1] + t2 * kernel[kSize + t2]) / tmp[i];
            //offset[i] = t1 + kernel[kSize + t2] / tmp[i];
            total += tmp[i];
        }


        for (int i = 0; i < rSize; ++i) {
            kernel[i] = (tmp[i] / total);
        }

        return rSize;
    }

    void setGaussianBlurParams(MaterialParams& params, int ksize, float sigma, bool isHorizontal)
    {
        std::vector<float> kernel(ksize), offset(ksize);
        createGaussianKernel(ksize, sigma, &kernel[0]);
        int rSize = createGaussianDirectionalKernel(ksize, &kernel[0], &offset[0]);
        kernel.resize(rSize);
        offset.resize(rSize);
        params.setUniform(UniformName::GaussianKernel, kernel);
        params.setUniform(UniformName::GaussianOffset, offset);
        params.setUniform(UniformName::GaussianMSize, ksize);
        params.setUniform(UniformName::GaussianDir, static_cast<int>(isHorizontal));
    }
}

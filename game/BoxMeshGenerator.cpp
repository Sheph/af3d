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

#include "BoxMeshGenerator.h"
#include "Mesh.h"

namespace af3d
{
    BoxMeshGenerator::BoxMeshGenerator(const btVector3& size, const std::array<Color, 6>& colors)
    : size_(toVector3f(size / 2)),
      colors_(colors),
      colored_(true)
    {
    }

    BoxMeshGenerator::BoxMeshGenerator(const btVector3& size)
    : size_(toVector3f(size / 2)),
      colored_(false)
    {
    }

    void BoxMeshGenerator::load(Resource& res, HardwareContext& ctx)
    {
        Mesh& mesh = static_cast<Mesh&>(res);

        auto vbo = mesh.subMeshes()[0]->vaSlice().va()->vbos()[0];
        auto ebo = mesh.subMeshes()[0]->vaSlice().va()->ebo();

        vbo->resize(4 * 6, ctx); // 4 verts per face
        ebo->resize(6 * 6, ctx); // 6 indices per face

        float *verts, *vertsStart;
        std::uint16_t *indices, *indicesStart;

        verts = vertsStart = (float*)vbo->lock(HardwareBuffer::WriteOnly, ctx);
        indices = indicesStart = (std::uint16_t*)ebo->lock(HardwareBuffer::WriteOnly, ctx);

        std::uint16_t lastIdx = 0;
        int face = 0;

        for (int x = -1; x <= 1; x++) {
            for (int y = -1; y <= 1; y++) {
                for (int z = -1; z <= 1; z++) {
                    if (x == 0 && y == 0 && z == 0) {
                        continue;
                    }
                    if (std::abs(x) + std::abs(y) + std::abs(z) > 1) {
                        continue;
                    }

                    Vector3f vDir = Vector3f_zero;
                    Vector3f vAdd[4] = {Vector3f_zero, Vector3f_zero, Vector3f_zero, Vector3f_zero};

                    if (std::abs(x)){
                        vDir.setX(x);
                        vAdd[3].setY(1); vAdd[3].setZ(1);
                        vAdd[2].setY(-1); vAdd[2].setZ(1);
                        vAdd[1].setY(-1); vAdd[1].setZ(-1);
                        vAdd[0].setY(1); vAdd[0].setZ(-1);
                    } else if(std::abs(y)) {
                        vDir.setY(y);
                        vAdd[3].setZ(1); vAdd[3].setX(1);
                        vAdd[2].setZ(-1); vAdd[2].setX(1);
                        vAdd[1].setZ(-1); vAdd[1].setX(-1);
                        vAdd[0].setZ(1); vAdd[0].setX(-1);
                    } else if(std::abs(z)) {
                        vAdd[3].setY(1); vAdd[3].setX(1);
                        vAdd[2].setY(1); vAdd[2].setX(-1);
                        vAdd[1].setY(-1); vAdd[1].setX(-1);
                        vAdd[0].setY(-1); vAdd[0].setX(1);
                        vDir.setZ(z);
                    }

                    const Color& c = colors_[face];

                    for (int i = 0; i < 4; ++i) {
                        int idx = i;
                        if (x + y + z > 0) {
                            idx = 3 - i;
                        }

                        Vector3f pos = (vDir + vAdd[idx]) * size_;

                        Vector2f tex;
                        if (std::abs(x)) {
                            tex.setX(vAdd[i].z());
                            tex.setY(vAdd[i].y());
                        } else if (std::abs(y)) {
                            tex.setX(vAdd[i].x());
                            tex.setY(vAdd[i].z());
                        } else if (std::abs(z)) {
                            tex.setX(vAdd[i].x());
                            tex.setY(vAdd[i].y());
                        }
                        //Inverse for negative directions
                        if (x + y + z < 0) {
                            tex.setX(-tex.x());
                            tex.setY(-tex.y());
                        }
                        tex.setX((tex.x() + 1) * 0.5f);
                        tex.setY((tex.y() + 1) * 0.5f);

                        std::memcpy(verts, &pos.v[0], 12);
                        verts += 3;
                        std::memcpy(verts, &tex.v[0], 8);
                        verts += 2;
                        if (colored_) {
                            std::memcpy(verts, &c.v[0], 16);
                            verts += 4;
                        } else {
                            std::memcpy(verts, &vDir[0], 12);
                            verts += 3;
                        }
                    }

                    for (std::uint16_t i = 0; i < 3; ++i) {
                        *indices = lastIdx + i;
                        ++indices;
                    }

                    *indices = lastIdx + 2;
                    ++indices;
                    *indices = lastIdx + 3;
                    ++indices;
                    *indices = lastIdx + 0;
                    ++indices;

                    lastIdx += 4;
                    ++face;
                }
            }
        }

        btAssert((verts - vertsStart) * 4 == vbo->sizeInBytes(ctx));
        btAssert((indices - indicesStart) == ebo->count(ctx));

        ebo->unlock(ctx);
        vbo->unlock(ctx);
    }
}

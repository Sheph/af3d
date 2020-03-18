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

#ifndef _AF3D_BYTE_ORDER_H_
#define _AF3D_BYTE_ORDER_H_

#include "af3d/Types.h"

#if defined(__i386) || defined(__x86_64__) || defined(_WIN32)
#define AF3D_LITTLE_ENDIAN
#else
#error "Unknown architecture"
#endif

namespace af3d
{
    inline bool isCpuLittleEndian()
    {
#ifdef AF3D_LITTLE_ENDIAN
        return true;
#else
        return false;
#endif
    }

    inline std::uint16_t swapBytes(std::uint16_t value)
    {
        return ((value >> 8) & 0x00FF) | ((value << 8) & 0xFF00);
    }

    inline std::int16_t swapBytes(std::int16_t value)
    {
        return static_cast<std::int16_t>(swapBytes(static_cast<std::uint16_t>(value)));
    }

    inline std::uint32_t swapBytes(std::uint32_t value)
    {
        return ((value >> 24) & 0x000000FF) | ((value >> 8) & 0x0000FF00) |
           ((value << 8) & 0x00FF0000) | ((value << 24) & 0xFF000000);
    }

    inline std::int32_t swapBytes(std::int32_t value)
    {
        return static_cast<std::int32_t>(swapBytes(static_cast<std::uint32_t>(value)));
    }

    inline std::uint64_t swapBytes(std::uint64_t value)
    {
        std::uint32_t hi = static_cast<std::uint32_t>(value >> 32);
        std::uint32_t lo = static_cast<std::uint32_t>(value & 0xFFFFFFFF);
        return static_cast<std::uint64_t>(swapBytes(hi)) |
           (static_cast<std::uint64_t>(swapBytes(lo)) << 32);
    }

    inline std::int64_t swapBytes(std::int64_t value)
    {
        return static_cast<std::int64_t>(swapBytes(static_cast<std::uint64_t>(value)));
    }

#define AF3D_DEFINE_CPUTO_NOOP(type, endian) \
    inline type to##endian(type value) \
    { \
        return value; \
    }

#define AF3D_DEFINE_CPUTO_SWAP(type, endian) \
    inline type to##endian(type value) \
    { \
        return swapBytes(value); \
    }

#define AF3D_DEFINE_TOCPU_NOOP(type, endian) \
    inline type from##endian(type value) \
    { \
        return value; \
    }

#define AF3D_DEFINE_TOCPU_SWAP(type, endian) \
    inline type from##endian(type value) \
    { \
        return swapBytes(value); \
    }

#ifdef AF3D_LITTLE_ENDIAN
#   define AF3D_DEFINE_CPUTO_LE(type) AF3D_DEFINE_CPUTO_NOOP(type, LittleEndian)
#   define AF3D_DEFINE_CPUTO_BE(type) AF3D_DEFINE_CPUTO_SWAP(type, BigEndian)
#   define AF3D_DEFINE_TOCPU_LE(type) AF3D_DEFINE_TOCPU_NOOP(type, LittleEndian)
#   define AF3D_DEFINE_TOCPU_BE(type) AF3D_DEFINE_TOCPU_SWAP(type, BigEndian)
#else
#   define AF3D_DEFINE_CPUTO_LE(type) AF3D_DEFINE_CPUTO_SWAP(type, LittleEndian)
#   define AF3D_DEFINE_CPUTO_BE(type) AF3D_DEFINE_CPUTO_NOOP(type, BigEndian)
#   define AF3D_DEFINE_TOCPU_LE(type) AF3D_DEFINE_TOCPU_SWAP(type, LittleEndian)
#   define AF3D_DEFINE_TOCPU_BE(type) AF3D_DEFINE_TOCPU_NOOP(type, BigEndian)
#endif

    AF3D_DEFINE_CPUTO_LE(std::uint16_t)
    AF3D_DEFINE_CPUTO_LE(std::uint32_t)
    AF3D_DEFINE_CPUTO_LE(std::uint64_t)
    AF3D_DEFINE_CPUTO_BE(std::uint16_t)
    AF3D_DEFINE_CPUTO_BE(std::uint32_t)
    AF3D_DEFINE_CPUTO_BE(std::uint64_t)
    AF3D_DEFINE_TOCPU_LE(std::uint16_t)
    AF3D_DEFINE_TOCPU_LE(std::uint32_t)
    AF3D_DEFINE_TOCPU_LE(std::uint64_t)
    AF3D_DEFINE_TOCPU_BE(std::uint16_t)
    AF3D_DEFINE_TOCPU_BE(std::uint32_t)
    AF3D_DEFINE_TOCPU_BE(std::uint64_t)
    AF3D_DEFINE_CPUTO_LE(std::int16_t)
    AF3D_DEFINE_CPUTO_LE(std::int32_t)
    AF3D_DEFINE_CPUTO_LE(std::int64_t)
    AF3D_DEFINE_CPUTO_BE(std::int16_t)
    AF3D_DEFINE_CPUTO_BE(std::int32_t)
    AF3D_DEFINE_CPUTO_BE(std::int64_t)
    AF3D_DEFINE_TOCPU_LE(std::int16_t)
    AF3D_DEFINE_TOCPU_LE(std::int32_t)
    AF3D_DEFINE_TOCPU_LE(std::int64_t)
    AF3D_DEFINE_TOCPU_BE(std::int16_t)
    AF3D_DEFINE_TOCPU_BE(std::int32_t)
    AF3D_DEFINE_TOCPU_BE(std::int64_t)
}

#endif

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

#ifndef _AF3D_UTILS_H_
#define _AF3D_UTILS_H_

#include "af3d/Types.h"
#include <iostream>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <boost/functional/hash.hpp>

namespace af3d
{
    extern const std::string string_empty;

    using ScopedLock = std::lock_guard<std::mutex>;
    using ScopedLockA = std::unique_lock<std::mutex>;

    template<class T>
    struct EnumHash
    {
        inline size_t operator()(const T& elem) const
        {
            return std::hash<typename std::underlying_type<T>::type>()(
                static_cast<typename std::underlying_type<T>::type>(elem));
        }
    };

    struct NullDeleter
    {
        void operator()(const void*) const {}
    };

    template <class T>
    std::shared_ptr<T> makeSharedNullDeleter(T* ptr)
    {
        return std::shared_ptr<T>(ptr, NullDeleter());
    }

    template <class KeyT, class ValueT>
    using EnumUnorderedMap = std::unordered_map<KeyT, ValueT, EnumHash<KeyT>>;

    template <class KeyT>
    using EnumUnorderedSet = std::unordered_set<KeyT, EnumHash<KeyT>>;

    template <class KeyT, class ValueT>
    using BHUnorderedMap = std::unordered_map<KeyT, ValueT, boost::hash<KeyT>>;

    template <class KeyT>
    using BHUnorderedSet = std::unordered_set<KeyT, boost::hash<KeyT>>;

    void initTimeUs();

    std::uint64_t getTimeUs();

    /*
     * Returns a random in range [minVal, maxVal).
     */
    float getRandom(float minVal, float maxVal);

    int getRandomInt(int minVal, int maxVal);

    bool readStream(std::istream& is, std::string& str);

    inline float lerp(float v1, float v2, float t)
    {
        return v1 + (v2 - v1) * t;
    }

    inline btQuaternion script_shortestArcQuatNormalize2(btVector3 v0, btVector3 v1)
    {
        return shortestArcQuatNormalize2(v0, v1);
    }

    inline bool btIsValid(const btVector3& v)
    {
        return btIsValid(v.x()) && btIsValid(v.y()) && btIsValid(v.z());
    }

    inline bool btIsValid(const btQuaternion& q)
    {
        return btIsValid(q.x()) && btIsValid(q.y()) && btIsValid(q.z()) && btIsValid(q.w());
    }

    inline float btZeroNormalize(btVector3& v)
    {
        float l2 = v.length2();
        if (l2 >= SIMD_EPSILON * SIMD_EPSILON) {
            float d = btSqrt(l2);
            v /= d;
            return d;
        } else {
            v.setZero();
            return 0.0f;
        }
    }

    inline btVector3 btZeroNormalized(const btVector3& v)
    {
        btVector3 n = v;
        btZeroNormalize(n);
        return n;
    }

    btTransform toTransform(const btVector3& v);

    btMatrix3x3 makeLookBasis(const btVector3& dir, const btVector3& up);

    btQuaternion makeLookRotation(const btVector3& dir, const btVector3& up);

    btTransform makeLookDir(const btVector3& pos, const btVector3& dir, const btVector3& up);

    btTransform makeLookAt(const btVector3& pos, const btVector3& target, const btVector3& up);

    btMatrix3x3 basisFromEuler(float eulerX, float eulerY, float eulerZ);

    btQuaternion rotationFromEuler(float eulerX, float eulerY, float eulerZ);
}

#endif

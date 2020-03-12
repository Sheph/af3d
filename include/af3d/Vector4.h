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

#ifndef _AF3D_VECTOR4_H_
#define _AF3D_VECTOR4_H_

#include "af3d/Types.h"
#include <type_traits>

namespace af3d
{
    template <class T>
    class Vector4
    {
    public:
        T v[4];

        Vector4() {}
        Vector4(T _x, T _y, T _z, T _w)
        {
            v[0] = _x;
            v[1] = _y;
            v[2] = _z;
            v[3] = _w;
        }

        Vector4<T>& operator+=(const Vector4<T>& other)
        {
            v[0] += other.v[0];
            v[1] += other.v[1];
            v[2] += other.v[2];
            v[3] += other.v[3];
            return *this;
        }

        Vector4<T>& operator-=(const Vector4<T>& other)
        {
            v[0] -= other.v[0];
            v[1] -= other.v[1];
            v[2] -= other.v[2];
            v[3] -= other.v[3];
            return *this;
        }

        Vector4<T>& operator*=(T s)
        {
            v[0] *= s;
            v[1] *= s;
            v[2] *= s;
            v[3] *= s;
            return *this;
        }

        Vector4<T>& operator/=(T s)
        {
            btFullAssert(s != T(0));
            v[0] /= s;
            v[1] /= s;
            v[2] /= s;
            v[3] /= s;
            return *this;
        }

        T dot(const Vector4<T>& other) const
        {
            return v[0] * other.v[0] +
                v[1] * other.v[1] +
                v[2] * other.v[2] +
                v[3] * other.v[3];
        }

        T length2() const
        {
            return dot(*this);
        }

        T length() const
        {
            return btSqrt(length2());
        }

        T norm() const
        {
            return length();
        }

        T safeNorm() const
        {
            T d = length2();
            if (d > SIMD_EPSILON) {
                return btSqrt(d);
            }
            return T(0);
        }

        T distance2(const Vector4<T>& other) const;

        T distance(const Vector4<T>& other) const;

        T safeNormalize()
        {
            T l2 = length2();
            if (l2 >= SIMD_EPSILON * SIMD_EPSILON) {
                T d = btSqrt(l2);
                (*this) /= d;
                return d;
            } else {
                setValue(1, 0, 0, 0);
                return T(0);
            }
        }

        T normalize()
        {
            btAssert(!fuzzyZero());
            T d = length();
            *this /= d;
            return d;
        }

        Vector4<T> normalized() const;

        Vector4<T> absolute() const
        {
            return Vector4<T>(
                std::abs(v[0]),
                std::abs(v[1]),
                std::abs(v[2]),
                std::abs(v[3]));
        }

        void setInterpolate3(const Vector4<T>& v0, const Vector4<T>& v1, T rt)
        {
            T s = T(1) - rt;
            v[0] = s * v0.v[0] + rt * v1.v[0];
            v[1] = s * v0.v[1] + rt * v1.v[1];
            v[2] = s * v0.v[2] + rt * v1.v[2];
            v[3] = s * v0.v[3] + rt * v1.v[3];
        }

        Vector4<T> lerp(const Vector4<T>& other, T t) const
        {
            return Vector4<T>(v[0] + (other.v[0] - v[0]) * t,
                v[1] + (other.v[1] - v[1]) * t,
                v[2] + (other.v[2] - v[2]) * t,
                v[3] + (other.v[3] - v[3]) * t);
        }

        Vector4<T>& operator*=(const Vector4<T>& other)
        {
            v[0] *= other.v[0];
            v[1] *= other.v[1];
            v[2] *= other.v[2];
            v[3] *= other.v[3];
            return *this;
        }

        T getX() const { return v[0]; }
        T getY() const { return v[1]; }
        T getZ() const { return v[2]; }
        T getW() const { return v[3]; }

        void setX(T _x) { v[0] = _x; };
        void setY(T _y) { v[1] = _y; };
        void setZ(T _z) { v[2] = _z; };
        void setW(T _w) { v[3] = _w; };

        T x() const { return v[0]; }
        T y() const { return v[1]; }
        T z() const { return v[2]; }
        T w() const { return v[3]; }

        T& operator[](int i) { return v[i]; }
        T operator[](int i) const { return v[i]; }

        bool operator==(const Vector4<T>& other) const
        {
            return (v[3] == other.v[3]) &&
                (v[2] == other.v[2]) &&
                (v[1] == other.v[1]) &&
                (v[0] == other.v[0]);
        }

        bool operator!=(const Vector4<T>& other) const
        {
            return !(*this == other);
        }

        void setMax(const Vector4<T>& other)
        {
            btSetMax(v[0], other.v[0]);
            btSetMax(v[1], other.v[1]);
            btSetMax(v[2], other.v[2]);
            btSetMax(v[3], other.v[3]);
        }

        void setMin(const Vector4<T>& other)
        {
            btSetMin(v[0], other.v[0]);
            btSetMin(v[1], other.v[1]);
            btSetMin(v[2], other.v[2]);
            btSetMin(v[3], other.v[3]);
        }

        void setValue(T _x, T _y, T _z, T _w)
        {
            v[0] = _x;
            v[1] = _y;
            v[2] = _z;
            v[3] = _w;
        }

        void setZero()
        {
            setValue(T(0), T(0), T(0), T(0));
        }

        bool isZero() const
        {
            return v[0] == T(0) && v[1] == T(0) && v[2] == T(0) && v[3] == T(0);
        }

        bool fuzzyZero() const
        {
            return length2() < SIMD_EPSILON * SIMD_EPSILON;
        }
    };

    template <class T>
    inline Vector4<T> operator+(const Vector4<T>& v1, const Vector4<T>& v2)
    {
        return Vector4<T>(
            v1.v[0] + v2.v[0],
            v1.v[1] + v2.v[1],
            v1.v[2] + v2.v[2],
            v1.v[3] + v2.v[3]);
    }

    template <class T>
    inline Vector4<T> operator*(const Vector4<T>& v1, const Vector4<T>& v2)
    {
        return Vector4<T>(
            v1.v[0] * v2.v[0],
            v1.v[1] * v2.v[1],
            v1.v[2] * v2.v[2],
            v1.v[3] * v2.v[3]);
    }

    template <class T>
    inline Vector4<T> operator-(const Vector4<T>& v1, const Vector4<T>& v2)
    {
        return Vector4<T>(
            v1.v[0] - v2.v[0],
            v1.v[1] - v2.v[1],
            v1.v[2] - v2.v[2],
            v1.v[3] - v2.v[3]);
    }

    template <class T>
    inline Vector4<T> operator-(const Vector4<T>& v)
    {
        return Vector4<T>(-v.v[0], -v.v[1], -v.v[2], -v.v[3]);
    }

    template <class T>
    inline Vector4<T> operator*(const Vector4<T>& v, T s)
    {
        return Vector4<T>(v.v[0] * s, v.v[1] * s, v.v[2] * s, v.v[3] * s);
    }

    template <class T>
    inline Vector4<T> operator*(T s, const Vector4<T>& v)
    {
        return v * s;
    }

    template <class T>
    inline Vector4<T> operator/(const Vector4<T>& v, T s)
    {
        btFullAssert(s != T(0));
        return v * (T(1) / s);
    }

    template <class T>
    inline Vector4<T> operator/(const Vector4<T>& v1, const Vector4<T>& v2)
    {
        return Vector4<T>(
            v1.v[0] / v2.v[0],
            v1.v[1] / v2.v[1],
            v1.v[2] / v2.v[2],
            v1.v[3] / v2.v[3]);
    }

    template <class T>
    inline T btDot(const Vector4<T>& v1, const Vector4<T>& v2)
    {
        return v1.dot(v2);
    }

    template <class T>
    inline T btDistance2(const Vector4<T>& v1, const Vector4<T>& v2)
    {
        return v1.distance2(v2);
    }

    template <class T>
    inline T btDistance(const Vector4<T>& v1, const Vector4<T>& v2)
    {
        return v1.distance(v2);
    }

    template <class T>
    inline Vector4<T> lerp(const Vector4<T>& v1, const Vector4<T>& v2, T t)
    {
        return v1.lerp(v2, t);
    }

    template <class T>
    T Vector4<T>::distance2(const Vector4<T>& v) const
    {
        return (v - *this).length2();
    }

    template <class T>
    T Vector4<T>::distance(const Vector4<T>& v) const
    {
        return (v - *this).length();
    }

    template <class T>
    Vector4<T> Vector4<T>::normalized() const
    {
        Vector4<T> nrm = *this;
        nrm.normalize();
        return nrm;
    }

    using Vector4f = Vector4<float>;
    using Vector4i = Vector4<int>;
    using Color = Vector4f;

    static_assert(sizeof(Vector4f) == 16, "Bad Vector4f size");
    static_assert(sizeof(Vector4i) == 16, "Bad Vector4i size");

    extern const Vector4f Vector4f_zero;
    extern const Vector4i Vector4i_zero;

    inline bool btIsValid(const Vector4f& v)
    {
        return btIsValid(v.x()) && btIsValid(v.y()) && btIsValid(v.z()) && btIsValid(v.w());
    }

    template <class T>
    inline T btZeroNormalize(Vector4<T>& v)
    {
        T l2 = v.length2();
        if (l2 >= SIMD_EPSILON * SIMD_EPSILON) {
            T d = btSqrt(l2);
            v /= d;
            return d;
        } else {
            v.setZero();
            return T(0);
        }
    }

    template <class T>
    inline Vector4<T> btZeroNormalized(const Vector4<T>& v)
    {
        Vector4<T> n = v;
        btZeroNormalize(n);
        return n;
    }
}

#endif

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

#ifndef _AF3D_VECTOR2_H_
#define _AF3D_VECTOR2_H_

#include "af3d/Types.h"
#include <type_traits>
#include <ostream>
#include <boost/functional/hash.hpp>

namespace af3d
{
    template <class T>
    class Vector2
    {
    public:
        T v[2];

        Vector2() = default;
        Vector2(T _x, T _y)
        {
            v[0] = _x;
            v[1] = _y;
        }

        static Vector2<T> fromVector2i(const Vector2<int>& other)
        {
            return Vector2<T>(other.x(), other.y());
        }

        static Vector2<T> fromVector2f(const Vector2<float>& other)
        {
            return Vector2<T>(other.x(), other.y());
        }

        Vector2<T>& operator+=(const Vector2<T>& other)
        {
            v[0] += other.v[0];
            v[1] += other.v[1];
            return *this;
        }

        Vector2<T>& operator-=(const Vector2<T>& other)
        {
            v[0] -= other.v[0];
            v[1] -= other.v[1];
            return *this;
        }

        Vector2<T>& operator*=(T s)
        {
            v[0] *= s;
            v[1] *= s;
            return *this;
        }

        Vector2<T>& operator/=(T s)
        {
            btFullAssert(s != T(0));
            v[0] /= s;
            v[1] /= s;
            return *this;
        }

        T dot(const Vector2<T>& other) const
        {
            return v[0] * other.v[0] +
                v[1] * other.v[1];
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

        T distance2(const Vector2<T>& other) const;

        T distance(const Vector2<T>& other) const;

        T safeNormalize()
        {
            T l2 = length2();
            if (l2 >= SIMD_EPSILON * SIMD_EPSILON) {
                T d = btSqrt(l2);
                (*this) /= d;
                return d;
            } else {
                setValue(1, 0);
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

        Vector2<T> normalized() const;

        T angle(const Vector2<T>& other) const
        {
            return btAtan2(cross(other), dot(other));
        }

        Vector2<T> absolute() const
        {
            return Vector2<T>(
                std::abs(v[0]),
                std::abs(v[1]));
        }

        T cross(const Vector2<T>& other) const
        {
            return v[0] * other.v[1] - v[1] * other.v[0];
        }

        int minAxis() const
        {
            return v[0] < v[1] ? 0 : 1;
        }

        int maxAxis() const
        {
            return v[0] < v[1] ? 1 : 0;
        }

        void setInterpolate3(const Vector2<T>& v0, const Vector2<T>& v1, T rt)
        {
            T s = T(1) - rt;
            v[0] = s * v0.v[0] + rt * v1.v[0];
            v[1] = s * v0.v[1] + rt * v1.v[1];
        }

        Vector2<T> lerp(const Vector2<T>& other, T t) const
        {
            return Vector2<T>(v[0] + (other.v[0] - v[0]) * t,
                v[1] + (other.v[1] - v[1]) * t);
        }

        Vector2<T>& operator*=(const Vector2<T>& other)
        {
            v[0] *= other.v[0];
            v[1] *= other.v[1];
            return *this;
        }

        T getX() const { return v[0]; }
        T getY() const { return v[1]; }

        void setX(T _x) { v[0] = _x; };
        void setY(T _y) { v[1] = _y; };

        T x() const { return v[0]; }
        T y() const { return v[1]; }

        T& operator[](int i) { return v[i]; }
        T operator[](int i) const { return v[i]; }

        bool operator==(const Vector2<T>& other) const
        {
            return (v[1] == other.v[1]) &&
                (v[0] == other.v[0]);
        }

        bool operator!=(const Vector2<T>& other) const
        {
            return !(*this == other);
        }

        bool operator<(const Vector2<T>& other) const
        {
            if (v[0] != other.v[0]) {
                return v[0] < other.v[0];
            } else {
                return v[1] < other.v[1];
            }
        }

        void setMax(const Vector2<T>& other)
        {
            btSetMax(v[0], other.v[0]);
            btSetMax(v[1], other.v[1]);
        }

        void setMin(const Vector2<T>& other)
        {
            btSetMin(v[0], other.v[0]);
            btSetMin(v[1], other.v[1]);
        }

        void setValue(T _x, T _y)
        {
            v[0] = _x;
            v[1] = _y;
        }

        void setZero()
        {
            setValue(T(0), T(0));
        }

        bool isZero() const
        {
            return v[0] == T(0) && v[1] == T(0);
        }

        bool fuzzyZero() const
        {
            return length2() < SIMD_EPSILON * SIMD_EPSILON;
        }
    };

    template <class T>
    inline Vector2<T> operator+(const Vector2<T>& v1, const Vector2<T>& v2)
    {
        return Vector2<T>(
            v1.v[0] + v2.v[0],
            v1.v[1] + v2.v[1]);
    }

    template <class T>
    inline Vector2<T> operator*(const Vector2<T>& v1, const Vector2<T>& v2)
    {
        return Vector2<T>(
            v1.v[0] * v2.v[0],
            v1.v[1] * v2.v[1]);
    }

    template <class T>
    inline Vector2<T> operator-(const Vector2<T>& v1, const Vector2<T>& v2)
    {
        return Vector2<T>(
            v1.v[0] - v2.v[0],
            v1.v[1] - v2.v[1]);
    }

    template <class T>
    inline Vector2<T> operator-(const Vector2<T>& v)
    {
        return Vector2<T>(-v.v[0], -v.v[1]);
    }

    template <class T>
    inline Vector2<T> operator*(const Vector2<T>& v, T s)
    {
        return Vector2<T>(v.v[0] * s, v.v[1] * s);
    }

    template <class T>
    inline Vector2<T> operator*(T s, const Vector2<T>& v)
    {
        return v * s;
    }

    template <class T>
    inline Vector2<T> operator/(const Vector2<T>& v, T s)
    {
        btFullAssert(s != T(0));
        return v * (T(1) / s);
    }

    template <class T>
    inline Vector2<T> operator/(const Vector2<T>& v1, const Vector2<T>& v2)
    {
        return Vector2<T>(
            v1.v[0] / v2.v[0],
            v1.v[1] / v2.v[1]);
    }

    template <class T>
    inline T btDot(const Vector2<T>& v1, const Vector2<T>& v2)
    {
        return v1.dot(v2);
    }

    template <class T>
    inline T btDistance2(const Vector2<T>& v1, const Vector2<T>& v2)
    {
        return v1.distance2(v2);
    }

    template <class T>
    inline T btDistance(const Vector2<T>& v1, const Vector2<T>& v2)
    {
        return v1.distance(v2);
    }

    template <class T>
    inline T btAngle(const Vector2<T>& v1, const Vector2<T>& v2)
    {
        return v1.angle(v2);
    }

    template <class T>
    inline T btCross(const Vector2<T>& v1, const Vector2<T>& v2)
    {
        return v1.cross(v2);
    }

    template <class T>
    inline Vector2<T> lerp(const Vector2<T>& v1, const Vector2<T>& v2, T t)
    {
        return v1.lerp(v2, t);
    }

    template <class T>
    T Vector2<T>::distance2(const Vector2<T>& v) const
    {
        return (v - *this).length2();
    }

    template <class T>
    T Vector2<T>::distance(const Vector2<T>& v) const
    {
        return (v - *this).length();
    }

    template <class T>
    Vector2<T> Vector2<T>::normalized() const
    {
        Vector2<T> nrm = *this;
        nrm.normalize();
        return nrm;
    }

    using Vector2f = Vector2<float>;
    using Vector2i = Vector2<int>;
    using Vector2u = Vector2<std::uint32_t>;

    static_assert(sizeof(Vector2f) == 8, "Bad Vector2f size");
    static_assert(sizeof(Vector2i) == 8, "Bad Vector2i size");
    static_assert(sizeof(Vector2u) == 8, "Bad Vector2u size");

    extern const Vector2f Vector2f_zero;
    extern const Vector2i Vector2i_zero;
    extern const Vector2u Vector2u_zero;
    extern const Vector2f Vector2f_one;
    extern const Vector2i Vector2i_one;
    extern const Vector2u Vector2u_one;

    inline bool btIsValid(const Vector2f& v)
    {
        return btIsValid(v.x()) && btIsValid(v.y());
    }

    template <class T>
    inline T btZeroNormalize(Vector2<T>& v)
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
    inline Vector2<T> btZeroNormalized(const Vector2<T>& v)
    {
        Vector2<T> n = v;
        btZeroNormalize(n);
        return n;
    }

    inline std::size_t hash_value(const Vector2u& v)
    {
        std::size_t seed = 0;
        boost::hash_combine(seed, v.x());
        boost::hash_combine(seed, v.y());
        return seed;
    }
}

template <class T>
inline std::ostream& operator <<(std::ostream& os, const af3d::Vector2<T>& value)
{
    return (os << "(" << value.x() << "," << value.y() << ")");
}

#endif

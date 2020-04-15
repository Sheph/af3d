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

#ifndef _AF3D_VECTOR3_H_
#define _AF3D_VECTOR3_H_

#include "af3d/Types.h"
#include <type_traits>
#include <ostream>

namespace af3d
{
    template <class T>
    class Vector3
    {
    public:
        T v[3];

        Vector3() = default;
        Vector3(T _x, T _y, T _z)
        {
            v[0] = _x;
            v[1] = _y;
            v[2] = _z;
        }

        Vector3<T>& operator+=(const Vector3<T>& other)
        {
            v[0] += other.v[0];
            v[1] += other.v[1];
            v[2] += other.v[2];
            return *this;
        }

        Vector3<T>& operator-=(const Vector3<T>& other)
        {
            v[0] -= other.v[0];
            v[1] -= other.v[1];
            v[2] -= other.v[2];
            return *this;
        }

        Vector3<T>& operator*=(T s)
        {
            v[0] *= s;
            v[1] *= s;
            v[2] *= s;
            return *this;
        }

        Vector3<T>& operator/=(T s)
        {
            btFullAssert(s != T(0));
            v[0] /= s;
            v[1] /= s;
            v[2] /= s;
            return *this;
        }

        T dot(const Vector3<T>& other) const
        {
            return v[0] * other.v[0] +
                v[1] * other.v[1] +
                v[2] * other.v[2];
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

        T distance2(const Vector3<T>& other) const;

        T distance(const Vector3<T>& other) const;

        T safeNormalize()
        {
            T l2 = length2();
            if (l2 >= SIMD_EPSILON * SIMD_EPSILON) {
                T d = btSqrt(l2);
                (*this) /= d;
                return d;
            } else {
                setValue(1, 0, 0);
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

        Vector3<T> normalized() const;

        T angle(const Vector3<T>& other) const
        {
            T s = btSqrt(length2() * other.length2());
            btFullAssert(s != T(0));
            return btAcos(dot(other) / s);
        }

        Vector3<T> absolute() const
        {
            return Vector3<T>(
                std::abs(v[0]),
                std::abs(v[1]),
                std::abs(v[2]));
        }

        Vector3<T> cross(const Vector3<T>& other) const
        {
            return Vector3<T>(
                v[1] * other.v[2] - v[2] * other.v[1],
                v[2] * other.v[0] - v[0] * other.v[2],
                v[0] * other.v[1] - v[1] * other.v[0]);
        }

        T triple(const Vector3<T>& v1, const Vector3<T>& v2) const
        {
            return v[0] * (v1.v[1] * v2.v[2] - v1.v[2] * v2.v[1]) +
                v[1] * (v1.v[2] * v2.v[0] - v1.v[0] * v2.v[2]) +
                v[2] * (v1.v[0] * v2.v[1] - v1.v[1] * v2.v[0]);
        }

        int minAxis() const
        {
            return v[0] < v[1] ? (v[0] < v[2] ? 0 : 2) : (v[1] < v[2] ? 1 : 2);
        }

        int maxAxis() const
        {
            return v[0] < v[1] ? (v[1] < v[2] ? 2 : 1) : (v[0] < v[2] ? 2 : 0);
        }

        int furthestAxis() const
        {
            return absolute().minAxis();
        }

        int closestAxis() const
        {
            return absolute().maxAxis();
        }

        void setInterpolate3(const Vector3<T>& v0, const Vector3<T>& v1, T rt)
        {
            T s = T(1) - rt;
            v[0] = s * v0.v[0] + rt * v1.v[0];
            v[1] = s * v0.v[1] + rt * v1.v[1];
            v[2] = s * v0.v[2] + rt * v1.v[2];
        }

        Vector3<T> lerp(const Vector3<T>& other, T t) const
        {
            return Vector3<T>(v[0] + (other.v[0] - v[0]) * t,
                v[1] + (other.v[1] - v[1]) * t,
                v[2] + (other.v[2] - v[2]) * t);
        }

        Vector3<T>& operator*=(const Vector3<T>& other)
        {
            v[0] *= other.v[0];
            v[1] *= other.v[1];
            v[2] *= other.v[2];
            return *this;
        }

        T getX() const { return v[0]; }
        T getY() const { return v[1]; }
        T getZ() const { return v[2]; }

        void setX(T _x) { v[0] = _x; };
        void setY(T _y) { v[1] = _y; };
        void setZ(T _z) { v[2] = _z; };

        T x() const { return v[0]; }
        T y() const { return v[1]; }
        T z() const { return v[2]; }

        T& operator[](int i) { return v[i]; }
        T operator[](int i) const { return v[i]; }

        bool operator==(const Vector3<T>& other) const
        {
            return (v[2] == other.v[2]) &&
                (v[1] == other.v[1]) &&
                (v[0] == other.v[0]);
        }

        bool operator!=(const Vector3<T>& other) const
        {
            return !(*this == other);
        }

        bool operator<(const Vector3<T>& other) const
        {
            if (v[0] != other.v[0]) {
                return v[0] < other.v[0];
            } else if (v[1] != other.v[1]) {
                return v[1] < other.v[1];
            } else {
                return v[2] < other.v[2];
            }
        }

        void setMax(const Vector3<T>& other)
        {
            btSetMax(v[0], other.v[0]);
            btSetMax(v[1], other.v[1]);
            btSetMax(v[2], other.v[2]);
        }

        void setMin(const Vector3<T>& other)
        {
            btSetMin(v[0], other.v[0]);
            btSetMin(v[1], other.v[1]);
            btSetMin(v[2], other.v[2]);
        }

        void setValue(T _x, T _y, T _z)
        {
            v[0] = _x;
            v[1] = _y;
            v[2] = _z;
        }

        void setZero()
        {
            setValue(T(0), T(0), T(0));
        }

        bool isZero() const
        {
            return v[0] == T(0) && v[1] == T(0) && v[2] == T(0);
        }

        bool fuzzyZero() const
        {
            return length2() < SIMD_EPSILON * SIMD_EPSILON;
        }
    };

    template <class T>
    inline Vector3<T> operator+(const Vector3<T>& v1, const Vector3<T>& v2)
    {
        return Vector3<T>(
            v1.v[0] + v2.v[0],
            v1.v[1] + v2.v[1],
            v1.v[2] + v2.v[2]);
    }

    template <class T>
    inline Vector3<T> operator*(const Vector3<T>& v1, const Vector3<T>& v2)
    {
        return Vector3<T>(
            v1.v[0] * v2.v[0],
            v1.v[1] * v2.v[1],
            v1.v[2] * v2.v[2]);
    }

    template <class T>
    inline Vector3<T> operator-(const Vector3<T>& v1, const Vector3<T>& v2)
    {
        return Vector3<T>(
            v1.v[0] - v2.v[0],
            v1.v[1] - v2.v[1],
            v1.v[2] - v2.v[2]);
    }

    template <class T>
    inline Vector3<T> operator-(const Vector3<T>& v)
    {
        return Vector3<T>(-v.v[0], -v.v[1], -v.v[2]);
    }

    template <class T>
    inline Vector3<T> operator*(const Vector3<T>& v, T s)
    {
        return Vector3<T>(v.v[0] * s, v.v[1] * s, v.v[2] * s);
    }

    template <class T>
    inline Vector3<T> operator*(T s, const Vector3<T>& v)
    {
        return v * s;
    }

    template <class T>
    inline Vector3<T> operator/(const Vector3<T>& v, T s)
    {
        btFullAssert(s != T(0));
        return v * (T(1) / s);
    }

    template <class T>
    inline Vector3<T> operator/(const Vector3<T>& v1, const Vector3<T>& v2)
    {
        return Vector3<T>(
            v1.v[0] / v2.v[0],
            v1.v[1] / v2.v[1],
            v1.v[2] / v2.v[2]);
    }

    template <class T>
    inline T btDot(const Vector3<T>& v1, const Vector3<T>& v2)
    {
        return v1.dot(v2);
    }

    template <class T>
    inline T btDistance2(const Vector3<T>& v1, const Vector3<T>& v2)
    {
        return v1.distance2(v2);
    }

    template <class T>
    inline T btDistance(const Vector3<T>& v1, const Vector3<T>& v2)
    {
        return v1.distance(v2);
    }

    template <class T>
    inline T btAngle(const Vector3<T>& v1, const Vector3<T>& v2)
    {
        return v1.angle(v2);
    }

    template <class T>
    inline Vector3<T> btCross(const Vector3<T>& v1, const Vector3<T>& v2)
    {
        return v1.cross(v2);
    }

    template <class T>
    inline T btTriple(const Vector3<T>& v1, const Vector3<T>& v2, const Vector3<T>& v3)
    {
        return v1.triple(v2, v3);
    }

    template <class T>
    inline Vector3<T> lerp(const Vector3<T>& v1, const Vector3<T>& v2, T t)
    {
        return v1.lerp(v2, t);
    }

    template <class T>
    T Vector3<T>::distance2(const Vector3<T>& v) const
    {
        return (v - *this).length2();
    }

    template <class T>
    T Vector3<T>::distance(const Vector3<T>& v) const
    {
        return (v - *this).length();
    }

    template <class T>
    Vector3<T> Vector3<T>::normalized() const
    {
        Vector3<T> nrm = *this;
        nrm.normalize();
        return nrm;
    }

    using Vector3f = Vector3<float>;
    using Vector3i = Vector3<int>;

    static_assert(sizeof(Vector3f) == 12, "Bad Vector3f size");
    static_assert(sizeof(Vector3i) == 12, "Bad Vector3i size");

    using TriFace = Vector3i;

    static_assert(std::is_pod<TriFace>::value, "TriFace must be POD type");

    extern const Vector3f Vector3f_zero;
    extern const Vector3i Vector3i_zero;
    extern const btVector3 btVector3_zero;
    extern const btVector3 btVector3_up;
    extern const btVector3 btVector3_down;
    extern const btVector3 btVector3_forward;
    extern const btVector3 btVector3_back;
    extern const btVector3 btVector3_right;
    extern const btVector3 btVector3_left;
    extern const btVector3 btVector3_one;

    inline bool btIsValid(const Vector3f& v)
    {
        return btIsValid(v.x()) && btIsValid(v.y()) && btIsValid(v.z());
    }

    inline btVector3 fromVector3f(const Vector3f& v)
    {
        return btVector3(v.x(), v.y(), v.z());
    }

    inline Vector3f toVector3f(const btVector3& v)
    {
        return Vector3f(v.x(), v.y(), v.z());
    }

    template <class T>
    inline T btZeroNormalize(Vector3<T>& v)
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
    inline Vector3<T> btZeroNormalized(const Vector3<T>& v)
    {
        Vector3<T> n = v;
        btZeroNormalize(n);
        return n;
    }

    inline btVector3 btPerpendicular(const btVector3& v)
    {
        return btFabs(v.x()) > btFabs(v.z()) ?
            btVector3(-v.y(), v.x(), 0.0f) : btVector3(0.0f, -v.z(), v.y());
    }
}

template <class T>
inline std::ostream& operator <<(std::ostream& os, const af3d::Vector3<T>& value)
{
    return (os << "(" << value.x() << "," << value.y() << "," << value.z() << ")");
}

inline std::ostream& operator <<(std::ostream& os, const btVector3& value)
{
    return (os << "(" << value.x() << "," << value.y() << "," << value.z() << ")");
}

inline std::ostream& operator <<(std::ostream& os, const btQuaternion& value)
{
    return (os << "[" << value.getAxis() << "," << btDegrees(value.getAngle()) << "deg]");
}

inline std::ostream& operator <<(std::ostream& os, const btTransform& value)
{
    return (os << "{" << value.getOrigin() << "," << value.getRotation() << "}");
}

#endif

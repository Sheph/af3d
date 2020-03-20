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

#ifndef _AF3D_MATRIX4_H_
#define _AF3D_MATRIX4_H_

#include "af3d/Vector4.h"
#include <cstring>

namespace af3d
{
    template <class T>
    class Matrix4
    {
    public:
        union
        {
            T v[16];
            T m[4][4];
            Vector4<T> row[4];
        };

        Matrix4() = default;
        Matrix4(T xx, T xy, T xz, T xw,
            T yx, T yy, T yz, T yw,
            T zx, T zy, T zz, T zw,
            T wx, T wy, T wz, T ww)
        {
            setValue(xx, xy, xz, xw,
                yx, yy, yz, yw,
                zx, zy, zz, zw,
                wx, wy, wz, ww);
        }

        Matrix4(const Vector4<T>& v0, const Vector4<T>& v1, const Vector4<T>& v2, const Vector4<T>& v3)
        {
            row[0] = v0;
            row[1] = v1;
            row[2] = v2;
            row[3] = v3;
        }

        explicit Matrix4(const btTransform& xf)
        {
            setTransform(xf);
        }

        Matrix4(const btVector3& t, const btQuaternion& q)
        {
            setTransform(t, q);
        }

        void setTransform(const btTransform& xf)
        {
            setTranslation(xf.getOrigin());
            setBasis(xf.getBasis());
        }

        void setTransform(const btVector3& t, const btQuaternion& q)
        {
            setTransform(btTransform(q, t));
        }

        void setTranslation(const btVector3& t)
        {
            row[0][3] = t[0];
            row[1][3] = t[1];
            row[2][3] = t[2];
            row[3][3] = T(1);
        }

        void setRotation(const btQuaternion& q)
        {
            setBasis(btMatrix3x3(q));
        }

        void setBasis(const btMatrix3x3& basis)
        {
            v[0] = basis[0].x();
            v[1] = basis[0].y();
            v[2] = basis[0].z();
            v[4] = basis[1].x();
            v[5] = basis[1].y();
            v[6] = basis[1].z();
            v[8] = basis[2].x();
            v[9] = basis[2].y();
            v[10] = basis[2].z();
            v[12] = T(0);
            v[13] = T(0);
            v[14] = T(0);
            v[15] = T(1);
        }

        Vector4<T> getColumn(int i) const
        {
            return Vector4<T>(m[0][i], m[1][i], m[2][i], m[3][i]);
        }

        const Vector4<T>& getRow(int i) const
        {
            return row[i];
        }

        Vector4<T>& operator[](int i)
        {
            return row[i];
        }

        const Vector4<T>& operator[](int i) const
        {
            return row[i];
        }

        Matrix4<T>& operator*=(const Matrix4<T>& other);

        Matrix4<T>& operator+=(const Matrix4<T>& other);

        Matrix4<T>& operator-=(const Matrix4<T>& other);

        void setValue(T xx, T xy, T xz, T xw,
            T yx, T yy, T yz, T yw,
            T zx, T zy, T zz, T zw,
            T wx, T wy, T wz, T ww)
        {
            row[0].setValue(xx, xy, xz, xw);
            row[1].setValue(yx, yy, yz, yw);
            row[2].setValue(zx, zy, zz, zw);
            row[3].setValue(wx, wy, wz, ww);
        }

        void setIdentity()
        {
            setValue(T(1), T(0), T(0), T(0),
                     T(0), T(1), T(0), T(0),
                     T(0), T(0), T(1), T(0),
                     T(0), T(0), T(0), T(1));
        }

        void setPerspective(T left, T right, T bottom, T top, T zNear, T zFar)
        {
            T invW = T(1) / (right - left);
            T invH = T(1) / (top - bottom);
            T invD = T(1) / (zFar - zNear);

            T A = T(2) * zNear * invW;
            T B = T(2) * zNear * invH;
            T C = (right + left) * invW;
            T D = (top + bottom) * invH;
            T q = -(zFar + zNear) * invD;
            T qn = T(-2) * (zFar * zNear) * invD;

            std::memset(&v[0], 0, sizeof(T[16]));

            m[0][0] = A;
            m[0][2] = C;
            m[1][1] = B;
            m[1][2] = D;
            m[2][2] = q;
            m[2][3] = qn;
            m[3][2] = T(-1);
        }

        void setOrtho(T left, T right, T bottom, T top, T zNear, T zFar)
        {
            T a = T(2) / (right - left);
            T b = T(2) / (top - bottom);
            T c = T(-2) / (zFar - zNear);

            T tx = -(right + left) / (right - left);
            T ty = -(top + bottom) / (top - bottom);
            T tz = -(zFar + zNear) / (zFar - zNear);

            std::memset(&v[0], 0, sizeof(T[16]));

            m[0][0] = a;
            m[1][1] = b;
            m[2][2] = c;
            m[0][3] = tx;
            m[1][3] = ty;
            m[2][3] = tz;
            m[3][3] = T(1);
        }

        static const Matrix4<T>& getIdentity()
        {
            static const Matrix4<T>
                identityMatrix(
                    T(1), T(0), T(0), T(0),
                    T(0), T(1), T(0), T(0),
                    T(0), T(0), T(1), T(0),
                    T(0), T(0), T(0), T(1));
            return identityMatrix;
        }

        Matrix4<T> scaled(const btVector3& s) const
        {
            return scaled(Vector4<T>(s.x(), s.y(), s.z(), T(1)));
        }

        Matrix4<T> scaled(const Vector4<T>& s) const
        {
            return Matrix4<T>(
                row[0].x() * s.x(), row[0].y() * s.y(), row[0].z() * s.z(), row[0].w() * s.w(),
                row[1].x() * s.x(), row[1].y() * s.y(), row[1].z() * s.z(), row[1].w() * s.w(),
                row[2].x() * s.x(), row[2].y() * s.y(), row[2].z() * s.z(), row[2].w() * s.w(),
                row[3].x() * s.x(), row[3].y() * s.y(), row[3].z() * s.z(), row[3].w() * s.w());
        }

        T tdotx(const Vector4<T>& other) const
        {
            return row[0].x() * other.x() + row[1].x() * other.y() + row[2].x() * other.z() + row[3].x() * other.w();
        }

        T tdoty(const Vector4<T>& other) const
        {
            return row[0].y() * other.x() + row[1].y() * other.y() + row[2].y() * other.z() + row[3].y() * other.w();
        }

        T tdotz(const Vector4<T>& other) const
        {
            return row[0].z() * other.x() + row[1].z() * other.y() + row[2].z() * other.z() + row[3].z() * other.w();
        }

        T tdotw(const Vector4<T>& other) const
        {
            return row[0].w() * other.x() + row[1].w() * other.y() + row[2].w() * other.z() + row[3].w() * other.w();
        }

        T cofac(int r1, int c1, int r2, int c2, int r3, int c3) const
        {
            return row[r1][c1] * (row[r2][c2] * row[r3][c3] - row[r3][c2] * row[r2][c3]) -
                row[r1][c2] * (row[r2][c1] * row[r3][c3] - row[r3][c1] * row[r2][c3]) +
                row[r1][c3] * (row[r2][c1] * row[r3][c2] - row[r3][c1] * row[r2][c2]);
        }

        T determinant() const;
        Matrix4<T> transpose() const;
        Matrix4<T> inverse() const;
    };

    template <class T>
    inline Matrix4<T>& Matrix4<T>::operator*=(const Matrix4<T>& other)
    {
        setValue(
            other.tdotx(row[0]), other.tdoty(row[0]), other.tdotz(row[0]), other.tdotw(row[0]),
            other.tdotx(row[1]), other.tdoty(row[1]), other.tdotz(row[1]), other.tdotw(row[1]),
            other.tdotx(row[2]), other.tdoty(row[2]), other.tdotz(row[2]), other.tdotw(row[2]),
            other.tdotx(row[3]), other.tdoty(row[3]), other.tdotz(row[3]), other.tdotw(row[3]));
        return *this;
    }

    template <class T>
    inline Matrix4<T>& Matrix4<T>::operator+=(const Matrix4<T>& other)
    {
        setValue(
            row[0][0] + other.row[0][0],
            row[0][1] + other.row[0][1],
            row[0][2] + other.row[0][2],
            row[0][3] + other.row[0][3],
            row[1][0] + other.row[1][0],
            row[1][1] + other.row[1][1],
            row[1][2] + other.row[1][2],
            row[1][3] + other.row[1][3],
            row[2][0] + other.row[2][0],
            row[2][1] + other.row[2][1],
            row[2][2] + other.row[2][2],
            row[2][3] + other.row[2][3],
            row[3][0] + other.row[3][0],
            row[3][1] + other.row[3][1],
            row[3][2] + other.row[3][2],
            row[3][3] + other.row[3][3]);
        return *this;
    }

    template <class T>
    inline Matrix4<T>& Matrix4<T>::operator-=(const Matrix4<T>& other)
    {
        setValue(
            row[0][0] - other.row[0][0],
            row[0][1] - other.row[0][1],
            row[0][2] - other.row[0][2],
            row[0][3] - other.row[0][3],
            row[1][0] - other.row[1][0],
            row[1][1] - other.row[1][1],
            row[1][2] - other.row[1][2],
            row[1][3] - other.row[1][3],
            row[2][0] - other.row[2][0],
            row[2][1] - other.row[2][1],
            row[2][2] - other.row[2][2],
            row[2][3] - other.row[2][3],
            row[3][0] - other.row[3][0],
            row[3][1] - other.row[3][1],
            row[3][2] - other.row[3][2],
            row[3][3] - other.row[3][3]);
        return *this;
    }

    template <class T>
    inline T Matrix4<T>::determinant() const
    {
        return row[0][0] * cofac(1, 1, 2, 2, 3, 3) -
            row[0][1] * cofac(1, 0, 2, 2, 3, 3) +
            row[0][2] * cofac(1, 0, 2, 1, 3, 3) -
            row[0][3] * cofac(1, 0, 2, 1, 3, 2);
    }

    template <class T>
    inline Matrix4<T> Matrix4<T>::transpose() const
    {
        return Matrix4<T>(row[0].x(), row[1].x(), row[2].x(), row[3].x(),
            row[0].y(), row[1].y(), row[2].y(), row[3].y(),
            row[0].z(), row[1].z(), row[2].z(), row[3].z(),
            row[0].w(), row[1].w(), row[2].w(), row[3].w());
    }

    template <class T>
    inline Matrix4<T> Matrix4<T>::inverse() const
    {
        T m00 = m[0][0], m01 = m[0][1], m02 = m[0][2], m03 = m[0][3];
        T m10 = m[1][0], m11 = m[1][1], m12 = m[1][2], m13 = m[1][3];
        T m20 = m[2][0], m21 = m[2][1], m22 = m[2][2], m23 = m[2][3];
        T m30 = m[3][0], m31 = m[3][1], m32 = m[3][2], m33 = m[3][3];

        T v0 = m20 * m31 - m21 * m30;
        T v1 = m20 * m32 - m22 * m30;
        T v2 = m20 * m33 - m23 * m30;
        T v3 = m21 * m32 - m22 * m31;
        T v4 = m21 * m33 - m23 * m31;
        T v5 = m22 * m33 - m23 * m32;

        T t00 = + (v5 * m11 - v4 * m12 + v3 * m13);
        T t10 = - (v5 * m10 - v2 * m12 + v1 * m13);
        T t20 = + (v4 * m10 - v2 * m11 + v0 * m13);
        T t30 = - (v3 * m10 - v1 * m11 + v0 * m12);

        T invDet = 1 / (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);

        T d00 = t00 * invDet;
        T d10 = t10 * invDet;
        T d20 = t20 * invDet;
        T d30 = t30 * invDet;

        T d01 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
        T d11 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
        T d21 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
        T d31 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

        v0 = m10 * m31 - m11 * m30;
        v1 = m10 * m32 - m12 * m30;
        v2 = m10 * m33 - m13 * m30;
        v3 = m11 * m32 - m12 * m31;
        v4 = m11 * m33 - m13 * m31;
        v5 = m12 * m33 - m13 * m32;

        T d02 = + (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
        T d12 = - (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
        T d22 = + (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
        T d32 = - (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

        v0 = m21 * m10 - m20 * m11;
        v1 = m22 * m10 - m20 * m12;
        v2 = m23 * m10 - m20 * m13;
        v3 = m22 * m11 - m21 * m12;
        v4 = m23 * m11 - m21 * m13;
        v5 = m23 * m12 - m22 * m13;

        T d03 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
        T d13 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
        T d23 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
        T d33 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

        return Matrix4<T>(
            d00, d01, d02, d03,
            d10, d11, d12, d13,
            d20, d21, d22, d23,
            d30, d31, d32, d33);
    }

    template <class T>
    inline Matrix4<T> operator*(const Matrix4<T>& m, T k)
    {
        return Matrix4<T>(
            m[0].x() * k, m[0].y() * k, m[0].z() * k, m[0].w() * k,
            m[1].x() * k, m[1].y() * k, m[1].z() * k, m[1].w() * k,
            m[2].x() * k, m[2].y() * k, m[2].z() * k, m[2].w() * k,
            m[3].x() * k, m[3].y() * k, m[3].z() * k, m[3].w() * k);
    }

    template <class T>
    inline Matrix4<T> operator+(const Matrix4<T>& m1, const Matrix4<T>& m2)
    {
        return Matrix4<T>(
            m1[0][0] + m2[0][0],
            m1[0][1] + m2[0][1],
            m1[0][2] + m2[0][2],
            m1[0][3] + m2[0][3],

            m1[1][0] + m2[1][0],
            m1[1][1] + m2[1][1],
            m1[1][2] + m2[1][2],
            m1[1][3] + m2[1][3],

            m1[2][0] + m2[2][0],
            m1[2][1] + m2[2][1],
            m1[2][2] + m2[2][2],
            m1[2][3] + m2[2][3],

            m1[3][0] + m2[3][0],
            m1[3][1] + m2[3][1],
            m1[3][2] + m2[3][2],
            m1[3][3] + m2[3][3]);
    }

    template <class T>
    inline Matrix4<T> operator-(const Matrix4<T>& m1, const Matrix4<T>& m2)
    {
        return Matrix4<T>(
            m1[0][0] - m2[0][0],
            m1[0][1] - m2[0][1],
            m1[0][2] - m2[0][2],
            m1[0][3] - m2[0][3],

            m1[1][0] - m2[1][0],
            m1[1][1] - m2[1][1],
            m1[1][2] - m2[1][2],
            m1[1][3] - m2[1][3],

            m1[2][0] - m2[2][0],
            m1[2][1] - m2[2][1],
            m1[2][2] - m2[2][2],
            m1[2][3] - m2[2][3],

            m1[3][0] - m2[3][0],
            m1[3][1] - m2[3][1],
            m1[3][2] - m2[3][2],
            m1[3][3] - m2[3][3]);
    }

    template <class T>
    inline Vector4<T> operator*(const Matrix4<T>& m, const Vector4<T>& v)
    {
        return Vector4<T>(m[0].dot(v), m[1].dot(v), m[2].dot(v), m[3].dot(v));
    }

    template <class T>
    inline Vector4<T> operator*(const Vector4<T>& v, const Matrix4<T>& m)
    {
        return Vector4<T>(m.tdotx(v), m.tdoty(v), m.tdotz(v), m.tdotw(v));
    }

    template <class T>
    inline Matrix4<T> operator*(const Matrix4<T>& m1, const Matrix4<T>& m2)
    {
        return Matrix4<T>(
            m2.tdotx(m1[0]), m2.tdoty(m1[0]), m2.tdotz(m1[0]), m2.tdotw(m1[0]),
            m2.tdotx(m1[1]), m2.tdoty(m1[1]), m2.tdotz(m1[1]), m2.tdotw(m1[1]),
            m2.tdotx(m1[2]), m2.tdoty(m1[2]), m2.tdotz(m1[2]), m2.tdotw(m1[2]),
            m2.tdotx(m1[3]), m2.tdoty(m1[3]), m2.tdotz(m1[3]), m2.tdotw(m1[3]));
    }

    template <class T>
    inline bool operator==(const Matrix4<T>& m1, const Matrix4<T>& m2)
    {
        return (m1[0] == m2[0] && m1[1] == m2[1] && m1[2] == m2[2] && m1[3] == m2[3]);
    }

    template <class T>
    inline bool operator!=(const Matrix4<T>& m1, const Matrix4<T>& m2)
    {
        return !(m1 == m2);
    }

    using Matrix4f = Matrix4<float>;
    using Matrix4i = Matrix4<int>;

    static_assert(sizeof(Matrix4f) == 64, "Bad Matrix4f size");
    static_assert(sizeof(Matrix4i) == 64, "Bad Matrix4i size");
}

#endif

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

#ifndef _AF3D_MATRIX3_H_
#define _AF3D_MATRIX3_H_

#include "af3d/Vector3.h"

namespace af3d
{
    template <class T>
    class Matrix3
    {
    public:
        union
        {
            T v[9];
            T m[3][3];
            Vector3<T> row[3];
        };

        Matrix3() = default;
        Matrix3(T xx, T xy, T xz,
            T yx, T yy, T yz,
            T zx, T zy, T zz)
        {
            setValue(xx, xy, xz,
                yx, yy, yz,
                zx, zy, zz);
        }

        Matrix3(const Vector3<T>& v0, const Vector3<T>& v1, const Vector3<T>& v2)
        {
            row[0] = v0;
            row[1] = v1;
            row[2] = v2;
        }

        explicit Matrix3(const btQuaternion& q)
        {
            setRotation(q);
        }

        explicit Matrix3(const btMatrix3x3& basis)
        {
            setBasis(basis);
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
            v[3] = basis[1].x();
            v[4] = basis[1].y();
            v[5] = basis[1].z();
            v[6] = basis[2].x();
            v[7] = basis[2].y();
            v[8] = basis[2].z();
        }

        Vector3<T> getColumn(int i) const
        {
            return Vector3<T>(m[0][i], m[1][i], m[2][i]);
        }

        const Vector3<T>& getRow(int i) const
        {
            return row[i];
        }

        Vector3<T>& operator[](int i)
        {
            return row[i];
        }

        const Vector3<T>& operator[](int i) const
        {
            return row[i];
        }

        Matrix3<T>& operator*=(const Matrix3<T>& other);

        Matrix3<T>& operator+=(const Matrix3<T>& other);

        Matrix3<T>& operator-=(const Matrix3<T>& other);

        void setValue(T xx, T xy, T xz,
            T yx, T yy, T yz,
            T zx, T zy, T zz)
        {
            row[0].setValue(xx, xy, xz);
            row[1].setValue(yx, yy, yz);
            row[2].setValue(zx, zy, zz);
        }

        void setIdentity()
        {
            setValue(T(1), T(0), T(0),
                     T(0), T(1), T(0),
                     T(0), T(0), T(1));
        }

        static const Matrix3<T>& getIdentity()
        {
            static const Matrix3<T>
                identityMatrix(
                    T(1), T(0), T(0),
                    T(0), T(1), T(0),
                    T(0), T(0), T(1));
            return identityMatrix;
        }

        Matrix3<T> scaled(const btVector3& s) const
        {
            return scaled(Vector3<T>(s.x(), s.y(), s.z()));
        }

        Matrix3<T> scaled(const Vector3<T>& s) const
        {
            return Matrix3<T>(
                row[0].x() * s.x(), row[0].y() * s.y(), row[0].z() * s.z(),
                row[1].x() * s.x(), row[1].y() * s.y(), row[1].z() * s.z(),
                row[2].x() * s.x(), row[2].y() * s.y(), row[2].z() * s.z());
        }

        T tdotx(const Vector3<T>& other) const
        {
            return row[0].x() * other.x() + row[1].x() * other.y() + row[2].x() * other.z();
        }

        T tdoty(const Vector3<T>& other) const
        {
            return row[0].y() * other.x() + row[1].y() * other.y() + row[2].y() * other.z();
        }

        T tdotz(const Vector3<T>& other) const
        {
            return row[0].z() * other.x() + row[1].z() * other.y() + row[2].z() * other.z();
        }

        T cofac(int r1, int c1, int r2, int c2) const
        {
            return row[r1][c1] * row[r2][c2] - row[r1][c2] * row[r2][c1];
        }

        T determinant() const;
        Matrix3<T> transpose() const;
        Matrix3<T> inverse() const;
    };

    template <class T>
    inline Matrix3<T>& Matrix3<T>::operator*=(const Matrix3<T>& other)
    {
        setValue(
            other.tdotx(row[0]), other.tdoty(row[0]), other.tdotz(row[0]),
            other.tdotx(row[1]), other.tdoty(row[1]), other.tdotz(row[1]),
            other.tdotx(row[2]), other.tdoty(row[2]), other.tdotz(row[2]));
        return *this;
    }

    template <class T>
    inline Matrix3<T>& Matrix3<T>::operator+=(const Matrix3<T>& other)
    {
        setValue(
            row[0][0] + other.row[0][0],
            row[0][1] + other.row[0][1],
            row[0][2] + other.row[0][2],
            row[1][0] + other.row[1][0],
            row[1][1] + other.row[1][1],
            row[1][2] + other.row[1][2],
            row[2][0] + other.row[2][0],
            row[2][1] + other.row[2][1],
            row[2][2] + other.row[2][2]);
        return *this;
    }

    template <class T>
    inline Matrix3<T>& Matrix3<T>::operator-=(const Matrix3<T>& other)
    {
        setValue(
            row[0][0] - other.row[0][0],
            row[0][1] - other.row[0][1],
            row[0][2] - other.row[0][2],
            row[1][0] - other.row[1][0],
            row[1][1] - other.row[1][1],
            row[1][2] - other.row[1][2],
            row[2][0] - other.row[2][0],
            row[2][1] - other.row[2][1],
            row[2][2] - other.row[2][2]);
        return *this;
    }

    template <class T>
    inline T Matrix3<T>::determinant() const
    {
        return btTriple((*this)[0], (*this)[1], (*this)[2]);
    }

    template <class T>
    inline Matrix3<T> Matrix3<T>::transpose() const
    {
        return Matrix3<T>(row[0].x(), row[1].x(), row[2].x(),
                row[0].y(), row[1].y(), row[2].y(),
                row[0].z(), row[1].z(), row[2].z());
    }

    template <class T>
    inline Matrix3<T> Matrix3<T>::inverse() const
    {
        Vector3<T> co(cofac(1, 1, 2, 2), cofac(1, 2, 2, 0), cofac(1, 0, 2, 1));
        T det = (*this)[0].dot(co);
        btAssert(det != T(0));
        T s = T(1) / det;
        return Matrix3<T>(co.x() * s, cofac(0, 2, 2, 1) * s, cofac(0, 1, 1, 2) * s,
            co.y() * s, cofac(0, 0, 2, 2) * s, cofac(0, 2, 1, 0) * s,
            co.z() * s, cofac(0, 1, 2, 0) * s, cofac(0, 0, 1, 1) * s);
    }

    template <class T>
    inline Matrix3<T> operator*(const Matrix3<T>& m, T k)
    {
        return Matrix3<T>(
            m[0].x() * k, m[0].y() * k, m[0].z() * k,
            m[1].x() * k, m[1].y() * k, m[1].z() * k,
            m[2].x() * k, m[2].y() * k, m[2].z() * k);
    }

    template <class T>
    inline Matrix3<T> operator+(const Matrix3<T>& m1, const Matrix3<T>& m2)
    {
        return Matrix3<T>(
            m1[0][0] + m2[0][0],
            m1[0][1] + m2[0][1],
            m1[0][2] + m2[0][2],

            m1[1][0] + m2[1][0],
            m1[1][1] + m2[1][1],
            m1[1][2] + m2[1][2],

            m1[2][0] + m2[2][0],
            m1[2][1] + m2[2][1],
            m1[2][2] + m2[2][2]);
    }

    template <class T>
    inline Matrix3<T> operator-(const Matrix3<T>& m1, const Matrix3<T>& m2)
    {
        return Matrix3<T>(
            m1[0][0] - m2[0][0],
            m1[0][1] - m2[0][1],
            m1[0][2] - m2[0][2],

            m1[1][0] - m2[1][0],
            m1[1][1] - m2[1][1],
            m1[1][2] - m2[1][2],

            m1[2][0] - m2[2][0],
            m1[2][1] - m2[2][1],
            m1[2][2] - m2[2][2]);
    }

    template <class T>
    inline Vector3<T> operator*(const Matrix3<T>& m, const Vector3<T>& v)
    {
        return Vector3<T>(m[0].dot(v), m[1].dot(v), m[2].dot(v));
    }

    template <class T>
    inline Vector3<T> operator*(const Vector3<T>& v, const Matrix3<T>& m)
    {
        return Vector3<T>(m.tdotx(v), m.tdoty(v), m.tdotz(v));
    }

    template <class T>
    inline Matrix3<T> operator*(const Matrix3<T>& m1, const Matrix3<T>& m2)
    {
        return Matrix3<T>(
            m2.tdotx(m1[0]), m2.tdoty(m1[0]), m2.tdotz(m1[0]),
            m2.tdotx(m1[1]), m2.tdoty(m1[1]), m2.tdotz(m1[1]),
            m2.tdotx(m1[2]), m2.tdoty(m1[2]), m2.tdotz(m1[2]));
    }

    template <class T>
    inline bool operator==(const Matrix3<T>& m1, const Matrix3<T>& m2)
    {
        return (m1[0] == m2[0] && m1[1] == m2[1] && m1[2] == m2[2]);
    }

    template <class T>
    inline bool operator!=(const Matrix3<T>& m1, const Matrix3<T>& m2)
    {
        return !(m1 == m2);
    }

    using Matrix3f = Matrix3<float>;
    using Matrix3i = Matrix3<int>;

    static_assert(sizeof(Matrix3f) == 36, "Bad Matrix3f size");
    static_assert(sizeof(Matrix3i) == 36, "Bad Matrix3i size");

    inline btMatrix3x3 fromMatrix3f(const Matrix3f& m)
    {
        return btMatrix3x3(m[0][0], m[0][1], m[0][2],
            m[1][0], m[1][1], m[1][2],
            m[2][0], m[2][1], m[2][2]);
    }

    inline Matrix3f toMatrix3f(const btMatrix3x3& m)
    {
        return Matrix3f(m[0][0], m[0][1], m[0][2],
            m[1][0], m[1][1], m[1][2],
            m[2][0], m[2][1], m[2][2]);
    }
}

#endif

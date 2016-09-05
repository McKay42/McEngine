//============ Copyright (c) 2011, Song Ho Ahn, All rights reserved. =============//
//
// Purpose:		2/3/4D Vectors | http://www.songho.ca/opengl/gl_matrix.html
//
// $NoKeywords: $vec
//================================================================================//

#ifndef VECTORS_H
#define VECTORS_H

#include <cmath>
#include <iostream>

//*****************************************************************************
// 2D vector
//*****************************************************************************
struct Vector2
{
    float x;
    float y;


    Vector2() : x(0), y(0) {};
    Vector2(float x, float y) : x(x), y(y) {};


    void		zero();
    void        set(float x, float y);
    float       length() const;
    float       distance(const Vector2& vec) const;
    Vector2&    normalize();
    float       dot(const Vector2& vec) const;
    bool        equal(const Vector2& vec, float e) const;


    Vector2     operator-() const;
    Vector2     operator+(const Vector2& rhs) const;
    Vector2     operator-(const Vector2& rhs) const;
    Vector2&    operator+=(const Vector2& rhs);
    Vector2&    operator-=(const Vector2& rhs);
    Vector2     operator*(const float scale) const;
    Vector2     operator*(const Vector2& rhs) const;
    Vector2&    operator*=(const float scale);
    Vector2&    operator*=(const Vector2& rhs);
    Vector2     operator/(const float scale) const;
    Vector2&    operator/=(const float scale);
    bool        operator==(const Vector2& rhs) const;
    bool        operator!=(const Vector2& rhs) const;
    bool        operator<(const Vector2& rhs) const;
    float       operator[](int index) const;
    float&      operator[](int index);


    friend Vector2 operator*(const float a, const Vector2 vec);
    friend std::ostream& operator<<(std::ostream& os, const Vector2& vec);
};



//*****************************************************************************
// 3D vector
//*****************************************************************************
struct Vector3
{
    float x;
    float y;
    float z;


    Vector3() : x(0), y(0), z(0) {};
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {};

    void		zero();
    void        set(float x, float y, float z);
    void		setLength(float length);
    float       length() const;
    float       distance(const Vector3& vec) const;
    Vector3&    normalize();
    float       dot(const Vector3& vec) const;
    Vector3     cross(const Vector3& vec) const;
    bool        equal(const Vector3& vec, float e) const;


    Vector3     operator-() const;
    Vector3     operator+(const Vector3& rhs) const;
    Vector3     operator-(const Vector3& rhs) const;
    Vector3&    operator+=(const Vector3& rhs);
    Vector3&    operator-=(const Vector3& rhs);
    Vector3     operator*(const float scale) const;
    Vector3     operator*(const Vector3& rhs) const;
    Vector3&    operator*=(const float scale);
    Vector3&    operator*=(const Vector3& rhs);
    Vector3     operator/(const float scale) const;
    Vector3&    operator/=(const float scale);
    bool        operator==(const Vector3& rhs) const;
    bool        operator!=(const Vector3& rhs) const;
    bool        operator<(const Vector3& rhs) const;
    float       operator[](int index) const;
    float&      operator[](int index);


    friend Vector3 operator*(const float a, const Vector3 vec);
    friend std::ostream& operator<<(std::ostream& os, const Vector3& vec);
};



//*****************************************************************************
// 4D vector
//*****************************************************************************
struct Vector4
{
    float x;
    float y;
    float z;
    float w;


    Vector4() : x(0), y(0), z(0), w(0) {};
    Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {};


    void 		zero();
    void        set(float x, float y, float z, float w);
    float       length() const;
    float       distance(const Vector4& vec) const;
    Vector4&    normalize();
    float       dot(const Vector4& vec) const;
    bool        equal(const Vector4& vec, float e) const;


    Vector4     operator-() const;
    Vector4     operator+(const Vector4& rhs) const;
    Vector4     operator-(const Vector4& rhs) const;
    Vector4&    operator+=(const Vector4& rhs);
    Vector4&    operator-=(const Vector4& rhs);
    Vector4     operator*(const float scale) const;
    Vector4     operator*(const Vector4& rhs) const;
    Vector4&    operator*=(const float scale);
    Vector4&    operator*=(const Vector4& rhs);
    Vector4     operator/(const float scale) const;
    Vector4&    operator/=(const float scale);
    bool        operator==(const Vector4& rhs) const;
    bool        operator!=(const Vector4& rhs) const;
    bool        operator<(const Vector4& rhs) const;
    float       operator[](int index) const;
    float&      operator[](int index);


    friend Vector4 operator*(const float a, const Vector4 vec);
    friend std::ostream& operator<<(std::ostream& os, const Vector4& vec);
};



// fast invSqrt, stolen from the Doom 3 SDK, hah!
/*
inline float invSqrt(float x)
{
    float xhalf = 0.5f * x;
    int i = *(int*)&x;          // get bits for floating value
    i = 0x5f3759df - (i>>1);    // gives initial guess
    x = *(float*)&i;            // convert bits back to float
    x = x * (1.5f - xhalf*x*x); // Newton step
    return x;
}
*/


//*****************************************************************************
// inline Vector2
//*****************************************************************************
inline Vector2 Vector2::operator- () const
{
    return Vector2(-x, -y);
}

inline Vector2 Vector2::operator+ (const Vector2& rhs) const
{
    return Vector2(x+rhs.x, y+rhs.y);
}

inline Vector2 Vector2::operator- (const Vector2& rhs) const
{
    return Vector2(x-rhs.x, y-rhs.y);
}

inline Vector2& Vector2::operator+= (const Vector2& rhs)
{
    x += rhs.x;
    y += rhs.y;
    return *this;
}

inline Vector2& Vector2::operator-= (const Vector2& rhs)
{
    x -= rhs.x;
    y -= rhs.y;
    return *this;
}

inline Vector2 Vector2::operator* (const float a) const
{
    return Vector2(x*a, y*a);
}

inline Vector2 Vector2::operator* (const Vector2& rhs) const
{
    return Vector2(x*rhs.x, y*rhs.y);
}

inline Vector2& Vector2::operator*= (const float a)
{
    x *= a;
    y *= a;
    return *this;
}

inline Vector2& Vector2::operator*= (const Vector2& rhs)
{
    x *= rhs.x;
    y *= rhs.y;
    return *this;
}

inline Vector2 Vector2::operator/ (const float a) const
{
    return Vector2(x/a, y/a);
}

inline Vector2& Vector2::operator/= (const float a)
{
    x /= a;
    y /= a;
    return *this;
}

inline bool Vector2::operator== (const Vector2& rhs) const
{
    return (x == rhs.x) && (y == rhs.y);
}

inline bool Vector2::operator!= (const Vector2& rhs) const
{
    return (x != rhs.x) || (y != rhs.y);
}

inline bool Vector2::operator< (const Vector2& rhs) const
{
    if(x < rhs.x) return true;
    if(x > rhs.x) return false;
    if(y < rhs.y) return true;
    if(y > rhs.y) return false;
    return false;
}

inline float Vector2::operator[] (int index) const
{
    return (&x)[index];
}

inline float& Vector2::operator[] (int index)
{
    return (&x)[index];
}

inline void Vector2::zero()
{
	x = 0;
	y = 0;
}

inline void Vector2::set(float x, float y)
{
    this->x = x; this->y = y;
}

inline float Vector2::length() const
{
    return std::sqrt(x*x + y*y);
}

inline float Vector2::distance(const Vector2& vec) const
{
    return std::sqrt((vec.x-x)*(vec.x-x) + (vec.y-y)*(vec.y-y));
}

inline Vector2& Vector2::normalize()
{
    const float EPSILON = 0.000001f;
    float xxyy = x*x + y*y;
    if(xxyy < EPSILON)
        return *this;

    //float invLength = invSqrt(xxyy);
    float invLength = 1.0f / std::sqrt(xxyy);
    x *= invLength;
    y *= invLength;
    return *this;
}

inline float Vector2::dot(const Vector2& rhs) const
{
    return (x*rhs.x + y*rhs.y);
}

inline bool Vector2::equal(const Vector2& rhs, float epsilon) const
{
    return std::abs(x - rhs.x) < epsilon && std::abs(y - rhs.y) < epsilon;
}

inline Vector2 operator*(const float a, const Vector2 vec)
{
    return Vector2(a*vec.x, a*vec.y);
}

inline std::ostream& operator<<(std::ostream& os, const Vector2& vec)
{
    os << "(" << vec.x << ", " << vec.y << ")";
    return os;
}




//*****************************************************************************
// inline Vector3
//*****************************************************************************
inline Vector3 Vector3::operator- () const
{
    return Vector3(-x, -y, -z);
}

inline Vector3 Vector3::operator+ (const Vector3& rhs) const
{
    return Vector3(x+rhs.x, y+rhs.y, z+rhs.z);
}

inline Vector3 Vector3::operator- (const Vector3& rhs) const
{
    return Vector3(x-rhs.x, y-rhs.y, z-rhs.z);
}

inline Vector3& Vector3::operator+= (const Vector3& rhs)
{
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
}

inline Vector3& Vector3::operator-= (const Vector3& rhs)
{
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    return *this;
}

inline Vector3 Vector3::operator* (const float a) const
{
    return Vector3(x*a, y*a, z*a);
}

inline Vector3 Vector3::operator* (const Vector3& rhs) const
{
    return Vector3(x*rhs.x, y*rhs.y, z*rhs.z);
}

inline Vector3& Vector3::operator*= (const float a)
{
    x *= a;
    y *= a;
    z *= a;
    return *this;
}

inline Vector3& Vector3::operator*= (const Vector3& rhs)
{
    x *= rhs.x;
    y *= rhs.y;
    z *= rhs.z;
    return *this;
}

inline Vector3 Vector3::operator/ (const float a) const
{
    return Vector3(x/a, y/a, z/a);
}

inline Vector3& Vector3::operator/= (const float a)
{
    x /= a;
    y /= a;
    z /= a;
    return *this;
}

inline bool Vector3::operator== (const Vector3& rhs) const
{
    return (x == rhs.x) && (y == rhs.y) && (z == rhs.z);
}

inline bool Vector3::operator!= (const Vector3& rhs) const
{
    return (x != rhs.x) || (y != rhs.y) || (z != rhs.z);
}

inline bool Vector3::operator< (const Vector3& rhs) const
{
    if(x < rhs.x) return true;
    if(x > rhs.x) return false;
    if(y < rhs.y) return true;
    if(y > rhs.y) return false;
    if(z < rhs.z) return true;
    if(z > rhs.z) return false;
    return false;
}

inline float Vector3::operator[] (int index) const
{
    return (&x)[index];
}

inline float& Vector3::operator[] (int index)
{
    return (&x)[index];
}

inline void Vector3::zero()
{
	x = 0;
	y = 0;
	z = 0;
}

inline void Vector3::set(float x, float y, float z)
{
    this->x = x;
    this->y = y;
    this->z = z;
}

inline void Vector3::setLength(float length)
{
	normalize();
	x *= length;
	y *= length;
	z *= length;
}

inline float Vector3::length() const
{
    return std::sqrt(x*x + y*y + z*z);
}

inline float Vector3::distance(const Vector3& vec) const
{
    return std::sqrt((vec.x-x)*(vec.x-x) + (vec.y-y)*(vec.y-y) + (vec.z-z)*(vec.z-z));
}

inline Vector3& Vector3::normalize()
{
    const float EPSILON = 0.000001f;
    float xxyyzz = x*x + y*y + z*z;
    if(xxyyzz < EPSILON)
        return *this; // do nothing if it is ~zero vector

    //float invLength = invSqrt(xxyyzz);
    float invLength = 1.0f / std::sqrt(xxyyzz);
    x *= invLength;
    y *= invLength;
    z *= invLength;
    return *this;
}

inline float Vector3::dot(const Vector3& rhs) const
{
    return (x*rhs.x + y*rhs.y + z*rhs.z);
}

inline Vector3 Vector3::cross(const Vector3& rhs) const
{
    return Vector3(y*rhs.z - z*rhs.y, z*rhs.x - x*rhs.z, x*rhs.y - y*rhs.x);
}

inline bool Vector3::equal(const Vector3& rhs, float epsilon) const
{
    return std::abs(x - rhs.x) < epsilon && std::abs(y - rhs.y) < epsilon && std::abs(z - rhs.z) < epsilon;
}

inline Vector3 operator*(const float a, const Vector3 vec)
{
    return Vector3(a*vec.x, a*vec.y, a*vec.z);
}

inline std::ostream& operator<<(std::ostream& os, const Vector3& vec)
{
    os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
    return os;
}




//*****************************************************************************
// inline Vector4
//*****************************************************************************
inline Vector4 Vector4::operator- () const
{
    return Vector4(-x, -y, -z, -w);
}

inline Vector4 Vector4::operator+ (const Vector4& rhs) const
{
    return Vector4(x+rhs.x, y+rhs.y, z+rhs.z, w+rhs.w);
}

inline Vector4 Vector4::operator- (const Vector4& rhs) const
{
    return Vector4(x-rhs.x, y-rhs.y, z-rhs.z, w-rhs.w);
}

inline Vector4& Vector4::operator+= (const Vector4& rhs)
{
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    w += rhs.w;
    return *this;
}

inline Vector4& Vector4::operator-= (const Vector4& rhs)
{
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    w -= rhs.w;
    return *this;
}

inline Vector4 Vector4::operator* (const float a) const
{
    return Vector4(x*a, y*a, z*a, w*a);
}

inline Vector4 Vector4::operator* (const Vector4& rhs) const
{
    return Vector4(x*rhs.x, y*rhs.y, z*rhs.z, w*rhs.w);
}

inline Vector4& Vector4::operator*= (const float a)
{
    x *= a; y *= a; z *= a; w *= a; return *this;
}

inline Vector4& Vector4::operator*= (const Vector4& rhs)
{
    x *= rhs.x;
    y *= rhs.y;
    z *= rhs.z;
    w *= rhs.w;
    return *this;
}

inline Vector4 Vector4::operator/ (const float a) const
{
    return Vector4(x/a, y/a, z/a, w/a);
}

inline Vector4& Vector4::operator/= (const float a)
{
    x /= a;
    y /= a;
    z /= a;
    w /= a;
    return *this;
}

inline bool Vector4::operator== (const Vector4& rhs) const
{
    return (x == rhs.x) && (y == rhs.y) && (z == rhs.z) && (w == rhs.w);
}

inline bool Vector4::operator!= (const Vector4& rhs) const
{
    return (x != rhs.x) || (y != rhs.y) || (z != rhs.z) || (w != rhs.w);
}

inline bool Vector4::operator< (const Vector4& rhs) const
{
    if(x < rhs.x) return true;
    if(x > rhs.x) return false;
    if(y < rhs.y) return true;
    if(y > rhs.y) return false;
    if(z < rhs.z) return true;
    if(z > rhs.z) return false;
    if(w < rhs.w) return true;
    if(w > rhs.w) return false;
    return false;
}

inline float Vector4::operator[] (int index) const
{
    return (&x)[index];
}

inline float& Vector4::operator[] (int index)
{
    return (&x)[index];
}

inline void Vector4::zero()
{
	x = 0;
	y = 0;
	z = 0;
	w = 0;
}

inline void Vector4::set(float x, float y, float z, float w)
{
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
}

inline float Vector4::length() const
{
    return std::sqrt(x*x + y*y + z*z + w*w);
}

inline float Vector4::distance(const Vector4& vec) const
{
    return std::sqrt((vec.x-x)*(vec.x-x) + (vec.y-y)*(vec.y-y) + (vec.z-z)*(vec.z-z) + (vec.w-w)*(vec.w-w));
}

inline Vector4& Vector4::normalize()
{
    // NOTE: leave w-component untouched
    const float EPSILON = 0.000001f;
    float xxyyzz = x*x + y*y + z*z;
    if(xxyyzz < EPSILON)
        return *this; // do nothing if it is zero vector

    //float invLength = invSqrt(xxyyzz);
    float invLength = 1.0f / std::sqrt(xxyyzz);
    x *= invLength;
    y *= invLength;
    z *= invLength;
    return *this;
}

inline float Vector4::dot(const Vector4& rhs) const
{
    return (x*rhs.x + y*rhs.y + z*rhs.z + w*rhs.w);
}

inline bool Vector4::equal(const Vector4& rhs, float epsilon) const
{
    return std::abs(x - rhs.x) < epsilon && std::abs(y - rhs.y) < epsilon && std::abs(z - rhs.z) < epsilon && std::abs(w - rhs.w) < epsilon;
}

inline Vector4 operator*(const float a, const Vector4 vec)
{
    return Vector4(a*vec.x, a*vec.y, a*vec.z, a*vec.w);
}

inline std::ostream& operator<< (std::ostream& os, const Vector4& vec)
{
    os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
    return os;
}

#endif


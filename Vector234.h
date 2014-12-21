/**
 * Copyright (c) 2013 - Jorma Rebane
 */
#pragma once
#ifndef VECTOR234_H
#define VECTOR234_H


#include "GL\glm\glm.hpp" // vec3, vec2
#include <algorithm>
using std::min;
using std::max;
#include "MathEx.h"

/**
 * @note This thing is a beast implementation of Vector2/Vector3/Vector4
 * @note Special attention was paid to implementing swizzle operators
 * @note These Vector implementations are fully compatible with glm::vec2/glm::vec3/glm::vec4
 * @note Vector4 is terribly beast
 */

struct float2
{
	union {
		struct { float x, y; };
		struct { float s, t; };
		struct { float w, h; };
		struct { float width, height; };
	};
};
struct int2
{
	union {
		struct { int x, y; };
		struct { int width, height; };
	};
};
struct float3
{
	union {
		struct { float x, y, z; };
	};
};
struct float4
{
	union {
		struct { float x, y, z, w; };
		struct { float r, g, b, a; };
		struct { float s, t, u, v; };
	};
};


/**
 * A powerful Vector2 implementation compatible with glm::vec2
 */
struct Vector2
{
	union {
		struct { float x, y; };
		struct { float s, t; };
		struct { float w, h; };
		struct { float width, height; };
		float2 xy;
	};
	inline Vector2() : x(0.0f), y(0.0f) {}
	inline Vector2(float v) : x(v), y(v) {}
	inline Vector2(float x, float y) : x(x), y(y) {}
	inline Vector2(const float2& xy) : xy(xy) {}
	inline Vector2(const glm::vec2& v) : x(v.x), y(v.y) {}


	inline operator glm::vec2&() { return *(glm::vec2*)this; }
	inline operator const glm::vec2&() const { return *(glm::vec2*)this; }


	inline Vector2& operator+=(const Vector2& v){ x += v.x, y += v.y; return *this; }
	inline Vector2& operator+=(const float v)	{ x += v;	y += v;	  return *this; }
	inline Vector2& operator-=(const Vector2& v){ x -= v.x, y -= v.y; return *this; }
	inline Vector2& operator-=(const float v)	{ x -= v;	y -= v;   return *this; }
	inline Vector2& operator*=(const Vector2& v){ x *= v.x, y *= v.y; return *this; }
	inline Vector2& operator*=(const float v)	{ x *= v;	y *= v;	  return *this; }
	inline Vector2& operator/=(const Vector2& v){ x /= v.x, y /= v.y; return *this; }
	inline Vector2& operator/=(const float v)	{ x /= v;	y /= v;	  return *this; }


	static const Vector2 ZERO;
	static const Vector2 UP;		// +Y
	static const Vector2 DOWN;		// -Y
	static const Vector2 LEFT;		// -X
	static const Vector2 RIGHT;		// +X
	static const Vector2 UPLEFT;	// -X +Y
	static const Vector2 UPRIGHT;	// +X +Y
	static const Vector2 DOWNLEFT;	// -X -Y
	static const Vector2 DOWNRIGHT; // +X -Y



	/** @note Sets the values of this Vector */
	inline void set(float X, float Y)
	{
		x = X, y = Y;
	}
	/** @return Length of this Vector2 */
	inline float length() const
	{
		return sqrtf(x*x + y*y);
	}
	/** @return Squared length of this Vector2 */
	inline float sqlength() const
	{
		return x*x + y*y;
	}
	/** @note Normalizes this vector in-place and multiplies by magnitude*/
	void normalize(const float magnitude = 1.0f)
	{
		float len = x*x + y*y;
		if(len < 0.000001f) {
			x = 0.0f, y = 0.0f;
			return;
		}
		len = magnitude / sqrtf(len);
		x *= len, y *= len;
	}
	/** @return A normalized copy of this vector multiplied by magnitude */
	Vector2 normalized(const float magnitude = 1.0f) const
	{
		float len = x*x + y*y;
		if(len < 0.000001f)
			return Vector2(0.0f, 0.0f);
		len = magnitude / sqrtf(len);
		return Vector2(x * len, y * len);
	}
	/** @return Dot product of two vectors */
	inline float dot(const Vector2& v) const
	{
		return x*v.x + y*v.y;
	}
};

inline Vector2 operator+(const Vector2& a, const Vector2& b){ return Vector2(a.x + b.x, a.y + b.y); }
inline Vector2 operator+(const Vector2& a, const float b)	{ return Vector2(a.x + b, a.y + b); }
inline Vector2 operator+(const float a, const Vector2& b)	{ return Vector2(a + b.x, a + b.y); }
inline Vector2 operator-(const Vector2& a, const Vector2& b){ return Vector2(a.x - b.x, a.y - b.y); }
inline Vector2 operator-(const Vector2& a, const float b)	{ return Vector2(a.x - b, a.y - b); }
inline Vector2 operator-(const float a, const Vector2& b)	{ return Vector2(a - b.x, a - b.y); }
inline Vector2 operator*(const Vector2& a, const Vector2& b){ return Vector2(a.x * b.x, a.y * b.y); }
inline Vector2 operator*(const Vector2& a, const float b)	{ return Vector2(a.x * b, a.y * b); }
inline Vector2 operator*(const float a, const Vector2& b)	{ return Vector2(a * b.x, a * b.y); }
inline Vector2 operator/(const Vector2& a, const Vector2& b){ return Vector2(a.x / b.x, a.y / b.y); }
inline Vector2 operator/(const Vector2& a, const float b)	{ return Vector2(a.x / b, a.y / b); }
inline Vector2 operator/(const float a, const Vector2& b)	{ return Vector2(a / b.x, a / b.y); }




/**
 * A powerful Vector3 implementation compatible with glm::vec3
 */
struct Vector3
{
	union {
		struct { float x, y, z; };
		float3 xyz;
		float2 xy;
		struct { float _a0; float2 yz; };
	};
	inline Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
	inline Vector3(float v) : x(v), y(v), z(v) {}
	inline Vector3(float x, float y, float z = 0.0f) : x(x), y(y), z(z) {}
	inline Vector3(const float2& xy, float z) : xy(xy), z(z) {}
	inline Vector3(float x, const float2& yz) : x(x), yz(yz) {}
	inline Vector3(const float3& xyz) : xyz(xyz) {}
	inline Vector3(const glm::vec3& v) : x(v.x), y(v.y), z(v.z) {}

	inline Vector3(const Vector2& xy, float z) : x(xy.x), y(xy.y), z(z) {}
	inline Vector3(float x, const Vector2& yz) : x(x), y(yz.x), z(yz.y) {}
	inline Vector3(const glm::vec2& v, float z) : x(v.x), y(v.y), z(z) {}
	inline Vector3(float x, const glm::vec2& v) : x(x), y(v.x), z(v.y) {}

	inline operator glm::vec3&() { return *(glm::vec3*)this; }
	inline operator const glm::vec3&() const { return *(const glm::vec3*)this; }
	inline operator float3&() { return *(float3*)this; }
	inline operator const float3&() const { return *(const float3*)this; }

	inline Vector3& operator+=(const Vector3& v){ x += v.x, y += v.y, z += v.z; return *this; }
	inline Vector3& operator+=(const float v)	{ x += v;   y += v,   z += v;   return *this; }
	inline Vector3& operator-=(const Vector3& v){ x -= v.x, y -= v.y, z -= v.z; return *this; }
	inline Vector3& operator-=(const float v)	{ x -= v;   y -= v,   z -= v;   return *this; }
	inline Vector3& operator*=(const Vector3& v){ x *= v.x, y *= v.y, z *= v.z; return *this; }
	inline Vector3& operator*=(const float v)	{ x *= v;   y *= v,   z *= v;   return *this; }
	inline Vector3& operator/=(const Vector3& v){ x /= v.x, y /= v.y, z /= v.z; return *this; }
	inline Vector3& operator/=(const float v)	{ x /= v;   y /= v,   z /= v;   return *this; }

	/** @note Sets the values of this Vector */
	inline void set(float X, float Y, float Z)
	{
		x = X, y = Y, z = Z;
	}
	/** @return Length of this Vector3 */
	inline float length() const
	{
		return sqrtf(x*x + y*y + z*z);
	}
	/** @return Squared length of this Vector3 */
	inline float sqlength() const
	{
		return x*x + y*y + z*z;
	}
	/** @note Normalizes this vector in-place and multiplies by magnitude*/
	void normalize(const float magnitude = 1.0f)
	{
		float len = x*x + y*y + z*z;
		if(len < 0.000001f) {
			x = 0.0f, y = 0.0f, z = 0.0f;
			return;
		}
		len = magnitude / sqrtf(len);
		x *= len, y *= len, z *= len;
	}
	/** @return A normalized copy of this vector multiplied by magnitude */
	Vector3 normalized(const float magnitude = 1.0f) const
	{
		float len = x*x + y*y + z*z;
		if(len < 0.000001f)
			return Vector3(0.0f, 0.0f, 0.0f);
		len = magnitude / len;
		return Vector3(x * len, y * len, z * len);
	}
	/** @return Dot product of two vectors */
	inline float dot(const Vector3& v) const
	{
		return x*v.x + y*v.y + z*v.z;
	}
	/** @return Cross product of two vectors */
	inline Vector3 cross(const Vector3& v)
	{
		return Vector3(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);
	}
};

inline Vector3 operator+(const Vector3& a, const Vector3& b){ return Vector3(a.x + b.x, a.y + b.y, a.z + b.z); }
inline Vector3 operator+(const Vector3& a, const float b)	{ return Vector3(a.x + b, a.y + b, a.z + b); }
inline Vector3 operator+(const float a, const Vector3& b)	{ return Vector3(a + b.x, a + b.y, a + b.z); }
inline Vector3 operator-(const Vector3& a, const Vector3& b){ return Vector3(a.x - b.x, a.y - b.y, a.z - b.z); }
inline Vector3 operator-(const Vector3& a, const float b)	{ return Vector3(a.x - b, a.y - b, a.z - b); }
inline Vector3 operator-(const float a, const Vector3& b)	{ return Vector3(a - b.x, a - b.y, a - b.z); }
inline Vector3 operator*(const Vector3& a, const Vector3& b){ return Vector3(a.x * b.x, a.y * b.y, a.z * b.z); }
inline Vector3 operator*(const Vector3& a, const float b)	{ return Vector3(a.x * b, a.y * b, a.z * b); }
inline Vector3 operator*(const float a, const Vector3& b)	{ return Vector3(a * b.x, a * b.y, a * b.z); }
inline Vector3 operator/(const Vector3& a, const Vector3& b){ return Vector3(a.x / b.x, a.y / b.y, a.z / b.z); }
inline Vector3 operator/(const Vector3& a, const float b)	{ return Vector3(a.x / b, a.y / b, a.z / b); }
inline Vector3 operator/(const float a, const Vector3& b)	{ return Vector3(a / b.x, a / b.y, a / b.z); }




/**
 * A powerful Vector4 implementation compatible with glm::vec4
 */
struct Vector4
{
	union {
		struct { float x, y, z, w; };
		struct { float r, g, b, a; };
		struct { float s, t, u, v; };
		float4 xyzw, rgba;
		float3 xyz, rgb;
		float2 xy, rg;
		struct { float _a0; float3 yzw; };
		struct { float _a1; float2 yz; };
		struct { float2 _a2; float2 zw; };
	};
	inline Vector4() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
	inline Vector4(float v) : x(v), y(v), z(v), w(1.0f) {}
	inline Vector4(float x, float y, float z = 0.0f, float w = 1.0f)
		: x(x), y(y), z(z), w(w) {}
	inline Vector4(const glm::vec4& v)
		: x(v.x), y(v.y), z(v.z), w(v.w) {}
	inline Vector4(const float4& xyzw) : xyzw(xyzw) {}


	inline Vector4(const float2& xy, float z = 0.0f, float w = 1.0f) // xy, z, w
		: xy(xy), z(z), w(w) {}
	inline Vector4(float x, const float2& yz, float w = 1.0f) // x, yz, w
		: x(x), yz(yz), w(w) {}
	inline Vector4(float x, float y, const float2& zw) // x, y, zw
		: x(x), y(y), zw(zw) {}
	inline Vector4(const float2& xy, const float2& zw) // xy, zw
		: xy(xy), zw(zw) {}

	inline Vector4(const Vector2& xy, float z = 0.0f, float w = 1.0f) // xy, z, w
		: x(xy.x), y(xy.y), z(z), w(w) {}
	inline Vector4(float x, const Vector2& yz, float w = 1.0f) // x, yz, w
		: x(x), y(yz.x), z(yz.y), w(w) {}
	inline Vector4(float x, float y, const Vector2& zw) // x, y, zw
		: x(x), y(y), z(zw.x), w(zw.y) {}
	inline Vector4(const Vector2& xy, const Vector2& zw) // xy, zw
		: x(xy.x), y(xy.y), z(zw.x), w(zw.y) {}

	inline Vector4(const glm::vec2& xy, float z = 0.0f, float w = 1.0f) // xy, z, w
		: x(xy.x), y(xy.y), z(z), w(w) {}
	inline Vector4(float x, const glm::vec2& yz, float w = 1.0f) // x, yz, w
		: x(x), y(yz.x), z(yz.y), w(w) {}
	inline Vector4(float x, float y, const glm::vec2& zw) // x, y, zw
		: x(x), y(y), z(zw.x), w(zw.y) {}
	inline Vector4(const glm::vec2& xy, const glm::vec2& zw) // xy, zw
		: x(xy.x), y(xy.y), z(zw.x), w(zw.y) {}


	inline Vector4(const float3& xyz, float w = 1.0f) // xyz, w
		: xyz(xyz), w(w) {}
	inline Vector4(float x, const float3& yzw) // x, yzw
		: x(x), yzw(yzw) {}

	inline Vector4(const Vector3& xyz, float w = 1.0f) // xyz, w
		: x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}
	inline Vector4(float x, const Vector3& yzw) // x, yzw
		: x(x), y(yzw.x), z(yzw.y), w(yzw.z) {}

	inline Vector4(const glm::vec3& xyz, float w = 1.0f) // xyz, w
		: x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}
	inline Vector4(float x, const glm::vec3& yzw) // x, yzw
		: x(x), y(yzw.x), z(yzw.y), w(yzw.z) {}


	inline operator glm::vec4&() { return *(glm::vec4*)this; }
	inline operator const glm::vec4&() const { return *(glm::vec4*)this; }


	inline Vector4& operator+=(const Vector4& v){ x += v.x, y += v.y, z += v.z, w += v.w; return *this; }
	inline Vector4& operator+=(const float v)	{ x += v;   y += v,   z += v,   w += v;   return *this; }
	inline Vector4& operator-=(const Vector4& v){ x -= v.x, y -= v.y, z -= v.z, w -= v.w; return *this; }
	inline Vector4& operator-=(const float v)	{ x -= v;   y -= v,   z -= v,   w -= v;   return *this; }
	inline Vector4& operator*=(const Vector4& v){ x *= v.x, y *= v.y, z *= v.z, w *= v.w; return *this; }
	inline Vector4& operator*=(const float v)	{ x *= v;   y *= v,   z *= v,   w *= v;   return *this; }
	inline Vector4& operator/=(const Vector4& v){ x /= v.x, y /= v.y, z /= v.z, w /= v.w; return *this; }
	inline Vector4& operator/=(const float v)	{ x /= v;   y /= v,   z /= v,   w /= v;   return *this; }


	/** @note Sets the values of this Vector */
	inline void set(float X, float Y, float Z, float W)
	{
		x = X, y = Y, z = Z, w = W;
	}
	/** @return Length of this Vector3 */
	inline float length() const
	{
		return sqrtf(x*x + y*y + z*z + w*w);
	}
	/** @return Squared length of this Vector3 */
	inline float sqlength() const
	{
		return x*x + y*y + z*z + w*w;
	}
	/** @note Normalizes this vector in-place and multiplies by magnitude*/
	void normalize(const float magnitude = 1.0f)
	{
		float len = x*x + y*y + z*z + w*w;
		if(len < 0.000001f) {
			x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;
			return;
		}
		len = magnitude / sqrtf(len);
		x *= len, y *= len, z *= len, w *= len;
	}
	/** @return A normalized copy of this vector multiplied by magnitude */
	Vector4 normalized(const float magnitude = 1.0f) const
	{
		float len = x*x + y*y + z*z + w*w;
		if(len < 0.000001f)
			return Vector4(0.0f, 0.0f, 0.0f, 0.0f);
		len = magnitude / sqrtf(len);
		return Vector4(x * len, y * len, z * len, w * len);
	}
	/** @return Dot product of two vectors */
	inline float dot(const Vector4& v) const
	{
		return x*v.x + y*v.y + z*v.z + w*v.w;
	}
};

inline Vector4 operator+(const Vector4& a, const Vector4& b){ return Vector4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
inline Vector4 operator+(const Vector4& a, const float b)	{ return Vector4(a.x + b, a.y + b, a.z + b, a.w + b); }
inline Vector4 operator+(const float a, const Vector4& b)	{ return Vector4(a + b.x, a + b.y, a + b.z, a + b.w); }
inline Vector4 operator-(const Vector4& a, const Vector4& b){ return Vector4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
inline Vector4 operator-(const Vector4& a, const float b)	{ return Vector4(a.x - b, a.y - b, a.z - b, a.w - b); }
inline Vector4 operator-(const float a, const Vector4& b)	{ return Vector4(a - b.x, a - b.y, a - b.z, a - b.w); }
inline Vector4 operator*(const Vector4& a, const Vector4& b){ return Vector4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w); }
inline Vector4 operator*(const Vector4& a, const float b)	{ return Vector4(a.x * b, a.y * b, a.z * b, a.w * b); }
inline Vector4 operator*(const float a, const Vector4& b)	{ return Vector4(a * b.x, a * b.y, a * b.z, a * b.w); }
inline Vector4 operator/(const Vector4& a, const Vector4& b){ return Vector4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w); }
inline Vector4 operator/(const Vector4& a, const float b)	{ return Vector4(a.x / b, a.y / b, a.z / b, a.w / b); }
inline Vector4 operator/(const float a, const Vector4& b)	{ return Vector4(a / b.x, a / b.y, a / b.z, a / b.w); }













/**
 * A powerful Vector2i implementation compatible with glm::ivec2
 */
struct Vector2i
{
	union {
		struct { int x, y; };
		struct { int width, height; };
		int2 xy;
	};
	inline Vector2i() : x(0), y(0) {}
	inline Vector2i(int v) : x(v), y(v) {}
	inline Vector2i(int x, int y) : x(x), y(y) {}
	inline Vector2i(const int2& xy) : xy(xy) {}
	inline Vector2i(const glm::ivec2& v) : x(v.x), y(v.y) {}


	inline operator glm::ivec2&() { return *(glm::ivec2*)this; }
	inline operator const glm::ivec2&() const { return *(glm::ivec2*)this; }

	inline operator Vector2() { return Vector2(float(x), float(y)); }
	inline operator const Vector2() const { return Vector2(float(x), float(y)); }


	inline Vector2i& operator+=(const Vector2i& v){ x += v.x, y += v.y; return *this; }
	inline Vector2i& operator+=(const int v)	  { x += v;	  y += v;	return *this; }
	inline Vector2i& operator-=(const Vector2i& v){ x -= v.x, y -= v.y; return *this; }
	inline Vector2i& operator-=(const int v)	  { x -= v;	  y -= v;   return *this; }
	inline Vector2i& operator*=(const Vector2i& v){ x *= v.x, y *= v.y; return *this; }
	inline Vector2i& operator*=(const int v)	  { x *= v;	  y *= v;	return *this; }
	inline Vector2i& operator/=(const Vector2i& v){ x /= v.x, y /= v.y; return *this; }
	inline Vector2i& operator/=(const int v)	  { x /= v;	  y /= v;	return *this; }


	static const Vector2i ZERO;
	static const Vector2i UP;		// +Y
	static const Vector2i DOWN;		// -Y
	static const Vector2i LEFT;		// -X
	static const Vector2i RIGHT;	// +X
	static const Vector2i UPLEFT;	// -X +Y
	static const Vector2i UPRIGHT;	// +X +Y
	static const Vector2i DOWNLEFT;	// -X -Y
	static const Vector2i DOWNRIGHT;// +X -Y


	/** @note Sets the values of this Vector */
	inline void set(int X, int Y)
	{
		x = X, y = Y;
	}
	/** @return Length of this Vector2 */
	inline float length() const
	{
		return sqrtf(float(x*x + y*y));
	}
	/** @return Squared length of this Vector2 */
	inline int sqlength() const
	{
		return x*x + y*y;
	}
	/** 
	 * Normalizes this vector in-place and multiplies by magnitude
	 * @note Normalizing an integer vector clamps its values to [-1..1]
	 */
	void normalize(int magnitude = 1)
	{
		x = max(-magnitude, min(x, +magnitude));
		y = max(-magnitude, min(y, +magnitude));
	}
	/** 
	 * @return A normalized copy of this vector multiplied by magnitude
	 * @note Normalizing an integer vector clamps its values to [-1..1]
	 */
	Vector2i normalized(int magnitude = 1) const
	{
		return Vector2i(
			max(-magnitude, min(x, +magnitude)), 
			max(-magnitude, min(y, +magnitude)));
	}
	/** @return Dot product of two vectors */
	inline int dot(const Vector2i& v) const
	{
		return x*v.x + y*v.y;
	}
};

inline Vector2i operator+(const Vector2i& a, const Vector2i& b){ return Vector2i(a.x + b.x, a.y + b.y); }
inline Vector2i operator+(const Vector2i& a, const int b)	   { return Vector2i(a.x + b, a.y + b); }
inline Vector2i operator+(const int a, const Vector2i& b)	   { return Vector2i(a + b.x, a + b.y); }
inline Vector2i operator-(const Vector2i& a, const Vector2i& b){ return Vector2i(a.x - b.x, a.y - b.y); }
inline Vector2i operator-(const Vector2i& a, const int b)	   { return Vector2i(a.x - b, a.y - b); }
inline Vector2i operator-(const int a, const Vector2i& b)	   { return Vector2i(a - b.x, a - b.y); }
inline Vector2i operator*(const Vector2i& a, const Vector2i& b){ return Vector2i(a.x * b.x, a.y * b.y); }
inline Vector2i operator*(const Vector2i& a, const int b)	   { return Vector2i(a.x * b, a.y * b); }
inline Vector2i operator*(const int a, const Vector2i& b)	   { return Vector2i(a * b.x, a * b.y); }
inline Vector2i operator/(const Vector2i& a, const Vector2i& b){ return Vector2i(a.x / b.x, a.y / b.y); }
inline Vector2i operator/(const Vector2i& a, const int b)	   { return Vector2i(a.x / b, a.y / b); }
inline Vector2i operator/(const int a, const Vector2i& b)	   { return Vector2i(a / b.x, a / b.y); }




//////////////////////////////////////////// 
////// float2, int2, float3, float4 external operators:


inline Vector2 operator+(const float2& a, const float2& b)	{ return Vector2(a.x + b.x, a.y + b.y); }
inline Vector2 operator+(const float2& a, const float b)	{ return Vector2(a.x + b, a.y + b); }
inline Vector2 operator+(const float a, const float2& b)	{ return Vector2(a + b.x, a + b.y); }
inline Vector2 operator-(const float2& a, const float2& b)	{ return Vector2(a.x - b.x, a.y - b.y); }
inline Vector2 operator-(const float2& a, const float b)	{ return Vector2(a.x - b, a.y - b); }
inline Vector2 operator-(const float a, const float2& b)	{ return Vector2(a - b.x, a - b.y); }
inline Vector2 operator*(const float2& a, const float2& b)	{ return Vector2(a.x * b.x, a.y * b.y); }
inline Vector2 operator*(const float2& a, const float b)	{ return Vector2(a.x * b, a.y * b); }
inline Vector2 operator*(const float a, const float2& b)	{ return Vector2(a * b.x, a * b.y); }
inline Vector2 operator/(const float2& a, const float2& b)	{ return Vector2(a.x / b.x, a.y / b.y); }
inline Vector2 operator/(const float2& a, const float b)	{ return Vector2(a.x / b, a.y / b); }
inline Vector2 operator/(const float a, const float2& b)	{ return Vector2(a / b.x, a / b.y); }


inline Vector3 operator+(const float3& a, const float3& b)	{ return Vector3(a.x + b.x, a.y + b.y, a.z + b.z); }
inline Vector3 operator+(const float3& a, const float b)	{ return Vector3(a.x + b, a.y + b, a.z + b); }
inline Vector3 operator+(const float a, const float3& b)	{ return Vector3(a + b.x, a + b.y, a + b.z); }
inline Vector3 operator-(const float3& a, const float3& b)	{ return Vector3(a.x - b.x, a.y - b.y, a.z - b.z); }
inline Vector3 operator-(const float3& a, const float b)	{ return Vector3(a.x - b, a.y - b, a.z - b); }
inline Vector3 operator-(const float a, const float3& b)	{ return Vector3(a - b.x, a - b.y, a - b.z); }
inline Vector3 operator*(const float3& a, const float3& b)	{ return Vector3(a.x * b.x, a.y * b.y, a.z * b.z); }
inline Vector3 operator*(const float3& a, const float b)	{ return Vector3(a.x * b, a.y * b, a.z * b); }
inline Vector3 operator*(const float a, const float3& b)	{ return Vector3(a * b.x, a * b.y, a * b.z); }
inline Vector3 operator/(const float3& a, const float3& b)	{ return Vector3(a.x / b.x, a.y / b.y, a.z / b.z); }
inline Vector3 operator/(const float3& a, const float b)	{ return Vector3(a.x / b, a.y / b, a.z / b); }
inline Vector3 operator/(const float a, const float3& b)	{ return Vector3(a / b.x, a / b.y, a / b.z); }


inline Vector4 operator+(const float4& a, const float4& b)	{ return Vector4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
inline Vector4 operator+(const float4& a, const float b)	{ return Vector4(a.x + b, a.y + b, a.z + b, a.w + b); }
inline Vector4 operator+(const float a, const float4& b)	{ return Vector4(a + b.x, a + b.y, a + b.z, a + b.w); }
inline Vector4 operator-(const float4& a, const float4& b)	{ return Vector4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
inline Vector4 operator-(const float4& a, const float b)	{ return Vector4(a.x - b, a.y - b, a.z - b, a.w - b); }
inline Vector4 operator-(const float a, const float4& b)	{ return Vector4(a - b.x, a - b.y, a - b.z, a - b.w); }
inline Vector4 operator*(const float4& a, const float4& b)	{ return Vector4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w); }
inline Vector4 operator*(const float4& a, const float b)	{ return Vector4(a.x * b, a.y * b, a.z * b, a.w * b); }
inline Vector4 operator*(const float a, const float4& b)	{ return Vector4(a * b.x, a * b.y, a * b.z, a * b.w); }
inline Vector4 operator/(const float4& a, const float4& b)	{ return Vector4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w); }
inline Vector4 operator/(const float4& a, const float b)	{ return Vector4(a.x / b, a.y / b, a.z / b, a.w / b); }
inline Vector4 operator/(const float a, const float4& b)	{ return Vector4(a / b.x, a / b.y, a / b.z, a / b.w); }


inline Vector2i operator+(const int2& a, const int2& b)	{ return Vector2i(a.x + b.x, a.y + b.y); }
inline Vector2i operator+(const int2& a, const int b)	{ return Vector2i(a.x + b, a.y + b); }
inline Vector2i operator+(const int a, const int2& b)	{ return Vector2i(a + b.x, a + b.y); }
inline Vector2i operator-(const int2& a, const int2& b)	{ return Vector2i(a.x - b.x, a.y - b.y); }
inline Vector2i operator-(const int2& a, const int b)	{ return Vector2i(a.x - b, a.y - b); }
inline Vector2i operator-(const int a, const int2& b)	{ return Vector2i(a - b.x, a - b.y); }
inline Vector2i operator*(const int2& a, const int2& b)	{ return Vector2i(a.x * b.x, a.y * b.y); }
inline Vector2i operator*(const int2& a, const int b)	{ return Vector2i(a.x * b, a.y * b); }
inline Vector2i operator*(const int a, const int2& b)	{ return Vector2i(a * b.x, a * b.y); }
inline Vector2i operator/(const int2& a, const int2& b)	{ return Vector2i(a.x / b.x, a.y / b.y); }
inline Vector2i operator/(const int2& a, const int b)	{ return Vector2i(a.x / b, a.y / b); }
inline Vector2i operator/(const int a, const int2& b)	{ return Vector2i(a / b.x, a / b.y); }






#endif // VECTOR234_H
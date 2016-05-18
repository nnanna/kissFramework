
#pragma once

#include "ks_Maths.h"


namespace ks {


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// vec3
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	inline vec3::vec3()
	{
		x = y = z = 0.0f;
	}


	inline vec3::vec3(const vec3& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
	}


	inline vec3::vec3(const float *r)
	{
#ifdef DEBUG_VALIDATE
		if (!r)
			return;
#endif

		x = *(r);
		y = *(r + 1);
		z = *(r + 2);
	}

	inline vec3::vec3(float a, float b, float c)
	{
		x = a; y = b; z = c;
	}


	// assign
	inline vec3& vec3::operator += (const vec3& rhs)
	{
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this;
	}

	inline vec3& vec3::operator -= (const vec3& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		return *this;
	}

	inline vec3& vec3::operator *= (const vec3& rhs)
	{
		x *= rhs.x;
		y *= rhs.y;
		z *= rhs.z;
		return *this;
	}


	inline vec3 vec3::operator + () const
	{
		return *this;
	}


	inline vec3 vec3::operator- () const
	{
		return vec3(-x, -y, -z);
	}

	inline vec3 vec3::operator- (const vec3& r) const
	{
		return vec3(x - r.x, y - r.y, z - r.z);
	}

	inline vec3 vec3::operator+ (const vec3& r) const
	{
		return vec3(x + r.x, y + r.y, z + r.z);
	}


	inline vec3 vec3::operator * (float r) const
	{
		return vec3(x * r, y * r, z * r);
	}

	inline vec3& vec3::operator *= (float r)
	{
		x *= r; y *= r; z *= r;
		return *this;
	}


	inline vec3 vec3::operator /(float f) const
	{
		float fInv = 1.0f / f;
		return vec3(x * fInv, y * fInv, z * fInv);
	}

	inline vec3& vec3::operator /= (float r)
	{
		x /= r; y /= r; z /= r;
		return *this;
	}


	inline vec3 operator * (float f, const struct vec3& v)
	{
		return vec3(f * v.x, f * v.y, f * v.z);
	}

	inline float vec3::Dot(const vec3& rhs) const
	{
		return (x*rhs.x) + (y*rhs.y) + (z*rhs.z);
	}

	inline float vec3::LengthSq() const
	{
		return (x*x) + (y*y) + (z*z);
	}

	inline float vec3::Length() const
	{
		return Sqrt(LengthSq());
	}

	inline void vec3::Normalize()
	{
		float len = LengthSq();
		len = Sqrt(len);
		if (len != 0.f) { x /= len; y /= len; z /= len; }
	}

	inline void vec3::FastNormalize()
	{
		float len = LengthSq();
		len = InvSqrt(len);
		x *= len; y *= len; z *= len;
	}

	inline void vec3::reflect(const vec3& pNormal, vec3& rReflection) const
	{
		float dot = Dot(pNormal);
		rReflection = pNormal * dot * 2.f;
		rReflection = *this - rReflection;
	}


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// vec4
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	inline float vec4::Normalise()
	{
		float	len(Length());
		if (len > 0.0001f)
		{
			float r(1.0f / len);
			x *= r;
			y *= r;
			z *= r;
			w *= r;
		}

		return len;
	}

	inline vec4 vec4::operator+(const vec4 & v) const
	{
		return vec4(x + v.x, y + v.y, z + v.z, w + v.w);
	}
	
	inline vec4 vec4::operator-(const vec4 & v) const
	{
		return vec4(x - v.x, y - v.y, z - v.z, w - v.w);
	}

	inline vec4 vec4::operator*(float s) const
	{
		return vec4(x * s, y * s, z * s, w * s);
	}


	inline float vec4::Length() const
	{
		return 1.0f / InvSqrt((x*x) + (y*y) + (z*z) + (w*w));
	}

	inline float vec4::Dot(const vec4 & rhs) const
	{
		return (x*rhs.x) + (y*rhs.y) + (z*rhs.z) + (w*rhs.w);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	COLOR
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	inline COLOR::COLOR()
	{
		r = g = b = a = 0;
	}

	inline COLOR::COLOR(float fr, float fg, float fb, float fa)
	{
		r = fr;
		g = fg;
		b = fb;
		a = fa;
	}

	//=======================================================================================================================

}


#include "ks_Maths.h"
#include <stdlib.h>

namespace ks {

	const vec3 vec3::UNIT_X = vec3(1.f, 0.f, 0.f);
	const vec3 vec3::UNIT_Y = vec3(0.f, 1.f, 0.f);
	const vec3 vec3::UNIT_Z = vec3(0.f, 0.f, 1.f);
	const vec3 vec3::ZERO_VEC = vec3(0.f, 0.f, 0.f);

	static inline float frand()
	{
		int x = (rand()*rand()) & 0x007fffff;
		x |= 0x3f800000;				// 1.0f in IEEE fp
		return (*(float*)&x) - 1.0f;
	}

	static inline float frand_range(float lo, float hi)
	{
		return lo + (hi - lo) * frand();
	}


	RNG::RNG()
	{
		mSeed = rand() ^ u32(this);
	}

	RNG::RNG(u32 seed) : mSeed(seed)
	{}

	// http://school.anhb.uwa.edu.au/personalpages/kwessen/shared/Marsaglia03.html
	u32 RNG::GetU32()
	{
		return (mSeed = 69069 * mSeed + 362437);
	}

	float RNG::GetFloatBetween(float a, float b)
	{
		if (a == b)	return a;
		if (a > b)
		{
			float temp = a; a = b; b = temp;
		}

		float f = (GetU32() % 10001) * 0.0001f;			// Get random float in [0, 1] interval.

		f = f * (b - a) + a;
		return f;
	}

	void RNG::GetV3PlusMinus(vec3& pDest, const vec3& pRange)
	{
		pDest.x = GetFloatBetween(-pRange.x, pRange.x);
		pDest.y = GetFloatBetween(-pRange.y, pRange.y);
		pDest.z = GetFloatBetween(-pRange.z, pRange.z);
	}

}
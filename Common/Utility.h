/*
	Nnanna Kama
	Utility funcs. TODO: move to more appropriate locs
*/

#ifndef KS_UTILITY_H
#define KS_UTILITY_H

#include <Debug.h>
#include <Maths\ks_Maths.inl>

namespace ks {

inline float FAST_FLOOR(float a)			{ return (float)int(a); }
inline float FAST_FMOD(float a, float b)	{ return (a - (FAST_FLOOR( a / b ) * b) ); }


	static const float INV_FACTORIAL2(1.f / 2.f);
	static const float INV_FACTORIAL3((1.f / 3.f) * INV_FACTORIAL2);
	static const float INV_FACTORIAL4((1.f / 4.f) * INV_FACTORIAL3);
	static const float INV_FACTORIAL5((1.f / 5.f) * INV_FACTORIAL4);
	static const float INV_FACTORIAL6((1.f / 6.f) * INV_FACTORIAL5);
	static const float INV_FACTORIAL7((1.f / 7.f) * INV_FACTORIAL6);
	static const float INV_180(1.f / 180.f);
	static const int sFrequencySignMultiplier[] = { 1, -1 };


	inline float FAST_SIN(float pDegree)
	{
		const int multiple = int(pDegree * INV_180);
		pDegree = (pDegree - (multiple * 180.f));		// mod

		const float x = DEG_TO_RAD(pDegree);
		const float x2 = x * x;
		const float x3 = x2 * x;
		const float x5 = x2 * x3;
		const float x7 = x2 * x5;

		const float fsin = x - (x3 * INV_FACTORIAL3) + (x5 * INV_FACTORIAL5) - (x7 * INV_FACTORIAL7);	// Taylor series expansion
		return fsin * sFrequencySignMultiplier[(multiple & 1)];										// [multiple & 1 === multiple % 2]. fishes out the odd multiples and applies the right sign
	}


	inline float FAST_COS(float pDegree)
	{
		return FAST_SIN(pDegree + 90.f);
	}

	inline float FAST_EXP(float pVal)
	{
		const float x = pVal;
		const float x2 = x * x;
		const float x3 = x * x2;
		const float x4 = x * x3;
		const float x5 = x * x4;

		const float fexp = 1.f + x + (x2 * INV_FACTORIAL2) + (x3 * INV_FACTORIAL3) + (x4 * INV_FACTORIAL4) + (x5 * INV_FACTORIAL5);
		return fexp;
	}


	// https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
	// http://www.cs.virginia.edu/~gfx/Courses/2003/ImageSynthesis/papers/Acceleration/Fast%20MinimumStorage%20RayTriangle%20Intersection.pdf
	bool raycast_triangle(const vec3& pEdgeVert, const vec3& pTriEdge1, const vec3& pTriEdge2, const vec3& pRayOrigin, const vec3& pRayDir, const float pInvDet, vec2& pOutUV)
	{
		vec3 plane = pRayDir.Cross(pTriEdge2);

		const vec3 toOrigin = pRayOrigin - pEdgeVert;
		const float u = toOrigin.Dot(plane) * pInvDet;

		if (u < 0.f || u > 1.f)
			return false;

		vec3 q = toOrigin.Cross(pTriEdge1);
		const float v = pRayDir.Dot(q) * pInvDet;

		if (v < 0.f || u + v > 1.f)
			return false;

		pOutUV.Set(u, v);
		return true;
	}

	// http://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-plane-and-ray-disk-intersection
	bool raycast_plane(const vec4& pPlane, const vec3& pRayOrigin, const vec3& pRayDir, vec3& pOut)
	{
		const float inv = pPlane.Dot(pRayDir);
		KS_ASSERT(inv >= -1.f && inv <= 1.f && "un-normalized vectors detected");

		if (fabs(inv) < 0.001f)
			return false;

		const float dist = pRayOrigin.Dot(pPlane);
		vec3::AddScale(pOut, pRayOrigin, pRayDir, -(dist - pPlane.w) / inv);

		return true;
	}

}

#endif
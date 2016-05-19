
/*
	Nnanna Kama
	Structure for encapsulating generic render data
	includes VertexBuffer implementation @todo
	All members public for now @todo?
	Render modes/states could also be encapsulated in here @todo
*/

#pragma once


#include <defines.h>



namespace ks
{
	class Matrix;
	struct	Material;

	/*
	Duplicates OpenGL rendermode info for easier interop
	*/

	enum RENDERMODE
	{
		ePoints = 0,
		eLines,
		eLineLoop,
		eLineStrip,
		eTriangles,
		eTriStrip,
		eTriFan,
		eQuad,
		eQuadStrip,
		ePolygon
	};

	typedef RENDERMODE PrimType;


	class RenderData
	{
	public:
		RenderData(const void* pVB, const ksU32* pIB, const Matrix& pTrans)
			: vertexBuffer(pVB)
			, vertexSize(0)
			, indexBuffer(pIB)
			, renderMode(0)
			, stride(0)
			, numIndices(0)
			, normOffset(0)
			, material(nullptr)
			, Transform(pTrans)
		{}

		const void*		vertexBuffer;

		ks32			vertexSize;	/*Specifies the number of coordinates per vertex. Must be 2, 3, or 4.*/

		const ksU32*	indexBuffer;

		ks32			renderMode;

		ks32			stride;

		ks32			numIndices;

		ks32			normOffset;

		Material*		material;

		const Matrix&	Transform;

	};

}


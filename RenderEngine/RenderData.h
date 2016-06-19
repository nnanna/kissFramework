
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
	class GPUBuffer;
	struct Material;

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
		RenderData(const ksU32* pIB, const Matrix& pTrans);

		~RenderData();

		ks32			vertexSize;	/*Specifies the number of coordinates per vertex. Must be 2, 3, or 4.*/

		const ksU32*	indexBuffer;

		ks32			renderMode;

		ks32			stride;

		ks32			numIndices;

		ks32			normOffset;

		Material*		material;

		GPUBuffer*		mGPUBuffer;

		const Matrix&	Transform;

	};

}


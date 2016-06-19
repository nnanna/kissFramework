
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
		RenderData(GPUBuffer* pVertexBuffer, GPUBuffer* pIndexBuffer, const Matrix& pTrans);

		~RenderData();

		GPUBuffer*		mVertexBuffer;	// TODO: replace with opaque VertexBuffer and IndexBuffer classes

		GPUBuffer*		mIndexBuffer;

		ks32			vertexSize;		// Specifies the number of coordinates per vertex. Must be 2, 3, or 4.

		RENDERMODE		renderMode;

		ks32			stride;

		ks32			numIndices;

		ks32			normOffset;

		Material*		material;

		const Matrix&	Transform;

	};

}


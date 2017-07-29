
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

#define	USE_16_BIT_INDEX_BUFFER	0

#if USE_16_BIT_INDEX_BUFFER
	typedef ksUShort	IndexBufferType;
#else
	typedef ksU32		IndexBufferType;
#endif


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

		GPUBuffer*		mInstanceBuffer;

		Material*		material;

		const Matrix&	Transform;

		RENDERMODE		renderMode;

		u32				stride;

		u32				numIndices;

		u32				normOffset;

		ksU8			vertexSize;		// Specifies the number of coordinates per vertex. Must be 2, 3, or 4.

	};

}


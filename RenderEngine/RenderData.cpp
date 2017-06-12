
#include "RenderData.h"
#include "GPUBuffer.h"

namespace ks {

	RenderData::RenderData(GPUBuffer* pVertexBuffer, GPUBuffer* pIndexBuffer, const Matrix& pTrans)
		: mVertexBuffer(pVertexBuffer)
		, mIndexBuffer(pIndexBuffer)
		, vertexSize(0)
		, renderMode(ePoints)
		, stride(0)
		, numIndices(0)
		, normOffset(0)
		, material(nullptr)
		, Transform(pTrans)
	{}


	RenderData::~RenderData()
	{
		delete mVertexBuffer;
		delete mIndexBuffer;
	}

}
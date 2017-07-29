
#include "RenderData.h"
#include "GPUBuffer.h"

namespace ks {

	RenderData::RenderData(GPUBuffer* pVertexBuffer, GPUBuffer* pIndexBuffer, const Matrix& pTrans)
		: mVertexBuffer(pVertexBuffer)
		, mIndexBuffer(pIndexBuffer)
		, mInstanceBuffer(nullptr)
		, Transform(pTrans)
		, material(nullptr)
		, renderMode(ePoints)
		, stride(0)
		, numIndices(0)
		, normOffset(0)
		, vertexSize(0)
	{}


	RenderData::~RenderData()
	{
		delete mVertexBuffer;
		delete mIndexBuffer;
		delete mInstanceBuffer;
	}

}
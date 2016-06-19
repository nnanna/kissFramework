
#include "RenderData.h"
#include "GPUBuffer.h"

namespace ks {

	RenderData::RenderData(const ksU32* pIB, const Matrix& pTrans)
		: vertexSize(0)
		, indexBuffer(pIB)
		, renderMode(0)
		, stride(0)
		, numIndices(0)
		, normOffset(0)
		, material(nullptr)
		, mGPUBuffer(nullptr)
		, Transform(pTrans)
	{}


	RenderData::~RenderData()
	{
		delete mGPUBuffer;
	}

}
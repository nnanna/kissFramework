
#ifndef KS_MODEL_H
#define KS_MODEL_H

#include <RenderEngine\RenderData.h>

namespace ks
{

	class Model
	{
	public:
		Model();

		~Model();

		bool Load(const char* pFilepath, const u32 pFlags = 0);

		bool makeCube(float pSize);

		bool makeQuad(float pSize);

		ks32 getPositionSize() const;

		void computeNormals();

		void compileModel(PrimType prim = eTriangles);

		const float* getCompiledVertices() const;

		const IndexBufferType* getCompiledIndices(PrimType prim = eTriangles) const;

		ks32 getCompiledVertexSize() const;

		ks32 getCompiledVertexCount() const;

		ks32 getCompiledIndexCount(PrimType prim = eTriangles) const;

		ks32 getCompiledNormalOffset() const;

		PrimType getPrimType() const;

	private:
		void destroy();

		float*				mCustomVB;
		IndexBufferType*	mIndices;

		ks32		mNumVertices;
		ks32		mNumNormals;
		ks32		mIndexCount;
		ks32		mVertexSize;
		ks32		mNormalsOffset;
		PrimType	mPrimType;
	};
}

#endif
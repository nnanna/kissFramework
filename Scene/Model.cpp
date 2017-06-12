
#include "Model.h"
#define TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_USE_CUSTOM_VECTOR		!_DEBUG
#define TINY_OBJ_LOADER_OLD_FLOAT_PARSER	1
#include "tiny_obj_loader.h"
#undef TINYOBJLOADER_IMPLEMENTATION

namespace ks {

static IndexBufferType mCubeIndices[] = { 0, 1, 2, 3,			// front
										  4, 5, 1, 0,			// top
										  3, 2, 6, 7,			// bottom
										  5, 4, 7, 6,			// back
										  1, 5, 6, 2,			// right
										  4, 0, 3, 7 };			// left


	Model::Model() 
		: mCustomVB(nullptr)
		, mIndices(nullptr)
		, mNumVertices(0)
		, mNumNormals(0)
		, mNormalsOffset(0)
		, mVertexSize(0)
	{}

	Model::~Model()
	{
		//auto primName = enumToString(mPrimType);	//Refl(mPrimType).ToString();
		destroy();
	}

	void Model::destroy()
	{
		delete[] mCustomVB;
		delete[] mIndices;

		mCustomVB = nullptr;
		mIndices = nullptr;
		mNumVertices = mNumNormals = mNormalsOffset = mVertexSize = mIndexCount = 0;
	}

	bool Model::Load(const char* pFilepath, u32 pFlags /*= 0*/)
	{
		tinyobj::ArrayType<tinyobj::shape_t> shapes;
		tinyobj::ArrayType<tinyobj::material_t> materials;
		std::string err;
		bool success = tinyobj::LoadObj(shapes, materials, err, pFilepath, NULL, pFlags);

		if (success)
		{
			destroy();

			// create render friendly data into mCustomVB (TODO)
			//for (u32 i = 0; i < shapes.size(); ++i)	// TODO
			u32 i(0);
			{
				const tinyobj::mesh_t& mesh = shapes[i].mesh;
				ksU32 posSize = mesh.positions.size() * sizeof(tinyobj::float3);
				ksU32 normSize = mesh.normals.size() * sizeof(tinyobj::float3);
				ksU32 indSize = mesh.indices.size() * sizeof(IndexBufferType);
				mCustomVB = new float[(posSize + normSize) / sizeof(float)];
				mIndices = new IndexBufferType[mesh.indices.size()];

				memcpy_s(mCustomVB, posSize, mesh.positions.data(), posSize);
				memcpy_s((char*)mCustomVB + posSize, normSize, mesh.normals.data(), normSize);

				memcpy_s(mIndices, indSize, mesh.indices.data(), indSize);

				mVertexSize = mesh.max_num_vertices;		// assume homogenous verts per face
				mIndexCount = mesh.indices.size();
				mNumVertices = mesh.positions.size();
				mNumNormals = mesh.normals.size();
				mNormalsOffset = mesh.positions.size() * mVertexSize;
				mPrimType = eTriangles;
			}
		}

		return success;
	}
	
	bool Model::makeCube(float dim)
	{
		destroy();

		mCustomVB = new float[60] {-dim,	dim,	dim,
									dim,	dim,	dim,
									dim,   -dim,	dim,
								   -dim,   -dim,	dim,
								   -dim,	dim,	-dim,
									dim,	dim,	-dim,
									dim,   -dim,	-dim,
								   -dim,   -dim,	-dim,
									// normals begin here
									0.0, 0.0, 1.0,
									0.0, 0.0, 1.0,

									0.0, 1.0, 0.0,
									0.0, 1.0, 0.0,

									0.0, -1.0, 0.0,
									0.0, -1.0, 0.0,

									0.0, 0.0, -1.0,
									0.0, 0.0, -1.0,

									1.0, 0.0, 0.0,
									1.0, 0.0, 0.0,

									-1.0, 0.0, 0.0,
									-1.0, 0.0, 0.0 };

		mVertexSize		= 3;
		mNumVertices	= 8;
		mNumNormals		= 12;
		mNormalsOffset	= 24;
		mIndexCount		= 24;
		mPrimType		= eQuad;

		return true;
	}

	bool Model::makeQuad(float dim)
	{
		destroy();

		mCustomVB = new float[24] {-dim,	dim,	dim,
									dim,	dim,	dim,
									dim,   -dim,	dim,
								   -dim,   -dim,	dim,
									// normals begin here
									0.0, 0.0, 1.0,
									0.0, 0.0, 1.0,
									0.0, 0.0, 1.0,
									0.0, 0.0, 1.0, };

		mVertexSize		= 3;
		mNumVertices	= 4;
		mNumNormals		= 4;
		mNormalsOffset	= 12;
		mIndexCount		= 4;
		mPrimType		= eQuad;

		return true;
	}

	ks32 Model::getPositionSize() const
	{
		return mVertexSize;
	}

	void Model::computeNormals()
	{}

	void Model::compileModel(PrimType prim)
	{}

	const float* Model::getCompiledVertices() const
	{
		return mCustomVB;
	}

	const IndexBufferType* Model::getCompiledIndices(PrimType prim) const
	{
		return mIndices ? mIndices : mCubeIndices;
	}

	ks32 Model::getCompiledVertexSize() const
	{
		return mVertexSize;
	}

	ks32 Model::getCompiledVertexCount() const
	{
		return mNumVertices + mNumNormals;
	}

	ks32 Model::getCompiledIndexCount(PrimType prim) const
	{
		return mIndexCount;
	}

	ks32 Model::getCompiledNormalOffset() const
	{
		return mNormalsOffset;
	}

	PrimType Model::getPrimType() const
	{
		return mPrimType;
	}
}
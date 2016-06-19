/*
	Nnanna Kama
	Render class Impl
*/



#include "RenderResourceFactory.h"
#include "Material.h"
#include "defines.h"
#include "GLRenderer.h"
#include "Light.h"
#include "RenderData.h"
#include "GPUBuffer.h"
#include "RenderResourceFactory.h"
#include <Service.h>
#include <RenderEngine\GL\glew.h>
#include <Concurrency\Mutex.h>

namespace ks {

	static Material* sMRUMaterial(nullptr);

	static Mutex gRenderDataGuard;


	///////////////////////////////////////////////////////////////////////////////////
	template<>				ksU32 getGLType(const float* pT)		{ return GL_FLOAT; }
	template<>				ksU32 getGLType(const ksU32* pT)		{ return GL_UNSIGNED_INT; }
	template<>				ksU32 getGLType(const ksUShort* pT)		{ return GL_UNSIGNED_SHORT; }
	template<>				ksU32 getGLType(const ksByte* pT)		{ return GL_UNSIGNED_BYTE; }
	///////////////////////////////////////////////////////////////////////////////////

	//================================================================================================================

	GLRenderer::GLRenderer() : mMRUShader(NULL)
	{}


	//================================================================================================================


	GLRenderer::~GLRenderer()
	{
		mRenderData.clear();
		RenderResourceFactory::shutDown();
	}



	//================================================================================================================

	void GLRenderer::update(float pElaspedS)
	{
		CHECK_GL_ERROR;
	}


	//================================================================================================================

	void GLRenderer::addRenderData(const RenderData* r)
	{
		ScopedLock lock(gRenderDataGuard);
		mRenderData.push_back(r);
	}

	//================================================================================================================
	/*
		This should be auto-called every frame
		to prevent the buffers from growing infinitely
		*/

	void GLRenderer::flushRenderData()
	{
		mRenderData.clear();
	}


	void GLRenderer::uploadShaderConstants(Material* pMat, const vec3& camPos, const Matrix& view, const Matrix& projection, const Matrix& pTransform)
	{
		SimpleShaderContainer* shader = pMat->ShaderContainer;

		Matrix inv(pTransform.Inverse());
		Matrix mvp(projection * view * pTransform);

		vec3 invEye(inv.TransformNormal(camPos));
		vec3 invLight(inv.TransformNormal(Light::getDefault().position));


		if (mMRUShader != shader)
		{
			if (mMRUShader)
			{
				mMRUShader->disableVertProfile();
				mMRUShader->disableFragProfile();
				mMRUShader->unbindProgram();
			}

			shader->bindProgram();
			shader->enableVertProfile();
			shader->enableFragProfile();

			shader->setVectorParameter(Material::gConstantsRegistry[sci_eye_pos], &invEye);
			shader->setVectorParameter(Material::gConstantsRegistry[sci_light_pos], &invLight);
		}

		shader->setMatrixParameter(Material::gConstantsRegistry[sci_mvp], &mvp);		// Set parameter with row-major matrix.

		if (sMRUMaterial != pMat)
		{
			sMRUMaterial = pMat;
			pMat->SetShaderParams();
		}
	}


	//================================================================================================================

	void GLRenderer::render(const vec3& camPos, const Matrix& view, const Matrix& projection)
	{
		CHECK_GL_ERROR;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnableClientState(GL_VERTEX_ARRAY);

		for (const RenderData* rd : mRenderData)
		{
			Material *mat = rd->material;

			uploadShaderConstants(mat, camPos, view, projection, rd->Transform);

			const ksU32 vert_type	= GL_FLOAT;
			const ksU32 ib_type		= GL_UNSIGNED_INT;
			GPUBuffer* ib			= rd->mIndexBuffer;
			int vert_size			= rd->vertexSize;
			int stride				= rd->stride;
			int num_indices			= rd->numIndices;

			if (rd->mVertexBuffer)
			{
				rd->mVertexBuffer->bind();
				glVertexPointer(vert_size, vert_type, 0, 0);								// last param represents offset in this case
				glVertexAttribPointer(SA_POSITION, vert_size, vert_type, GL_FALSE, 0, 0);	// these correspond to glBindAttribLocation()
			}
			else
			{
				KS_ASSERT(0 && "unsupported");
				//glVertexPointer(vert_size, vert_type, stride, vb);
				//glVertexAttribPointer(SA_POSITION, vert_size, vert_type, GL_FALSE, stride, vb);
			}
			
			if (rd->normOffset)
			{
				const float* norms	= (float*)rd->normOffset;
				glEnableClientState(GL_NORMAL_ARRAY);
				glNormalPointer(GET_GLTYPE(norms), stride, norms);
				glEnableVertexAttribArray(SA_NORMAL);
				glVertexAttribPointer(SA_NORMAL, vert_size, GET_GLTYPE(norms), GL_FALSE, stride, norms);
			}

			if (ib)
			{
				ib->bind();
				glDrawElements(rd->renderMode, num_indices, ib_type, 0);
				ib->unbind();
			}
			else
			{
				glDrawArrays(rd->renderMode, 0, num_indices);
			}

			mMRUShader = mat->ShaderContainer;

			if (rd->mVertexBuffer)
				rd->mVertexBuffer->unbind();
		}

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		if (mMRUShader)
			mMRUShader->unbindProgram();

		flushRenderData();

		CHECK_GL_ERROR;
	}

}	// namespace ks
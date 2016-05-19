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
#include "RenderResourceFactory.h"
#include <Service.h>
#include <RenderEngine\GL\glew.h>

namespace ks {

	static Material* sMRUMaterial(nullptr);


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
		mRenderData.push_back(r);
	}

	//================================================================================================================
	/*
		This should be auto-called every frame
		to prevent the buffers from growing infinitely
		TODO: clearing and repopulating the render-list every frame is potentially expensive
		Only remove items when necessary.
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

			const float* vb = (const float*)rd->vertexBuffer;
			const ksU32* ib = rd->indexBuffer;
			int vert_size = rd->vertexSize;
			int stride = rd->stride;
			int num_indices = rd->numIndices;
			const float* norms = vb + rd->normOffset;

			glVertexPointer(vert_size, GET_GLTYPE(vb), stride, vb);
			if (rd->normOffset)
			{
				glEnableClientState(GL_NORMAL_ARRAY);
				glNormalPointer(GET_GLTYPE(norms), stride, norms);
#ifdef TARGET_GL_SHADERS
				glEnableVertexAttribArray(SA_NORMAL);
				glVertexAttribPointer(SA_NORMAL, vert_size, GET_GLTYPE(norms), GL_FALSE, stride, norms);
#endif
			}

#ifdef TARGET_GL_SHADERS
			// these correspond to glBindAttribLocation()
			glVertexAttribPointer(SA_POSITION, vert_size, GET_GLTYPE(vb), GL_FALSE, stride, vb);
#endif
			if (ib)
				glDrawElements(rd->renderMode, num_indices, GET_GLTYPE(ib), ib);
			else
				glDrawArrays(rd->renderMode, 0, num_indices);

			mMRUShader = mat->ShaderContainer;
		}

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		if (mMRUShader)
			mMRUShader->unbindProgram();

		flushRenderData();

		CHECK_GL_ERROR;
	}

}	// namespace ks
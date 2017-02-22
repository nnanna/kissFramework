#ifndef GL_RENDENDER
#define GL_RENDENDER

/*
	Nnanna Kama
	Render class
*/

#include <Containers\Array.h>
#include <Concurrency\AsyncResource.h>


class SimpleShaderContainer;

namespace ks
{
	struct Material;
	class RenderData;
	struct vec3;
	class Matrix;


	typedef Array<const RenderData*>	RenderDataArray;

	class GLRenderer
	{
	public:

		GLRenderer();

		~GLRenderer();

		void	render(const vec3& camPos, const Matrix& view, const Matrix& projection);

		void	update(float pElapsedS);

		void	addRenderData(const RenderData* r);


	private:

		void	flushRenderData();

		void	uploadShaderConstants(Material* pMat, const vec3& camPos, const Matrix& view, const Matrix& projection, const Matrix& pTransform);

		SimpleShaderContainer* mMRUShader;					// most recently used shader.


		/*
		data: array of vertex & index buffers for rendering
		*/
		AsyncResource<RenderDataArray>	mRenderData;

	};

	template<typename T>	ksU32 getGLType(const T* pT);

#define GET_GLTYPE(x)	getGLType(x)
}


#endif
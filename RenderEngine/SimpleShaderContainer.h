
/*
	Nnanna Kama
	Barebones class for managing Shaders
	Supports both GLSL and CG shaders (which don't have any good 3rd-party profiling tools).
	Uses a map to store parameter handles, so it's owning class don't gotta worry about such.
	ALL MATRIX GET & SET accessors presume a row-major format.
*/

#ifndef TARGET_CG_SHADERS
#define TARGET_GL_SHADERS	1
#endif

#ifndef SIMPLESHADERCONTAINER_H
#define SIMPLESHADERCONTAINER_H

#include <defines.h>
#include <unordered_map>
#include "crc32.h"

#if TARGET_CG_SHADERS
#include "..\shared_include\Cg\cg.h"
#include "..\shared_include\Cg\cgGL.h"

	typedef CGprofile		ShaderProfile;
	typedef CGcontext		ShaderContext;
	typedef CGparameter		ShaderParameter;
	typedef	CGprogram		ShaderProgram;
#else
	typedef ksU32			ShaderProfile;
	typedef ksU32			ShaderContext;
	typedef int				ShaderParameter;
	typedef ksU32			ShaderProgram;

	typedef enum ShaderAttribute
	{
		SA_POSITION		= 1,
		SA_NORMAL
	}ShaderAttribute;
#endif

#define DECLARE_SHADER_CONSTANT(x)	inline ksU32 _getShaderConstant##x()	{ static ksU32 p = CRC32(#x); return p; }
#define SHADER_CONSTANT_UID(x)		_getShaderConstant##x()

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
namespace ks {

	struct ShaderKey
	{
		ShaderKey(ksU32 pKey, const char* pName) : key(pKey), name(pName)
		{}

		ksU32 key;
		const char* name;
	};

#define SHADER_KEY_DECL(x)	ks::ShaderKey( SHADER_CONSTANT_UID(x), #x )


#if SUPPORT_GL_1_2
	#define SP_POS				"in_pos"
	#define SP_NOR				"in_nor"
#endif



	typedef std::unordered_map<u32, ShaderParameter >	ParamShaderMap;


	class SimpleShaderContainer
	{

	public:
		SimpleShaderContainer(const char* filename);

		~SimpleShaderContainer();

		static	ShaderContext	getStaticContext();

		static	void			destroyContext();

		static	void			destroyProgram(ShaderProgram& program);

		static	void			shaderErrorCallback();


		ShaderContext		getContext()	{ return SimpleShaderContainer::getStaticContext(); }

		ShaderProgram		getVertProgram() const	{ return mVertProgram; }

		ShaderProgram		getFragProgram() const	{ return mFragProgram; }

#if TARGET_CG_SHADERS
		ShaderProfile		getVertProfile() const	{return mVertProfile;}

		ShaderProfile		getFragProfile() const	{return mFragProfile;}
#endif

		void				registerConstant(ShaderKey& attrib);


		// get named vertex and fragment program parameters

		ShaderParameter		getNamedParam(const char* name);
		ShaderParameter		getNamedParam(const u32 pKey);


		template<typename N>
		void	setFloatParameter(N param, const float value);

		template<typename N, typename V>
		void	setVectorParameter(N param, const V* value);

		template<typename N, typename V>
		void	setMatrixParameter(N param, const V* value);

		void	updateProgramParameters();

		void	getMatrixParameter(ShaderParameter, float*);

		void	bindVertProgram()	{ bindProgram(mVertProgram); }

		void	bindFragProgram()	{ bindProgram(mFragProgram); }

		void	bindProgram()		{ bindProgram(mShaderProgram); }

		void	bindProgram(ShaderProgram& prog);

		void	unbindProgram();

		void	unbindVertProgram();

		void	unbindFragProgram();


	private:

		void			loadShader(const char* filename);

		ShaderProgram	loadProgram(int numFiles, ...);

		ShaderProgram	loadProgram(const char *filename, const char *entry, ShaderProfile profile);

		static ShaderContext	_gShaderContext;

#if TARGET_CG_SHADERS
		ShaderProfile			mVertProfile;

		ShaderProfile			mFragProfile;
#endif

		ShaderProgram			mVertProgram;

		ShaderProgram			mFragProgram;

		ShaderProgram			mShaderProgram;

		ParamShaderMap			mShaderParamMap;

		char					mName[MAX_NAME];
	};

#define GL_DEBUGGING		!FINAL_BUILD

#if GL_DEBUGGING
#define CHECK_GL_ERROR		SimpleShaderContainer::shaderErrorCallback()
#else
#define CHECK_GL_ERROR
#endif

}	// namespace ks

#endif		//SIMPLESHADERCONTAINER_H
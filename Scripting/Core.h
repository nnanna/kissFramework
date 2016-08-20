////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Only to be included by c++ scripts.
//
// References
// https://blog.molecular-matters.com/2014/05/10/using-runtime-compiled-c-code-as-a-scripting-language-under-the-hood/
// https://github.com/RuntimeCompiledCPlusPlus/RuntimeCompiledCPlusPlus
// http://www.catch22.net/tuts/reducing-executable-size
// https://github.com/i-saint/DynamicPatcher
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "..\Common\defines.h"
#include "ScriptInterface.h"
#include "ScriptEnvironment.h"

#include "Windows.h"
#include <stdio.h>
#include <new>

#pragma comment(lib, "kernel32.lib")
#pragma comment(linker, "/entry:_DllMainCRTStartup")
#pragma comment(linker, "/INCREMENTAL:NO")

#if OMITTING_DEFAULT_LIBRARIES
#ifdef __cplusplus
extern "C" {
#endif
	int _fltused = 0;
#ifdef __cplusplus
}
#endif

inline void *__cdecl operator new(size_t, void *_Where)
{
	return (_Where);	/* placement new */
}
#endif

using namespace ks;

#define KS_SCRIPT_DEFAULT_MEMBERS			\
	ScriptEnvironment*		mEnv;			\
	ScriptDataContext		mDataContext;	\


#define KS_SCRIPT_ON_INIT				void Initialise(ScriptEnvironment* pEnv, ScriptDataContext pDataContext, ScriptAttributes& rOutAttrib) override

#define KS_SCRIPT_ON_INIT_DEFAULT_BODY	mDataContext = pDataContext; mEnv = pEnv

#define KS_SCRIPT_ON_DESTROY			void Destroy() override

#define KS_SCRIPT_ON_UPDATE				void Update(float pDelta) override

#define KS_SCRIPT_EXPORT(Class)												\
	char sMem[sizeof(Class)];												\
	EXTERN_C __declspec(dllexport) ScriptInterface* CreateScript()			\
	{																		\
		Class* instance = new(sMem) Class;									\
		return instance;													\
	}																		\
	BOOL __stdcall DllMain( HINSTANCE h, DWORD d, LPVOID v ) {return true;}	\
	BOOL __stdcall _DllMainCRTStartup(HINSTANCE h, DWORD d, LPVOID v)		\
	{																		\
		return DllMain(h,d,v);												\
	}																		\


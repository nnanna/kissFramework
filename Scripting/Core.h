
// References
// https://github.com/i-saint/DynamicPatcher
// https://github.com/RuntimeCompiledCPlusPlus/RuntimeCompiledCPlusPlus
// https://blog.molecular-matters.com/2014/05/10/using-runtime-compiled-c-code-as-a-scripting-language-under-the-hood/
// http://www.catch22.net/tuts/reducing-executable-size


#pragma comment(lib, "kernel32.lib")

#include "..\Common\defines.h"
#include "ScriptInterface.h"
#include "ScriptEnvironment.h"
#include "Windows.h"

#pragma comment(linker, "/entry:_DllMainCRTStartup")
#pragma comment(linker, "/INCREMENTAL:NO")

EXTERN_C int _fltused = 0;

void * __cdecl operator new(unsigned int bytes)
{
	return 0; // HeapAlloc(GetProcessHeap(), 0, bytes);
}

void __cdecl operator delete(void *ptr)
{
	//if (ptr) HeapFree(GetProcessHeap(), 0, ptr);
}
//EXTERN_C int __cdecl __purecall()
//{
//	return 0;
//}
inline void *__cdecl operator new(size_t, void *_Where)
{
	return (_Where);	/* placement new */
}


using namespace ks;

#define KS_SCRIPT_DEFAULT_MEMBERS			\
	ScriptEnvironment*		mEnv;			\
	ScriptDataContext		mDataContext;	\

//#define STRINGIFY(x)		#x
//#define KS_SCRIPT_IMPORT(x)	STRINGIFY( ..\..\..\kissFramework\\##x )

#define KS_SCRIPT_BODY_INIT	void Initialise(ScriptEnvironment* pEnv, ScriptDataContext pDataContext, ScriptAttributes& rOutAttrib) override

#define KS_SCRIPT_INIT_INTERNAL	mDataContext = pDataContext; mEnv = pEnv

#define KS_SCRIPT_BODY_DESTROY	void Destroy() override

#define KS_SCRIPT_BODY_UPDATE	void Update(float pDelta) override

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
		DllMain(h,d,v);														\
		return true;														\
	}																		\


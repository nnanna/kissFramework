
//////////////////////////////////////////////////////////////////////////
///
/// Copyright (c)
///	@author		Nnanna Kama
///	@date		24/07/2016
///	@brief		
///
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
/// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
/// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
/// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//////////////////////////////////////////////////////////////////////////

#include "defines.h"
#include "Windows.h"
#include "Debug.h"
#include "crc32.h"
#include "ScriptFactory.h"
#include "ScriptInterface.h"
#include <string>
#include <unordered_map>
#include <Containers\Array.h>

namespace ks{

	class ScriptCollection
	{
	public:
		void Cache(const char* pName, ScriptInterface* pScript, HMODULE pHandle)
		{
			if (Find(pName) < 0)
			{
				ScriptKey key = { CRC32(pName), pScript, pHandle };
				mLoadedScripts.push_back(key);
			}
		}

		int Find(const char* pName) const
		{
			const ksU32 id = CRC32(pName);
			for (ksU32 i = 0; i < mLoadedScripts.size(); ++i)
			{
				if (mLoadedScripts[i].mUID == id)
					return i;
			}
			return -1;
		}

		int Find(ScriptInterface* pScript) const
		{
			for (ksU32 i = 0; i < mLoadedScripts.size(); ++i)
			{
				if (mLoadedScripts[i].mScript == pScript)
					return i;
			}
			return -1;
		}

		ScriptInterface* operator[](ksU32 i) const
		{
			if (i < mLoadedScripts.size())
				return mLoadedScripts[i].mScript;

			return nullptr;
		}

		void Erase(ksU32 i)
		{
			if (i < mLoadedScripts.size())
			{
				::FreeLibrary(mLoadedScripts[i].mModule);
				
				mLoadedScripts[i] = mLoadedScripts.back();
				mLoadedScripts.pop_back();
			}
		}

	private:
		struct ScriptKey
		{
			ksU32				mUID;
			ScriptInterface*	mScript;
			HMODULE				mModule;
		};
		Array<ScriptKey>		mLoadedScripts;
	};

	ScriptFactory::ScriptFactory(ScriptEnvironment* pEnv) : mEnv(pEnv), mProcessReadHandle(nullptr), mProcessWriteHandle(nullptr), mVersioning(0)
	{
		mScripts = new ScriptCollection();

		SECURITY_ATTRIBUTES saAttr = {};
		{
			saAttr.nLength				= sizeof(SECURITY_ATTRIBUTES);
			saAttr.bInheritHandle		= TRUE;				// Set the bInheritHandle flag so pipe handles are inherited.
			saAttr.lpSecurityDescriptor = NULL;
		}

		if (!CreatePipe(&mProcessReadHandle, &mProcessWriteHandle, &saAttr, 0))	// for accessing spawned process's debug output
			KS_ASSERT(0 && "Process error output pipe failure");

		const char* Version = "12.0";		// TODO: support more vc versions
		const char* keyName = "SOFTWARE\\Microsoft\\VisualStudio\\SxS\\VC7";
		char value[MAX_PATH];
		DWORD size(MAX_PATH);
		HKEY key;
		LONG errCode = ::RegOpenKeyExA(HKEY_LOCAL_MACHINE, keyName, 0, KEY_READ | KEY_WOW64_32KEY, &key);
		errCode |= ::RegQueryValueExA(key, Version, NULL, NULL, (LPBYTE)value, &size);
		if (errCode == ERROR_SUCCESS)
		{
			sVCVars[0] = '"';
			sVCVars[1] = '\0';
			strcat_s(sVCVars, value);				//sVCVars += value;
			strcat_s(sVCVars, "vcvarsall.bat");		//sVCVars += "vcvarsall.bat";
			strcat_s(sVCVars, "\"");
#ifdef _WIN64
			strcat_s(sVCVars, " amd64");			//sVCVars += " amd64";
#else
			strcat_s(sVCVars, " x86");				//sVCVars += " x86";
#endif
		}
	}

	ScriptFactory::~ScriptFactory()
	{
		::CloseHandle(mProcessWriteHandle);
		::CloseHandle(mProcessReadHandle);
	}

	ScriptInterface* ScriptFactory::Load(const char* pName, ScriptDataContext pCtx, bool pReload /*= false*/)
	{
		ksU32 version = 0;
#if SHIPPING_BUILD
		pReload		= false;
#else
		static volatile bool FORCE_GENERATE_DEBUG_LIBS(true);
		if (pReload || FORCE_GENERATE_DEBUG_LIBS)
			version		= ++mVersioning;
#endif

		const int index = mScripts->Find(pName);
		if ( index >= 0)
		{
			if (pReload)
				mScripts->Erase(index);
			else
				return (*mScripts)[index];
		}

		typedef ScriptInterface*(*CreateScript)();
		ScriptInterface* script = nullptr;
		if( compileScript( pName, version ) )
		{
			std::string fullpath	= "modules\\";
			fullpath		+= pName;
			if (version)
				fullpath	+= std::to_string(version);
			fullpath		+= ".dll";
			auto hInstance	= ::LoadLibraryA(fullpath.c_str());
			auto creator	= (CreateScript)GetProcAddress(hInstance, "CreateScript");
			if (creator)
				script		= creator();

			if (script)
			{
				ScriptAttributes rOutAttrib;
				script->Initialise(mEnv, pCtx, rOutAttrib);
				mScripts->Cache(pName, script, hInstance);
			}
		}
		return script;
	}

	void ScriptFactory::Unload(ScriptInterface*& pScript)
	{
		int i = mScripts->Find(pScript);
		if (i >= 0)
		{
			mScripts->Erase(i);
		}

		pScript = nullptr;
	}


	bool ScriptFactory::compileScript(const char *filename, ksU32 pVersion)
	{
		bool success = false;
		std::string cmd;
		cmd.reserve(512);
		cmd				= sVCVars;
		cmd				+= " && cl /LD /MD /EHsc /GR- /nologo ";
		if (pVersion)
		{
#if !SHIPPING_BUILD
			cmd			+= "/Z7 ";	// emit a .pdb debug file
#endif
		}

		// include paths
		cmd				+= "/I \"..\\..\\kissFramework\" /I \"..\\..\\kissFramework\\Common\" /I \"..\\..\\kissFramework\\Containers\" ";

		// preprocessor defines
#if !SHIPPING_BUILD
		cmd				+= "/DSCRIPT_BUILD=1 ";
#endif
#if _DEBUG
		cmd				+= "/DDEBUG_VERSION /D_DEBUG /DSCRIPT_COMPILE ";
#endif

		// target cpp file.
		cmd				+= "Scripts\\";
		cmd				+= filename;
		cmd				+= ".cpp";

		//output paths
		if (pVersion)
		{
			std::string incrVer	= filename + std::to_string(pVersion);
			cmd			+= " /Fomodules\\ /Femodules\\" + incrVer;		// apply versioning on output file as VC doesn't unload PDB files.
		}
		else
		{
			cmd			+= " /Fomodules\\ /Femodules\\";
		}

		// cleanup intermediate files
		{
			std::string removefiles = " && del .\\modules\\";
			removefiles				+= filename;

			cmd						+= removefiles + "*.obj";
			cmd						+= removefiles + "*.lib";
			cmd						+= removefiles + "*.exp";
			if (!pVersion)
			{
				cmd					+= removefiles + "*.pdb";	// won't delete access locked files anyway
			}
		}
		
		STARTUPINFOA si			= {};
		PROCESS_INFORMATION pi	= {};
		si.cb					= sizeof(si);
		si.hStdError			= mProcessWriteHandle;
		si.hStdOutput			= mProcessWriteHandle;
		si.dwFlags				|= STARTF_USESTDHANDLES;
		if (::CreateProcessA(NULL, (LPSTR)cmd.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi) == TRUE)
		{
			DWORD exit_code = 0;
			::WaitForSingleObject(pi.hThread, INFINITE);
			::WaitForSingleObject(pi.hProcess, INFINITE);
			::GetExitCodeProcess(pi.hProcess, &exit_code);
			::CloseHandle(pi.hThread);
			::CloseHandle(pi.hProcess);
			if (exit_code != 0)
			{
#define	ERROR_BUF_SIZE	1024
				DWORD dwRead;
				char chBuf[ERROR_BUF_SIZE];
				BOOL bSuccess = ReadFile(mProcessReadHandle, chBuf, ERROR_BUF_SIZE, &dwRead, NULL);

				if (bSuccess)
				{
					dwRead = dwRead < ERROR_BUF_SIZE ? dwRead : (ERROR_BUF_SIZE - 1);
					chBuf[dwRead] = '\0';
					OutputDebugStringA(chBuf);
				}
				KS_ASSERT(0);
			}
			else
				success = true;
		}

		return success;
	}

}
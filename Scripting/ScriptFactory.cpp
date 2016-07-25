
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
#include "ScriptFactory.h"
#include "ScriptInterface.h"
#include <string>

namespace ks{

	static std::string sVCVars;

	ScriptFactory::ScriptFactory() : mProcessReadHandle(nullptr), mProcessWriteHandle(nullptr)
	{
		SECURITY_ATTRIBUTES saAttr = {};
		{
			saAttr.nLength				= sizeof(SECURITY_ATTRIBUTES);
			saAttr.bInheritHandle		= TRUE;				// Set the bInheritHandle flag so pipe handles are inherited.
			saAttr.lpSecurityDescriptor = NULL;
		}

		if (!CreatePipe(&mProcessReadHandle, &mProcessWriteHandle, &saAttr, 0))	// for accessing spawned process's debug output
			KS_ASSERT(0 && "Process error output pipe failure");

		if (sVCVars.empty())		// why i lazy like dat
		{
			const char* Version = "12.0";		// TODO: support more vc versions
			const char* keyName = "SOFTWARE\\Microsoft\\VisualStudio\\SxS\\VC7";
			char value[MAX_PATH];
			DWORD size(MAX_PATH);
			HKEY key;
			LONG errCode	= ::RegOpenKeyExA(HKEY_LOCAL_MACHINE, keyName, 0, KEY_READ | KEY_WOW64_32KEY, &key);
			errCode			|= ::RegQueryValueExA(key, Version, NULL, NULL, (LPBYTE)value, &size);
			if (errCode == ERROR_SUCCESS)
			{
				sVCVars += '"';
				sVCVars += value;
				sVCVars += "vcvarsall.bat";
				sVCVars += '"';
#ifdef _WIN64
				sVCVars += " amd64";
#else
				sVCVars += " x86";
#endif
			}
		}

	}

	ScriptFactory::~ScriptFactory()
	{
		::CloseHandle(mProcessWriteHandle);
		::CloseHandle(mProcessReadHandle);
	}

	ScriptInterface* ScriptFactory::Load(const char* pName, bool pReload /*= false*/)
	{
		typedef ScriptInterface*(*ScriptConstructor)();
		ScriptInterface* script = nullptr;
		std::string fullpath = "Scripts\\";
		fullpath += pName;
		fullpath += ".cpp";
		if( compileScript( fullpath.c_str() ) )
		{
			fullpath		= pName;
			fullpath		+= ".dll";
			auto hInstance	= ::LoadLibraryA(fullpath.c_str());
			auto creator	= (ScriptConstructor)GetProcAddress(hInstance, "CreateScript");
			if (creator)
				script		= creator();

			::FreeLibrary(hInstance);
		}
		return script;
	}

	void ScriptFactory::Unload(ScriptInterface*& pScript)
	{}


	bool ScriptFactory::compileScript(const char *filename)
	{
		bool success = false;

		std::string cmd	= sVCVars;
		cmd				+= " && cl /LD ";
		cmd				+= filename;
		cmd				+= " /nologo";
		
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
				DWORD dwRead;
				char chBuf[512];
				BOOL bSuccess = ReadFile(mProcessReadHandle, chBuf, 512, &dwRead, NULL);

				if (bSuccess && dwRead < 512)
				{
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
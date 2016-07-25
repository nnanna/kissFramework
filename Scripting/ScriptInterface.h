
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


namespace ks {

	class ScriptInterface
	{
	public:
		virtual void SetEnvironment(class ScriptEnvironment* si) = 0;
		virtual void SetDataContext(void* pDataContext) = 0;		// TODO: type-safety for data context. i.e use a ScriptData arg wrapper with intrinsic type validation.
		virtual const char* GetName() const = 0;

		virtual void Update(float pDelta) = 0;
	};

#define KS_SCRIPT_DEFAULT_MEMBERS			\
	ScriptEnvironment*		mEnv;			\
	void*					mDataContext;	\

#define KS_SCRIPT_DEFAULT_METHODS																\
	void SetEnvironment(ScriptEnvironment* pEnv) override	{ mEnv = pEnv; }					\
	void SetDataContext(void* pDataContext) override		{ mDataContext = pDataContext; }	\


#define KS_SCRIPT_EXPORT(Class)												\
	extern "C" __declspec(dllexport) ScriptInterface* CreateScript()		\
	{																		\
		Class* instance = new Class;										\
		return instance;													\
	}																		\

}
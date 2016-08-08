
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

	struct ScriptAttributes
	{
		ScriptAttributes() : is_threadsafe(false), update_frequency_ms(0), update_hierarchy(0)
		{}

		bool is_threadsafe;			// set if script can be updated in parallel to other scripts
		short update_frequency_ms;	// minimum millisecond interval between updates
		short update_hierarchy;		// order calls to Update() from lowest to highest
	};

	typedef void*	ScriptDataContext;

	class __declspec(novtable) ScriptInterface
	{
	public:
		// ScriptEnvironment - provide access to core app systems from script.
		// ScriptDataContext - anonymous state communication between script and app client(s)
		// ScriptAttributes - metadata for ScriptEngine, see ScriptAttributes struct
		virtual void Initialise(class ScriptEnvironment* pEnv, ScriptDataContext pDataContext, ScriptAttributes& rOutAttrib) = 0;

		// Free up any data created via ScriptEnvironment, or ScriptDataContext
		virtual void Destroy() = 0;

		virtual void Update(float pDelta) = 0;
	};

}
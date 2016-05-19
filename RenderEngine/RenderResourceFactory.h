/***
 This should not need to be instantiated as it's just a static class.
 The idea is to manage render elements so they're created just once and reused... forever.
 Client classes won't need to worry about memory management & the rest; just ask and receive, not questions asked.
***/

#pragma once

#include "SimpleShaderContainer.h"
#include "Material.h"


namespace ks {

	typedef std::unordered_map<ks32, SimpleShaderContainer*>		ShaderLibraryMap;
	typedef std::unordered_map<ks32, Material>						MaterialLibraryMap;

	class RenderResourceFactory
	{
	public:
		RenderResourceFactory();
		~RenderResourceFactory();

		static void init();

		static void shutDown();

		static void onShaderDelete(SimpleShaderContainer* shader);

		static SimpleShaderContainer* findShader(const char* shader_path);

		static SimpleShaderContainer* findOrCreateShader(const char* shader_path);

	private:

		static ShaderLibraryMap		sShaderLibrary;
		static MaterialLibraryMap	sMaterialLibrary;
		static bool					sActive;
	};

}	// namespace ks


#include "defines.h"
#include "RenderResourceFactory.h"
#include "crc32.h"

namespace ks {

	ShaderLibraryMap	RenderResourceFactory::sShaderLibrary;
	bool				RenderResourceFactory::sActive = false;


	RenderResourceFactory::RenderResourceFactory()
	{}


	RenderResourceFactory::~RenderResourceFactory()
	{}

	void RenderResourceFactory::init()
	{
		sActive = true;
	}

	void RenderResourceFactory::shutDown()
	{
		sActive = false;

		for (ShaderLibraryMap::iterator itr = sShaderLibrary.begin(); itr != sShaderLibrary.end(); itr++)
		{
			SAFE_DELETE(itr->second);
		}

		sShaderLibrary.clear();

		SimpleShaderContainer::destroyContext();
	}


	void RenderResourceFactory::onShaderDelete(SimpleShaderContainer* shader)
	{
		if (sActive)
		{
			ShaderLibraryMap::iterator itr = sShaderLibrary.begin();
			for (; itr != sShaderLibrary.end(); ++itr)
			{
				if (itr->second == shader)
				{
					sShaderLibrary.erase(itr);
					break;
				}
			}
		}
	}

	SimpleShaderContainer* RenderResourceFactory::findShader(const char* shader_name)
	{
		ks32 key = CRC32(shader_name);
		ShaderLibraryMap::iterator itr = sShaderLibrary.find(key);
		return itr != sShaderLibrary.end() ? itr->second : nullptr;
	}


	SimpleShaderContainer* RenderResourceFactory::findOrCreateShader(const char* shader_name)
	{
		ks32 key = CRC32(shader_name);
		ShaderLibraryMap::iterator itr = sShaderLibrary.find(key);
		if (itr != sShaderLibrary.end())
		{
			return itr->second;
		}

		SimpleShaderContainer* shader = new SimpleShaderContainer(shader_name);

		// prepend and append filename directories and stuff like that here.
		// @todo


		sShaderLibrary[key] = shader;

		return shader;
	}



}	// namespace ks
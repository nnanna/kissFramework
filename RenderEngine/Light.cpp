
#include "Light.h"
#include <math.h>

namespace ks {


	Light Light::create(const vec3& pPos, const vec3& pCol)
	{
		Light l = { pPos, pCol };
		return l;
	}

	const Light& Light::getDefault()
	{
		static const float rotAngle = -0.4f;
		const float sLightRadius = 25.f;
		static Light sDefaultLight = { vec3(sLightRadius*sin(rotAngle), 50.5f, sLightRadius*cos(rotAngle)), vec3(0.95f, 0.95f, 0.95f) };

		return sDefaultLight;
	}
}
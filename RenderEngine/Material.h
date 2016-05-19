
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Nnanna Kama : Simple MATERIAL STRUCT:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef KS_MATERIAL_H
#define KS_MATERIAL_H

namespace ks
{
	struct vec3;
	class SimpleShaderContainer;


	enum ShaderConstantsID
	{
		sci_diffuse,
		sci_ambient,
		sci_emissive,
		sci_specular,
		sci_shininess,
		sci_global_amb,
		sci_light_col,
		sci_eye_pos,
		sci_light_pos,
		sci_mvp,
	};

	struct Material
	{
		float	Ambient[3];
		float	Diffuse[3];
		float	Emissive[3];
		float	Specular[3];
		float	Shininess;

		SimpleShaderContainer*	ShaderContainer;

		Material();

		Material(SimpleShaderContainer* pShader);

		void SetEmissive(float x, float y, float z);
		void SetDiffuse(float x, float y, float z);
		void SetAmbient(float x, float y, float z);
		void SetSpecular(float x, float y, float z);

		void SetShaderParams();


		static void setBrassMaterial(Material* mat);

		static void setRedPlasticMaterial(Material* mat);

		static void setEmissiveLightColorOnly(Material* mat, const ks::vec3& pEmissiveCol);

		static const struct ShaderKey*	gConstantsRegistry;	// fast hack for passing shader semantics from app to framework. TODO
	};
}


#endif
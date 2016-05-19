//=======================================================================================================================
// Some code courtesy http://sourceforge.net/projects/daedalus-n64/ | FFXMaths.h
// Modified for use.
//=======================================================================================================================

#ifndef KISS_MATHS_H
#define KISS_MATHS_H

#include <defines.h>


#ifndef KISS_INFINITY
#define KISS_INFINITY	3.402823466e+38F		/* max float val */
#endif

#ifndef PI
#define PI	3.14159265358979323846
#endif


#define RAD_TO_DEG(rad)		((rad) * (180.f / PI)	)
#define DEG_TO_RAD(deg)		((deg) * (PI / 180.f)	)


namespace ks {

	static inline float InvSqrt(float x)
	{
		float xhalf = 0.5f * x;
		int i		= *(int*)&x;				// get bits for floating value
		i			= 0x5f3759df - (i >> 1);	// gives initial guess y0
		x			= *(float*)&i;				// convert bits back to float
		x			= x*(1.5f - xhalf*x*x);		// Newton step, repeat to increase accuracy
	
		return x;
	}
	
	static inline float Sqrt(float x)
	{
		return x * InvSqrt(x);
	}

	//=======================================================================================================================
	// vec4
	//=======================================================================================================================
	class vec4
	{
	public:
		vec4() {}
		vec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}

		float Normalise();

		vec4 operator+(const vec4 & v) const;
		vec4 operator-(const vec4 & v) const;
		vec4 operator*(float s) const;

		float Length() const;

		float Dot(const vec4 & rhs) const;


		float x, y, z, w;
	};



	//=======================================================================================================================
	// vec3
	//=======================================================================================================================

	struct vec3
	{
	public:
		vec3();
		vec3(const float *);
		vec3(const vec3&);
		vec3(float x, float y, float z);

		operator float* ();
		operator const float* () const;

		// assignment operators
		vec3& operator += (const vec3&);
		vec3& operator -= (const vec3&);
		vec3& operator *= (const vec3&);
		vec3& operator *= (float);
		vec3& operator /= (float);

		// unary operators
		vec3 operator + () const;
		vec3 operator - () const;

		// binary operators
		vec3 operator + (const vec3&) const;
		vec3 operator - (const vec3&) const;
		vec3 operator * (float) const;
		vec3 operator / (float) const;

		friend vec3 operator * (float, const struct vec3&);

		bool operator == (const vec3&) const;
		bool operator != (const vec3&) const;

		float Dot(const vec3& rhs) const;
		float Length() const;
		float LengthSq() const;
		void Normalize();
		void FastNormalize();

		static const vec3 UNIT_X;
		static const vec3 UNIT_Y;
		static const vec3 UNIT_Z;
		static const vec3 ZERO_VEC;

		// pNormal must be normalised
		void reflect(const vec3& pNormal, vec3& rReflection) const;

		float x, y, z;
	};


	//=======================================================================================================================
	// COLOR
	//=======================================================================================================================

	struct COLOR
	{
		COLOR();
		COLOR(float, float, float, float);

		// assignment
		COLOR& operator += (const COLOR&);
		COLOR& operator -= (const COLOR&);
		COLOR& operator *= (float);
		COLOR& operator /= (float);

		// unary
		COLOR operator + () const;
		COLOR operator - () const;

		// binary
		COLOR operator + (const COLOR&) const;
		COLOR operator - (const COLOR&) const;
		COLOR operator * (float) const;
		COLOR operator / (float) const;

		friend COLOR operator * (float, const COLOR&);

		bool operator == (const COLOR&) const;
		bool operator != (const COLOR&) const;

		float r, g, b, a;
	};


	//=======================================================================================================================
	// Bounding Volumes
	//=======================================================================================================================
	struct AABB
	{
		AABB()
			: minPt(KISS_INFINITY, KISS_INFINITY, KISS_INFINITY)
			, maxPt(-KISS_INFINITY, -KISS_INFINITY, -KISS_INFINITY)
		{}

		vec3 center() const		{ return 0.5f*(minPt + maxPt); }

		vec3 minPt;
		vec3 maxPt;
	};


	struct BoundingSphere
	{
		BoundingSphere() : pos(0.0f, 0.0f, 0.0f), radius(0.0f)
		{}

		vec3 pos;
		float radius;
	};

	//=======================================================================================================================
	// Random Number Generator
	//=======================================================================================================================
	struct RNG
	{
		RNG();
		RNG(u32 seed);

		u32		GetU32();
		float	GetFloatBetween(float x, float y);
		void	GetV3PlusMinus(vec3& pDest, const vec3& pRange);

	private:
		u32 mSeed;
	};



	//=======================================================================================================================
	// Matrix
	//=======================================================================================================================
	class Matrix
	{
	public:

		Matrix()
		{}

		Matrix(float _11, float _12, float _13, float _14,
			float _21, float _22, float _23, float _24,
			float _31, float _32, float _33, float _34,
			float _41, float _42, float _43, float _44)
			: m11(_11), m12(_12), m13(_13), m14(_14)
			, m21(_21), m22(_22), m23(_23), m24(_24)
			, m31(_31), m32(_32), m33(_33), m34(_34)
			, m41(_41), m42(_42), m43(_43), m44(_44)
		{}

		Matrix operator*(const Matrix & rhs) const;

		Matrix & SetIdentity();
		Matrix & SetScaling(float scale);
		Matrix & SetRotateX(float angle);
		Matrix & SetRotateY(float angle);
		Matrix & SetRotateZ(float angle);

		Matrix Transpose() const;
		Matrix Inverse() const;

		vec3 TransformCoord(const vec3 & vec) const;
		vec3 TransformNormal(const vec3 & vec) const;

		vec3 Transform(const vec3 & vec) const;
		vec4 Transform(const vec4 & vec) const;

		void	print() const;

	public:
		union
		{
			struct
			{
				float	m11, m12, m13, m14;
				float	m21, m22, m23, m24;
				float	m31, m32, m33, m34;
				float	m41, m42, m43, m44;
			};

			float	m[4][4];
		};


		static const Matrix IDENTITY;
	};

	void buildProjectionMatrix(float fieldOfView,
		float aspectRatio,
		float zNear, float zFar,
		float m[16]);

	void buildViewMatrix(float eyex, float eyey, float eyez,
		float centerx, float centery, float centerz,
		float upx, float upy, float upz,
		float m[16]);


}	// namespace ks

#endif
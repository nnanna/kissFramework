//=======================================================================================================================
//	Code courtesy http://sourceforge.net/projects/daedalus-n64/
//	From Daedalus - An open-source N64 emulator.
//	Used as is, except for commented-out/replaced headers, and namepaces
//=======================================================================================================================

#include <stdio.h>
#include <math.h>
#include <Common/Debug.h>
#include <Maths/ks_Maths.h>
#include <Utility.h>

namespace ks {

	const Matrix Matrix::IDENTITY = Matrix(1, 0, 0, 0,
													0, 1, 0, 0,
													0, 0, 1, 0,
													0, 0, 0, 1);
	
	// http://forums.ps2dev.org/viewtopic.php?t=5557
	// http://bradburn.net/mr.mr/vfpu.html


#ifdef DAEDALUS_PSP_USE_VFPU

	float mysqrt(float val)
	{
		float ret;

		__asm__ volatile (
			"mtv %1, S000\n"
			"vsqrt.s S001, S000\n"
			"mfv %0, S001\n"
			: "=r"(ret) : "r"(val) );

		return ret;
	}

	void vsincosf(float radians, vec4* result)
	{
		__asm__ volatile (
			"mtv %1, S000\n"
			"vrot.q C010, S000, [s, c, 0, 0]\n"
			"usv.q C010, 0 + %0\n"
			: "+m"(*result) : "r"(radians));
	}
	void matrixMultiplyUnaligned(Matrix * m_out, const Matrix *mat_a, const Matrix *mat_b)
	{
		pspvfpu_use_matrices(gDaedalusVFPUContext, 0, VMAT0 | VMAT1 | VMAT2);

		__asm__ volatile (

			"ulv.q   R000, 0  + %1\n"
			"ulv.q   R001, 16 + %1\n"
			"ulv.q   R002, 32 + %1\n"
			"ulv.q   R003, 48 + %1\n"

			"ulv.q   R100, 0  + %2\n"
			"ulv.q   R101, 16 + %2\n"
			"ulv.q   R102, 32 + %2\n"
			"ulv.q   R103, 48 + %2\n"

			"vmmul.q   M200, M000, M100\n"

			"usv.q   R200, 0  + %0\n"
			"usv.q   R201, 16 + %0\n"
			"usv.q   R202, 32 + %0\n"
			"usv.q   R203, 48 + %0\n"

			: "=m" (*m_out) : "m" (*mat_a) ,"m" (*mat_b) : "memory" );
	} 

	void matrixMultiplyAligned(Matrix * m_out, const Matrix *mat_a, const Matrix *mat_b)
	{
		pspvfpu_use_matrices(gDaedalusVFPUContext, 0, VMAT0 | VMAT1 | VMAT2);

		__asm__ volatile (

			"lv.q   R000, 0  + %1\n"
			"lv.q   R001, 16 + %1\n"
			"lv.q   R002, 32 + %1\n"
			"lv.q   R003, 48 + %1\n"

			"lv.q   R100, 0  + %2\n"
			"lv.q   R101, 16 + %2\n"
			"lv.q   R102, 32 + %2\n"
			"lv.q   R103, 48 + %2\n"

			"vmmul.q   M200, M000, M100\n"

			"sv.q   R200, 0  + %0\n"
			"sv.q   R201, 16 + %0\n"
			"sv.q   R202, 32 + %0\n"
			"sv.q   R203, 48 + %0\n"

			: "=m" (*m_out) : "m" (*mat_a) ,"m" (*mat_b) : "memory" );
	} 


	void myCopyMatrix(Matrix *m_out, const Matrix *m_in)
	{
		__asm__ volatile (
			"lv.q   R000, 0x0(%1)\n"
			"lv.q   R001, 0x10(%1)\n"
			"lv.q   R002, 0x20(%1)\n"
			"lv.q   R003, 0x30(%1)\n"

			"sv.q   R000, 0x0(%0)\n"
			"sv.q   R001, 0x10(%0)\n"
			"sv.q   R002, 0x20(%0)\n"
			"sv.q   R003, 0x30(%0)\n"
			: : "r" (m_out) , "r" (m_in) );
	}

	void myApplyMatrix(vec4 *v_out, const Matrix *mat, const vec4 *v_in)
	{
		__asm__ volatile (
			"lv.q   R000, 0x0(%1)\n"
			"lv.q   R001, 0x10(%1)\n"
			"lv.q   R002, 0x20(%1)\n"
			"lv.q   R003, 0x30(%1)\n"

			"lv.q   R100, 0x0(%2)\n"

			"vtfm4.q R200, E000, R100\n"
			"sv.q   R200, 0x0(%0)\n"
			: : "r" (v_out) , "r" (mat) ,"r" (v_in) );
	}

#endif // DAEDALUS_PSP_USE_VFPU

	Matrix & Matrix::SetIdentity()
	{
		*this = IDENTITY;
		return *this;
	}

	Matrix & Matrix::SetScaling(float scale)
	{
		for (u32 r = 0; r < 3; ++r)
		{
			for (u32 c = 0; c < 3; ++c)
			{
				m[r][c] = (r == c) ? scale : 0;
			}
		}
		return *this;
	}

	Matrix & Matrix::SetRotateX(float radians)
	{
		float	s(sinf(radians));
		float	c(cosf(radians));

		m11 = 1;	m12 = 0;	m13 = 0;	m14 = 0;
		m21 = 0;	m22 = c;	m23 = -s;	m24 = 0;
		m31 = 0;	m32 = s;	m33 = c;	m34 = 0;
		m41 = 0;	m42 = 0;	m43 = 0;	m44 = 1;
		return *this;
	}

	Matrix & Matrix::SetRotateY(float radians)
	{
		float	s(sinf(radians));
		float	c(cosf(radians));

		m11 = c;	m12 = 0;	m13 = s;	m14 = 0;
		m21 = 0;	m22 = 1;	m23 = 0;	m24 = 0;
		m31 = -s;	m32 = 0;	m33 = c;	m34 = 0;
		m41 = 0;	m42 = 0;	m43 = 0;	m44 = 1;
		return *this;
	}

	Matrix & Matrix::SetRotateZ(float radians)
	{
		float	s(sinf(radians));
		float	c(cosf(radians));

		m11 = c;	m12 = -s;	m13 = 0;	m14 = 0;
		m21 = s;	m22 = c;	m23 = 0;	m24 = 0;
		m31 = 0;	m32 = 0;	m33 = 1;	m34 = 0;
		m41 = 0;	m42 = 0;	m43 = 0;	m44 = 1;
		return *this;
	}

	void Matrix::SetRotationFast(const vec3 & pAxis, float pTheta)
	{
		const float x = pAxis.x;
		const float y = pAxis.y;
		const float z = pAxis.z;
		const float s = FAST_SIN(pTheta);
		const float c = FAST_COS(pTheta);
		const float t = 1 - c;
		const float sx = s*x;
		const float sy = s*y;
		const float sz = s*z;
		const float txy = t*x*y;
		const float txz = t*x*z;
		const float tyz = t*y*z;

		m[0][0] = (t*x*x) + c;
		m[0][1] = txy + sz;
		m[0][2] = txz - sy;

		m[1][0] = txy - sz;
		m[1][1] = (t*y*y) + c;
		m[1][2] = tyz + sx;

		m[2][0] = txz + sy;
		m[2][1] = tyz - sx;
		m[2][2] = (t*z*z) + c;
	}


	vec3 Matrix::TransformCoord(const vec3 & vec) const
	{
		return vec3(vec.x * m11 + vec.y * m21 + vec.z * m31 + m41,
			vec.x * m12 + vec.y * m22 + vec.z * m32 + m42,
			vec.x * m13 + vec.y * m23 + vec.z * m33 + m43);
	}

	vec3 Matrix::TransformNormal(const vec3 & vec) const
	{
		return vec3(vec.x * m11 + vec.y * m21 + vec.z * m31,
			vec.x * m12 + vec.y * m22 + vec.z * m32,
			vec.x * m13 + vec.y * m23 + vec.z * m33);
	}

	vec4 Matrix::Transform(const vec4 & vec) const
	{
		return vec4(vec.x * m11 + vec.y * m21 + vec.z * m31 + vec.w * m41,
			vec.x * m12 + vec.y * m22 + vec.z * m32 + vec.w * m42,
			vec.x * m13 + vec.y * m23 + vec.z * m33 + vec.w * m43,
			vec.x * m14 + vec.y * m24 + vec.z * m34 + vec.w * m44);
	}

	vec3 Matrix::Transform(const vec3 & vec) const
	{
		vec4	trans(vec.x * m11 + vec.y * m21 + vec.z * m31 + m41,
			vec.x * m12 + vec.y * m22 + vec.z * m32 + m42,
			vec.x * m13 + vec.y * m23 + vec.z * m33 + m43,
			vec.x * m14 + vec.y * m24 + vec.z * m34 + m44);

		if (fabsf(trans.w) > 0.0f)
		{
			return vec3(trans.x / trans.w, trans.y / trans.w, trans.z / trans.w);
		}

		return vec3(trans.x, trans.y, trans.z);
	}



	template< typename Type > void Swap(Type & a, Type & b)
	{
		Type temp(a);
		a = b;
		b = temp;
	}

	Matrix		Matrix::Transpose() const
	{
		return Matrix(m11, m21, m31, m41,
						m12, m22, m32, m42,
						m13, m23, m33, m43,
						m14, m24, m34, m44);
	}

	Matrix	Matrix::Inverse() const
	{
		float	augmented[4][8];

		//
		// Init augmented array
		//
		for (u32 r = 0; r < 4; ++r)
		{
			for (u32 c = 0; c < 8; ++c)
			{
				if (c < 4)
				{
					augmented[r][c] = m[r][c];
				}
				else
				{
					augmented[r][c] = float((c == r + 4) ? 1 : 0);
				}
			}
		}

		for (u32 j = 0; j < 4; ++j)
		{
			bool	found(false);
			for (u32 i = j; i < 4; ++i)
			{
				if (augmented[i][j] != 0)
				{
					if (i != j)
					{
						// Exchange rows i and j
						for (u32 k = 0; k < 8; ++k)
						{
							Swap(augmented[i][k], augmented[j][k]);
						}
					}
					found = true;
					break;
				}
			}

			if (found == false)
			{
				return IDENTITY;
			}

			//
			// Multiply the row by 1/Mjj
			//
			float	rcp(1.0f / augmented[j][j]);
			for (u32 k = 0; k < 8; ++k)
			{
				augmented[j][k] *= rcp;
			}

			for (u32 r = 0; r < 4; ++r)
			{
				float	q(-augmented[r][j]);
				if (r != j)
				{
					for (u32 k = 0; k < 8; k++)
					{
						augmented[r][k] += q * augmented[j][k];
					}
				}
			}
		}


		return Matrix(augmented[0][4], augmented[0][5], augmented[0][6], augmented[0][7],
			augmented[1][4], augmented[1][5], augmented[1][6], augmented[1][7],
			augmented[2][4], augmented[2][5], augmented[2][6], augmented[2][7],
			augmented[3][4], augmented[3][5], augmented[3][6], augmented[3][7]);
	}

	void myMulMatrixCPU(Matrix * m_out, const Matrix *mat_a, const Matrix *mat_b)
	{
		for (u32 i = 0; i < 4; ++i)
		{
			for (u32 j = 0; j < 4; ++j)
			{
				m_out->m[i][j] = mat_a->m[i][0] * mat_b->m[0][j] +
					mat_a->m[i][1] * mat_b->m[1][j] +
					mat_a->m[i][2] * mat_b->m[2][j] +
					mat_a->m[i][3] * mat_b->m[3][j];
			}
		}

	}


	Matrix Matrix::operator*(const Matrix & rhs) const
	{
		Matrix r;

#ifdef DAEDALUS_PSP_USE_VFPU
		matrixMultiplyUnaligned( &r, this, &rhs );
#else
		for (u32 i = 0; i < 4; ++i)
		{
			for (u32 j = 0; j < 4; ++j)
			{
				r.m[i][j] = m[i][0] * rhs.m[0][j] +
					m[i][1] * rhs.m[1][j] +
					m[i][2] * rhs.m[2][j] +
					m[i][3] * rhs.m[3][j];
			}
		}
#endif

		return r;
	}

	void	Matrix::print() const
	{
		printf(
			" %#+12.5f %#+12.5f %#+12.5f %#+12.5f\n"
			" %#+12.5f %#+12.5f %#+12.5f %#+12.5f\n"
			" %#+12.5f %#+12.5f %#+12.5f %#+12.5f\n"
			" %#+12.5f %#+12.5f %#+12.5f %#+12.5f\n",
			m[0][0], m[0][1], m[0][2], m[0][3],
			m[1][0], m[1][1], m[1][2], m[1][3],
			m[2][0], m[2][1], m[2][2], m[2][3],
			m[3][0], m[3][1], m[3][2], m[3][3]);
	}



	//=======================================================================================================================

	// misc functionality
	// Build a row-major (C-style) 4x4 matrix transform based on the parameters for gluLookAt.
	void buildViewMatrix(float eyex, float eyey, float eyez,
		float centerx, float centery, float centerz,
		float upx, float upy, float upz,
		float m[16])
	{
		float x[3], y[3], z[3];

		/* Difference eye and center vectors to make Z vector. */
		z[0] = eyex - centerx;
		z[1] = eyey - centery;
		z[2] = eyez - centerz;
		/* Normalize Z. */
		float mag = sqrt(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
		if (mag) {
			z[0] /= mag;
			z[1] /= mag;
			z[2] /= mag;
		}

		/* Up vector makes Y vector. */
		y[0] = upx;
		y[1] = upy;
		y[2] = upz;

		/* X vector = Y cross Z. */
		x[0] = y[1] * z[2] - y[2] * z[1];
		x[1] = -y[0] * z[2] + y[2] * z[0];
		x[2] = y[0] * z[1] - y[1] * z[0];

		/* Recompute Y = Z cross X. */
		y[0] = z[1] * x[2] - z[2] * x[1];
		y[1] = -z[0] * x[2] + z[2] * x[0];
		y[2] = z[0] * x[1] - z[1] * x[0];

		/* Normalize X. */
		mag = sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
		if (mag) {
			x[0] /= mag;
			x[1] /= mag;
			x[2] /= mag;
		}

		/* Normalize Y. */
		mag = sqrt(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
		if (mag) {
			y[0] /= mag;
			y[1] /= mag;
			y[2] /= mag;
		}

		/* Build resulting view matrix. */
		m[0 * 4 + 0] = x[0];  m[0 * 4 + 1] = x[1];
		m[0 * 4 + 2] = x[2];  m[0 * 4 + 3] = -x[0] * eyex + -x[1] * eyey + -x[2] * eyez;

		m[1 * 4 + 0] = y[0];  m[1 * 4 + 1] = y[1];
		m[1 * 4 + 2] = y[2];  m[1 * 4 + 3] = -y[0] * eyex + -y[1] * eyey + -y[2] * eyez;

		m[2 * 4 + 0] = z[0];  m[2 * 4 + 1] = z[1];
		m[2 * 4 + 2] = z[2];  m[2 * 4 + 3] = -z[0] * eyex + -z[1] * eyey + -z[2] * eyez;

		m[3 * 4 + 0] = 0.0;   m[3 * 4 + 1] = 0.0;  m[3 * 4 + 2] = 0.0;  m[3 * 4 + 3] = 1.0;
	}

	//=======================================================================================================================

	/* Build a row-major (C-style) 4x4 matrix transform based on the
	parameters for gluPerspective. */
	void buildProjectionMatrix(const float fieldOfView,
		const float aspectRatio,
		const float zNear, const float zFar,
		float m[16])
	{
		const float radians = (fieldOfView * 0.5f) * ((float)PI / 180.0f);

		const float deltaZ = zFar - zNear;
		const float sine = sinf(radians);
		
		KS_ASSERT(deltaZ && sine && aspectRatio);	// Should be non-zero to avoid division by zero.
		const float cotangent = cosf(radians) / sine;

		m[0 * 4 + 0] = cotangent / aspectRatio;
		m[0 * 4 + 1] = 0.0;
		m[0 * 4 + 2] = 0.0;
		m[0 * 4 + 3] = 0.0;

		m[1 * 4 + 0] = 0.0;
		m[1 * 4 + 1] = cotangent;
		m[1 * 4 + 2] = 0.0;
		m[1 * 4 + 3] = 0.0;

		m[2 * 4 + 0] = 0.0;
		m[2 * 4 + 1] = 0.0;
		m[2 * 4 + 2] = -(zFar + zNear) / deltaZ;
		m[2 * 4 + 3] = -2 * zNear * zFar / deltaZ;

		m[3 * 4 + 0] = 0.0;
		m[3 * 4 + 1] = 0.0;
		m[3 * 4 + 2] = -1;
		m[3 * 4 + 3] = 0;
	}


	//=======================================================================================================================

	/* Simple 4x4 matrix by 4x4 matrix multiply. */
	void multMatrix(float dst[16], const float src1[16], const float src2[16])
	{
		float tmp[16];

		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				tmp[i * 4 + j] = src1[i * 4 + 0] * src2[0 * 4 + j] +
					src1[i * 4 + 1] * src2[1 * 4 + j] +
					src1[i * 4 + 2] * src2[2 * 4 + j] +
					src1[i * 4 + 3] * src2[3 * 4 + j];
			}
		}
		/* Copy result to dst (so dst can also be src1 or src2). */
		for (int i = 0; i < 16; ++i)
			dst[i] = tmp[i];
	}

	//=======================================================================================================================

	/* Invert a row-major (C-style) 4x4 matrix. */
	void invertMatrix(float *out, const float *m)
	{
		/* Assumes matrices are ROW major. */
#define SWAP_ROWS(a, b) { float *_tmp = a; (a)=(b); (b)=_tmp; }
#define MAT(m,r,c) (m)[(r)*4+(c)]

		float wtmp[4][8];
		float m0, m1, m2, m3, s;
		float *r0, *r1, *r2, *r3;

		r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];

		r0[0] = MAT(m, 0, 0), r0[1] = MAT(m, 0, 1),
			r0[2] = MAT(m, 0, 2), r0[3] = MAT(m, 0, 3),
			r0[4] = 1.0, r0[5] = r0[6] = r0[7] = 0.0,

			r1[0] = MAT(m, 1, 0), r1[1] = MAT(m, 1, 1),
			r1[2] = MAT(m, 1, 2), r1[3] = MAT(m, 1, 3),
			r1[5] = 1.0, r1[4] = r1[6] = r1[7] = 0.0,

			r2[0] = MAT(m, 2, 0), r2[1] = MAT(m, 2, 1),
			r2[2] = MAT(m, 2, 2), r2[3] = MAT(m, 2, 3),
			r2[6] = 1.0, r2[4] = r2[5] = r2[7] = 0.0,

			r3[0] = MAT(m, 3, 0), r3[1] = MAT(m, 3, 1),
			r3[2] = MAT(m, 3, 2), r3[3] = MAT(m, 3, 3),
			r3[7] = 1.0, r3[4] = r3[5] = r3[6] = 0.0;

		/* Choose PI, or die. */
		if (fabs(r3[0])>fabs(r2[0])) SWAP_ROWS(r3, r2);
		if (fabs(r2[0])>fabs(r1[0])) SWAP_ROWS(r2, r1);
		if (fabs(r1[0])>fabs(r0[0])) SWAP_ROWS(r1, r0);
		if (0.0 == r0[0]) {
			KS_ASSERT(!"could not invert matrix");
		}

		/* Eliminate first variable. */
		m1 = r1[0] / r0[0]; m2 = r2[0] / r0[0]; m3 = r3[0] / r0[0];
		s = r0[1]; r1[1] -= m1 * s; r2[1] -= m2 * s; r3[1] -= m3 * s;
		s = r0[2]; r1[2] -= m1 * s; r2[2] -= m2 * s; r3[2] -= m3 * s;
		s = r0[3]; r1[3] -= m1 * s; r2[3] -= m2 * s; r3[3] -= m3 * s;
		s = r0[4];
		if (s != 0.0) { r1[4] -= m1 * s; r2[4] -= m2 * s; r3[4] -= m3 * s; }
		s = r0[5];
		if (s != 0.0) { r1[5] -= m1 * s; r2[5] -= m2 * s; r3[5] -= m3 * s; }
		s = r0[6];
		if (s != 0.0) { r1[6] -= m1 * s; r2[6] -= m2 * s; r3[6] -= m3 * s; }
		s = r0[7];
		if (s != 0.0) { r1[7] -= m1 * s; r2[7] -= m2 * s; r3[7] -= m3 * s; }

		/* Choose PI, or die. */
		if (fabs(r3[1])>fabs(r2[1])) SWAP_ROWS(r3, r2);
		if (fabs(r2[1])>fabs(r1[1])) SWAP_ROWS(r2, r1);
		if (0.0 == r1[1]) {
			KS_ASSERT(!"could not invert matrix");
		}

		/* Eliminate second variable. */
		m2 = r2[1] / r1[1]; m3 = r3[1] / r1[1];
		r2[2] -= m2 * r1[2]; r3[2] -= m3 * r1[2];
		r2[3] -= m2 * r1[3]; r3[3] -= m3 * r1[3];
		s = r1[4]; if (0.0 != s) { r2[4] -= m2 * s; r3[4] -= m3 * s; }
		s = r1[5]; if (0.0 != s) { r2[5] -= m2 * s; r3[5] -= m3 * s; }
		s = r1[6]; if (0.0 != s) { r2[6] -= m2 * s; r3[6] -= m3 * s; }
		s = r1[7]; if (0.0 != s) { r2[7] -= m2 * s; r3[7] -= m3 * s; }

		/* Choose PI, or die. */
		if (fabs(r3[2])>fabs(r2[2])) SWAP_ROWS(r3, r2);
		if (0.0 == r2[2]) {
			KS_ASSERT(!"could not invert matrix");
		}

		/* Eliminate third variable. */
		m3 = r3[2] / r2[2];
		r3[3] -= m3 * r2[3], r3[4] -= m3 * r2[4],
			r3[5] -= m3 * r2[5], r3[6] -= m3 * r2[6],
			r3[7] -= m3 * r2[7];

		/* Last check. */
		if (0.0f == r3[3]) {
			KS_ASSERT(!"could not invert matrix");
		}

		s = 1.0f / r3[3];              /* Now back substitute row 3. */
		r3[4] *= s; r3[5] *= s; r3[6] *= s; r3[7] *= s;

		m2 = r2[3];                 /* Now back substitute row 2. */
		s = 1.0f / r2[2];
		r2[4] = s * (r2[4] - r3[4] * m2), r2[5] = s * (r2[5] - r3[5] * m2),
			r2[6] = s * (r2[6] - r3[6] * m2), r2[7] = s * (r2[7] - r3[7] * m2);
		m1 = r1[3];
		r1[4] -= r3[4] * m1, r1[5] -= r3[5] * m1,
			r1[6] -= r3[6] * m1, r1[7] -= r3[7] * m1;
		m0 = r0[3];
		r0[4] -= r3[4] * m0, r0[5] -= r3[5] * m0,
			r0[6] -= r3[6] * m0, r0[7] -= r3[7] * m0;

		m1 = r1[2];                 /* Now back substitute row 1. */
		s = 1.0f / r1[1];
		r1[4] = s * (r1[4] - r2[4] * m1), r1[5] = s * (r1[5] - r2[5] * m1),
			r1[6] = s * (r1[6] - r2[6] * m1), r1[7] = s * (r1[7] - r2[7] * m1);
		m0 = r0[2];
		r0[4] -= r2[4] * m0, r0[5] -= r2[5] * m0,
			r0[6] -= r2[6] * m0, r0[7] -= r2[7] * m0;

		m0 = r0[1];                 /* Now back substitute row 0. */
		s = 1.0f / r0[0];
		r0[4] = s * (r0[4] - r1[4] * m0), r0[5] = s * (r0[5] - r1[5] * m0),
			r0[6] = s * (r0[6] - r1[6] * m0), r0[7] = s * (r0[7] - r1[7] * m0);

		MAT(out, 0, 0) = r0[4]; MAT(out, 0, 1) = r0[5],
			MAT(out, 0, 2) = r0[6]; MAT(out, 0, 3) = r0[7],
			MAT(out, 1, 0) = r1[4]; MAT(out, 1, 1) = r1[5],
			MAT(out, 1, 2) = r1[6]; MAT(out, 1, 3) = r1[7],
			MAT(out, 2, 0) = r2[4]; MAT(out, 2, 1) = r2[5],
			MAT(out, 2, 2) = r2[6]; MAT(out, 2, 3) = r2[7],
			MAT(out, 3, 0) = r3[4]; MAT(out, 3, 1) = r3[5],
			MAT(out, 3, 2) = r3[6]; MAT(out, 3, 3) = r3[7];

#undef MAT
#undef SWAP_ROWS
	}

	//=======================================================================================================================


	/* Build a row-major (C-style) 4x4 matrix transform based on the
	parameters for glRotatef. */
	void makeRotateMatrix(float angle,
		float ax, float ay, float az,
		float m[16])
	{
		float axis[3];
		float mag;

		axis[0] = ax;
		axis[1] = ay;
		axis[2] = az;
		mag = sqrt(axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2]);
		if (mag) {
			axis[0] /= mag;
			axis[1] /= mag;
			axis[2] /= mag;
		}

		const float radians = angle * (float)PI / 180.0f;
		const float sine = sinf(radians);
		const float cosine = cosf(radians);
		const float ab = axis[0] * axis[1] * (1 - cosine);
		const float bc = axis[1] * axis[2] * (1 - cosine);
		const float ca = axis[2] * axis[0] * (1 - cosine);
		const float tx = axis[0] * axis[0];
		const float ty = axis[1] * axis[1];
		const float tz = axis[2] * axis[2];

		m[0] = tx + cosine * (1 - tx);
		m[1] = ab + axis[2] * sine;
		m[2] = ca - axis[1] * sine;
		m[3] = 0.0f;
		m[4] = ab - axis[2] * sine;
		m[5] = ty + cosine * (1 - ty);
		m[6] = bc + axis[0] * sine;
		m[7] = 0.0f;
		m[8] = ca + axis[1] * sine;
		m[9] = bc - axis[0] * sine;
		m[10] = tz + cosine * (1 - tz);
		m[11] = 0;
		m[12] = 0;
		m[13] = 0;
		m[14] = 0;
		m[15] = 1;
	}

	//=======================================================================================================================

	/* Build a row-major (C-style) 4x4 matrix transform based on the
	parameters for glTranslatef. */
	void makeTranslateMatrix(float x, float y, float z, float m[16])
	{
		m[0] = 1;  m[1] = 0;  m[2] = 0;  m[3] = x;
		m[4] = 0;  m[5] = 1;  m[6] = 0;  m[7] = y;
		m[8] = 0;  m[9] = 0;  m[10] = 1; m[11] = z;
		m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;
	}

	//=======================================================================================================================

	/* Simple 4x4 matrix by 4-component column vector multiply. */
	void transform(float dst[4], const float mat[16], const float vec[4])
	{
		float tmp[4];

		for (int i = 0; i<4; i++)
		{
			tmp[i] = mat[i * 4 + 0] * vec[0] +
				mat[i * 4 + 1] * vec[1] +
				mat[i * 4 + 2] * vec[2] +
				mat[i * 4 + 3] * vec[3];
		}
		float invW = 1 / tmp[3];
		
		/* Apply perspective divide and copy to dst (so dst can vec). */
		for (int i = 0; i<3; i++)
			dst[i] = tmp[i] * tmp[3];
		dst[3] = 1;
	}

	static void transform(float dst[4], const float mat[16], vec3 vec)
	{
		float vecf[4] = { vec.x, vec.y, vec.z, 1.0 };
		transform(dst, mat, vecf);
	}


}	// namespace ks
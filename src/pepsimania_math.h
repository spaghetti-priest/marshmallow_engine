#ifndef _PEPSIMANIA_MATH_H

/*************************************************************************
* PEPSIMANIA MATH: Version 0.00.1
* @Note: This file is not a very suitable math library since most of the functions
* are not battle tested. So it is probably better to use glm or another math library
* when possible
* ***********************************************************************/

// @Cleanup: Might export all of the functionality (e.g math functions) into a .c file perhaps. Should also battle test this file
// @Cleanup: Might export all of the functionality (e.g math functions) into a .c file perhaps. Should also battle test this file
// @Cleanup: Might export all of the functionality (e.g math functions) into a .c file perhaps. Should also battle test this file

#include <float.h>
#include <stdio.h>
#include <iostream>

#define MIN(a, b) 		(a < b ? a : b)
#define MAX(a, b) 		(a > b ? a : b)
#define MIN3(a, b, c) 	(MIN(MIN(a, b), c))
#define MAX3(a, b, c) 	(MAX(MAX(a, b), c))
#define SQUARED(a) 		(a * a)
#define CUBED(a) 			(a * a * a)

/*************************************************************************
*
* VECTOR 2 DEFINITIONS AND FUNCTIONS
*
* ***********************************************************************/
typedef union V2S V2S;
union V2S {
	struct
	{
		s32 x, y;
	};
	s32 e[2];
};

typedef union V2U V2U;
union V2U {
	struct
	{
		u32 x, y;
	};

	u32 e[2];
};

typedef union V2 V2;
union V2 {
	struct
	{
		f32 x, y;
	};
	f32 e[2];
	inline float operator[](u32 i) {
		return e[i];
	}
};

// @Note: That Eskril steenberg video did cause me to reflect and might possibly rewrite the math functionality in more
// explicit terms rather than operator overloading. But operator overloading is useful. Test out in test code.
// If I write the function code in .c it will remove operator overloading anyways so \_(-_-)_/
// Integer v2
inline V2S v2s(int x, int y)				{V2S r = {x, y}; 						return r;}
inline V2S operator+(V2S a, V2S b) 		{V2S r = {a.x + b.x, a.y + b.y}; return r;}
inline V2S operator-(V2S a, V2S b) 		{V2S r = {a.x - b.x, a.y - b.y}; return r;}
inline V2S operator*(V2S a, V2S b) 		{V2S r = {a.x * b.x, a.y * b.y}; return r;}
inline V2S operator*(V2S a, s32 b) 		{V2S r = {a.x * b, a.y * b}; 	 	return r;}
inline V2S operator*(s32 a, V2S b) 		{V2S r = {a * b.x, a * b.y}; 	 	return r;}
inline V2S operator/(V2S a, s32 b) 		{V2S r = (1 / b) * a;				return r;}
inline V2S operator/(s32 a, V2S b) 		{V2S r = {a / b.x, a / b.y}; 		return r;}
inline V2S &operator*=(V2S a, V2S b) 	{a = a * b; 							return a;}
inline V2S &operator/=(V2S &a, s32 b) 	{a = a / b; 							return a;}
inline s32 dot(V2S a, V2S b)				{s32 r = a.x * b.x + a.y * b.y; 	return r;}
inline bool operator==(V2S a, V2S b) 	{bool r = ((a.x == b.x) && (a.y == b.y)); return r;}

// Float v2
inline V2 v2(float x, float y) 		{V2 r = {x, y};						return r;}
inline V2 operator+(V2 a, V2 b) 		{V2 r = {a.x + b.x, a.y + b.y}; 	return r;}
inline V2 operator-(V2 a, V2 b) 		{V2 r = {a.x - b.x, a.y - b.y};	return r;}
inline V2 operator*(V2 a, V2 b) 		{V2 r = {a.x * b.x, a.y * b.y}; 	return r;}
inline V2 operator*(V2 a, f32 b) 	{V2 r = {a.x * b, a.y * b}; 		return r;}
inline V2 operator*(f32 a, V2 b) 	{V2 r = {a * b.x, a * b.y};		return r;}
inline V2 operator/(V2 a, f32 b) 	{V2 r = (1.0f/b)*a; 					return r;}
inline V2 operator/(f32 a, V2 b) 	{V2 r = {a / b.x, a / b.y}; 		return r;}
inline V2 operator*=(V2 a, f32 b) 	{a.x *= b; a.y *= b;					return a;}
inline V2 &operator*=(V2 a, V2 b) 	{a = a * b; 							return a;}
inline V2 &operator/=(V2 &a, f32 b) {a = a / b; 							return a;}
inline f32 dot(V2 a, V2 b)				{f32 r = a.x * b.x + a.y * b.y; 	return r;}

/*************************************************************************
*
* VECTOR 3 DEFINITIONS AND FUNCTIONS
*
* ***********************************************************************/
typedef union V3 V3;
union V3 {
	struct
	{
		f32 x, y, z;
	};
	struct
	{
		f32 r, g, b;
	};
	float e[3];

	inline float operator[](u32 i) {
		return e[i];
	}
};

inline V3 v3(f32 a, f32 b, f32 c) {
	V3 r = {};
	r.x = a;
	r.y = b;
	r.z = c;
	return r;
}

inline V3 operator+(V3 a, V3 b) 	   {V3 r = {a.x + b.x, a.y + b.y, a.z + b.z}; 													return r;}
inline V3 operator*(f32 a, V3 b) 	{V3 r = {a * b.x, a * b.y, a * b.z}; 														return r;}
inline V3 operator*(V3 a, f32 b) 	{V3 r = {a.x * b, a.y * b, a.z * b}; 														return r;} //@Note: not sure if this is legal
inline V3 operator/(V3 a, f32 b) 	{V3 r = {a.x / b, a.y / b, a.z / b}; 														return r;}
inline V3 operator/=(V3 a, f32 b) 	{V3 r = {a.x / b, a.y / b, a.z / b};														return r;}
inline V3 operator-(V3 a, V3 b) 	   {V3 r = {a.x - b.x, a.y - b.y, a.z - b.z};													return r;}
inline f32 length(V3 a) 			   {f32 r = sqrt(abs((a.x * a.x) + (a.y * a.y) + (a.z * a.z))); 								        return r;}
inline f32 dot(V3 a, V3 b) 		   {f32 r = ((a.x * b.x) + (a.y * b.y) + (a.z * b.z)); 										           return r;}
inline V3 cross(V3 a, V3 b) 		   {V3 r = {(a.y * b.z) - (a.z * b.y), (a.z * b.x) - (a.x * b.z), (a.x * b.y) - (a.y * b.x)};  return r;}
inline V3 normalize(V3 a) 			   {V3 r = a / length(a); 																		return r;}

inline V3 debug_normalize(V3 a) {
	float l = length(a);
	V3 result = a / l;
	return result;
}

inline V3
get_unit ()
{
	return v3(0,0,0);
}

#if 0
inline v3 operator/(v3 a, f32 b) {
	v3 r = (1.0f / b) * a;

	return r;
}
#endif
/*// Homogenous 2 Cartesian Coordinates
inline v3 perspective_divide(v3 a) {
	float w = 1;
	a.x /= w;
	a.y /= w;
	a.z /= w;
	return a;
}*/


typedef union V3S V3S;
union V3S {
	struct
	{
		s32 x, y, z;
	};
	s32 e[3];
};

inline V3S v3s(s32 x, s32 y, s32 z) {V3S r = {x, y, z,}; return r;}
/*************************************************************************
*
* VECTOR 4 DEFINITIONS AND FUNCTIONS
*
* ***********************************************************************/
typedef union V4 V4;
union V4 {
	struct
	{
		f32 x, y, z, w;
	};
	struct
	{
		f32 r, g, b, a;
	};

	f32 e[4];
};

inline V4 v4(float x, float y, float z, float w)
{
	V4 r;
	r.x = x;
	r.y = y;
	r.z = z;
	r.w = w;

	return r;
}
inline V4 operator+(V4 a, V4 b) 	   {V4 r = {a.x + b.x, a.y + b.y,	a.z + b.z, 	a.w + b.w}; return r;}
inline V4 operator-(V4 a, V4 b) 	   {V4 r = {a.x - b.x, a.y - b.y,	a.z - b.z, 	a.w - b.w}; return r;}
inline V4 operator*(V4 a, V4 b) 	   {V4 r = {a.x * b.x, a.y * b.y, 	a.z * b.z, 	a.w * b.w};	return r;}
inline V4 operator*(V4 a, f32 b) 	{V4 r = {a.x * b, 	a.y * b, 	a.z * b, 	a.w * b};	return r;}
inline V4 operator*(f32 a, V4 b) 	{V4 r = {a * b.x, 	a * b.y, 	a * b.z, 	a * b.w}; 	return r;}
inline V4 operator/(f32 a, V4 b) 	{V4 r = {a / b.x, 	a / b.y, 	a / b.z, 	a / b.w}; 	return r;}
inline V4 operator*=(V4 a, f32 b) 			{a.x *= b; 	a.y *= b; 	a.z *= b; 	a.w *= b; 		return a;}
inline V4 &operator*=(V4 a, V4 b) 			{a = a * b;												      return a;}
inline f32 dot(V4 a, V4 b)			   {f32 r = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;	return r;}


// @@Optimize: @@Speed: This format is in row major order meaning that this is not as
// cache friendly since the computer acesses a 'nearby' value that is not nearby in memory
// I am not taking performance into account here, so this needs to be remedied in the future
// during performance critical parts
union mat4 {
	struct
	{
		f32 m11, m12, m13, m14,
			m21, m22, m23, m24,
			m31, m32, m33, m34,
			m41, m42, m43, m44;
	};

	struct
	{
		V4 a, b, c, d;
		//_m128 m1, m2, m3, m4;
	};

	float m[4][4];
	float *operator[] (u32 i) { return m[i]; };
};

/*************************************************************************
*
* MATRIX 4 DEFINITIONS AND FUNCTIONS
*
* ***********************************************************************/
inline mat4
MAT4 (V3 a, V3 b, V3 c)
{
	mat4 r;
	for (int i = 0; i < 2; ++i) {
		r.m[0][i] = a[i];
		r.m[1][i] = b[i];
		r.m[2][i] = c[i];
		r.m[3][i] = 0.0;
	}
	return r;
}

inline mat4
MAT4(float m11, float m12, float m13, float m14,
	 float m21, float m22, float m23, float m24,
	 float m31, float m32, float m33, float m34,
	 float m41, float m42, float m43, float m44)
{
	mat4 r;
	r.m[0][0] = m11; r.m[0][1] = m12; r.m[0][2] = m13; r.m[0][3] = m14;
	r.m[1][0] = m21; r.m[1][1] = m22; r.m[1][2] = m23; r.m[1][3] = m24;
	r.m[2][0] = m31; r.m[2][1] = m32; r.m[2][2] = m33; r.m[2][3] = m34;
	r.m[3][0] = m41; r.m[3][1] = m42; r.m[3][2] = m43; r.m[3][3] = m44;

	return r;
}

//@Copypasta: Unroll this loop
inline
mat4 operator* (mat4 a, mat4 b) {
	mat4 r;

	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 4; j++) {
			for(int k = 0; k < 4; k++) {
				r.m[i][j] += a.m[i][k] * b.m[k][j];
			}
		}
	}

	return r;
}

inline mat4
get_identity ()
{
	mat4 r;
	r.m[0][0] = 1; r.m[0][1] = 0; r.m[0][2] = 0; r.m[0][3] = 0;
	r.m[1][0] = 0; r.m[1][1] = 1; r.m[1][2] = 0; r.m[1][3] = 0;
	r.m[2][0] = 0; r.m[2][1] = 0; r.m[2][2] = 1; r.m[2][3] = 0;
	r.m[3][0] = 0; r.m[3][1] = 0; r.m[3][2] = 0; r.m[3][3] = 1;

	return r;
}

// Column Major Order
/*
	1 0 0 0
	0 1 0 0
	0 0 1 0
	x y z w
*/
// Row Major Order
/*
	1 0 0 x
	0 1 0 y
	0 0 1 z
	0 0 0 w
*/
inline mat4
column_to_row_major(mat4 _in)
{
	mat4 _out;
	_out[3][0] = _in[0][3]; _out[3][1] = _in[1][3]; _out[3][2] = _in[2][3];
}

inline V4
mat_mul_point (V4 a, mat4 b)
{
	V4 r;
	// Column major order mult
#if 0
	r.x = (a.x * b.m[0][0]) + (a.y * b.m[1][0]) + (a.z * b.m[2][0]) + b.m[3][0];
	r.y = (a.x * b.m[0][1]) + (a.y * b.m[1][1]) + (a.z * b.m[2][1]) + b.m[3][1];
	r.z = (a.x * b.m[0][2]) + (a.y * b.m[1][2]) + (a.z * b.m[2][2]) + b.m[3][2];
	r.w = (a.x * b.m[0][3]) + (a.y * b.m[1][3]) + (a.z * b.m[2][3]) + b.m[3][3];
#endif
	r.x = (a.x * b.m[0][0]) + (a.y * b.m[0][1]) + (a.z * b.m[0][2]) + b.m[0][3];
	r.y = (a.x * b.m[1][0]) + (a.y * b.m[1][1]) + (a.z * b.m[1][2]) + b.m[1][3];
	r.z = (a.x * b.m[2][0]) + (a.y * b.m[2][1]) + (a.z * b.m[2][2]) + b.m[2][3];
	r.w = (a.x * b.m[3][0]) + (a.y * b.m[3][1]) + (a.z * b.m[3][2]) + b.m[3][3];

	return r;;
}

//inline V4
//mat_mul_vec (mat4 m, V4 v)
//{
//}

inline V4
mat_2_vec (mat4 a)
{
	V4 r;
	r = {a.m[0][0] / a.m[3][0], a.m[1][0] / a.m[3][0], a.m[2][0] / a.m[3][0], a.m[3][0]};
	return r;
}

inline mat4
vec_2_mat (V3 a)
{
	mat4 r;
	r.m[0][0] = a.x;
	r.m[1][0] = a.y;
	r.m[2][0] = a.z;
	r.m[3][0] = 1;

	return r;
}

inline mat4
transform (float x, float y, float z, float w = 1.0)
{
	// Column Major Order
	/*
		1 0 0 0
		0 1 0 0
		0 0 1 0
		x y z w
	*/
	mat4 r;
	r = get_identity();
#if 0
	r.m[3][0] = x;
	r.m[3][1] = y;
	r.m[3][2] = z;
	r.m[3][3] = w;
#endif
	// Row Major Order
	/*
		1 0 0 x
		0 1 0 y
		0 0 1 z
		0 0 0 w
	*/
	r.m[0][0] = x;
	r.m[0][1] = y;
	r.m[0][2] = z;
	r.m[0][3] = w;

	return r;
}

inline mat4
rotation(float x, float y, float z, float w = 1.0)
{
	mat4 r;

	float a = sin(x), b = cos(x);
	float c = sin(y), d = cos(y);
	float e = sin(z), f = cos(z);

	float ac = a*c;
	float bc = b*c;

	//@@Speed; This is in row major order
	r.m[0][0] = d * f;
	r.m[0][1] = ac + f - b * e;
	r.m[0][2] = bc + f + a * e;
	r.m[0][3] = 0.0;
	r.m[1][0] = d * e;
	r.m[1][1] = ac * e + b * f;
	r.m[1][2] = bc * e - a * f;
	r.m[1][3] = 0.0;
	r.m[2][0] = -c;
	r.m[2][1] = a * d;
	r.m[2][2] = b * d;
	r.m[2][3] = 0.0;
	r.m[3][0] = 0;
	r.m[3][1] = 0;
	r.m[3][2] = 0;
	r.m[3][3] = 1;

	return r;
}

inline mat4
orthographic(float left, float right, float bottom, float top)
{
	mat4 r = get_identity();
	// float w =
}
/*************************************************************************
*
* EXTRANAEOUS DEFINITIONS AND FUNCTIONS
*
* ***********************************************************************/
typedef struct RectF32 {
   V2 min;
   V2 max;
} RectF32;

typedef struct RectS32 {
   V2S min;
   V2S max;
} RectS32;

inline s32
round_f32_to_s32 (f32 value)
{
    // s32 result = _mm_cvtss_si32(_mm_set_ss(value));
    s32 result = (s32)value;
    return result ;
}

inline u32
round_f32_to_u32 (f32 value)
{
    // u32 result = (u32)_mm_cvtss_si32(_mm_set_ss(value));
    u32 result = (u32)value;
    return result ;
}

inline s32
convert_s32_to_u32 (s32 value)
{
	u32 r = (u32)value;
	return r;
}

inline float
float_from_uchar(unsigned char v)
{
	return v / 255.0f;
}

inline u32
pack_BGRA (f32 red, f32 green, f32 blue, f32 alpha)
{
	u32 a = round_f32_to_u32(alpha * 255.0f) << 24;
	u32 r = round_f32_to_u32(red   * 255.0f) << 16;
	u32 g = round_f32_to_u32(green * 255.0f) << 8;
	u32 b = round_f32_to_u32(blue  * 255.0f);

	return (b | g | r | a);
   // return (r | g | b | a);
}

inline V4
unpack_BGRA (u32 pack)
{
	V4 result = v4((f32)((pack >> 24) & 0xFF), // alpha)
	 					(f32)((pack >> 16) & 0xFF), // red)
	 					(f32)((pack >> 8)  & 0xFF), // green)
	 					(f32)((pack >> 0)  & 0xFF) ); // blue)
	return result;
}

inline u32
manhattan_distance (int x0, int y0, int x1, int y1, int scale = 1)
{
   s32 mx = abs(x0 - x1);
   s32 my = abs(y0 - y1);
   return scale * (mx + my);
}

inline u32
euclidean_distance (int x0, int y0, int x1, int y1, int scale = 1)
{
   s32 ex = (s32)pow((x1 - x0), 2);
   s32 ey = (s32)pow((y1 - y0), 2);
   return scale * sqrt(ex + ey);
}

// @Todo: Could create a struct that stores the x and y dir aswell as the total distance
inline u32
chebyshev_distance (V2S p0, V2S p1, int scale = 1)
{
   // @Note: D2 can be changed into sqrt(2) maybe we should set that to be the octile distance?
   s32 D1 = scale;
   s32 D2 = scale;
   s32 cx = abs(p0.x - p1.x);
   s32 cy = abs(p0.y - p1.y);
   return D1 * (cx + cy) + (D2 - 2 * D1) * MIN(cx, cy);
}

inline s32
clamp(s32 x, s32 min, s32 max)
{
	if (x < min)
		x = min;
	if (x > max)
		x = max;
	return x;
}

#define _PEPSIMANIA_MATH_H
#endif

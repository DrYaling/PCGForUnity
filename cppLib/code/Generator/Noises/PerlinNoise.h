/* coherent noise function over 1, 2 or 3 dimensions */
/* (copyright Ken Perlin) */
#ifndef _PERLIN_NOISE_H
#define _PERLIN_NOISE_H
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "define.h"
typedef float plfloat;
namespace PerlinNoise {
	static inline int random()
	{
		return std::rand();
	}
	static int   perm[512] =
	{
		151,160,137,91,90,15,
		131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
		190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
		88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
		77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
		102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
		135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
		5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
		223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
		129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
		251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
		49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
		138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
		151,160,137,91,90,15,
		131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
		190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
		88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
		77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
		102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
		135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
		5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
		223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
		129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
		251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
		49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
		138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
	};
	inline plfloat fade(plfloat t)
	{
		return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
	}

	inline float lerp(plfloat t, plfloat a, plfloat b)
	{
		return a + t * (b - a);
	}

	inline float grad(int hash, plfloat x, plfloat y, plfloat z)
	{
		//float u = (h < 8) ? x : y;
		//float v = (h < 4) ? y : ((h == 12 || h == 14) ? x : z);
		//return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);

		switch (hash & 0xF)
		{
		case 0x0: return  x + y;
		case 0x1: return -x + y;
		case 0x2: return  x - y;
		case 0x3: return -x - y;
		case 0x4: return  x + x;
		case 0x5: return -x + x;
		case 0x6: return  x - x;
		case 0x7: return -x - x;
		case 0x8: return  y + x;
		case 0x9: return -y + x;
		case 0xA: return  y - x;
		case 0xB: return -y - x;
		case 0xC: return  y + z;
		case 0xD: return -y + x;
		case 0xE: return  y - x;
		case 0xF: return -y - z;
		default: return 0; // never happens
		}
	}
	inline static float grad2(int hash, float x, float y) {
		return grad(hash, x, y, 0);
	}
	static inline float __fastcall noise(plfloat x, plfloat y, plfloat z)
	{
		int ix, iy, iz, gx, gy, gz;
		int a0, b0, aa, ab, ba, bb;
		plfloat aa0, ab0, ba0, bb0;
		plfloat aa1, ab1, ba1, bb1;
		plfloat a1, a2, a3, a4, a5, a6, a7, a8;
		plfloat u, v, w, a8_5, a4_1;

		ix = (int)x; x -= ix;
		iy = (int)y; y -= iy;
		iz = (int)z; z -= iz;

		gx = ix & 0xFF;
		gy = iy & 0xFF;
		gz = iz & 0xFF;

		a0 = gy + perm[gx];
		b0 = gy + perm[gx + 1];
		aa = gz + perm[a0];
		ab = gz + perm[a0 + 1];
		ba = gz + perm[b0];
		bb = gz + perm[b0 + 1];

		aa0 = perm[aa]; aa1 = perm[aa + 1];
		ab0 = perm[ab]; ab1 = perm[ab + 1];
		ba0 = perm[ba]; ba1 = perm[ba + 1];
		bb0 = perm[bb]; bb1 = perm[bb + 1];

		a1 = grad(bb1, x - 1, y - 1, z - 1);
		a2 = grad(ab1, x, y - 1, z - 1);
		a3 = grad(ba1, x - 1, y, z - 1);
		a4 = grad(aa1, x, y, z - 1);
		a5 = grad(bb0, x - 1, y - 1, z);
		a6 = grad(ab0, x, y - 1, z);
		a7 = grad(ba0, x - 1, y, z);
		a8 = grad(aa0, x, y, z);

		u = fade(x);
		v = fade(y);
		w = fade(z);

		a8_5 = lerp(v, lerp(u, a8, a7), lerp(u, a6, a5));
		a4_1 = lerp(v, lerp(u, a4, a3), lerp(u, a2, a1));
		return lerp(w, a8_5, a4_1);
	}
#define Abs(x) ((x>0)?x:-x)
	static inline float __fastcall noise(plfloat x, plfloat y)
	{
		x = Abs(x);
		y = Abs(y);

		int floorX = int(x);
		int floorY = int(y);

		int X = floorX & 255;                  // FIND UNIT CUBE THAT
		int Y = floorY & 255;                  // CONTAINS POINT.
		x -= floorX;                                // FIND RELATIVE X,Y,Z
		y -= floorY;                                // OF POINT IN CUBE.
		float u = fade(min(x, 1.0f));          // COMPUTE FADE CURVES
		float v = fade(min(y, 1.0f));          // FOR EACH OF X,Y,Z.
		int A = perm[X] + Y, AA = perm[A], AB = perm[A + 1],      // HASH COORDINATES OF
			B = perm[X + 1] + Y, BA = perm[B], BB = perm[B + 1];      // THE 8 CUBE CORNERS,

		float res = lerp(v, lerp(u, grad2(perm[AA], x, y),  // AND ADD
			grad2(perm[BA], x - 1, y)), // BLENDED
			lerp(u, grad2(perm[AB], x, y - 1),  // RESULTS
				grad2(perm[BB], x - 1, y - 1)));// FROM  8
		return res;
	}
	// Returns noise between 0 - 1
	inline static float NoiseNormalized(float x, float y, float z)
	{
		//-0.697 - 0.795 + 0.697
		float value = noise(x, y, z);
		value = (value + 0.69F) / (0.793F + 0.69F);
		return value;
	}

	// Returns noise between 0 - 1
	inline static float NoiseNormalized(float x, float y)
	{
		//-0.697 - 0.795 + 0.697
		float value = noise(x, y);
		value = (value + 0.69F) / (0.793F + 0.69F);
		return value;
	}
}
#endif
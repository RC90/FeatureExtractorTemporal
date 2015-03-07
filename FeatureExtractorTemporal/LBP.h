///////////////////////////////////////////////////////////////////////////////
// Author:   Jia WU
// Date:     Dec. 2010
// Description:  Calculate Histogram of LBP(local binary patterns) or LBP-TOP
//               (local binary patterns on three orthogonal planes)
//
// Copyright (C) 2009-2010 OpenPR
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of OpenPR nor the names of its 
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL HOLDER AND CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///////////////////////////////////////////////////////////////////////////////

#ifndef LBP_H
#define LBP_H

#include <vector>
#include <map>

using namespace std;

typedef unsigned char uchar;
typedef unsigned int uint;

typedef struct Number
{
	int xy;
	int xt;
	int yt;
}
Number;  //numbers of sample points

typedef struct Radius
{
	int xR;
	int yR;
	int tR;
}
Radius;  //sampling radii

typedef struct Hist
{
	float* pHist_xy;
	float* pHist_xt;
	float* pHist_yt;
}
Hist;   //LBP based histograms

typedef struct Code
{
	uint xy;  //x-y plane; previous frame
	uint xt;  //x-t plane; current frame
	uint yt;  //y-t plane; post frame
}
Code;  //values of LBP of a pixel

typedef struct Point3D
{
	int x;
	int y;
	int t;
}
Point3D;  //3d point

typedef struct Point3D32f
{
	float x;
	float y;
	float t;
}
Point3D32f;  //3d float point

//#define LBP     0
//#define LBP_TOP 1
//#define RIVLBP  2

class LBP
{
public:
	LBP();
	//LBP(struct Radius r, struct Number p, bool u, bool ip, bool n);
	~LBP();
	
	void CreateHistogram(uchar*** pppArray, int algorithm, int type=0);
	
private:
	struct Code LBPCode(uchar*** pppArray, int algorithm, int x, int y, int t);
	
	uint CalcRotationInvariant(uint lbp_code, int number);
	map<uint,uint> CalcRIMap(int number);
	uint CalcUniform(uint val, int number);	
	void RIVLBP_Old(int number, map<uint,uint>& rivlbp, map<uint,uint>& ri_pattern);
	void RIVLBP_New(int number, map<uint,uint>& rivlbp, map<uint,uint>& ri_pattern);
	void RIVLBP(int number, int type, map<uint,uint>& rivlbp, map<uint,uint>& ri_pattern);
	
	//for LBP and TOP-LBP 
	vector<uint> uniform_pattern(int number);	
	void UniformHistogram();    
	int CalcUniHist(float** pphist1, float* phist2, int num, int r1, int r2, int len1, int len2, int seq_num);
	
	void Normalization();
	void CalcNormHist(float** pphist, int bin);
	
	void Release();
	
public:    
	int width;     //width of the input image(s)
	int height;    //height of the input image(s)
	int tlength;   //number of the input images

	struct Radius R;	     //the sampling radii
	struct Number SN;        //the number of the sample points
	
	bool uni;        //1 if only using the uniform patterns; 0 if not
	bool interp;     //1 if using bilinear interpolation; 0 if not
	bool norm;       //1 if normalized histogram is calculated; 0 if not
	
	struct Number basic_bin;	//number of the basic patterns
	struct Number uni_bin;      //number of uniform patterns
	struct Hist basic_hist;     
	struct Hist uni_hist;
	
	float* pHist_ri;        //(uniform)RIVLBP based histogram
	int ri_bin;             //number of the (uniform)RIVLBP patterns
};

#endif  

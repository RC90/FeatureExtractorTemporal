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


#include <cstring>
#include <cmath>
#include <algorithm>
#include <iostream>

#include "LBP.h"

using namespace std;

#define PI 3.1415926535897932

#define	BOUND(x, lowerbound, upperbound)  { (x) = (x) > (lowerbound) ? (x) : (lowerbound); \
                                            (x) = (x) < (upperbound) ? (x) : (upperbound); };                                          

/*********************constructor****************************/
/************************************************************/
LBP::LBP()
{
	R.xR = 0;
	R.yR = 0;
	R.tR = 0;
	
	SN.xy = 0;
	SN.xt = 0;
	SN.yt = 0;
	
	uni = 0;
	interp = 0;
	norm = 0;
	
	basic_bin.xy = 0;
	basic_bin.xt = 0;
	basic_bin.yt = 0;	
	
	uni_bin.xy = 0;
	uni_bin.xt = 0;
	uni_bin.yt = 0;
	
	basic_hist.pHist_xy = NULL;
	basic_hist.pHist_xt = NULL;
	basic_hist.pHist_yt = NULL;
	
	uni_hist.pHist_xy = NULL;
	uni_hist.pHist_xt = NULL;
	uni_hist.pHist_yt = NULL;	
	
	pHist_ri = NULL;
	ri_bin = 0;
}

/*********************constructor****************************/
/* r:  the radii of the circle
/* p:  the numbers of the sample points
/* u:  1 if only using the uniform patterns; 0 if not
/* ip: 1 if using bilinear interpolation; 0 if not
/* n:  1 if normalized histogram is calculated; 0 if not
/************************************************************/
/*LBP::LBP(struct Radius r, struct Number p, bool u, bool ip, bool n)
{
	R = r;
	SN = p;
	
	uni = u;
	interp = ip;
	norm = n;
	
	basic_bin.xy = 0;
	basic_bin.xt = 0;
	basic_bin.yt = 0;	
	
	uni_bin.xy = 0;
	uni_bin.xt = 0;
	uni_bin.yt = 0;
	
	basic_hist.pHist_xy = NULL;
	basic_hist.pHist_xt = NULL;
	basic_hist.pHist_yt = NULL;
	
	uni_hist.pHist_xy = NULL;
	uni_hist.pHist_xt = NULL;
	uni_hist.pHist_yt = NULL;	
	
	pHist_ri = NULL;
	ri_bin = 0;
}*/

/*******************deconstructor****************************/
/************************************************************/
LBP::~LBP()
{
	Release();
}

/****calculate the LBP based histogram of the input image****/
/* pppArray:   the input data of image(s)
/* algorithm:  0 for histogram of LBP
/*             1 for histograms of LBP-TOP
/*             2 for histogram of RIVLBP
/* type:       0(default) for old rotation invariant descriptor published in ECCV workshop 2006
/*             1 for new rotation invariant descriptor published in PAMI 2007
/************************************************************/
void LBP::CreateHistogram(uchar*** pppArray, int algorithm, int type)
{	
	if(!pppArray)
	{
		cout<<"No input data!"<<endl;
		return;
	}

	int bin;
	int x, y, t;
	struct Code res_code ={0,0,0};	
	
	if(algorithm == 0) //LBP based histogram
	{
		//check size
		if(width<=2*R.xR || height<=2*R.yR)
		{
			cout<<"Can not calculate the histogram. Parameters don't match."<<endl;
			return;
		}	
		
		//clear 
		Release();
		
		//allocate memory
		basic_bin.xy = (int)pow(2.0f, SN.xy);
		bin = basic_bin.xy;
		basic_hist.pHist_xy = new float[bin];	memset(basic_hist.pHist_xy, 0, 4*bin);	
		
		//calculate the histogram
		t = 0;
		for(y=R.yR; y<height-R.yR; y++)
		{
			for(x=R.xR; x<width-R.xR; x++)
			{
				res_code = LBPCode(pppArray, algorithm, x, y, t);
				basic_hist.pHist_xy[res_code.xy]++;
			}
		}		
	}
	
	if(algorithm == 1)  //LBP-TOP based histogram
	{
		//check size
		if(width<=2*R.xR || height<=2*R.yR || tlength<=2*R.tR)
		{
			cout<<"Can not calculate the histograms. Parameters don't match."<<endl;
			return;
		}

		//clear 
		Release();
	
		//allocate memory
		basic_bin.xy = (int)pow(2.0f, SN.xy);
		bin = basic_bin.xy;		
		basic_hist.pHist_xy = new float[bin];	memset(basic_hist.pHist_xy, 0, 4*bin);
		basic_bin.xt = (int)pow(2.0f, SN.xt);
		bin = basic_bin.xt;	
		basic_hist.pHist_xt = new float[bin];  memset(basic_hist.pHist_xt, 0, 4*bin);
		basic_bin.yt = (int)pow(2.0f, SN.yt);
		bin = basic_bin.yt;		
		basic_hist.pHist_yt = new float[bin];	memset(basic_hist.pHist_yt, 0, 4*bin);
		
		//calculate the histograms
		for(t=R.tR; t<tlength-R.tR; t++)
		{
			for(y=R.yR; y<height-R.yR; y++)
			{
				for(x=R.xR; x<width-R.xR; x++)
				{				
					res_code = LBPCode(pppArray, algorithm, x, y, t);
					
					basic_hist.pHist_xy[res_code.xy]++;
					basic_hist.pHist_xt[res_code.xt]++;			
					basic_hist.pHist_yt[res_code.yt]++;
				}
			}
		}		
	}
	
	if(algorithm == 2)  //(uniform)RIVLBP based histogram
	{
		//check size
		if(width<=2*R.xR || height<=2*R.xR || tlength<=2*R.tR)
		{
			cout<<"Can not calculate the histograms. Parameters don't match."<<endl;
			return;
		}
		
		//clear 
		Release();				
		
		int i;
		
		map<uint,uint> rivlbp;				//vlbp - (uniform)rivlbp map
		map<uint,uint> ri_pattern;		    //(uniform)rotation invariant VLBP
		
		map<uint,uint>::iterator it;
		map<uint,uint>::iterator ij;
		
		int number = SN.xy;                 //number of sample points
		uint code = 0;                      //basic VLBP value
		
		//basic_bin.xy = pow(2, 3*number+2);
		//basic_hist.pHist_xy = new float[basic_bin.xy];  memset(basic_hist.pHist_xy, 0, 4*basic_bin.xy);		
	
		RIVLBP(number, type, rivlbp, ri_pattern);
		
		ri_bin = ri_pattern.size();
		pHist_ri = new float[ri_bin];  memset(pHist_ri, 0, 4*ri_bin);	
		
		for(t=R.tR; t<tlength-R.tR; t++)
		{
			for(y=R.xR; y<height-R.xR; y++)
			{
				for(x=R.xR; x<width-R.xR; x++)
				{
					uint preC, preN, curN, posN, posC;
					
					preC = (pppArray[t-R.tR][y][x]>=pppArray[t][y][x])?1:0;
					posC = (pppArray[t+R.tR][y][x]>=pppArray[t][y][x])?1:0;
					
					//calculate the preN, curN, posN
					res_code = LBPCode(pppArray, algorithm, x, y, t);	
					
					preN = res_code.xy;
					curN = res_code.xt;
					posN = res_code.yt;
					
					code = preC+
						   (preN<<1)+
						   (curN<<(number+1))+
						   (posN<<(2*number+1))+
						   (posC<<(3*number+1));		
						   
					//basic_hist.pHist_xy[(int)code]++;
						   
					it = rivlbp.find(code);
					ij = ri_pattern.find(it->second);
					ij->second += 1; 												
				}
			}
		}
		
		for(it=ri_pattern.begin(),i=0; it!=ri_pattern.end(); it++,i++)		
		{
			pHist_ri[i] = it->second;
		}
	}
	
	
	if(uni==1)  //only use the uniform patterns
	{
		UniformHistogram();   //for LBP and TOP-LBP
	}
	
	if(norm==1)  //normalize the histogram
	{
		Normalization();		
	}
}	

/**********calculate the values of uniform patterns**********/
/* number: the number of the sample points
/* return: the values of all the uniform patterns
/************************************************************/
vector<uint> LBP::uniform_pattern(int number)
{
	int i, j, k, idx;
	vector<uint> uni_val;
	
	int* parray = new int[number];  //an array which stores a uniform pattern
	uint val = 0;                  //the decimal value of a uniform pattern
	int tmp;
	
	for(i=0; i<=number; i++)
	{
		memset(parray, 0, 4*number);
		
		//set bits to "1"
		for(idx=0; idx<i; idx++)
		{
			parray[idx] = 1;
		}
		
		for(k=0; k<=number-1; k++)
		{
			//right shift array 			
			tmp = parray[number-1];
			for(idx=number-1; idx>=1; idx--)
			{
				parray[idx] = parray[idx-1];
			}		
			parray[0] = tmp;
			
			//get the decimal values of uniform patterns
			val = 0;
			for(j=0; j<number; j++)
			{
				val += parray[j]*pow(2.0f,number-1-j);
			}
			uni_val.push_back(val);	
			if(val==0 || val==pow(2.0f,number)-1)
			{
				break;
			}											
		}		
	}
	
	//sort the elements in the vecotr into ascending order
	sort(uni_val.begin(), uni_val.end());
	
	delete []parray;
	parray = NULL;
	
	return uni_val;
}

/********calculate the histogram of uniform patterns*********/
/* pphist1:    histogram of uniform patterns
/* phist2:     histogram of all patterns
/* num:        number of sample points
/* r1:         sampling radius of axis1
/* r2:         sampling radius of axis2
/* len1:       size of data along axis1
/* len2:	   size of data along axis2
/* seq_num:    number of axis1-axis2 planes
/* return:     the number of uniform patterns
/************************************************************/
int LBP::CalcUniHist(float** pphist1, float* phist2, int num, int r1, int r2, int len1, int len2, int seq_num)
{	
	int uni_number;
	
	int i;
	vector<uint> uni_patterns;
	vector<uint>::iterator it;
	int bin, total_bin;
	uint* uni_idx = NULL;	
	int idx;
	float total, uni_total;
	
	if(phist2)
	{
		uni_patterns = uniform_pattern(num);
		bin = uni_patterns.size();
		uni_number = bin;  //number of uniform patterns
		
		//write the uniform pattern values into an array
		uni_idx = new uint[bin];
		idx=0;
		for(it=uni_patterns.begin(); it<uni_patterns.end(); it++)
		{
			uni_idx[idx++] = *it;
		}
		
		//consider all the other non-uniform patterns as one pattern
		total_bin = bin+1;		

		//calculate the histogram of uniform patterns		
		*pphist1 = new float[total_bin];
		for(i=0; i<total_bin-1; i++)
		{
			(*pphist1)[i] = phist2[uni_idx[i]];
		}				
		
		total = (len1-2*r1)*(len2-2*r2)*seq_num;
		uni_total = 0;
		for(i=0; i<total_bin-1; i++)
		{
			uni_total += (*pphist1)[i];
		}
		(*pphist1)[total_bin-1] = total-uni_total;		
	}
	
	return uni_number;
}

/*******calculate the histograms of uniform patterns*********/
/* for LBP and TOP-LBP
/************************************************************/
void LBP::UniformHistogram()
{
	if(tlength==1 && basic_hist.pHist_xy)
	{
		uni_bin.xy = CalcUniHist(&uni_hist.pHist_xy, basic_hist.pHist_xy, SN.xy, R.xR, R.yR, width, height, 1);
	}
	else
	{
		if(basic_hist.pHist_xy)
		{
			uni_bin.xy = CalcUniHist(&uni_hist.pHist_xy, basic_hist.pHist_xy, SN.xy, R.xR, R.yR, width, height, tlength-2*R.tR);
		}
		
		if(basic_hist.pHist_xt)
		{
			uni_bin.xt = CalcUniHist(&uni_hist.pHist_xt, basic_hist.pHist_xt, SN.xt, R.xR, R.tR, width, tlength, height-2*R.yR);
		}	
		
		if(basic_hist.pHist_yt)
		{
			uni_bin.yt = CalcUniHist(&uni_hist.pHist_yt, basic_hist.pHist_yt, SN.yt, R.yR, R.tR, height, tlength, width-2*R.xR);
		}	
	}
}	

/*****************normalize the histograms*******************/
/************************************************************/
void LBP::Normalization()
{
	if(uni==1)
	{
		if(uni_hist.pHist_xy)
		{
			CalcNormHist(&uni_hist.pHist_xy, uni_bin.xy+1);
		}
		
		if(uni_hist.pHist_xt)
		{
			CalcNormHist(&uni_hist.pHist_xt, uni_bin.xt+1);
		}
		
		if(uni_hist.pHist_yt)
		{
			CalcNormHist(&uni_hist.pHist_yt, uni_bin.yt+1);
		}				
	}
	else
	{	
		if(basic_hist.pHist_xy)
		{
			CalcNormHist(&basic_hist.pHist_xy, basic_bin.xy);
		}
		
		if(basic_hist.pHist_xt)
		{
			CalcNormHist(&basic_hist.pHist_xt, basic_bin.xt);
		}
		
		if(basic_hist.pHist_yt)
		{
			CalcNormHist(&basic_hist.pHist_yt, basic_bin.yt);
		}
	}
	
	if(pHist_ri)
	{
		CalcNormHist(&pHist_ri, ri_bin);   
	}
}

/*****************normalize the histograms*******************/
/*pphist:  the histogram
/*bin:     bin number of the histogram
/************************************************************/
void LBP::CalcNormHist(float** pphist, int bin)
{
	int i;
	
	float total=0;
	for(i=0; i<bin; i++)
	{
		total += (*pphist)[i];
	}
	
	for(i=0; i<bin; i++)
	{
		(*pphist)[i] /= total;
	}
}

/**********calculate the LBP code of each pixel**************/
/* pppArray:   data of the input image(sequence)
/* type:       0 for LBP
/*             1 for LBP-TOP
/*             2 for RIVLBP
/* x,y,t:      the coordinates of a pixel
/*             (x--width/column;y--height/row;t--time)
/* return:     the decimal value of the pattern
/************************************************************/
struct Code LBP::LBPCode(uchar*** pppArray, int algorithm, int x, int y, int t)
{
	struct Code res_code={0,0,0};
	
	if(!pppArray)
	{
		cout<<"No input data!"<<endl;
		return res_code;
	}
	
	int i, j;
	int number;
	Point3D32f* pSP = NULL;
	uint* pData = NULL;
	uint** ppData = NULL;
	uint code;
	
	Point3D p;
	Point3D ltp;
	Point3D lbp;
	Point3D rtp;
	Point3D rbp;
	Point3D32f fp;
	Point3D32f new_fp;
	
	//x-y plane
	if(algorithm==0 || algorithm==1)
	{
		number = SN.xy;
		
		pSP = new Point3D32f[number];
		pData = new uint[number];
	
		//calculate the coordinates of the sample points
		for(i=0; i<number; i++)
		{
			pSP[i].x = x+R.xR*cos((2*PI/number)*i);
			pSP[i].y = y-R.yR*sin((2*PI/number)*i);
		}
		
		//calculate the values of the sample points 	
		//without interpolation
		if(interp==0)	
		{
			for(i=0; i<number; i++)
			{
				p.x = (int)(pSP[i].x+0.5);
				p.y = (int)(pSP[i].y+0.5);
				
				BOUND(p.x, 0, width-1);
				BOUND(p.y, 0, height-1);
				
				pData[i] = (uint)(pppArray[t][p.y][p.x]);
			}	
		}
		else	//with bilinear interpolation
		{
			for(i=0; i<number; i++)
			{
				fp.x = pSP[i].x;
				fp.y = pSP[i].y;
			
				//calculate coordinates of four points which are used in bilinear interpolation
				ltp.x = floor(fp.x); ltp.y = floor(fp.y); 
				lbp.x = floor(fp.x); lbp.y = ceil(fp.y);  
				rtp.x = ceil(fp.x);  rtp.y = floor(fp.y); 
				rbp.x = ceil(fp.x);  rbp.y = ceil(fp.y);  
			
				new_fp.x = fp.x-ltp.x;
				new_fp.y = fp.y-ltp.y;
			
				if(new_fp.x<1e-6 && new_fp.y<1e-6)  //interpolation is not needed
				{
					pData[i] = (uint)pppArray[t][(int)fp.y][(int)fp.x];
				}
				else  //bilinear interpolation
				{
					pData[i] = (uint)(pppArray[t][ltp.y][ltp.x]*(1-new_fp.x)*(1-new_fp.y)+
									  pppArray[t][rtp.y][rtp.x]*new_fp.x*(1-new_fp.y)+
								      pppArray[t][lbp.y][lbp.x]*(1-new_fp.x)*new_fp.y+
									  pppArray[t][rbp.y][rbp.x]*new_fp.x*new_fp.y);
				}
			}
		}	
	
		//calculate the LBP code
		res_code.xy = 0;
		for(i=number-1; i>=0; i--)
		{
			res_code.xy = (res_code.xy<<1) + (pData[i] >= (uint)pppArray[t][y][x]);
		}		
	}	
	
	if(algorithm==1)
	{
		//x-t plane
		{
			if(SN.xt != SN.xy)
			{ 
				number = SN.xt;
	
				if(pSP)
				{
					delete []pSP;
					pSP = NULL;
				}		
				pSP = new Point3D32f[number];
				if(pData)
				{
					delete []pData;
					pData = NULL;
				}		
				pData = new uint[number];
			}
			
			//calculate the coordinates of the sample points
			for(i=0; i<number; i++)
			{
				pSP[i].x = x+R.xR*cos((2*PI/number)*i);
				pSP[i].t = t-R.tR*sin((2*PI/number)*i);
			}	
			
			//calculate the values of the sample points 	
			//without interpolation
			if(interp==0)	
			{
				for(i=0; i<number; i++)
				{
					p.x = (int)(pSP[i].x+0.5);
					p.t = (int)(pSP[i].t+0.5);
					
					BOUND(p.x, 0, width-1);
					BOUND(p.t, 0, tlength-1);
					
					pData[i] = (uint)pppArray[p.t][y][p.x];
				}	
			}
			else	//with bilinear interpolation
			{
				for(i=0; i<number; i++)
				{
					fp.x = pSP[i].x;
					fp.t = pSP[i].t;
				
					//calculate coordinates of four points which are used in bilinear interpolation
					ltp.x = floor(fp.x);  ltp.t = floor(fp.t);
					lbp.x = floor(fp.x);  lbp.t = ceil(fp.t);
					rtp.x = ceil(fp.x);   rtp.t = floor(fp.t);
					rbp.x = ceil(fp.x);   rbp.t = ceil(fp.t);
				
					new_fp.x = fp.x-ltp.x;
					new_fp.t = fp.t-ltp.t;
				
					if(new_fp.x<1e-6 && new_fp.t<1e-6)  //interpolation is not needed
					{
						pData[i] = (uint)pppArray[(int)fp.t][y][(int)fp.x];
					}
					else  //bilinear interpolation
					{
						pData[i] = (uint)(pppArray[ltp.t][y][ltp.x]*(1-new_fp.x)*(1-new_fp.t)+
										  pppArray[rtp.t][y][rtp.x]*new_fp.x*(1-new_fp.t)+
										  pppArray[lbp.t][y][lbp.x]*(1-new_fp.x)*new_fp.t+
										  pppArray[rbp.t][y][rbp.x]*new_fp.x*new_fp.t);
					}
				}
			}	
		
			//calculate the LBP code
			res_code.xt = 0;
			for(i=number-1; i>=0; i--)
			{
				res_code.xt = (res_code.xt<<1) + (pData[i] >= (uint)pppArray[t][y][x]);
			}				
		}
		
		//y-t plane
		{
			if(SN.yt != SN.xt)
			{
				number = SN.yt;
			
				if(pSP)
				{
					delete []pSP;
					pSP = NULL;
				}		
				pSP = new Point3D32f[number];
				if(pData)
				{
					delete []pData;
					pData = NULL;
				}		
				pData = new uint[number];
			}

			//calculate the coordinates of the sample points
			for(i=0; i<number; i++)
			{
				pSP[i].y = y+R.yR*cos((2*PI/number)*i);
				pSP[i].t = t-R.tR*sin((2*PI/number)*i);
			}
			
			//calculate the values of the sample points 	
			//without interpolation
			if(interp==0)	
			{
				for(i=0; i<number; i++)
				{
					p.y = (int)(pSP[i].y+0.5);
					p.t = (int)(pSP[i].t+0.5);
					
					BOUND(p.y, 0, height-1);
					BOUND(p.t, 0, tlength-1);
					
					pData[i] = (uint)pppArray[p.t][p.y][x];
				}	
			}
			else	//with bilinear interpolation
			{
				for(i=0; i<number; i++)
				{
					fp.y = pSP[i].y;
					fp.t = pSP[i].t;
				
					//calculate coordinates of four points which are used in bilinear interpolation
					ltp.y = floor(fp.y); ltp.t = floor(fp.t);
					lbp.y = floor(fp.y); lbp.t = ceil(fp.t);
					rtp.y = ceil(fp.y);  rtp.t = floor(fp.t);
					rbp.y = ceil(fp.y);  rbp.t = ceil(fp.t);
				
					new_fp.y = fp.y-ltp.y;
					new_fp.t = fp.t-ltp.t;
				
					if(new_fp.y<1e-6 && new_fp.t<1e-6)  //interpolation is not needed
					{
						pData[i] = (uint)pppArray[(int)fp.t][(int)fp.y][x];
					}
					else  //bilinear interpolation
					{
						pData[i] = (uint)(pppArray[ltp.t][ltp.y][x]*(1-new_fp.y)*(1-new_fp.t)+
										  pppArray[rtp.t][rtp.y][x]*new_fp.y*(1-new_fp.t)+
										  pppArray[lbp.t][lbp.y][x]*(1-new_fp.y)*new_fp.t+
										  pppArray[rbp.t][rbp.y][x]*new_fp.y*new_fp.t);
					}
				}
			}	
		
			//calculate the LBP code
			res_code.yt = 0;
			for(i=number-1; i>=0; i--)
			{
				res_code.yt = (res_code.yt<<1) + (pData[i] >= (uint)pppArray[t][y][x]);
			}						
		}
	}
	
	//calculate LBP code of previous, current and post frames
	int num_fr = 3;
	if(algorithm==2)
	{	
		number = SN.xy;
		
		pSP = new Point3D32f[number];
		
		ppData = new uint*[num_fr];
		for(i=0; i<num_fr; i++)
		{
			ppData[i] = new uint[number];
		}
		
		//calculate the coordinates of the sample points
		for(i=0; i<number; i++)
		{
			pSP[i].x = x+R.xR*cos((2*PI/number)*i);
			pSP[i].y = y-R.xR*sin((2*PI/number)*i);			
		}
		
		//calculate the values of the sample points 	
		//without interpolation
		if(interp==0)	
		{
			for(i=0; i<number; i++)
			{
				p.x = (int)(pSP[i].x+0.5);
				p.y = (int)(pSP[i].y+0.5);
				
				BOUND(p.x, 0, width-1);
				BOUND(p.y, 0, height-1);
				
				for(j=0; j<num_fr; j++)
				{
					ppData[j][i] = (uint)(pppArray[t-(1-j)*R.tR][p.y][p.x]);  //previous,current,post
				}
			}	
		}
		else	//with bilinear interpolation
		{
			for(i=0; i<number; i++)
			{
				fp.x = pSP[i].x;
				fp.y = pSP[i].y;
			
				//calculate coordinates of four points which are used in bilinear interpolation
				ltp.x = floor(fp.x); ltp.y = floor(fp.y); 
				lbp.x = floor(fp.x); lbp.y = ceil(fp.y);  
				rtp.x = ceil(fp.x);  rtp.y = floor(fp.y); 
				rbp.x = ceil(fp.x);  rbp.y = ceil(fp.y);  
			
				new_fp.x = fp.x-ltp.x;
				new_fp.y = fp.y-ltp.y;
			
				if(new_fp.x<1e-6 && new_fp.y<1e-6)  //interpolation is not needed
				{
					for(j=0; j<num_fr; j++)
					{
						ppData[j][i] = (uint)(pppArray[t-(1-j)*R.tR][(int)fp.y][(int)fp.x]);  //previous,current,post
					}
				}
				else  //bilinear interpolation
				{
					for(j=0; j<num_fr; j++)
					{
						ppData[j][i] = (uint)(pppArray[t-(1-j)*R.tR][ltp.y][ltp.x]*(1-new_fp.x)*(1-new_fp.y)+
										  	  pppArray[t-(1-j)*R.tR][rtp.y][rtp.x]*new_fp.x*(1-new_fp.y)+
									      	  pppArray[t-(1-j)*R.tR][lbp.y][lbp.x]*(1-new_fp.x)*new_fp.y+
										  	  pppArray[t-(1-j)*R.tR][rbp.y][rbp.x]*new_fp.x*new_fp.y);
					}
				}
			}
		}
		
		//calculate the LBP code
		res_code.xy = 0;
		res_code.xt = 0;
		res_code.yt = 0;
		for(i=number-1; i>=0; i--)
		{
			res_code.xy = (res_code.xy<<1) + (ppData[0][i] >= (uint)pppArray[t][y][x]);  //previous frame
			res_code.xt = (res_code.xt<<1) + (ppData[1][i] >= (uint)pppArray[t][y][x]);  //current frame
			res_code.yt = (res_code.yt<<1) + (ppData[2][i] >= (uint)pppArray[t][y][x]);  //post frame						
		}								
	}
	
	if(pSP)
	{
		delete []pSP;
		pSP = NULL;
	}
	if(pData)
	{
		delete []pData;
		pData = NULL;
	}
	if(ppData)
	{
		for(i=0; i<num_fr; i++)
		{
			if(ppData[i])
			{
				delete []ppData[i];
				ppData[i] = NULL;
			}
		}
		delete []ppData;
		ppData = NULL;
	}
	
	return res_code;	
}

/************************************************************/
/************************************************************/
uint LBP::CalcRotationInvariant(uint lbp_code, int number)
{
	uint minLBP = lbp_code;
	uint code = lbp_code;
	uint tmp;
	
	for(int i=0; i<number-1; i++)
	{		
		tmp = (code>>1)|((code&1)<<(number-1));  //shift right    
		
		if(tmp<minLBP)
		{
			minLBP = tmp;
		}
		
		code = tmp;
	}
	
	return minLBP;
}

/************************************************************/
/************************************************************/
map<uint,uint> LBP::CalcRIMap(int number)
{
	map<uint, uint> v_minv;
	
	int i;
	int bin;
	
	bin = pow(2.0f,number);
	uint* pArray = new uint[bin];
	
	for(i=0; i<bin; i++)
	{
		pArray[i] = (uint)i;  //all patterns
	}
			
	uint val, min_val;
	for(i=0; i<bin; i++)
	{
		val = pArray[i];
		min_val = CalcRotationInvariant(val, number);  //get the rotation invariant VLBP pattern
		
		v_minv.insert(pair<uint,uint>(val,min_val));
	}
	
	if(pArray)
	{
		delete []pArray;
		pArray = NULL;
	}	
	
	return v_minv;
}

/*******calculate the rotation invariant VLBP patterns*******/
/* number:        the number of sample points
/* rivlbp:        (uni==0)the map of VLBP and rotation invariant VLBP
/*                (uni==1)the map of VLBP and uniform rotation invariant VLBP
/* ri_pattern:    (uni==0)the rotation invariant VLBP patterns
/*                (uni==1)the value of uniform rotation invariant VLBP patterns
/************************************************************/
void LBP::RIVLBP_Old(int number, map<uint,uint>& rivlbp, map<uint,uint>& ri_pattern)
{	
	map<uint,uint> v_minv = CalcRIMap(number);	
	
	map<uint,uint>::iterator it;
	map<uint,uint>::iterator jt;
	map<uint,uint>::iterator kt;
	
	int i, j;
	
	for(i=0; i<2; i++)
	{
		for(it=v_minv.begin(); it!=v_minv.end(); it++)
		{
			for(jt=v_minv.begin(); jt!=v_minv.end(); jt++)
			{
				for(kt=v_minv.begin(); kt!=v_minv.end(); kt++)
				{
					for(j=0; j<2; j++)
					{
						uint val = 	i +
						            (((*it).first)*pow(2.0f,1))+
						            (((*jt).first)*pow(2.0f,number+1))+
						            (((*kt).first)*pow(2.0f,2*number+1))+
						            (j*pow(2.0f,3*number+1));
						            
						uint map_val = 	i +
						            (((*it).second)*pow(2.0f,1))+
						            (((*jt).second)*pow(2.0f,number+1))+
						            (((*kt).second)*pow(2.0f,2*number+1))+
						            (j*pow(2.0f,3*number+1));
						if(uni==0)
						{            
							rivlbp.insert(pair<uint,uint>(val,map_val));
							ri_pattern.insert(pair<uint,uint>(map_val,0));		
						}
						
						if(uni==1)
						{	
							uint riu2_val = CalcUniform(map_val, 3*number+2);
							rivlbp.insert(pair<uint,uint>(val,riu2_val));
							//ri_pattern.insert(pair<uint,uint>(riu2_val,0));
						}										
					}
				}
			}
		}
	}
	
	if(uni==1)
	{	
		for(i=0; i<3*number+4; i++)
		{
			ri_pattern.insert(pair<uint,uint>(i,0));
		}
	}
}

/*******calculate the rotation invariant VLBP patterns*******/
/* number:        the number of sample points
/* rivlbp:        (uni==0)the map of VLBP and rotation invariant VLBP
/*                (uni==1)the map of VLBP and uniform rotation invariant VLBP
/* ri_pattern:    (uni==0)the rotation invariant VLBP patterns
/*                (uni==1)the value of uniform rotation invariant VLBP patterns
/************************************************************/
void LBP::RIVLBP_New(int number, map<uint,uint>& rivlbp, map<uint,uint>& ri_pattern)
{
	int i, j, k, t;
	
	int bin = pow(2.0f, number);
	uint* pPattern = new uint[bin];
	
	uint preC, preN, curN, posN, posC;	
	
	map<uint,uint> tmp;
	map<uint,uint>::iterator it;
	
	for(i=0; i<bin; i++)
	{
		pPattern[i] = (uint)i;
	}
	
	for(i=0; i<bin; i++)
	{			
		for(j=0; j<bin; j++)
		{				
			for(k=0; k<bin; k++)
			{	
				preN = pPattern[i];
				curN = pPattern[j];
				posN = pPattern[k];
				
				uint val = (preN*pow(2.0f,1))+
				           (curN*pow(2.0f,(number+1)))+
				           (posN*pow(2.0f,(2*number+1)));
				
				uint tmp_val;
				uint minVal = pow(2.0f, 3*number+2)+1;
				uint tmp_pre, tmp_cur, tmp_pos;					
				for(t=0; t<number; t++)
				{
					tmp_pre = (preN>>1)|((preN&1)<<(number-1));  //shift right 
					tmp_cur = (curN>>1)|((curN&1)<<(number-1));  //shift right 
					tmp_pos = (posN>>1)|((posN&1)<<(number-1));  //shift right 
					
		   			preN = tmp_pre;
		   			curN = tmp_cur;
		   			posN = tmp_pos;							
														
					tmp_val = (tmp_pre<<1)+
				              (tmp_cur<<(number+1))+
				              (tmp_pos<<(2*number+1));

					if(tmp_val<minVal)
					{
						minVal = tmp_val;
					}       
				}
				
				tmp.insert(pair<uint,uint>(val,minVal));
			}
		}
	}

	for(i=0; i<2; i++)
	{
		for(j=0; j<2; j++)
		{
			for(it=tmp.begin(); it!=tmp.end(); it++)
			{
				uint val = (uint)i+(*it).first+(uint)j*pow(2.0f,(3*number+1));
				uint map_val = (uint)i+(*it).second+(uint)j*pow(2.0f,(3*number+1));
				
				if(uni==0)
				{
					rivlbp.insert(pair<uint,uint>(val,map_val));
					ri_pattern.insert(pair<uint,uint>(map_val,0));
				}
				
				if(uni==1)
				{	
					uint riu2_val = CalcUniform(map_val, 3*number+2);
					rivlbp.insert(pair<uint,uint>(val,riu2_val));
					//ri_pattern.insert(pair<uint,uint>(riu2_val,0));
				}
			}
		}	
	}
		
	if(uni==1)
	{	
		for(i=0; i<3*number+4; i++)
		{
			ri_pattern.insert(pair<uint,uint>((uint)i,0));
		}
	}
	
	if(pPattern)
	{
		delete []pPattern;
		pPattern = NULL;
	}
}

/*******calculate the rotation invariant VLBP patterns*******/
/* number:        the number of sample points
/* type:          the method of calculating rotation invariant 
/*                VLBP pattern; 0 if using old rotation descriptor,
/*                1 if using new rotation descriptor
/* rivlbp:        (uni==0)the map of VLBP and rotation invariant VLBP
/*                (uni==1)the map of VLBP and uniform rotation invariant VLBP
/* ri_pattern:    (uni==0)the rotation invariant VLBP patterns
/*                (uni==1)the value of uniform rotation invariant VLBP patterns
/************************************************************/
void LBP::RIVLBP(int number, int type, map<uint,uint>& rivlbp, map<uint,uint>& ri_pattern)
{	
	if(type==0)
	{
		RIVLBP_Old(number, rivlbp, ri_pattern);
	}
	
	if(type==1)
	{
		RIVLBP_New(number, rivlbp, ri_pattern);	
	}
}

/****calculate the corresponding uniform pattern value*******/
/* val:     the value of rotation invariant VLBP pattern
/* number:  the bit number of 'val'
/* return:  the uniform value of 'val'
/************************************************************/
uint LBP::CalcUniform(uint val, int number)
{
	int i, j;
	int cnt;
	
	uint riu2_val = 0;
	
	//get each bit value of the input 'val'
	int* pArray = new int[number];
	
	for(i=0; i<number; i++)
	{
		pArray[i] = (int)((val>>i) & 1);
	}
	
	//calculate the uniform value
	cnt = 0;
	for(i=0; i<number-1; i++)
	{
		if(pArray[i]!=pArray[i+1])
		{
			cnt++;
		}
	}
	if(pArray[0]!=pArray[number-1])
	{
		cnt += 1;
	}
	
	if(cnt>2)   //not uniform pattern
	{
		riu2_val = (uint)(number+1);  //number=3*P+2; value=3*P+3
	}
	else        //uniform pattern
	{
		riu2_val = 0;
		for(i=0; i<number; i++)
		{
			riu2_val += (uint)pArray[i];
		}
	}
	
	if(pArray)
	{
		delete []pArray;
		pArray = NULL;
	}
	
	return riu2_val;
}

/*******************release memories*************************/
/************************************************************/
void LBP::Release()
{
	if(basic_hist.pHist_xy)
	{	
		delete []basic_hist.pHist_xy;
		basic_hist.pHist_xy = NULL;
	}
	if(basic_hist.pHist_xt)
	{	
		delete []basic_hist.pHist_xt;
		basic_hist.pHist_xt = NULL;
	}
	if(basic_hist.pHist_yt)
	{	
		delete []basic_hist.pHist_yt;
		basic_hist.pHist_yt = NULL;
	}	
	
	if(uni==1)
	{
		if(uni_hist.pHist_xy)
		{	
			delete []uni_hist.pHist_xy;
			uni_hist.pHist_xy = NULL;
		}
		if(uni_hist.pHist_xt)
		{	
			delete []uni_hist.pHist_xt;
			uni_hist.pHist_xt = NULL;
		}
		if(uni_hist.pHist_yt)
		{	
			delete []uni_hist.pHist_yt;
			uni_hist.pHist_yt = NULL;
		}	
	}	
	
	if(pHist_ri)
	{
		delete []pHist_ri;
		pHist_ri = NULL;
	}			
}

#pragma once

#include <stdio.h>
#include <iostream>
#include <math.h>

extern "C" {
#include <jpeglib.h>
}

#include "define.h"
#include "cImageYCbCr.h"

enum extension_t {
	BMP,
	JPEG,
	PNG,
	OTHER, 	//unrecognized extension
	NOTYPE 	//no extension
};

class cImageYCbCr;

class cImageRGB
{
private:
	int iHeight;		//parameters
	int iWidth;
	unsigned char * pRGB;

public:
	int iWidthRGB();	//functions
	int iSizeRGB();

public:
	cImageRGB();
	virtual ~cImageRGB();

	int getWidth();
	int getHeight();
	void setHeight(int val);
	void setWidth(int val);
	void setpRGB(unsigned char * val);

public:
	int CreateFromBitmapFile(const char * filename, int *picWidth = NULL, int *picHeight = NULL);
	int CreateFromJpegFile(const char * filename, int *picWidth = NULL, int *picHeight = NULL);
	cImageYCbCr* CreateYCrCbFromRGB(int iSubW = 1, int iSubH = 1);
public:
	static int YFromRGB(int iR, int iG, int iB);
	static int CrFromRGB(int iR, int iG, int iB);
	static int CbFromRGB(int iR, int iG, int iB);
	//static int RFromYCbCr(int iY, int iCb, int iCr);
	//static int GFromYCbCr(int iY, int iCb, int iCr);
	//static int BFromYCbCr(int iY, int iCb, int iCr);
	static int ceilTo(int number, int base = 1, int remainder = 0);
	static extension_t extract_extension(const char * filename);
private:
	static int Limiting_a_b(double d, int a, int b);
public:
	int WriteToBitmapFile(char * filename);
	int WriteToJpegFile(char * filename, int quality);
	//static void Normalize(int iNumBands, int * pFilterLength, double** pFilters, double * mult, int * add, int iEvenOdd);
	cImageRGB* Copy();
	unsigned char* getReverse(unsigned char* pRGB);

};


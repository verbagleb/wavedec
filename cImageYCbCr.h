#pragma once
#include <stdio.h>
#include "define.h"
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <assert.h>

#include "cImageRGB.h"

using namespace std;

enum component
{
	Y,
	Cr,
	Cb
};

class cImageRGB;

class cImageYCbCr
{
private:
	int iFullHeight;
	int iFullWidth;
	int iFullHeight2;
	int iFullWidth2;
	int iEnhanceValue;
	int iSubW;
	int iSubH;
	unsigned char *pY0;
	unsigned char *pCr0;
	unsigned char *pCb0;
	
public:
	cImageYCbCr();
	virtual ~cImageYCbCr();

	void setFullHeight(int val);
	void setFullWidth(int val);
	void setFullHeight2(int val);
	void setFullWidth2(int val);
	void setEnhanceValue(int val);
	void setSubW(int val);
	void setSubH(int val);
	void setpY0(unsigned char * val);
	void setpCb0(unsigned char * val);
	void setpCr0(unsigned char * val);

	int getFullWidth();
	int getFullHeight();
	int getFullWidth2();
	int getFullHeight2();
	int getFullWidth(component comp);
	int getFullHeight(component comp);
	int getEnhanceValue();
	int getSubW();
	int getSubH();
	unsigned char* getPComp0(component comp);

public:
	static int Limiting_a_b(double d, int a, int b);
	static int RFromYCbCr(int iY, int iCb, int iCr);
	static int GFromYCbCr(int iY, int iCb, int iCr);
	static int BFromYCbCr(int iY, int iCb, int iCr);

	void setWhite();
	void setGrey();
	void paintComp(component comp, unsigned char val);
	cImageRGB* CreateRGB24FromYCbCr();

public:
	static int ceilTo(int number, int base = 1, int remainder = 0);
	cImageYCbCr* copy();
	static component next(component comp);
	static 	cImageYCbCr* difference(cImageYCbCr* pImageA, cImageYCbCr* pImageB, double mult = 1.0, char add = 128);
	void fillEnhancementC(component comp, int iW, int iH, int iEvenOdd = 1, int sgnH = 1, int sgnV = 1);
	void placeInto(component comp, cImageYCbCr *background, int columnL, int stringH, int fromColumn = 0, int toColumn = 0,
		int fromString = 0, int toString = 0, bool includingEnhancement = false, int decimation = 1, int sparsing = 1);
	double prms(component comp);
	cImageYCbCr* addAWGN(double sigma);
	static cImageYCbCr *takeAverage(cImageYCbCr **pImage_ar, int numImages);
	cImageYCbCr* setLayer(int iNB);
	void resetLayer(int iNB);
	void blockBorders(int blockSize);

};


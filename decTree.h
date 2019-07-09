#pragma once
#include <iostream>

#include "define.h"
#include "cImageYCbCr.h"
#include "userLib.h"
#include "cFilter.h"
#include "math.h"

class decTree
{
public:
	decTree();
	~decTree();

	bool isEmpty();
	bool isUndivided();
	int getNumH();
	int getNumW();

	//int loadBand(cBands_double * dBandY_n, cBands_double * dBandCr_n, cBands_double * dBandCb_n);
	int loadImage(cImageYCbCr *pImage, bool bShift = false);
	cImageYCbCr * createImage(bool bShift = false);
	int analyseBandW(cFilter *pFilter, bool ignoreMult = false);
	int analyseBandH(cFilter *pFilter, bool ignoreMult = false);
	int analyseBandWH(cFilter *pFilter, bool ignoreMult = false);
	double * enhance(component comp, bool byW, int iEnhanceValue, bool bOdd, int sgn, int iNumBands, int iRemainder);
	decTree * stepAt(int iByW, int iByH);
	double * sparse(component comp, bool byW, bool bOdd, int iNumBands, int iShift);
	int synthesizeBand();

	short * getSubCoef(double extension, component comp, int filenum, int * sub_width, int * sub_height); 
	int countBands();
	int getAllCoefs(short ** dest, int * sub_width, int * sub_height, double extension, component comp, int bandnum=0);
	void copyStructure(decTree * dest);
	void copyData(decTree * dest);
	void copyTree(decTree * dest);
	//void setSubCoef(short * subCoeff, int coeff_pred_ln, short * ref_sampl, int ref_sampl_ln, double extension, component comp, int stage, int filenum); 
	//int setAllCoefs(short ** subCoeff, int * coeff_pred_ln, short ** ref_sampl, int * ref_sampl_ln, double extension, component comp, int stage, int bandnum=0);
	void setSubCoef(short * subCoeff, double extension, component comp, int stage, int filenum); 
	int setAllCoefs(short ** subCoeff, double extension, component comp, int stage, int bandnum=0);

	static double getSubPSNR(decTree * pA, decTree * pB, component comp);
	static int getAllPSNR(double * dest, decTree * pA, decTree * pB, component comp, int bandnum = 0);
	static double getSubEnergy(decTree * pA, component comp, bool bShift = true);
	static int getAllEnergy(double * dest, decTree * pA, component comp, int bandnum = 0, 
			bool bShift = true);
	static int getAllNames(char ** dest, decTree * pA, component comp, int bandnum = 0,
			char * current_name = nullptr);
	decTree * extractSgn(double epsilon = 0.0);
	void addSgn(decTree * src);
	decTree * substractTree(decTree * pSub);

	void setMult(double mult);
	double getMult();

private:
	int synthesizeBandW();
	int synthesizeBandH();
	void extractSgn_it(decTree * dest, double epsilon);
	void substractTree_it(decTree * pSub);

private:
			//common type
	int iNumH;
	int iNumW;
	int iWidthY;
	int iHeightY;
	int iWidthC;
	int iHeightC;
	double mult;

			//divided type
	decTree * step;
	cFilter * pFilter;	//just stores a link

			//undivided type
	double * dBandY;
	double * dBandCr;
	double * dBandCb;
};

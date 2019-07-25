#pragma once
#include "define.h"
#include "math.h"

class cFilter
{
public:
	cFilter();
	~cFilter();

	char *sFilterName;

	int iNumBands;
	int *pDFilterLength;
	double ** pDFilters;
	int *pIFilterLength;
	double ** pIFilters;

	bool bOdd;		//filter is placed in integerpoints, if odd
	int iRemainder;		//remainder that the bandlength must be congrous to
	double *mult;		//data about how each band should be strengthened in image
	int iEnhanceValue;	//value that the image should be previously enhanced by
	int iInitialShift;	//the number of the first point to which LPF is applied
	int iAlternateShift;	//the step of the first point when switching to the next band

public:
	void normalize();
};

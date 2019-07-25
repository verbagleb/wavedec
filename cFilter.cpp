#include "cFilter.h"

cFilter::cFilter()
{
}

cFilter::~cFilter()
{
	delete[] sFilterName;
	for (int i=0;i<iNumBands;i++) 
	{
		delete[] pDFilters[i];
		delete[] pIFilters[i];
	}
	delete[] pDFilters;
	delete[] pDFilterLength;
	delete[] pIFilters;
	delete[] pIFilterLength;
	delete[] mult;
}

void cFilter::normalize()
{
	// Нормировка фильтров для одинакового квантования
	double dSum;
	int j;
	for (int k = 0; k < iNumBands; k++)
	{
		dSum = pIFilters[k][0] * pIFilters[k][0] * (bOdd ? 1:2);
		for (j = 1; j < pIFilterLength[k]; j++)
			dSum += 2 * pIFilters[k][j] * pIFilters[k][j];
		dSum = sqrt(dSum);
		pIFilters[k][0] /= (dSum);
		for (j = 1; j < pIFilterLength[k]; j++)
			pIFilters[k][j] /= (dSum);
		pDFilters[k][0] *= dSum;
		for (j = 1; j < pDFilterLength[k]; j++)
			pDFilters[k][j] *= dSum;
	}
	// Коэффициент нормирования низкочастотной области для отображения
	dSum = pDFilters[0][0] * (bOdd ? 1 : 2);
	for (int i = 1; i < pDFilterLength[0]; i++)
		dSum += 2 * pDFilters[0][i];
	mult[0] = 1.0 / dSum;		//one-dimensional!
}

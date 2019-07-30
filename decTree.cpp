#include "decTree.h"

const double deltacoef = 4.0;

decTree::decTree()
	: iNumH(0)
	, iNumW(0)
	, step(nullptr)
	, pFilter(nullptr)
	, iWidthY(0)
	, iHeightY(0)
	, iWidthC(0)
	, iHeightC(0)
	, dBandY(nullptr)
	, dBandCr(nullptr)
	, dBandCb(nullptr)
	, mult(0.0)
{
}

decTree::~decTree() //recursively
{
	if (isUndivided())
	{
		delete[] dBandY;
		delete[] dBandCr;
		delete[] dBandCb;
		dBandY = dBandCb = dBandCr = nullptr;
	}
	else if (!isEmpty()) 
	{
		delete[] step;
		step = nullptr;
	}
}

bool decTree::isEmpty()
{
	return !iNumH && !iNumW;
}

bool decTree::isUndivided()
{
	return iNumH==1 && iNumW==1;
}

int decTree::getNumH()
{
	return iNumH;
}

int decTree::getNumW()
{
	return iNumW;
}

decTree * decTree::stepAt(int iByW, int iByH)
{
	if (iByH<0 || iByH>=iNumH || iByW<0 || iByW>=iNumW)
		return nullptr;
	if (isUndivided() && iByH==0 && iByW==0)
		return this;
	return &step[iByW*iNumH+iByH];
}

/*int decTree::loadBand(cBands_double * dBandY_n,cBands_double * dBandCr_n,cBands_double * dBandCb_n)
{
	if (!isEmpty())
		return 1;
	
	dBandY = dBandY_n->copy();
	dBandCr = dBandCr_n->copy();
	dBandCb = dBandCb_n->copy();
	
	iNumH = 1;
	iNumW = 1;

	return 0;
}*/

//With conversion to double
int decTree::loadImage(cImageYCbCr *pImage, bool bShift)
{
	if (!isEmpty())
	{
		cout << "Loading image error: tree is not empty" << endl;
		return 1;
	}
	
	iNumH = 1;
	iNumW = 1;
	mult = 1.0;

	iHeightY = pImage->getFullHeight();
	iWidthY = pImage->getFullWidth();
	iHeightC = pImage->getFullHeight2();
	iWidthC = pImage->getFullWidth2();
	int iSizeY = iWidthY*iHeightY;
	int iSizeC = iWidthC*iHeightC;

	dBandY = new double [iSizeY];
	dBandCr = new double [iSizeC];
	dBandCb = new double [iSizeC];

	unsigned char * pY0 = pImage->getPComp0(Y);
	unsigned char * pCr0 = pImage->getPComp0(Cr);
	unsigned char * pCb0 = pImage->getPComp0(Cb);

	for (int i=0; i<iSizeY; i++)
		dBandY[i] = (double)pY0[i]-(bShift ? 128:0);
	for (int i=0; i<iSizeC; i++)
		dBandCr[i] = (double)pCr0[i]-(bShift ? 128:0);
	for (int i=0; i<iSizeC; i++)
		dBandCb[i] = (double)pCb0[i]-(bShift ? 128:0);
	return 0;
}

cImageYCbCr* decTree::createImage(bool bShift) 
{
	cImageYCbCr *pImage = new cImageYCbCr;
	if (!pImage)
		return nullptr;

	pImage->setEnhanceValue(0);
	pImage->setFullWidth(iWidthY);
	pImage->setFullHeight(iHeightY);
	pImage->setFullWidth2(iWidthC);
	pImage->setFullHeight2(iHeightC);
	// no data about iSub is gathered inside the tree
	
	int iSizeY = iWidthY*iHeightY;
	int iSizeC = iWidthC*iHeightC;
	unsigned char *pY0 = new unsigned char[iSizeY],
		*pCr0 = new unsigned char[iSizeC],
		*pCb0 = new unsigned char[iSizeC];
	if (!pY0 || !pCr0 || !pCb0)
		return nullptr;
	pImage->setpY0(pY0);
	pImage->setpCr0(pCr0);
	pImage->setpCb0(pCb0);

	if(isUndivided())
	{
		for (int i = 0; i<iSizeY; i++)
			pY0[i]=Limiting_a_b(mult*dBandY[i] +
					(bShift ? 128:0), 0, 255);
		for (int i = 0; i<iSizeC; i++)
			pCb0[i]=Limiting_a_b(mult*dBandCb[i] + 
					(bShift ? 128:0), 0, 255);
		for (int i = 0; i<iSizeC; i++)
			pCr0[i]=Limiting_a_b(mult*dBandCr[i] +
					(bShift ? 128:0), 0, 255);
	}
	else
	{
		pImage->setWhite();
		cImageYCbCr * part;
		int blockYW = iWidthY/iNumW;
		int blockYH = iHeightY/iNumH;
		int blockCW = iWidthC/iNumW;
		int blockCH = iHeightC/iNumH;
		for (int j = 0; j<iNumH; j++)
			for (int i=0; i<iNumW; i++)
			{
				int columnYL = blockYW*i;
				int stringYH = blockYH*j;
				int columnCL = blockCW*i;
				int stringCH = blockCH*j;
				part=stepAt(i,j)->createImage( bShift||i||j );
				part->placeInto(Y,pImage,columnYL+1,stringYH+1,1,blockYW-1,1,blockYH-1);
				part->placeInto(Cb,pImage,columnCL+1,stringCH+1,1,blockCW-1,1,blockCH-1);
				part->placeInto(Cr,pImage,columnCL+1,stringCH+1,1,blockCW-1,1,blockCH-1);
				delete part;
				part = nullptr;
			}
	}
	return pImage;
}

/*
// Instead of creating an enhanced image
// Logical address
double decTree::valueAt(component comp, int iS, int iC, cFilter *pFilter)
{
	double *dBand;
	int iWidth, iHeight;
	switch (comp) {
		case Y:
			dBand = dBandY;
			iWidth = iWidthY;
			iHeight = iHeightY;
			break;
		case Cr:
			dBand = dBandCr;
			iWidth = iWidthC;
			iHeight = iHeightC;
			break;
		case Cb:
			dBand = dBandCb;
			iWidth = iWidthC;
			iHeight = iHeightC;
			break;
	}

	int sgn=1;
	if (iS < 0)
	{
		if (pFilter.bOdd)
		{
			iS = -iS;
			sgn = 
	}
	
	if (iS >= 0 && iS < iHeight && iC >= 0 && iC < iWidth)
		return dBand[iC + iWidth*iS];

	if 
}
*/

int decTree::analyseBandW(cFilter *pFilter, bool ignoreMult)
{
	if (!isUndivided())
		return 1;

	int nBands = pFilter->iNumBands;
	step = new decTree[nBands];
	if (!step)
		error(2, errno, __func__);
	
	/* analysis */
	int iWidthCeiledY = ceilTo(iWidthY,nBands,pFilter->iRemainder);
	int iHeightCeiledY = ceilTo(iHeightY,nBands,pFilter->iRemainder);
	int iBWidthY_up = (iWidthCeiledY - pFilter->iRemainder)/nBands + 1;
	int iBHeightY = iHeightY;

	int iWidthCeiledC = ceilTo(iWidthC,nBands,pFilter->iRemainder);
	int iHeightCeiledC = ceilTo(iHeightC,nBands,pFilter->iRemainder);
	int iBWidthC_up = (iWidthCeiledC - pFilter->iRemainder)/nBands + 1 ;
	int iBHeightC = iHeightC;

	int sgn = 1;
	int shift = pFilter->iInitialShift;
	for (int iBand=0; iBand<nBands; iBand++)
	{
		int iBWidthY = iBWidthY_up - !!shift;	//don't know how it would look like for 
		int iBWidthC = iBWidthC_up - !!shift;	//arbitrary nBands and shift
		int iBSizeY=iBWidthY*iBHeightY;
		int iBSizeC=iBWidthC*iBHeightC;

		step[iBand].iNumH = 1;
		step[iBand].iNumW = 1;
		step[iBand].iWidthY = iBWidthY;
		step[iBand].iHeightY = iBHeightY;
		step[iBand].iWidthC = iBWidthC;
		step[iBand].iHeightC = iBHeightC;
		if (ignoreMult)
			step[iBand].mult=mult;
		else
			step[iBand].mult=mult*pFilter->mult[iBand];

		step[iBand].dBandY = new double [iBSizeY];
		step[iBand].dBandCb = new double [iBSizeC];
		step[iBand].dBandCr = new double [iBSizeC];
		if (!step[iBand].dBandY || !step[iBand].dBandCb || !step[iBand].dBandCr)
			error(3, errno, __func__);

		memset(step[iBand].dBandY, 0, iBSizeY*sizeof(double));
		memset(step[iBand].dBandCb, 0, iBSizeC*sizeof(double));
		memset(step[iBand].dBandCr, 0, iBSizeC*sizeof(double));

		double *dBandYE = enhance(Y,true,pFilter->iEnhanceValue,pFilter->bOdd, 1, nBands, pFilter->iRemainder);
		double *dBandCbE = enhance(Cb,true,pFilter->iEnhanceValue,pFilter->bOdd, 1, nBands, pFilter->iRemainder);
		double *dBandCrE = enhance(Cr,true,pFilter->iEnhanceValue,pFilter->bOdd, 1, nBands, pFilter->iRemainder);
		if (!dBandYE || !dBandCbE || !dBandCrE)
			error(4, errno, __func__);
		double sum, *p;
		int iFWYE = iWidthCeiledY+2*pFilter->iEnhanceValue;
		int iFWCE = iWidthCeiledC+2*pFilter->iEnhanceValue;
		for (int j = 0; j < iBHeightY; j++)
			for (int i = 0; i < iBWidthY; i++)
			{
				p = dBandYE+iFWYE*j+i*nBands+pFilter->iEnhanceValue+shift;
				if (pFilter->bOdd)
				{
					sum = pFilter->pDFilters[iBand][0]*p[0];
					for (int l=1;l<pFilter->pDFilterLength[iBand];l++)
						sum+=pFilter->pDFilters[iBand][l]*(p[-l]*sgn+p[l]);
					
				}
				else
				{
					sum = 0;
					for (int l=0;l<pFilter->pDFilterLength[iBand];l++)
						sum+=pFilter->pDFilters[iBand][l]*(p[-(l+1)]*sgn+p[l]);
				}
				step[iBand].dBandY[iBWidthY*j+i]=sum;
			}
#ifndef SKIP_COLORS	
		for (int j = 0; j < iBHeightC; j++)
			for (int i = 0; i < iBWidthC; i++)
			{
				p = dBandCbE+iFWCE*j+i*nBands+pFilter->iEnhanceValue+shift;
				if (pFilter->bOdd)
				{
					sum = pFilter->pDFilters[iBand][0]*p[0];
					for (int l=1;l<pFilter->pDFilterLength[iBand];l++)
						sum+=pFilter->pDFilters[iBand][l]*(p[-l]*sgn+p[l]);
					
				}
				else
				{
					sum = 0;
					for (int l=0;l<pFilter->pDFilterLength[iBand];l++)
						sum+=pFilter->pDFilters[iBand][l]*(p[-(l+1)]*sgn+p[l]);
				}
				step[iBand].dBandCb[iBWidthC*j+i]=sum;
			}
		for (int j = 0; j < iBHeightC; j++)
			for (int i = 0; i < iBWidthC; i++)
			{
				p = dBandCrE+iFWCE*j+i*nBands+pFilter->iEnhanceValue+shift;
				if (pFilter->bOdd)
				{
					sum = pFilter->pDFilters[iBand][0]*p[0];
					for (int l=1;l<pFilter->pDFilterLength[iBand];l++)
						sum+=pFilter->pDFilters[iBand][l]*(p[-l]*sgn+p[l]);
					
				}
				else
				{
					sum = 0;
					for (int l=0;l<pFilter->pDFilterLength[iBand];l++)
						sum+=pFilter->pDFilters[iBand][l]*(p[-(l+1)]*sgn+p[l]);
				}
				step[iBand].dBandCr[iBWidthC*j+i]=sum;
			}
#endif
		delete[] dBandYE;
		delete[] dBandCbE;
		delete[] dBandCrE;
		dBandYE = dBandCrE = dBandCbE = nullptr;
		if ((nBands > 2) || (!pFilter->bOdd))
			sgn *= -1;
		shift = (shift + pFilter->iAlternateShift) % nBands;
		
	}
	
	iNumH = 1;
	iNumW = nBands;
	this->pFilter = pFilter;
	delete[] dBandY;
	delete[] dBandCr;
	delete[] dBandCb;
	dBandY = dBandCr = dBandCb = nullptr;
	return 0;
}

int decTree::analyseBandH(cFilter *pFilter, bool ignoreMult)
{
	if (!isUndivided())
		return 1;

	int nBands = pFilter->iNumBands;
	step = new decTree[nBands];
	if (!step)
		error(2, errno, __func__);
	
	/* analysis */
	int iWidthCeiledY = ceilTo(iWidthY,nBands,pFilter->iRemainder);
	int iHeightCeiledY = ceilTo(iHeightY,nBands,pFilter->iRemainder);
	int iBWidthY = iWidthY;
	int iBHeightY_up = (iHeightCeiledY - pFilter->iRemainder)/nBands + 1 ;

	int iWidthCeiledC = ceilTo(iWidthC,nBands,pFilter->iRemainder);
	int iHeightCeiledC = ceilTo(iHeightC,nBands,pFilter->iRemainder);
	int iBWidthC = iWidthC;
	int iBHeightC_up = (iHeightCeiledC - pFilter->iRemainder)/nBands + 1 ;

	int sgn = 1;
	int shift = pFilter->iInitialShift;
	for (int iBand=0; iBand<nBands; iBand++)
	{
		int iBHeightY = iBHeightY_up - !!shift;	//don't know how it would look like for 
		int iBHeightC = iBHeightC_up - !!shift;	//arbitrary nBands and shift
		int iBSizeY=iBWidthY*iBHeightY;
		int iBSizeC=iBWidthC*iBHeightC;

		step[iBand].iNumH = 1;
		step[iBand].iNumW = 1;
		step[iBand].iWidthY = iBWidthY;
		step[iBand].iHeightY = iBHeightY;
		step[iBand].iWidthC = iBWidthC;
		step[iBand].iHeightC = iBHeightC;
		if (ignoreMult)
			step[iBand].mult=mult;
		else
			step[iBand].mult=mult*pFilter->mult[iBand];

		step[iBand].dBandY = new double [iBSizeY];
		step[iBand].dBandCb = new double [iBSizeC];
		step[iBand].dBandCr = new double [iBSizeC];
		if (!step[iBand].dBandY || !step[iBand].dBandCb || !step[iBand].dBandCr)
			error(3, errno, __func__);

		memset(step[iBand].dBandY, 0, iBSizeY*sizeof(double));
		memset(step[iBand].dBandCb, 0, iBSizeC*sizeof(double));
		memset(step[iBand].dBandCr, 0, iBSizeC*sizeof(double));

		double *dBandYE = enhance(Y,false,pFilter->iEnhanceValue,pFilter->bOdd, 1, nBands, pFilter->iRemainder);
		double *dBandCbE = enhance(Cb,false,pFilter->iEnhanceValue,pFilter->bOdd, 1, nBands, pFilter->iRemainder);
		double *dBandCrE = enhance(Cr,false,pFilter->iEnhanceValue,pFilter->bOdd, 1, nBands, pFilter->iRemainder);
		if (!dBandYE || !dBandCbE || !dBandCrE)
			error(2, errno, __func__);
		double sum, *p;
		//int iFHYE = iHeightCeiledY+2*pFilter->iEnhanceValue;
		//int iFHCE = iHeightCeiledC+2*pFilter->iEnhanceValue;
		for (int j = 0; j < iBHeightY; j++)
			for (int i = 0; i < iBWidthY; i++)
			{
				p = dBandYE+iWidthY*(j*nBands+pFilter->iEnhanceValue+shift)+i;
				if (pFilter->bOdd)
				{
					sum = pFilter->pDFilters[iBand][0]*p[0];
					for (int l=1;l<pFilter->pDFilterLength[iBand];l++)
						sum+=pFilter->pDFilters[iBand][l]*(p[-l*iWidthY]*sgn+p[l*iWidthY]);
				}
				else
				{
					sum = 0;
					for (int l=0;l<pFilter->pDFilterLength[iBand];l++)
						sum+=pFilter->pDFilters[iBand][l]*(p[-(l+1)*iWidthY]*sgn+p[l*iWidthY]);
				}
				step[iBand].dBandY[iBWidthY*j+i]=sum;
			}
#ifndef SKIP_COLORS	
		for (int j = 0; j < iBHeightC; j++)
			for (int i = 0; i < iBWidthC; i++)
			{
				p = dBandCbE+iWidthC*(j*nBands+pFilter->iEnhanceValue+shift)+i;
				if (pFilter->bOdd)
				{
					sum = pFilter->pDFilters[iBand][0]*p[0];
					for (int l=1;l<pFilter->pDFilterLength[iBand];l++)
						sum+=pFilter->pDFilters[iBand][l]*(p[-l*iWidthC]*sgn+p[l*iWidthC]);
					
				}
				else
				{
					sum = 0;
					for (int l=0;l<pFilter->pDFilterLength[iBand];l++)
						sum+=pFilter->pDFilters[iBand][l]*(p[-(l+1)*iWidthC]*sgn+p[l*iWidthC]);
				}
				step[iBand].dBandCb[iBWidthC*j+i]=sum;
			}
		for (int j = 0; j < iBHeightC; j++)
			for (int i = 0; i < iBWidthC; i++)
			{
				p = dBandCrE+iWidthC*(j*nBands+pFilter->iEnhanceValue+shift)+i;
				if (pFilter->bOdd)
				{
					sum = pFilter->pDFilters[iBand][0]*p[0];
					for (int l=1;l<pFilter->pDFilterLength[iBand];l++)
						sum+=pFilter->pDFilters[iBand][l]*(p[-l*iWidthC]*sgn+p[l*iWidthC]);
					
				}
				else
				{
					sum = 0;
					for (int l=0;l<pFilter->pDFilterLength[iBand];l++)
						sum+=pFilter->pDFilters[iBand][l]*(p[-(l+1)*iWidthC]*sgn+p[l*iWidthC]);
				}
				step[iBand].dBandCr[iBWidthC*j+i]=sum;
			}
#endif
		delete[] dBandYE;
		delete[] dBandCbE;
		delete[] dBandCrE;
		dBandYE = dBandCrE = dBandCbE = nullptr;
		if ((nBands > 2) || (!pFilter->bOdd))
			sgn *= -1;
		shift = (shift + pFilter->iAlternateShift) % nBands;
		
	}
	
	iNumH = nBands;
	iNumW = 1;
	this->pFilter = pFilter;
	delete[] dBandY;
	delete[] dBandCr;
	delete[] dBandCb;
	dBandY = dBandCr = dBandCb = nullptr;
	return 0;
}

int decTree::analyseBandWH(cFilter *pFilter, bool ignoreMult)
{
	int nBands = pFilter->iNumBands;
	analyseBandW(pFilter, ignoreMult);
	for (int i=0; i<nBands; i++)
		stepAt(i,0)->analyseBandH(pFilter, ignoreMult);
	

	decTree *newstep = new decTree[nBands*nBands];
	if (!newstep)
		error(1, errno, __func__);
	for (int i=0; i<nBands*nBands; i++)
	{
		decTree *curstep = stepAt(i/nBands,0)->stepAt(0,i%nBands);
		int iWY = curstep->iWidthY;
		int iHY = curstep->iHeightY;
		int iWC = curstep->iWidthC;
		int iHC = curstep->iHeightC;
		newstep[i].iNumH=1;
		newstep[i].iNumW=1;
		newstep[i].iWidthY = iWY;
		newstep[i].iHeightY = iHY;
		newstep[i].iWidthC = iWC;
		newstep[i].iHeightC = iHC;
		newstep[i].dBandY=new double[iWY*iHY];
		newstep[i].dBandCb=new double[iWC*iHC];
		newstep[i].dBandCr=new double[iWC*iHC];
		if (!newstep[i].dBandY || !newstep[i].dBandCb || !newstep[i].dBandCr)
			error(2, errno, __func__);

		memcpy(newstep[i].dBandY, curstep->dBandY, iWY*iHY*sizeof(double));
		memcpy(newstep[i].dBandCb, curstep->dBandCb, iWC*iHC*sizeof(double));
		memcpy(newstep[i].dBandCr, curstep->dBandCr, iWC*iHC*sizeof(double));
		newstep[i].mult = curstep->mult;
	}

	delete[] step;
	step = newstep;
	iNumH = nBands;
	iNumW = nBands;
	this->pFilter = pFilter;

	return 0;
}

double *decTree::enhance(component comp, bool byW, int iEnhanceValue, bool bOdd, int sgn, int iNumBands, int iRemainder)
{
	double *dBand;
	int iWidth, iHeight;
	switch (comp) {
		case Y:
			dBand = dBandY;
			iWidth = iWidthY;
			iHeight = iHeightY;
			break;
		case Cr:
			dBand = dBandCr;
			iWidth = iWidthC;
			iHeight = iHeightC;
			break;
		case Cb:
			dBand = dBandCb;
			iWidth = iWidthC;
			iHeight = iHeightC;
			break;
	}
	
	int iSize = iWidth*iHeight;
	double * dBandE;

	if (byW)		// enhancement by width
	{
		int iWC = ceilTo(iWidth, iNumBands, iRemainder);
		int iFW = iWC + 2 * iEnhanceValue;
		int iSizeE = iFW * iHeight;
		dBandE = new double[iSizeE];
		if (!dBandE)
			return nullptr;
	
		for (int j=0; j < iHeight; j++)
			for (int i=0; i < iWidth; i++)
				dBandE[iEnhanceValue+iFW*j+i]=
					dBand[iWidth*j+i];

		// copy pixels
		// appears after ceiling to iNumBand
		for (int j = 0; j < iHeight; j++)
			for (int i = iWidth; i < iWC; i++)
				dBandE[iEnhanceValue+iFW*j+i] = 
					dBand[iWidth*j+iWidth-1];
		if (bOdd)
		{
				// left&right
			double *pd = dBandE;
			for (int j = 0; j < iHeight; j++, pd += iFW)
				for (int i = 0; i < iEnhanceValue; i++)
				{
					pd[i] = pd[2*iEnhanceValue - i] * sgn;
					pd[i + iEnhanceValue + iWC] = pd[iEnhanceValue + iWC - 2 - i] * sgn;
				}
		}
		else
		{
				// left&right
			double *pd = dBandE;
			for (int j = 0; j < iHeight; j++, pd += iFW)
				for (int i = 0; i < iEnhanceValue; i++)
				{
					pd[i] = pd[2*iEnhanceValue-1-i] * sgn;
					pd[i + iEnhanceValue + iWC] = pd[iEnhanceValue + iWC - 1 - i] * sgn;
				}
		}
	}
	else			//enhancement by height
	{
		int iHC = ceilTo(iHeight, iNumBands, iRemainder);
		int iFH = iHC + 2 * iEnhanceValue;
		int iSizeE = iFH * iWidth;
		dBandE = new double[iSizeE];
		if (!dBandE)
			return nullptr;
	
		for (int j=0; j < iHeight; j++)
			for (int i=0; i < iWidth; i++)
				dBandE[(iEnhanceValue+j)*iWidth+i]=
					dBand[iWidth*j+i];

		// copy pixels
		// appears after ceiling to iNumBand
		for (int j = iHeight; j < iHC; j++)
			for (int i = 0; i < iWidth; i++)
				dBandE[(iEnhanceValue+j)*iWidth+i] = 
					dBand[iWidth*(iHeight-1)+i-1];
		if (bOdd)
		{
			// enhancement : mirror relatively last pixel
			/**/
					// top
			double *pd1 = dBandE + iWidth*(iEnhanceValue - 1);
			double *pd2 = dBandE + iWidth*(iEnhanceValue + 1);
			for (int j = iEnhanceValue - 1; j >= 0; j--, pd1 -= iWidth, pd2 += iWidth)
				for (int i = 0; i < iWidth; i++)
					pd1[i] = pd2[i] * sgn;

					// bottom
			pd2 = dBandE + iWidth*(iEnhanceValue + iHC - 2);
			pd1 = pd2 + 2 * iWidth;
			for (int j = 0; j < iEnhanceValue; j++, pd1 += iWidth, pd2 -= iWidth)
				for (int i = 0; i < iWidth; i++)
					pd1[i] = pd2[i] * sgn;
		}
		else
		{
			// enhancement : mirror relatively bound
			/**/
					// top
			double *pd1 = dBandE + iWidth*(iEnhanceValue - 1);
			double *pd2 = dBandE + iWidth*iEnhanceValue;
			for (int j = iEnhanceValue - 1; j >= 0; j--, pd1 -= iWidth, pd2 += iWidth)
				for (int i = 0; i < iWidth; i++)
					pd1[i] = pd2[i] * sgn;

					// bottom
			pd2 = dBandE + iWidth*(iEnhanceValue + iHC - 1);
			pd1 = pd2 + iWidth;
			for (int j = 0; j < iEnhanceValue; j++, pd1 += iWidth, pd2 -= iWidth)
				for (int i = 0; i < iWidth; i++)
					pd1[i] = pd2[i] * sgn;
		}
	}
	return dBandE;
}

int decTree::synthesizeBandW()
{
	if (iNumW==1)
		return 0;

	int nBands = pFilter->iNumBands;
	int iEV = pFilter->iEnhanceValue;
	//int iSizeY=iWidthY*iHeightY;
	//int iSizeC=iWidthC*iHeightC;

	if (iNumW!=nBands)
		return 1;

	for (int j=0;j<iNumH;j++)
		for (int i=0;i<iNumW;i++)
			stepAt(i,j)->synthesizeBand();

	decTree *newstep = new decTree[iNumH];
	if (!newstep)
		error(2, errno, __func__);

	for (int k=0;k<iNumH;k++)
	{
		newstep[k].iNumH=1;
		newstep[k].iNumW=1;
		newstep[k].iWidthY = iWidthY;
		int iHeightYn =
			newstep[k].iHeightY = stepAt(0,k)->iHeightY;
		newstep[k].iWidthC = iWidthC;
		int iHeightCn =
			newstep[k].iHeightC = stepAt(0,k)->iHeightC;
		newstep[k].mult = mult*pFilter->mult[k];

		int iSizeYn = iWidthY*iHeightYn;
		int iSizeCn = iWidthC*iHeightCn;
		newstep[k].dBandY = new double[iSizeYn];
		newstep[k].dBandCb = new double[iSizeCn];
		newstep[k].dBandCr = new double[iSizeCn];
		if (!newstep[k].dBandY || !newstep[k].dBandCb || !newstep[k].dBandCr)
			error(3, errno, __func__);

		memset(newstep[k].dBandY,0,iSizeYn*sizeof(double));
		memset(newstep[k].dBandCb,0,iSizeCn*sizeof(double));
		memset(newstep[k].dBandCr,0,iSizeCn*sizeof(double));

		int iWidthYS = (stepAt(0,0)->iWidthY-1+!!pFilter->iInitialShift)*nBands+1;
		int iWidthCS = (stepAt(0,0)->iWidthC-1+!!pFilter->iInitialShift)*nBands+1;
		int iWidthYSE = iWidthYS + 2*iEV;
		int iWidthCSE = iWidthCS + 2*iEV;

		int iShift = pFilter->iInitialShift;
		int iSgn = 1;
		for (int iBand=0; iBand<nBands; iBand++)
		{
			decTree *pDTE = new decTree;
			if (!pDTE)
				error(4, errno,  __func__);
			pDTE->iNumH = 1;
			pDTE->iNumW = 1;
			pDTE->iWidthY = iWidthYS;
			pDTE->iHeightY = stepAt(iBand,k)->iHeightY;
			pDTE->iWidthC = iWidthCS;
			pDTE->iHeightC = stepAt(iBand,k)->iHeightC;
			pDTE->dBandY = stepAt(iBand,k)->sparse(Y,true, true, nBands, iShift);
			pDTE->dBandCb = stepAt(iBand,k)->sparse(Cb,true, true, nBands, iShift);
			pDTE->dBandCr = stepAt(iBand,k)->sparse(Cr,true, true, nBands, iShift);
			pDTE->mult = stepAt(iBand,k)->mult;

			/*if (iBand==0)
			{
				cImageYCbCr * pImage = pDTE->createImage(false);
				if (!pImage)
					return 1;
				pImage->setSubW(1);
				pImage->setSubH(1);
				cImageRGB * pOut = pImage->CreateRGB24FromYCbCr420();
				pOut->WriteToBitmapFile("Enhanced.bmp");
			}*/

			double *dBandYSE = pDTE->enhance(Y,true,iEV,true,iSgn,nBands,1);
			double *dBandCbSE = pDTE->enhance(Cb,true,iEV,true,iSgn,nBands,1);
			double *dBandCrSE = pDTE->enhance(Cr,true,iEV,true,iSgn,nBands,1);
			if (!dBandYSE || !dBandCbSE || !dBandCrSE)
				error(5, errno, __func__);
			double *p,sum;

			for (int j=0; j<iHeightYn; j++)
				for (int i=0; i<iWidthY; i++)
				{
					p=dBandYSE+iWidthYSE*j+iEV+i;
					if (pFilter->bOdd)
					{
						sum = pFilter->pIFilters[iBand][0]*p[0];
						for (int l=1;l<pFilter->pIFilterLength[iBand];l++)
							sum+=pFilter->pIFilters[iBand][l] * (p[-l]*iSgn + p[l]);
					}
					else
					{
						sum = 0.0;
						for (int l=0;l<pFilter->pIFilterLength[iBand];l++)
							sum+=pFilter->pIFilters[iBand][l] * (p[-l]*iSgn + p[l+1]);
					}
					newstep[k].dBandY[iWidthY*j+i]+=sum;
				}
#ifndef SKIP_COLORS
			for (int j=0; j<iHeightCn; j++)
				for (int i=0; i<iWidthC; i++)
				{
					p=dBandCbSE+iWidthCSE*j+iEV+i;
					if (pFilter->bOdd)
					{
						sum = pFilter->pIFilters[iBand][0]*p[0];
						for (int l=1;l<pFilter->pIFilterLength[iBand];l++)
							sum+=pFilter->pIFilters[iBand][l] * (p[-l]*iSgn + p[l]);
					}
					else
					{
						sum = 0.0;
						for (int l=0;l<pFilter->pIFilterLength[iBand];l++)
							sum+=pFilter->pIFilters[iBand][l] * (p[-l]*iSgn + p[l+1]);
					}
					newstep[k].dBandCb[iWidthC*j+i]+=sum;
				}

			for (int j=0; j<iHeightCn; j++)
				for (int i=0; i<iWidthC; i++)
				{
					p=dBandCrSE+iWidthCSE*j+iEV+i;
					if (pFilter->bOdd)
					{
						sum = pFilter->pIFilters[iBand][0]*p[0];
						for (int l=1;l<pFilter->pIFilterLength[iBand];l++)
							sum+=pFilter->pIFilters[iBand][l] * (p[-l]*iSgn + p[l]);
					}
					else
					{
						sum = 0.0;
						for (int l=0;l<pFilter->pIFilterLength[iBand];l++)
							sum+=pFilter->pIFilters[iBand][l] * (p[-l]*iSgn + p[l+1]);
					}
					newstep[k].dBandCr[iWidthC*j+i]+=sum;
				}
#endif
			iShift = (iShift + pFilter->iAlternateShift)%nBands;
			if (nBands>2 || !pFilter->bOdd) 	//TODO
				iSgn=-iSgn;
			
			delete[] dBandYSE;
			delete[] dBandCbSE;
			delete[] dBandCrSE;
			dBandYSE = dBandCrSE = dBandCbSE = nullptr;
			delete pDTE;
			pDTE = nullptr;
			}

	}

	// iNumH = iNumH;
	iNumW = 1;
	delete[] step;
	step = nullptr;
	if (iNumH==1)
	{
		step = nullptr;
		dBandY = newstep->dBandY;
		dBandCb = newstep->dBandCb;
		dBandCr = newstep->dBandCr;
		newstep->dBandY = nullptr;
		newstep->dBandCb = nullptr;
		newstep->dBandCr = nullptr;
		delete[] newstep;
		newstep = nullptr;
		pFilter = nullptr;
	}
	else
		step = newstep;
	
	return 0;
}

int decTree::synthesizeBandH()
{
	if (iNumH==1)
		return 0;

	int nBands = pFilter->iNumBands;
	int iEV = pFilter->iEnhanceValue;
	//int iSizeY=iWidthY*iHeightY;
	//int iSizeC=iWidthC*iHeightC;

	if (iNumH!=nBands)
		return 1;

	for (int j=0;j<iNumH;j++)
		for (int i=0;i<iNumW;i++)
			stepAt(i,j)->synthesizeBand();

	decTree *newstep = new decTree[iNumW];
	if (!newstep)
		error(2, errno, __func__);

	for (int k=0;k<iNumW;k++)
	{
		newstep[k].iNumH=1;
		newstep[k].iNumW=1;
		newstep[k].iHeightY = iHeightY;
		int iWidthYn =
			newstep[k].iWidthY = stepAt(k,0)->iWidthY;
		newstep[k].iHeightC = iHeightC;
		int iWidthCn =
			newstep[k].iWidthC = stepAt(k,0)->iWidthC;
		newstep[k].mult = mult*pFilter->mult[k];

		int iSizeYn = iWidthYn*iHeightY;
		int iSizeCn = iWidthCn*iHeightC;
		newstep[k].dBandY = new double[iSizeYn];
		newstep[k].dBandCb = new double[iSizeCn];
		newstep[k].dBandCr = new double[iSizeCn];
		if (!newstep[k].dBandY || !newstep[k].dBandCb || !newstep[k].dBandCr)
			error(3, errno, __func__);
		memset(newstep[k].dBandY,0,iSizeYn*sizeof(double));
		memset(newstep[k].dBandCb,0,iSizeCn*sizeof(double));
		memset(newstep[k].dBandCr,0,iSizeCn*sizeof(double));

		int iHeightYS = (stepAt(0,0)->iHeightY-1+!!pFilter->iInitialShift)*nBands+1;
		int iHeightCS = (stepAt(0,0)->iHeightC-1+!!pFilter->iInitialShift)*nBands+1;
		int iHeightYSE = iHeightYS + 2*iEV;
		int iHeightCSE = iHeightCS + 2*iEV;

		int iShift = pFilter->iInitialShift;
		int iSgn = 1;
		for (int iBand=0; iBand<nBands; iBand++)
		{
			decTree *pDTE = new decTree;
			if (!pDTE)
				error(4, errno, __func__);
			pDTE->iNumH = 1;
			pDTE->iNumW = 1;
			pDTE->iHeightY = iHeightYS;
			pDTE->iWidthY = stepAt(k,iBand)->iWidthY;
			pDTE->iHeightC = iHeightCS;
			pDTE->iWidthC = stepAt(k,iBand)->iWidthC;
			pDTE->dBandY = stepAt(k,iBand)->sparse(Y,false, true, nBands, iShift);
			pDTE->dBandCb = stepAt(k,iBand)->sparse(Cb,false, true, nBands, iShift);
			pDTE->dBandCr = stepAt(k,iBand)->sparse(Cr,false, true, nBands, iShift);
			pDTE->mult = stepAt(k,iBand)->mult;

		/*	if (iBand==0)
			{
				cImageYCbCr * pImage = pDTE->createImage(false);
				if (!pImage)
					return 1;
				pImage->setSubW(1);
				pImage->setSubH(1);
				cImageRGB * pOut = pImage->CreateRGB24FromYCbCr420();
				pOut->WriteToBitmapFile("Enhanced.bmp");
			}
		*/
			double *dBandYSE = pDTE->enhance(Y,false,iEV,true,iSgn,nBands,1);
			double *dBandCbSE = pDTE->enhance(Cb,false,iEV,true,iSgn,nBands,1);
			double *dBandCrSE = pDTE->enhance(Cr,false,iEV,true,iSgn,nBands,1);
			if (!dBandYSE || !dBandCbSE || !dBandCrSE)
				error(5, errno, __func__);

			double *p,sum;

			for (int j=0; j<iHeightY; j++)
				for (int i=0; i<iWidthYn; i++)
				{
					p=dBandYSE+iWidthYn*(j+iEV)+i;
					if (pFilter->bOdd)
					{
						sum = pFilter->pIFilters[iBand][0]*p[0];
						for (int l=1;l<pFilter->pIFilterLength[iBand];l++)
							sum+=pFilter->pIFilters[iBand][l] * (p[-l*iWidthYn]*iSgn + p[l*iWidthYn]);
					}
					else
					{
						sum = 0.0;
						for (int l=0;l<pFilter->pIFilterLength[iBand];l++)
							sum+=pFilter->pIFilters[iBand][l] * (p[-l*iWidthYn]*iSgn + p[(l+1)*iWidthYn]);
					}
					newstep[k].dBandY[iWidthYn*j+i]+=sum;
				}
#ifndef SKIP_COLORS
			for (int j=0; j<iHeightC; j++)
				for (int i=0; i<iWidthCn; i++)
				{
					p=dBandCbSE+iWidthCn*(j+iEV)+i;
					if (pFilter->bOdd)
					{
						sum = pFilter->pIFilters[iBand][0]*p[0];
						for (int l=1;l<pFilter->pIFilterLength[iBand];l++)
							sum+=pFilter->pIFilters[iBand][l] * (p[-l*iWidthCn]*iSgn + p[l*iWidthCn]);
					}
					else
					{
						sum = 0.0;
						for (int l=0;l<pFilter->pIFilterLength[iBand];l++)
							sum+=pFilter->pIFilters[iBand][l] * (p[-l*iWidthCn]*iSgn + p[(l+1)*iWidthCn]);
					}
					newstep[k].dBandCb[iWidthCn*j+i]+=sum;
				}

			for (int j=0; j<iHeightC; j++)
				for (int i=0; i<iWidthCn; i++)
				{
					p=dBandCrSE+iWidthCn*(j+iEV)+i;
					if (pFilter->bOdd)
					{
						sum = pFilter->pIFilters[iBand][0]*p[0];
						for (int l=1;l<pFilter->pIFilterLength[iBand];l++)
							sum+=pFilter->pIFilters[iBand][l] * (p[-l*iWidthCn]*iSgn + p[l*iWidthCn]);
					}
					else
					{
						sum = 0.0;
						for (int l=0;l<pFilter->pIFilterLength[iBand];l++)
							sum+=pFilter->pIFilters[iBand][l] * (p[-l*iWidthCn]*iSgn + p[(l+1)*iWidthCn]);
					}
					newstep[k].dBandCr[iWidthCn*j+i]+=sum;
				}
#endif
			iShift = (iShift + pFilter->iAlternateShift)%nBands;
			if (nBands>2 || !pFilter->bOdd) 	//TODO
				iSgn=-iSgn;
			
			delete[] dBandYSE;
		   	delete[] dBandCbSE;
			delete[] dBandCrSE;
			dBandYSE = dBandCrSE = dBandCbSE = nullptr;
			delete pDTE;
			pDTE = nullptr;
			}

	}

	// iNumW = iNumW;
	iNumH = 1;
	delete[] step;
	if (iNumW==1)
	{
		step = nullptr;
		dBandY = newstep->dBandY;
		dBandCb = newstep->dBandCb;
		dBandCr = newstep->dBandCr;
		newstep->dBandY = nullptr;
		newstep->dBandCb = nullptr;
		newstep->dBandCr = nullptr;
		delete[] newstep;
		newstep = nullptr;
		pFilter = nullptr;
	}
	else
		step = newstep;
	
	return 0;
}

int decTree::synthesizeBand()
{
	int i = synthesizeBandH();
	if (i)
		return i;
	return synthesizeBandW();
}

double * decTree::sparse(component comp, bool byW, bool bOdd, int iNumBands, int iShift)
{
	double *dBand;
	int iWidth, iHeight;
	switch (comp) {
		case Y:
			dBand = dBandY;
			iWidth = iWidthY;
			iHeight = iHeightY;
			break;
		case Cr:
			dBand = dBandCr;
			iWidth = iWidthC;
			iHeight = iHeightC;
			break;
		case Cb:
			dBand = dBandCb;
			iWidth = iWidthC;
			iHeight = iHeightC;
			break;
	}
	
	//int iSize = iWidth*iHeight;
	double * dBandS;

	if (byW)
	{
		int iWidthE = (iWidth-1+!!iShift)*iNumBands+1;
		int iSizeE = iWidthE * iHeight;
		dBandS = new double[iSizeE];
		if (!dBandS)
			return nullptr;
		memset(dBandS,0,iSizeE*sizeof(double));

		for (int j=0; j<iHeight; j++)
			for (int i=0; i<iWidth; i++)
				dBandS[j*iWidthE+i*iNumBands+iShift]=dBand[j*iWidth+i];
	}
	else
	{
		int iHeightE = (iHeight-1+!!iShift)*iNumBands+1;
		int iSizeE = iWidth * iHeightE;
		dBandS = new double[iSizeE];
		if (!dBandS)
			return nullptr;
		memset(dBandS,0,iSizeE*sizeof(double));

		for (int j=0; j<iHeight; j++)
			for (int i=0; i<iWidth; i++)
				dBandS[(j*iNumBands+iShift)*iWidth+i]=dBand[j*iWidth+i];
	}
	return dBandS;	
}


short * decTree::getSubCoef(double extension, component comp, int filenum, int * sub_width, int * sub_height)
{
	if (isEmpty())
		return nullptr;

	char * char_comp;
	int iWidth, iHeight;
	double * dBand;

	switch (comp) {
	case Cr:
		char_comp = (char*)"Cr";
		iHeight = iHeightC;
		iWidth = iWidthC;
		dBand = dBandCr;
		break;
	case Cb:
		char_comp = (char*)"Cb";
		iHeight = iHeightC;
		iWidth = iWidthC;
		dBand = dBandCb;
		break;
	case 0:
	default:
		char_comp = (char*)"Y";
		iHeight = iHeightY;
		iWidth = iWidthY;
		dBand = dBandY;
		break;
	}

#ifdef PRINT_TEST_FILES
	const char dir_name[] = "files/";
	FILE * fileSub;
	char FileName[700];
	sprintf(FileName, "%sfileSub_%s_%d_orig.txt", dir_name, char_comp, filenum);
	//fopen_s(&fileSub, FileName, "w");
	fileSub = fopen(FileName, "w");
#endif

	*sub_width = iWidth;
	*sub_height = iHeight;

	short *subCoeff = new short[iHeight*iWidth];
	if (!subCoeff)
		return nullptr;
	memset(subCoeff, 0, iHeight*iWidth * sizeof(short));

	//double delta = deltacoef/extension;
	for (int j = 0; j < iHeight; j++) {
		for (int i = 0; i < iWidth; i++) 
		{
			double d=dBand[i+j*iWidth];
			subCoeff[i + j * iWidth] = (short)Limiting_a_b(dBand[i + j * iWidth]*extension,SHRT_MIN,SHRT_MAX);
			//subCoeff[i + j * iWidth] = (short)Limiting_a_b( d>delta/2 ? floor((d-delta/2)*extension)+1 : 
			//		d<-delta/2 ? ceil((d+delta/2)*extension)-1 : 0, SHRT_MIN, SHRT_MAX);
			if (subCoeff[i + j * iWidth]==SHRT_MAX || subCoeff[i + j * iWidth]==SHRT_MIN)
				cout << "Get : " << char_comp << " clipping !" << endl;
			#ifdef PRINT_TEST_FILES
			//fprintf(fileSub, "%d ", subCoeff[i + j * iWidth]);
			fprintf(fileSub, "%f ", d);
			#endif
		}
		#ifdef PRINT_TEST_FILES
		fprintf(fileSub, "\n");
		#endif	
	}

	#ifdef PRINT_TEST_FILES
	fclose(fileSub);
	#endif

	return subCoeff;

}

int decTree::countBands()
{
	if (isEmpty())
		return 0;

	if (isUndivided())
		return 1;

	int ret = 0;
	for (int i=0; i<iNumW; i++)
		for (int j=0; j<iNumH; j++)
			ret += stepAt(i,j)->countBands();

	return ret;
}

//Places array pointers to coeffs for component comp into bandnum point of array dest. 
//Quantisation for non-LL bands is set by extension.
//Filenum is the prefix for test files numbers
int decTree::getAllCoefs(short ** dest, int * sub_width, int * sub_height, double extension, component comp, int bandnum)
{
	
	if (isEmpty())
		return 0;

	if (isUndivided())
	{
		//dest[bandnum]=getSubCoef(bandnum?extension:1, comp, bandnum, &sub_width[bandnum], &sub_height[bandnum]); //KB: was changed for LL quantization
		dest[bandnum]=getSubCoef(extension, comp, bandnum, &sub_width[bandnum], &sub_height[bandnum]);
		return 1;
	}

	int ret = 0;
	for (int i=0; i<iNumW; i++)
		for (int j=0; j<iNumH; j++)
		{
			int f = stepAt(i,j)->getAllCoefs(dest, sub_width, sub_height, extension, comp, bandnum);
			if (!f)
				return 0; //some band was empty
			ret += f;
			bandnum += f;
		}
	return ret;
}

void decTree::copyStructure(decTree * dest)
{
	if (!dest)
		return;

	decTree* pNew = dest;	

	pNew->iNumW = iNumW;
	pNew->iNumH = iNumH;
	pNew->iWidthY = iWidthY;
	pNew->iHeightY = iHeightY;
	pNew->iWidthC = iWidthC;
	pNew->iHeightC = iHeightC;
	pNew->mult = mult;

	pNew->pFilter = pFilter;
	if (isUndivided())
	{
		pNew->step = nullptr;
		pNew->dBandY = new double[iWidthY*iHeightY];
		pNew->dBandCr = new double[iWidthC*iHeightC];
		pNew->dBandCb = new double[iWidthC*iHeightC];
		if (!pNew->dBandY || !pNew->dBandCr || !pNew->dBandCb)
		{
			perror(__func__);
			return;
		}
		memset(pNew->dBandY,0,iWidthY*iHeightY*sizeof(double));
		memset(pNew->dBandCr,0,iWidthC*iHeightC*sizeof(double));
		memset(pNew->dBandCb,0,iWidthC*iHeightC*sizeof(double));
	}
	else
	{
		pNew->step = new decTree[iNumW*iNumH];
		for (int i=0; i<iNumW; i++)
			for (int j=0; j<iNumH; j++)
				stepAt(i,j)->copyStructure(&(pNew->step[i*iNumH+j]));
	}
}

void decTree::copyData(decTree * dest)
{
	if (!dest)
		return;

	if (isUndivided())
	{
		memcpy(dest->dBandY, dBandY, iWidthY*iHeightY*sizeof(double));
		memcpy(dest->dBandCr, dBandCr, iWidthC*iHeightC*sizeof(double));
		memcpy(dest->dBandCb, dBandCb, iWidthC*iHeightC*sizeof(double));
	}
	else
		for (int i=0; i<iNumW; i++)
			for (int j=0; j<iNumH; j++)
				stepAt(i,j)->copyData(&dest->step[i*iNumH+j]);
}

void decTree::copyTree(decTree * dest)
{
	if (!dest)
		return;

	copyStructure(dest);
	copyData(dest);
}
/*
void decTree::setSubCoef(short * subCoeff, int coeff_pred_ln, short * ref_sampl, int ref_sampl_ln, double extension, component comp, int stage, int filenum)
{
	if (isEmpty())
		return nullptr;

	char * char_comp, *char_stage;
	int iWidth, iHeight;
	double * dBand;

	switch (comp) {
	case Cr:
		char_comp = (char*)"Cr";
		iHeight = iHeightC;
		iWidth = iWidthC;
		dBand = dBandCr;
		break;
	case Cb:
		char_comp = (char*)"Cb";
		iHeight = iHeightC;
		iWidth = iWidthC;
		dBand = dBandCb;
		break;
	case Y:
	default:
		char_comp = (char*)"Y";
		iHeight = iHeightY;
		iWidth = iWidthY;
		dBand = dBandY;
		break;
	}

	switch (stage) {
	case 1:
		char_stage = (char*)"pred";
		break;
	case 2:
	default:
		char_stage = (char*)"recon";
		break;
	}

#ifdef PRINT_TEST_FILES
	FILE * fileSub;
	char FileName[700];
	sprintf(FileName, "fileSub_%s_%d_%s.txt", char_comp, filenum, char_stage);
	//fopen_s(&fileSub, FileName, "w");
	fileSub = fopen(FileName, "w");
#endif

	if ((coeff_pred_ln && ref_sampl && ref_sampl_ln)){
		if (ref_sampl_ln == iHeight + iWidth - 1) {
			for (int i = 0; i < iWidth; i++){
				dBand[i] = (double)ref_sampl[i]/extension;
				#ifdef PRINT_TEST_FILES
				fprintf(fileSub, "%d ", ref_sampl[i]);
				#endif
			}

			for (int j = 0; j < iHeight - 1; j++) {
				dBand[(j+1)*iWidth] = (double)ref_sampl[j + iWidth]/extension;
				#ifdef PRINT_TEST_FILES
				fprintf(fileSub, "\n%d ", ref_sampl[j + iWidth]);
				#endif
				for (int i = 0; i < iWidth - 1; i++) {
					dBand[i + 1 + (j+1)*iWidth] = (double)subCoeff[i + j*(iWidth - 1)]/extension;
					#ifdef PRINT_TEST_FILES
					fprintf(fileSub, "%d ", subCoeff[i + j*(iWidth - 1)]);
					#endif
				}
			}
		} else if (ref_sampl_ln == iHeight) {
			
			for (int j = 0; j < iHeight; j++) {
				dBand[j*iWidth] = (double)ref_sampl[j]/extension;
				#ifdef PRINT_TEST_FILES
				fprintf(fileSub, "%d ", ref_sampl[j]);
				#endif
				for (int i = 0; i < iWidth - 1; i++) {
					dBand[i + 1 + j*iWidth] = (double)subCoeff[i + j*(iWidth - 1)]/extension;
					#ifdef PRINT_TEST_FILES
					fprintf(fileSub, "%d ", subCoeff[i + j*(iWidth - 1)]);
					#endif
				}
				#ifdef PRINT_TEST_FILES
				fprintf(fileSub, "\n");
				#endif
			}
		} else {
			for (int i = 0; i < iWidth; i++){
				dBand[i] = (double)ref_sampl[i]/extension;
				#ifdef PRINT_TEST_FILES
				fprintf(fileSub, "%d ", ref_sampl[i]);
				#endif
			}

			for (int j = 0; j < iHeight - 1; j++) {
				#ifdef PRINT_TEST_FILES
				fprintf(fileSub, "\n");
				#endif
				for (int i = 0; i < iWidth; i++) {
					dBand[i + (j+1)*iWidth] = (double)subCoeff[i + j*iWidth]/extension;
					#ifdef PRINT_TEST_FILES
					fprintf(fileSub, "%d ", subCoeff[i + j*iWidth]);
					#endif
				}
			}
		}
	} else {

		for (int j = 0; j < iHeight; j++) {
			for (int i = 0; i < iWidth; i++) 
			{
				dBand[i + j*iWidth] = (double)subCoeff[i + j*iWidth]/extension;
				#ifdef PRINT_TEST_FILES
				fprintf(fileSub, "%d ", subCoeff[i + j * iWidth]);
				#endif
			}
			#ifdef PRINT_TEST_FILES
			fprintf(fileSub, "\n");
			#endif	
		}

	}

	#ifdef PRINT_TEST_FILES
	fclose(fileSub);
	#endif

}
*/
/*
int decTree::setAllCoefs(short ** subCoeff, int * coeff_pred_ln, short ** ref_sampl, int * ref_sampl_ln,  double extension, component comp, int stage, int bandnum)
{
	
	if (isEmpty())
		return 0;

	if (isUndivided())
	{
		//setSubCoef(subCoeff[bandnum],bandnum?extension:1, comp, stage, bandnum); //KB: was changed for LL quantization
		if (coeff_pred_ln && ref_sampl && ref_sampl_ln) {
			setSubCoef(subCoeff[bandnum], coeff_pred_ln[bandnum], ref_sampl[bandnum], ref_sampl_ln[bandnum],extension, comp, stage, bandnum);
		} else {
			setSubCoef(subCoeff[bandnum], NULL, NULL, NULL, extension, comp, stage, bandnum);
		}
		return 1;
	}

	int ret = 0;
	for (int i=0; i<iNumW; i++)
		for (int j=0; j<iNumH; j++)
		{
			int f = stepAt(i,j)->setAllCoefs(subCoeff, coeff_pred_ln, ref_sampl, ref_sampl_ln, extension, comp, stage, bandnum);
			if (!f)
				return 0; //some band was empty
			ret += f;
			bandnum += f;
		}
	return ret;
}

*/

void decTree::setSubCoef(short * subCoeff, double extension, component comp, int stage, int filenum)
{
	if (isEmpty())
		return;

	char * char_comp, *char_stage;
	int iWidth, iHeight;
	double * dBand;

	switch (comp) {
	case Cr:
		char_comp = (char*)"Cr";
		iHeight = iHeightC;
		iWidth = iWidthC;
		dBand = dBandCr;
		break;
	case Cb:
		char_comp = (char*)"Cb";
		iHeight = iHeightC;
		iWidth = iWidthC;
		dBand = dBandCb;
		break;
	case Y:
	default:
		char_comp = (char*)"Y";
		iHeight = iHeightY;
		iWidth = iWidthY;
		dBand = dBandY;
		break;
	}

	switch (stage) {
	case 1:
		char_stage = (char*)"pred";
		break;
	case 2:
	default:
		char_stage = (char*)"recon";
		break;
	}

#ifdef PRINT_TEST_FILES
	const char dir_name[] = "files/";
	FILE * fileSub;
	char FileName[700];
	sprintf(FileName, "%sfileSub_%s_%d_%s.txt", dir_name, char_comp, filenum, char_stage);
	//fopen_s(&fileSub, FileName, "w");
	fileSub = fopen(FileName, "w");
#endif

	double eps = 1/extension;
	//double delta = deltacoef * eps;
	for (int j = 0; j < iHeight; j++) {
		for (int i = 0; i < iWidth; i++) 
		{
			short s=subCoeff[i+j*iWidth];
			dBand[i + j*iWidth] = (double)subCoeff[i + j*iWidth]/extension;
			//dBand[i + j*iWidth] = s>0 ? (double)s*eps+(delta/2-eps/2) :
			//	s<0 ? (double)s*eps-(delta/2-eps/2): 0.0;
			if (subCoeff[i + j * iWidth]==SHRT_MAX || subCoeff[i + j * iWidth]==SHRT_MIN)
				cout << "Set : " << char_comp << " clipping !" << endl;
			#ifdef PRINT_TEST_FILES
			fprintf(fileSub, "%d ", subCoeff[i + j * iWidth]);
			#endif
		}
		#ifdef PRINT_TEST_FILES
		fprintf(fileSub, "\n");
		#endif	
	}

	#ifdef PRINT_TEST_FILES
	fclose(fileSub);
	#endif

}

int decTree::setAllCoefs(short ** subCoeff, double extension, component comp, int stage, int bandnum)
{
	
	if (isEmpty())
		return 0;

	if (isUndivided())
	{
		//setSubCoef(subCoeff[bandnum],bandnum?extension:1, comp, stage, bandnum); //KB: was changed for LL quantization
		setSubCoef(subCoeff[bandnum],extension, comp, stage, bandnum);
		return 1;
	}

	int ret = 0;
	for (int i=0; i<iNumW; i++)
		for (int j=0; j<iNumH; j++)
		{
			int f = stepAt(i,j)->setAllCoefs(subCoeff, extension, comp, stage, bandnum);
			if (!f)
				return 0; //some band was empty
			ret += f;
			bandnum += f;
		}
	return ret;
}

double decTree::getSubPSNR(decTree *pA, decTree *pB, component comp)
{
	if (pA->isEmpty() || pB->isEmpty() ||
			!pA->isUndivided() || !pB->isUndivided() ||
			pA->iWidthY!=pB->iWidthY || pA->iHeightY!=pB->iHeightY ||
			pA->iWidthC!=pB->iWidthC || pA->iHeightC!=pB->iHeightC)
		return -1.0;
	double * dBandA, *dBandB;
	int iHeight, iWidth;

	switch (comp) {
	case Cr:
		iHeight = pA->iHeightC;
		iWidth = pA->iWidthC;
		dBandA = pA->dBandCr;
		dBandB = pB->dBandCr;
		break;
	case Cb:
		iHeight = pA->iHeightC;
		iWidth = pA->iWidthC;
		dBandA = pA->dBandCb;
		dBandB = pB->dBandCb;
		break;
	case Y:
	default:
		iHeight = pA->iHeightY;
		iWidth = pA->iWidthY;
		dBandA = pA->dBandY;
		dBandB = pB->dBandY;
		break;
	}

	double sum=0.0, val;
	int iSize = iHeight * iWidth;
	for (int i=0; i<iSize; i++)
	{
		val = dBandB[i] - dBandA[i];
		sum += val*val;
	}

	return 20.0*log10(UCHAR_MAX/sqrt(sum/iSize));
}

int decTree::getAllPSNR(double * dest, decTree *pA, decTree *pB, component comp, int bandnum)
{
	if (pA->isEmpty() || pB->isEmpty() || 
			pA->iNumW!=pB->iNumW || pA->iNumH!=pB->iNumH)
		return 0;

	if (pA->isUndivided())
	{
		dest[bandnum]=getSubPSNR(pA, pB, comp);
		if (dest[bandnum]==-1.0)
			return 0;
		else
			return 1;
	}

	int ret = 0;
	for (int i=0; i<pA->iNumW; i++)
		for (int j=0; j<pA->iNumH; j++)
		{
			int f = getAllPSNR(dest, pA->stepAt(i,j), pB->stepAt(i,j), comp, bandnum);
			if (!f)
				return 0; // some bands were empty
					  // or non-consistent
			ret += f;
			bandnum += f;
		}
	return ret;
}

double decTree::getSubEnergy(decTree *pA, component comp, bool bShift)
{
	if (pA->isEmpty() || !pA->isUndivided())
		return -1.0;
	double * dBandA;
	int iHeight, iWidth;

	switch (comp) {
	case Cr:
		iHeight = pA->iHeightC;
		iWidth = pA->iWidthC;
		dBandA = pA->dBandCr;
		break;
	case Cb:
		iHeight = pA->iHeightC;
		iWidth = pA->iWidthC;
		dBandA = pA->dBandCb;
		break;
	case Y:
	default:
		iHeight = pA->iHeightY;
		iWidth = pA->iWidthY;
		dBandA = pA->dBandY;
		break;
	}

	double mean=0.0, sum = 0.0, val;
	int iSize = iHeight * iWidth;
	for (int i=0; i<iSize; i++)
	{
		val = dBandA[i];
		mean += val;
	}
	mean /= iSize;
	for (int i=0; i<iSize; i++)
	{
		val = dBandA[i]-(bShift?mean:0.0);
		sum += val*val;
	}

	//return sum/iSize;
	return sum;
}

int decTree::getAllEnergy(double * dest, decTree *pA, component comp, int bandnum, bool bShift)
{
	if (!dest)
		return -1;

	if (pA->isEmpty())
		return 0;

	if (pA->isUndivided())
	{
		dest[bandnum]=getSubEnergy(pA, comp, bShift);
		if (dest[bandnum]==-1.0)
			return 0;
		else
			return 1;
	}

	int ret = 0;
	for (int i=0; i<pA->iNumW; i++)
		for (int j=0; j<pA->iNumH; j++)
		{
			int f = getAllEnergy(dest, pA->stepAt(i,j), comp, bandnum, bShift && !i && !j);
			if (!f)
				return 0; // some bands were empty
					  // or non-consistent
			ret += f;
			bandnum += f;
		}
	return ret;
}

int decTree::getAllNames(char ** dest, decTree *pA, int bandnum, char * current_name)
{
	if (!dest)
		return -1;

	if (pA->isEmpty())
		return 0;

	if (pA->isUndivided())
	{
		dest[bandnum]=new char[128];
		if (!dest[bandnum])
			return 0;
		sprintf(dest[bandnum],"%s",(current_name?current_name:""));
		return 1;
	}

	int ret = 0;
	for (int i=0; i<pA->iNumW; i++)
		for (int j=0; j<pA->iNumH; j++)
		{
			char new_name[128];
			sprintf(new_name, "%s(%d,%d)",(current_name?current_name:""),i,j);
			int f = getAllNames(dest, pA->stepAt(i,j), bandnum, new_name);
			if (!f)
				return 0; // some bands were empty
					  // or non-consistent
			ret += f;
			bandnum += f;
		}
	return ret;
}

//Splits tree into abs and sign and places the latest 
//into an object for return
//elements with module no larger than epsilon are considered zeros
decTree * decTree::extractSgn(double epsilon)
{
	decTree * dest = new decTree;
	copyStructure(dest);

	extractSgn_it(dest, epsilon);
	return dest;
}

void decTree::extractSgn_it(decTree * dest, double epsilon)
{
	if (isUndivided())
	{
		for (int i=0; i<iWidthY*iHeightY; i++)
		{
			dest->dBandY[i]=(dBandY[i]>epsilon) ? 1:
					(dBandY[i]<-epsilon) ?-1: 0;
			dBandY[i] = dBandY[i] * dest->dBandY[i];
		}
		for (int i=0; i<iWidthC*iHeightC; i++)
		{
			dest->dBandCr[i]=(dBandCr[i]>epsilon) ? 1:
					 (dBandCr[i]<-epsilon) ?-1: 0;
			dBandCr[i] = dBandCr[i] * dest->dBandCr[i];
		}
		for (int i=0; i<iWidthC*iHeightC; i++)
		{
			dest->dBandCb[i]=(dBandCb[i]>epsilon) ? 1:
				         (dBandCb[i]<-epsilon) ?-1: 0;
			dBandCb[i] = dBandCb[i] * dest->dBandCb[i];
		}
	}
	else
		for (int i=0; i<iNumW; i++)
			for (int j=0; j<iNumH; j++)
				stepAt(i,j)->extractSgn_it(dest->stepAt(i,j), epsilon);
}

void decTree::addSgn(decTree * src)
{
	if (isEmpty() || src->isEmpty() || 
			iNumW!=src->iNumW || iNumH!=src->iNumH)
	{
		cout << "Non-consistent trees to unite\n";
		return;
	}

	if (isUndivided())
	{
		for (int i=0; i<iWidthY*iHeightY; i++)
			dBandY[i] = dBandY[i] * src->dBandY[i];
		for (int i=0; i<iWidthC*iHeightC; i++)
			dBandCr[i] = dBandCr[i] * src->dBandCr[i];
		for (int i=0; i<iWidthC*iHeightC; i++)
			dBandCb[i] = dBandCb[i] * src->dBandCb[i];
	}
	else
		for (int i=0; i<iNumW; i++)
			for (int j=0; j<iNumH; j++)
				stepAt(i,j)->addSgn(src->stepAt(i,j));
}

decTree * decTree::substractTree(decTree * pSub)
{
	decTree * ret = new decTree;
	copyTree(ret);

	ret->substractTree_it(pSub);
	return ret;
}

void decTree::substractTree_it(decTree * pSub)
{
	if (isEmpty() || pSub->isEmpty() || 
			iNumW!=pSub->iNumW || iNumH!=pSub->iNumH)
	{
		cout << "Non-consistent trees to substract\n";
		return;
	}

	if (isUndivided())
	{
		for (int i=0; i<iWidthY*iHeightY; i++)
			dBandY[i] = dBandY[i] - pSub->dBandY[i];
		for (int i=0; i<iWidthC*iHeightC; i++)
			dBandCr[i] = dBandCr[i] - pSub->dBandCr[i];
		for (int i=0; i<iWidthC*iHeightC; i++)
			dBandCb[i] = dBandCb[i] - pSub->dBandCb[i];
	}
	else
		for (int i=0; i<iNumW; i++)
			for (int j=0; j<iNumH; j++)
				stepAt(i,j)->substractTree_it(pSub->stepAt(i,j));
}

void decTree::setMult(double mult)
{
	this->mult = mult;
}

double decTree::getMult()
{
	return mult;
}

#include "cImageYCbCr.h"



cImageYCbCr::cImageYCbCr()
	: iFullWidth(0)
	, iFullHeight(0)
	, iFullWidth2(0)
	, iFullHeight2(0)
	, iEnhanceValue(0)
	, pY0(nullptr)						// null when error
	, pCr0(nullptr)
	, pCb0(nullptr)
	, iSubW(0)
	, iSubH(0)
{
}


cImageYCbCr::~cImageYCbCr()
{
	if (pY0)
		delete[] pY0;
	if (pCr0)
		delete[] pCr0;
	if (pCb0)
		delete[] pCb0;
}

int cImageYCbCr::ceilTo(int number, int base, int remainder)
{
	return (number + base - 1 - remainder) / base*base + remainder;
}

cImageYCbCr * cImageYCbCr::copy()
{
	if (!pY0 || !pCr0 || !pCb0)
		return nullptr;

	cImageYCbCr *pCopy = new cImageYCbCr;
	pCopy->setEnhanceValue(iEnhanceValue);
	pCopy->setFullWidth(iFullWidth - 2 * iEnhanceValue + 2 * iEnhanceValue);
	pCopy->setFullHeight(iFullHeight - 2 * iEnhanceValue + 2 * iEnhanceValue);
	pCopy->setFullWidth2(iFullWidth2 - 2 * iEnhanceValue + 2 * iEnhanceValue);
	pCopy->setFullHeight2(iFullHeight2 - 2 * iEnhanceValue + 2 * iEnhanceValue);
	pCopy->setSubW(iSubW);
	pCopy->setSubH(iSubH);
	pCopy->setGrey();

	memcpy(pCopy->pY0, pY0, iFullWidth*iFullHeight);
	memcpy(pCopy->pCr0, pCr0, iFullWidth2*iFullHeight2);
	memcpy(pCopy->pCb0, pCb0, iFullWidth2*iFullHeight2);
	
	return pCopy;
}

void cImageYCbCr::setFullWidth(int val)
{
	iFullWidth = val;
}

void cImageYCbCr::setFullHeight(int val)
{
	iFullHeight = val;
}

void cImageYCbCr::setFullWidth2(int val)
{
	iFullWidth2 = val;
}

void cImageYCbCr::setFullHeight2(int val)
{
	iFullHeight2 = val;
}

void cImageYCbCr::setEnhanceValue(int val)
{
	iEnhanceValue = val;
}

void cImageYCbCr::setSubW(int val)
{
	iSubW = val;
}

void cImageYCbCr::setSubH(int val)
{
	iSubH = val;
}

void cImageYCbCr::setpY0(unsigned char * val)
{
	if (pY0)
		delete pY0;
	pY0 = val;
}

void cImageYCbCr::setpCb0(unsigned char * val)
{
	if (pCb0)
		delete pCb0;
	pCb0 = val;
}

void cImageYCbCr::setpCr0(unsigned char * val)
{
	if (pCr0)			
		delete pCr0;
	pCr0 = val;
}

int cImageYCbCr::getFullWidth()
{
	return iFullWidth;
}

int cImageYCbCr::getFullHeight()
{
	return iFullHeight;
}

int cImageYCbCr::getFullWidth2()
{
	return iFullWidth2;
}

int cImageYCbCr::getFullHeight2()
{
	return iFullHeight2;
}

int cImageYCbCr::getFullWidth(component comp)
{
	switch (comp)
	{
	case Y:
		return iFullWidth;
	case Cb:
	case Cr:
		return iFullWidth2;
	}
}

int cImageYCbCr::getFullHeight(component comp)
{
	switch (comp)
	{
	case Y:
		return iFullHeight;
	case Cb:
	case Cr:
		return iFullHeight2;
	}
}

int cImageYCbCr::getEnhanceValue()
{
	return iEnhanceValue;
}

int cImageYCbCr::getSubW()
{
	return iSubW;
}

int cImageYCbCr::getSubH()
{
	return iSubH;
}

unsigned char* cImageYCbCr::getPComp0(component comp)
{
	switch (comp)
	{
	case Y:
		return pY0;
	case Cb:
		return pCb0;
	case Cr:
		return pCr0;
	}
}

// JPEG conversion
// Y = 0.299*R + 0.587*G + 0.114*B;
// Cb = 128 - 0.168736*R - 0.331264*G + 0.5*B;
// Cr = 128 + 0.5*R - 0.418688*G - 0.081312*B;
// R = Y + 1.402*(Cr - 128);
// G = Y - 0.344136*(Cb - 128) - 0.714136*(Cr - 128);
// B = Y + 1.772*(Cb - 128);


int cImageYCbCr::RFromYCbCr(int iY, int iCb, int iCr)
{
	double d = iY + 1.402*(iCr - 128);

	return Limiting_a_b(d, 0, 255);
}


int cImageYCbCr::GFromYCbCr(int iY, int iCb, int iCr)
{
	double d = iY - 0.344136*(iCb - 128) - 0.714136*(iCr - 128);

	return Limiting_a_b(d, 0, 255);
}


int cImageYCbCr::BFromYCbCr(int iY, int iCb, int iCr)
{
	double d = iY + 1.772*(iCb - 128);

	return Limiting_a_b(d, 0, 255);
}

void cImageYCbCr::setWhite()
{
	if (pY0)
		delete[] pY0;
	pY0 = new unsigned char[iFullWidth*iFullHeight];
	if (pCr0)
		delete[] pCr0;
	pCr0 = new unsigned char[iFullWidth2*iFullHeight2];
	if (pCb0)
		delete[] pCb0;
	pCb0 = new unsigned char[iFullWidth2*iFullHeight2];
	memset(pY0, 255, iFullWidth*iFullHeight);
	memset(pCr0, 128, iFullWidth2*iFullHeight2);
	memset(pCb0, 128, iFullWidth2*iFullHeight2);
}

void cImageYCbCr::setGrey()
{
	if (pY0)
		delete[] pY0;
	pY0 = new unsigned char[iFullWidth*iFullHeight];
	if (pCr0)
		delete[] pCr0;
	pCr0 = new unsigned char[iFullWidth2*iFullHeight2];
	if (pCb0)
		delete[] pCb0;
	pCb0 = new unsigned char[iFullWidth2*iFullHeight2];
	memset(pY0, 128, iFullWidth*iFullHeight);
	memset(pCr0, 128, iFullWidth2*iFullHeight2);
	memset(pCb0, 128, iFullWidth2*iFullHeight2);
}

void cImageYCbCr::paintComp(component comp, unsigned char val)
{
	unsigned char * p0 = getPComp0(comp);
	int iWidth = getFullWidth(comp);
	int iHeight = getFullHeight(comp);
	if (p0)
		memset(p0, val, iWidth*iHeight);
}

int cImageYCbCr::Limiting_a_b(double d, int a, int b)
{
	int i = (int)round(d);

	if (i < a)
		i = a;
	else if (i > b)
		i = b;

	return i;
}

component cImageYCbCr::next(component comp)
{
	switch (comp)
	{
	case Y:
		return Cr;
	case Cr:
		return Cb;
	case Cb:
		return Y;
	}
}

// Ignores enhancement. Works when dimensions agree.
cImageYCbCr * cImageYCbCr::difference(cImageYCbCr * pImageA, cImageYCbCr * pImageB, double mult, char add)
{
	cout << "Creating a difference image... ";
	if ((pImageA->getFullWidth() - 2 * pImageA->getEnhanceValue()) != (pImageB->getFullWidth() - 2 * pImageB->getEnhanceValue()) || 
		(pImageA->getFullWidth2() - 2 * pImageA->getEnhanceValue()) != (pImageB->getFullWidth2() - 2 * pImageB->getEnhanceValue()) ||
				pImageA->getSubW() != pImageB->getSubW() || pImageA->getSubH() != pImageB->getSubH())
		return nullptr;
	cImageYCbCr *diff = new cImageYCbCr;
	if (!diff)
		return nullptr;

	diff->setEnhanceValue(0);
	diff->setFullWidth(pImageA->getFullWidth() - 2 * pImageA->getEnhanceValue());
	diff->setFullHeight(pImageA->getFullHeight() - 2 * pImageA->getEnhanceValue());
	diff->setFullWidth2(pImageA->getFullWidth2() - 2 * pImageA->getEnhanceValue());
	diff->setFullHeight2(pImageA->getFullHeight2() - 2 * pImageA->getEnhanceValue());
	diff->setSubW(pImageA->getSubW());
	diff->setSubH(pImageA->getSubH());
	diff->setGrey();

	unsigned char *pd, *pa, *pb;
	component comp = Y;
	do
	{
		pa = pImageA->getPComp0(comp) + pImageA->getEnhanceValue()*pImageA->getFullWidth(comp) + pImageA->getEnhanceValue();
		pb = pImageB->getPComp0(comp) + pImageB->getEnhanceValue()*pImageB->getFullWidth(comp) + pImageB->getEnhanceValue();
		pd = diff->getPComp0(comp);
		for (int j = 0; j < diff->getFullHeight(comp);
				j++, pa += pImageA->getFullWidth(comp), pb += pImageB->getFullWidth(comp), pd += diff->getFullWidth(comp))
			for (int i = 0; i < diff->getFullWidth(comp); i++)
				pd[i] = Limiting_a_b((unsigned char) ( ((signed char)pa[i] - (signed char)pb[i])*mult + add ), 0, 255);

		comp = next(comp);
		if (comp == Y)
		{
			cout << "Success" << endl;
			return diff;
		}

	}
	while (true);
	return nullptr;
}
 
void cImageYCbCr::fillEnhancementC(component comp, int iW, int iH, int iEvenOdd, int sgnH, int sgnV)
{
	// iFW, iFH are the paramters of matrix participating in coding;	// no more used
	// iW, iH are the parameters of matrix useful information (original size)
	unsigned char * pp;
	switch (comp)
	{
	case Y:
		pp = pY0;
		break;
	case Cr:
		pp = pCr0;
		break;
	case Cb:
		pp = pCb0;
		break;
	}
	unsigned char * p1;
	unsigned char * p2;
	int i, j, k;
	int iFW = getFullWidth(comp);
	int iFH = getFullHeight(comp);
	int iWC = iFW - 2 * iEnhanceValue;
	int iHC = iFH - 2 * iEnhanceValue;

	// copy pixels
	k = iWC - iW;		// appears after ceiling to iNumBand
	if (k > 0)
	{
		p1 = pp + iEnhanceValue*iFW + iEnhanceValue + iW - 1;
		for (j = 0; j < iH; j++, p1 += iFW)
			for (i = 0; i < k; i++)
				p1[i + 1] = p1[0];
	}
	k = iHC - iH;
	if (k > 0)
	{
		p1 = pp + (iEnhanceValue + iH - 1)*iFW + iEnhanceValue;
		for (i = 0; i < k; i++)
			memcpy(p1 + (i + 1)*iFW, p1, iH + k);
	}

	if (iEvenOdd)
	{
		// enhancement : mirror relatively last pixel
		/**/
		// top
		p1 = pp + iEnhanceValue + iFW*(iEnhanceValue - 1);
		p2 = pp + iEnhanceValue + iFW*(iEnhanceValue + 1);
		for (i = iEnhanceValue - 1; i >= 0; i--, p1 -= iFW, p2 += iFW)
			for (j = 0; j < iWC; j++)
				p1[j] = p2[j] * sgnV;
		//memcpy(p1, p2, iW);
		// bottom
		p2 = pp + iFW*(iEnhanceValue + iHC - 2) + iEnhanceValue;
		p1 = p2 + 2 * iFW;
		for (i = 0; i < iEnhanceValue; i++, p1 += iFW, p2 -= iFW)
			for (j = 0; j < iWC; j++)
				p1[j] = p2[j] * sgnV;
		//memcpy(p1, p2, iW);
		// left&right
		p1 = pp;
		for (i = 0; i < iFH; i++, p1 += iFW)
		{
			for (j = 0; j < iEnhanceValue; j++)
				p1[j] = p1[2 * iEnhanceValue - j] * sgnH;
			for (j = 0; j < iEnhanceValue; j++)
				p1[j+ iEnhanceValue + iWC] = p1[iEnhanceValue + iWC - 2 - j] * sgnH;
		}
		/**/
	}
	else
	{
		// enhancement : mirror relatively bound
		/**/
		// top
		p1 = pp + iEnhanceValue + iFW*(iEnhanceValue - 1);
		p2 = pp + iEnhanceValue + iFW*iEnhanceValue;
		for (i = iEnhanceValue - 1; i >= 0; i--, p1 -= iFW, p2 += iFW)
			for (j = 0; j < iWC; j++)
				p1[j] = p2[j] * sgnV;
		//memcpy(p1, p2, iW);
		// bottom
		p2 = pp + iFW*(iEnhanceValue + iHC - 1) + iEnhanceValue;
		p1 = p2 + iFW;
		for (i = 0; i < iEnhanceValue; i++, p1 += iFW, p2 -= iFW)
			for (j = 0; j < iWC; j++)
				p1[j] = p2[j] * sgnV;
		//memcpy(p1, p2, iW);
		// left&right
		p1 = pp;
		for (i = 0; i < iFH; i++, p1 += iFW)
		{
			for (j = 0; j < iEnhanceValue; j++)
				p1[j] = p1[2 * iEnhanceValue - 1 - j] * sgnH;
			for (j = 0; j < iEnhanceValue; j++)
				p1[j + iEnhanceValue + iWC] = p1[iEnhanceValue + iWC - 1 - j] * sgnH;
		}
		/**/
	}
}

// Works only if the relation between Dimensions and Dimensions2 is OK
cImageRGB* cImageYCbCr::CreateRGB24FromYCbCr420()
{
	cout << "Conversion YCbCr->RGB... ";
	if (!pY0 || !pCr0 || !pCb0)
		return nullptr;
	if (iFullWidth2 != ceilTo(iFullWidth, iSubW) / iSubW || iFullHeight2 != ceilTo(iFullHeight, iSubH) / iSubH)
		return nullptr;

	cImageRGB *ret = new cImageRGB;

	int iImageWidth = iFullWidth;
	int iImageHeight = iFullHeight;
	ret->setWidth(iImageWidth);
	ret->setHeight(iImageHeight);


	unsigned char* pRGB = new unsigned char[ret->iSizeRGB()];
	if (!pRGB)
		return nullptr;
	memset(pRGB, 0, ret->iSizeRGB());
	ret->setpRGB(pRGB);
	
	int i, j, k, l;

	unsigned char * p = pY0;
	unsigned char * p1 = pCr0;
	unsigned char * p2 = pCb0;
	unsigned char * pp = pRGB + ret->iWidthRGB()*(iImageHeight - 1);
	for (j = 0; j < iImageHeight / 2 * 2; j += 2, p += iFullWidth * 2, pp -= ret->iWidthRGB() * 2, p1 += iFullWidth2 * 2 / iSubH, p2 += iFullWidth2 * 2 / iSubH)
	{
		for (i = 0, k = 0, l = 0; i < iImageWidth / 2 * 2; i += 2, k += 2 / iSubW, l += 6)
		{
			int Y1 = p[i];
			int Y2 = p[i + 1];
			int Y3 = p[i + iFullWidth];
			int Y4 = p[i + iFullWidth + 1];
			int Cr1, Cr2, Cr3, Cr4, Cb1, Cb2, Cb3, Cb4;
			switch (10 * iSubW + iSubH)
			{
			case 11:
				Cr1 = p1[k];
				Cr2 = p1[k + 1];
				Cr3 = p1[k + iFullWidth2];
				Cr4 = p1[k + iFullWidth2 + 1];
				Cb1 = p2[k];
				Cb2 = p2[k + 1];
				Cb3 = p2[k + iFullWidth2];
				Cb4 = p2[k + iFullWidth2 + 1];
				break;
			case 12:
				Cr1 = Cr3 = p1[k];
				Cr2 = Cr4 = p1[k + 1];
				Cb1 = Cb3 = p2[k];
				Cb2 = Cb4 = p2[k + 1];
				break;
			case 21:
				Cr1 = Cr2 = p1[k];
				Cr3 = Cr4 = p1[k + iFullWidth2];
				Cb1 = Cb2 = p2[k];
				Cb3 = Cb4 = p2[k + iFullWidth2];
				break;
			case 22:
				Cr1 = Cr2 = Cr3 = Cr4 = p1[k];
				Cb1 = Cb2 = Cb3 = Cb4 = p2[k];
				break;
			default:
				return nullptr;
			}
			pp[l] = BFromYCbCr(Y1, Cb1, Cr1);
			pp[l + 1] = GFromYCbCr(Y1, Cb1, Cr1);
			pp[l + 2] = RFromYCbCr(Y1, Cb1, Cr1);
			pp[l + 3] = BFromYCbCr(Y2, Cb2, Cr2);
			pp[l + 4] = GFromYCbCr(Y2, Cb2, Cr2);
			pp[l + 5] = RFromYCbCr(Y2, Cb2, Cr2);
			pp[l - ret->iWidthRGB()] = BFromYCbCr(Y3, Cb3, Cr3);
			pp[l - ret->iWidthRGB() + 1] = GFromYCbCr(Y3, Cb3, Cr3);
			pp[l - ret->iWidthRGB() + 2] = RFromYCbCr(Y3, Cb3, Cr3);
			pp[l - ret->iWidthRGB() + 3] = BFromYCbCr(Y4, Cb4, Cr4);
			pp[l - ret->iWidthRGB() + 4] = GFromYCbCr(Y4, Cb4, Cr4);
			pp[l - ret->iWidthRGB() + 5] = RFromYCbCr(Y4, Cb4, Cr4);
		}
		if (i < iImageWidth)		// if iFullWidth is odd
		{
			int Y1 = p[i];
			int Y3 = p[i + iFullWidth];
			int Cr1, Cr3, Cb1, Cb3;
			switch (10 * iSubW + iSubH)
			{
			case 11:
			case 21:
				Cr1 = p1[k];
				Cr3 = p1[k + iFullWidth2];
				Cb1 = p2[k];
				Cb3 = p2[k + iFullWidth2];
				break;
			case 12:
			case 22:
				Cr1 = Cr3 = p1[k];
				Cb1 = Cb3 = p2[k];
				break;
			default:
				return nullptr;
			}
			pp[l] = BFromYCbCr(Y1, Cb1, Cr1);
			pp[l + 1] = GFromYCbCr(Y1, Cb1, Cr1);
			pp[l + 2] = RFromYCbCr(Y1, Cb1, Cr1);
			pp[l - ret->iWidthRGB()] = BFromYCbCr(Y3, Cb3, Cr3);
			pp[l - ret->iWidthRGB() + 1] = GFromYCbCr(Y3, Cb3, Cr3);
			pp[l - ret->iWidthRGB() + 2] = RFromYCbCr(Y3, Cb3, Cr3);
		}
	}
	if (j < iImageHeight)			// if iFullHeight is odd
	{
		for (i = 0, k = 0, l = 0; i < iImageWidth / 2 * 2; i += 2, k += 2 / iSubW, l += 6)
		{
			int Y1 = p[i];
			int Y2 = p[i + 1];
			int Cr1, Cr2, Cb1, Cb2;
			switch (10 * iSubW + iSubH)
			{
			case 11:
			case 12:
				Cr1 = p1[k];
				Cr2 = p1[k + 1];
				Cb1 = p2[k];
				Cb2 = p2[k + 1];
				break;
			case 21:
			case 22:
				Cr1 = Cr2 = p1[k];
				Cb1 = Cb2 = p2[k];
				break;
			default:
				return nullptr;
			}
			pp[l] = BFromYCbCr(Y1, Cb1, Cr1);
			pp[l + 1] = GFromYCbCr(Y1, Cb1, Cr1);
			pp[l + 2] = RFromYCbCr(Y1, Cb1, Cr1);
			pp[l + 3] = BFromYCbCr(Y2, Cb2, Cr2);
			pp[l + 4] = GFromYCbCr(Y2, Cb2, Cr2);
			pp[l + 5] = RFromYCbCr(Y2, Cb2, Cr2);
		}
		if (i < iImageWidth)		// if both iFullHeight and iFullWidth are odd
		{
			int Y1 = p[i];
			int Cr1, Cb1;
			Cr1 = p1[k];
			Cb1 = p2[k];
			pp[l] = BFromYCbCr(Y1, Cb1, Cr1);
			pp[l + 1] = GFromYCbCr(Y1, Cb1, Cr1);
			pp[l + 2] = RFromYCbCr(Y1, Cb1, Cr1);
		}
	}

	cout << "Success" << endl;
	return ret;
}

// places this image into background (with no enhancement in it) to area beginning from columnL and stringH right- and downwards
// parameters fromColumn-toColumn, fromString-toString bound the inserted image with corresponding area
// includingEnhancement forces the function to consider enhamncement of the inserted image as a a part of the image
// decimation > 1 rejects points but divisible by the parameter
// sparsing > 1 puts points to background with set interval
void cImageYCbCr::placeInto(component comp, cImageYCbCr * background, int columnL, int stringH, int fromColumn, int toColumn, 
	int fromString, int toString, bool includingEnhancement, int decimation, int sparsing)
{
	if (toColumn == 0)
		toColumn = (includingEnhancement ? getFullWidth(comp) - 1 : getFullWidth(comp) - 2 * iEnhanceValue - 1);
	if (toString == 0)
		toString = (includingEnhancement ? getFullHeight(comp) - 1 : getFullHeight(comp) - 2 * iEnhanceValue - 1);
	if ((toColumn - fromColumn + 1 > background->getFullWidth(comp) - columnL) ||
		(toString - fromString + 1 > background->getFullHeight(comp) - stringH))
	{
		cerr << "cImageYCbCr::placeInto: Image exeeds background dimensions" << endl;
		return;
	}
	unsigned char *ps0,*pd0;
	switch (comp)
	{
	case Y:
		ps0 = pY0;
		pd0 = background->pY0;
		break;
	case Cr:
		ps0 = pCr0;
		pd0 = background->pCr0;
		break;
	case Cb:
		ps0 = pCb0;
		pd0 = background->pCb0;
	}
	if (!ps0 || !pd0)
	{
		cerr << "cImageYCbCr::placeInto: Null pointers" << endl;
		return;
	}

	int iFWs = getFullWidth(comp), iFWd = background->getFullWidth(comp), 
		iFHs = getFullHeight(comp), iFHd = background->getFullHeight(comp);
	int i, j;
	unsigned char *psource, *pdest;
	for (j = 0, psource = ps0 + (includingEnhancement ? 0 : iFWs * iEnhanceValue + iEnhanceValue) + iFWs*fromString + fromColumn,
		pdest = pd0 + iFWd*stringH + columnL;
		j*decimation <= (toString - fromString);
		j++, psource += decimation*iFWs, pdest += sparsing*iFWd)
		for (i = 0; i*decimation <= (toColumn - fromColumn); i ++)
			pdest[i*sparsing]=psource[i*decimation];
}

double cImageYCbCr::prms(component comp)
{
	if (iEnhanceValue)
		printf("Enhancement is also taken into account");

	int i, j;
	unsigned char *p;
	double sum = 0.0;
	double pj;

	for (i = 0, p = getPComp0(comp);
		i < getFullHeight(comp);
		i++, p += getFullWidth(comp))
		for (j = 0; j < getFullWidth(comp); j++)
		{
			pj = (double)(signed char)(p[j] - 128);
			sum += pj*pj;
		}

	return (255.0 / sqrt(sum / getFullWidth(comp) / getFullHeight(comp)));
}

//Box-Muller (v1)
cImageYCbCr * cImageYCbCr::addAWGN(double sigma)
{
	cImageYCbCr *ret = new cImageYCbCr;

	ret->iFullHeight = iFullHeight;
	ret->iFullWidth = iFullWidth;
	ret->iFullHeight2 = iFullHeight2;
	ret->iFullWidth2 = iFullWidth2;
	ret->iEnhanceValue = iEnhanceValue;
	ret->iSubW = iSubW;
	ret->iSubH = iSubH;
	ret->setGrey();

	srand(152);
	int iSize = iFullHeight*iFullWidth;
	int iSize2 = iFullHeight2*iFullWidth2;
	double r, phi, z0; //z1;
	for (int i = 0; i < iSize; i++)
	{
		r = ((double)rand() + 1) / ((double)RAND_MAX + 1);
		phi = ((double)rand() + 1) / ((double)RAND_MAX + 1);
		z0 = cos(2*PI*phi)*sqrt(-2 * log(r));
		//z1 = cos(2 * PI*phi)*sqrt(-2 * log(r));
		ret->pY0[i] = Limiting_a_b(pY0[i] + (sigma*z0), 0, UCHAR_MAX);
	}
	for (int i = 0; i < iSize2; i++)
	{
		r = ((double)rand() + 1) / ((double)RAND_MAX + 1);
		phi = ((double)rand() + 1) / ((double)RAND_MAX + 1);
		z0 = cos(2*PI*phi)*sqrt(-2 * log(r));
		//z1 = cos(2 * PI*phi)*sqrt(-2 * log(r));
		ret->pCr0[i] = Limiting_a_b(pCr0[i] + (sigma*z0), 0, UCHAR_MAX);
	}
	for (int i = 0; i < iSize2; i++)
	{
		r = ((double)rand() + 1) / ((double)RAND_MAX + 1);
		phi = ((double)rand() + 1) / ((double)RAND_MAX + 1);
		z0 = cos(2*PI*phi)*sqrt(-2 * log(r));
		//z1 = cos(2 * PI*phi)*sqrt(-2 * log(r));
		ret->pCb0[i] = Limiting_a_b(pCb0[i] + (sigma*z0), 0, UCHAR_MAX);
	}
	return ret;
}

cImageYCbCr *cImageYCbCr::takeAverage(cImageYCbCr ** pImage_ar, int numImages)
{
	if (!numImages || !pImage_ar[0]->pY0 || !pImage_ar[0]->pCr0 || !pImage_ar[0]->pCb0)
		return nullptr;
	int iFW = pImage_ar[0]->iFullWidth;
	int iFH = pImage_ar[0]->iFullHeight;
	int iFW2 = pImage_ar[0]->iFullWidth2;
	int iFH2 = pImage_ar[0]->iFullHeight2;
	int iEV = pImage_ar[0]->iEnhanceValue;
	int iSW = pImage_ar[0]->iSubW;
	int iSH = pImage_ar[0]->iSubH;
	for (int k = 1; k < numImages; k++)
		if (iFW != pImage_ar[k]->iFullWidth || iFH != pImage_ar[k]->iFullHeight ||
			iFW2 != pImage_ar[k]->iFullWidth2 || iFH2 != pImage_ar[k]->iFullHeight2 || iEV != pImage_ar[k]->iEnhanceValue ||
			iSW != pImage_ar[k]->iSubW || iSH != pImage_ar[k]->iSubH || 
			!pImage_ar[k]->pY0 || !pImage_ar[k]->pCr0 || !pImage_ar[k]->pCb0)
		{
			cerr << "cBands::sum: bands are not consistent" << endl;
			return nullptr;
		}

	int iSize = iFW*iFH;
	int iSize2 = iFW2*iFH2;
	double *pY0d = new double[iSize];
	double *pCr0d = new double[iSize2];
	double *pCb0d = new double[iSize2];
	memset(pY0d, 0, iSize * sizeof(double));
	memset(pCr0d, 0, iSize2 * sizeof(double));
	memset(pCb0d, 0, iSize2 * sizeof(double));
	for (int k = 0; k < numImages; k++)
	{
		for (int i = 0; i < iSize; i++)
			pY0d[i] += (double)pImage_ar[k]->pY0[i] / numImages;
		for (int i = 0; i < iSize2; i++)
			pCr0d[i] += (double)pImage_ar[k]->pCr0[i] / numImages;
		for (int i = 0; i < iSize2; i++)
			pCb0d[i] += (double)pImage_ar[k]->pCb0[i] / numImages;
	}

	cImageYCbCr *pAverage = new cImageYCbCr;
	pAverage->iFullHeight = iFH;
	pAverage->iFullWidth = iFW;
	pAverage->iFullHeight2 = iFH2;
	pAverage->iFullWidth2 = iFW2;
	pAverage->iEnhanceValue = iEV;
	pAverage->iSubW = iSW;
	pAverage->iSubH = iSH;
	pAverage->setGrey();

	for (int i = 0; i < iSize; i++)
		pAverage->pY0[i] = (char)pY0d[i];
	for (int i = 0; i < iSize2; i++)
		pAverage->pCr0[i] = (char)pCr0d[i];
	for (int i = 0; i < iSize2; i++)
		pAverage->pCb0[i] = (char)pCb0d[i];

	delete[] pY0d;
	delete[] pCr0d;
	delete[] pCb0d;
	return pAverage;
}

cImageYCbCr * cImageYCbCr::setLayer(int iNB)
{
	int iLayer = iNB - 1;
	cImageYCbCr * imageL = copy();
	for (int i = 0; i < iFullWidth; i++)
		for (int j = 0; j < iLayer; j++)
		{
			imageL->pY0[i + j*iFullWidth] = imageL->pY0[i + iLayer*iFullWidth];
			imageL->pCr0[i + j*iFullWidth] = imageL->pCr0[i + iLayer*iFullWidth];
			imageL->pCb0[i + j*iFullWidth] = imageL->pCb0[i + iLayer*iFullWidth];
			imageL->pY0[i + (iFullHeight - 1 - j)*iFullWidth] = imageL->pY0[i + (iFullHeight - 1 - iLayer)*iFullWidth];
			imageL->pCr0[i + (iFullHeight - 1 - j)*iFullWidth] = imageL->pCr0[i + (iFullHeight - 1 - iLayer)*iFullWidth];
			imageL->pCb0[i + (iFullHeight - 1 - j)*iFullWidth] = imageL->pCb0[i + (iFullHeight - 1 - iLayer)*iFullWidth];
		}

	for (int i = 0; i < iFullHeight; i++)
		for (int j = 0; j < iLayer; j++)
		{
			imageL->pY0[i*iFullWidth + j] = imageL->pY0[i*iFullWidth + iLayer];
			imageL->pCr0[i*iFullWidth + j] = imageL->pCr0[i*iFullWidth + iLayer];
			imageL->pCb0[i*iFullWidth + j] = imageL->pCb0[i*iFullWidth + iLayer];
			imageL->pY0[i*iFullWidth + (iFullWidth - 1 - j)] = imageL->pY0[i*iFullWidth + (iFullWidth - 1 - iLayer)];
			imageL->pCr0[i*iFullWidth + (iFullWidth - 1 - j)] = imageL->pCr0[i*iFullWidth + (iFullWidth - 1 - iLayer)];
			imageL->pCb0[i*iFullWidth + (iFullWidth - 1 - j)] = imageL->pCb0[i*iFullWidth + (iFullWidth - 1 - iLayer)];
		}

	return imageL;
}

void cImageYCbCr::resetLayer(int iNB)
{
	assert(iNB == 2);

	for (int i = 0; i < iFullWidth; i++)
	{
		pY0[i] /= 2;
		pCr0[i] /= 2;
		pCb0[i] /= 2;
		pY0[(iFullHeight - 2)*iFullWidth + i] /= 2;
		pCr0[(iFullHeight - 2)*iFullWidth + i] /= 2;
		pCb0[(iFullHeight - 2)*iFullWidth + i] /= 2;
		pY0[(iFullHeight - 1)*iFullWidth + i] = 0;
		pCr0[(iFullHeight - 1)*iFullWidth + i] = 0;
		pCb0[(iFullHeight - 1)*iFullWidth + i] = 0;
	}

	for (int j = 0; j < iFullHeight; j++)
	{
		pY0[j*iFullWidth] /= 2;
		pCr0[j*iFullWidth] /= 2;
		pCb0[j*iFullWidth] /= 2;
		pY0[(iFullWidth - 2) + j*iFullWidth] /= 2;
		pCr0[(iFullWidth - 2) + j*iFullWidth] /= 2;
		pCb0[(iFullWidth - 2) + j*iFullWidth] /= 2;
		pY0[(iFullWidth - 1) + j*iFullWidth] = 0;
		pCr0[(iFullWidth - 1) + j*iFullWidth] = 0;
		pCb0[(iFullWidth - 1) + j*iFullWidth] = 0;
	}
}

void cImageYCbCr::blockBorders(int blockSize)
{
	for (int i=1; i<iFullWidth2; i+=blockSize/iSubW)
		for (int j=0; j<iFullHeight2; j++)
		{
			pCr0[iFullWidth2*j+i]=160;
			pCb0[iFullWidth2*j+i]=128;
		}
	for (int j=1; j<iFullHeight2; j+=blockSize/iSubH)
		for (int i=0; i<iFullWidth2; i++)
		{
			pCr0[iFullWidth2*j+i]=160;
			pCb0[iFullWidth2*j+i]=128;
		}
}

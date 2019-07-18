#include "cImageRGB.h"


int cImageRGB::iWidthRGB()
{
	return (iWidth * 3 + 3)&(~3);
}

int cImageRGB::iSizeRGB()
{
	return iWidthRGB()*iHeight;
}

cImageRGB::cImageRGB()
	: iWidth(0)
	, iHeight(0)
	, pRGB(NULL)
{
}


cImageRGB::~cImageRGB()
{
	if (pRGB)
		delete[] pRGB;
}

int cImageRGB::getWidth()
{
	return iWidth;
}

int cImageRGB::getHeight()
{
	return iHeight;
}


void cImageRGB::setHeight(int val)
{
	iHeight = val;
}

void cImageRGB::setWidth(int val)
{
	iWidth = val;
}

void cImageRGB::setpRGB(unsigned char * val)
{
	if (pRGB)
		delete[] pRGB;
	pRGB = val;
}

int cImageRGB::CreateFromBitmapFile(const char * filename, int *picWidth, int *picHeight)
{
	int i;
	FILE * fp;
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;
	assert(sizeof(bmfh)==14);
	assert(sizeof(bmih)==40);

	cout << "Reading a picture from " << filename << "... ";
	fp = fopen(filename, "rb");
	if (fp)
	{
		if ((i=fread(&bmfh, 14, 1, fp)) < 1)
		{
			fclose(fp);
			return 2;
		}

		if ((i=fread(&bmih, 40, 1, fp)) < 1)
		{
			fclose(fp);
			return 3;
		}

		if ((bmih.biPlanes == 1) &&
			(bmih.biBitCount == 24) &&
			(bmih.biCompression == 0))
		{
			iWidth = bmih.biWidth;
			iHeight = bmih.biHeight;

			if (pRGB)
				delete[] pRGB;
			pRGB = new unsigned char[iSizeRGB()];
			if (!pRGB)
			{
				fclose(fp);
				return 5;
			}

			if ((i=fread(pRGB, 1, iSizeRGB(), fp)) < iSizeRGB())
			{
				fclose(fp);
				return 6;
			}

			fclose(fp);
		}
		else
		{
			cerr << "Unsupported set of .bmp parameters\n";
			return 4;
		}
	}
	else
		return 1;

	if (picWidth)
		*picWidth = iWidth;
	if (picHeight)
		*picHeight = iHeight;
	cout << "Read an image " << iWidth << "x" << iHeight << endl;
	return 0;
}


cImageYCbCr* cImageRGB::CreateYCrCbFromRGB(int iSubW, int iSubH)
{
	if (!pRGB)
		return nullptr;

	cImageYCbCr * ret;
	cout << "Conversion RGB->YCbCr... ";

	int iWidth2 = ceilTo(iWidth, iSubW) / iSubW;
	int iHeight2 = ceilTo(iHeight, iSubH) / iSubH;
	int iSize = iHeight*iWidth;
	int iSize2 = iHeight2*iWidth2;

	unsigned char* pY0 = new unsigned char[iSize];
	unsigned char* pCr0 = new unsigned char[iSize2];
	unsigned char* pCb0 = new unsigned char[iSize2];
	if (!pY0 || !pCr0 || !pCb0)
		return nullptr;
	memset(pY0, 128, iSize);
	memset(pCr0, 128, iSize2);
	memset(pCb0, 128, iSize2);
	
	unsigned char * p = pY0;
	unsigned char * p1 = pCr0;
	unsigned char * p2 = pCb0;
	unsigned char * pp2 = pRGB + iWidthRGB()*(iHeight - 2);
	unsigned char * pp1 = pRGB + iWidthRGB()*(iHeight - 1);
	int i, j, k, l;
	for (i = 0; i < iHeight / 2 * 2; i += 2, pp1 -= iWidthRGB() * 2, pp2 -= iWidthRGB() * 2,
		p += iWidth * 2, p1 += iWidth2 * 2 / iSubH, p2 += iWidth2 * 2 / iSubH)
	{
		for (j = 0, k = 0, l = 0; j < iWidth / 2 * 2; j += 2, k += 6, l += 2 / iSubW)
		{
			int B1 = pp1[k];
			int G1 = pp1[k + 1];
			int R1 = pp1[k + 2];
			int B2 = pp1[k + 3];
			int G2 = pp1[k + 4];
			int R2 = pp1[k + 5];
			int B3 = pp2[k];
			int G3 = pp2[k + 1];
			int R3 = pp2[k + 2];
			int B4 = pp2[k + 3];
			int G4 = pp2[k + 4];
			int R4 = pp2[k + 5];
			int Y1 = YFromRGB(R1, G1, B1);
			int Y2 = YFromRGB(R2, G2, B2);
			int Y3 = YFromRGB(R3, G3, B3);
			int Y4 = YFromRGB(R4, G4, B4);
			int Cr1 = CrFromRGB(R1, G1, B1);
			int Cr2 = CrFromRGB(R2, G2, B2);
			int Cr3 = CrFromRGB(R3, G3, B3);
			int Cr4 = CrFromRGB(R4, G4, B4);
			int Cb1 = CbFromRGB(R1, G1, B1);
			int Cb2 = CbFromRGB(R2, G2, B2);
			int Cb3 = CbFromRGB(R3, G3, B3);
			int Cb4 = CbFromRGB(R4, G4, B4);
			p[j] = Y1;
			p[j + 1] = Y2;
			p[j + iWidth] = Y3;
			p[j + iWidth + 1] = Y4;
			switch (10 * iSubW + iSubH)
			{
			case 11:
				p1[l] = Cr1;
				p1[l + 1] = Cr2;
				p1[l + iWidth2] = Cr3;
				p1[l + iWidth2 + 1] = Cr4;
				p2[l] = Cb1;
				p2[l + 1] = Cb2;
				p2[l + iWidth2] = Cb3;
				p2[l + iWidth2 + 1] = Cb4;
				break;
			case 12:
				p1[l] = Limiting_a_b((Cr1 + Cr3) / 2.0, 0, 255);
				p1[l + 1] = Limiting_a_b((Cr2 + Cr4) / 2.0, 0, 255);
				p2[l] = Limiting_a_b((Cb1 + Cb3) / 2.0, 0, 255);
				p2[l + 1] = Limiting_a_b((Cb2 + Cb4) / 2.0, 0, 255);
				break;
			case 21:
				p1[l] = Limiting_a_b((Cr1 + Cr2) / 2.0, 0, 255);
				p1[l + iWidth2] = Limiting_a_b((Cr3 + Cr4) / 2.0, 0, 255);
				p2[l] = Limiting_a_b((Cb1 + Cb2) / 2.0, 0, 255);
				p2[l + iWidth2] = Limiting_a_b((Cb3 + Cb4) / 2.0, 0, 255);
				break;
			case 22:
				p1[l] = Limiting_a_b((Cr1 + Cr2 + Cr3 + Cr4) / 4.0, 0, 255);
				p2[l] = Limiting_a_b((Cb1 + Cb2 + Cb3 + Cb4) / 4.0, 0, 255);
				break;
			default:
				goto clearYCrCb;
			}
		}
		if (j < iWidth)		// if iFullWidth is odd
		{
			int B1 = pp1[k];
			int G1 = pp1[k + 1];
			int R1 = pp1[k + 2];
			int B3 = pp2[k];
			int G3 = pp2[k + 1];
			int R3 = pp2[k + 2];
			int Y1 = YFromRGB(R1, G1, B1);
			int Y3 = YFromRGB(R3, G3, B3);
			int Cr1 = CrFromRGB(R1, G1, B1);
			int Cr3 = CrFromRGB(R3, G3, B3);
			int Cb1 = CbFromRGB(R1, G1, B1);
			int Cb3 = CbFromRGB(R3, G3, B3);
			p[j] = Y1;
			p[j + iWidth] = Y3;
			switch (10 * iSubW + iSubH)
			{
			case 11:
			case 21:
				p1[l] = Cr1;
				p1[l + iWidth2] = Cr3;
				p2[l] = Cb1;
				p2[l + iWidth2] = Cb3;
				break;
			case 12:
			case 22:
				p1[l] = Limiting_a_b((Cr1 + Cr3) / 2.0, 0, 255);
				p2[l] = Limiting_a_b((Cb1 + Cb3) / 2.0, 0, 255);
				break;
			default:
				goto clearYCrCb;
			}
		}
	}
	if (i < iHeight)	//if iFullHeight is odd
	{
		for (j = 0, k = 0, l = 0; j < iWidth / 2 * 2; j += 2, k += 6, l += 2 / iSubW)
		{
			int B1 = pp1[k];
			int G1 = pp1[k + 1];
			int R1 = pp1[k + 2];
			int B2 = pp1[k + 3];
			int G2 = pp1[k + 4];
			int R2 = pp1[k + 5];
			int Y1 = YFromRGB(R1, G1, B1);
			int Y2 = YFromRGB(R2, G2, B2);
			int Cr1 = CrFromRGB(R1, G1, B1);
			int Cr2 = CrFromRGB(R2, G2, B2);
			int Cb1 = CbFromRGB(R1, G1, B1);
			int Cb2 = CbFromRGB(R2, G2, B2);
			p[j] = Y1;
			p[j + 1] = Y2;
			switch (10 * iSubW + iSubH)
			{
			case 11:
			case 12:
				p1[l] = Cr1;
				p1[l + 1] = Cr2;
				p2[l] = Cb1;
				p2[l + 1] = Cb2;
				break;
			case 21:
			case 22:
				p1[l] = Limiting_a_b((Cr1 + Cr2) / 2.0, 0, 255);
				p2[l] = Limiting_a_b((Cb1 + Cb2) / 2.0, 0, 255);
				break;
			default:
				goto clearYCrCb;
			}
		}
		if (j < iWidth)		// if both iFullHeight and iFullWidth are odd
		{
			int B1 = pp1[k];
			int G1 = pp1[k + 1];
			int R1 = pp1[k + 2];
			int Y1 = YFromRGB(R1, G1, B1);
			int Cr1 = CrFromRGB(R1, G1, B1);
			int Cb1 = CbFromRGB(R1, G1, B1);
			p[j] = Y1;
			p1[l] = Cr1;
			p2[l] = Cb1;
		}
	}

	ret = new cImageYCbCr;

	ret->setEnhanceValue(0);
	ret->setFullWidth(iWidth);
	ret->setFullHeight(iHeight);
	ret->setFullWidth2(ceilTo(iWidth, iSubW) / iSubW);
	ret->setFullHeight2(ceilTo(iHeight, iSubH) / iSubH);
	ret->setSubW(iSubW);
	ret->setSubH(iSubH);

	ret->setpY0(pY0);
	ret->setpCr0(pCr0);
	ret->setpCb0(pCb0);

//	memset(pCr0, 128, iSize2);		//kill!!
//	memset(pCb0, 128, iSize2);
	
	cout << "Success" << endl;
	return ret;

clearYCrCb:
	delete[] pY0;
	delete[] pCr0;
	delete[] pCb0;
	pY0 = pCr0 = pCb0 = nullptr;
	return nullptr;
}

// JPEG conversion
// Y = 0.299*R + 0.587*G + 0.114*B;
// Cb = 128 - 0.168736*R - 0.331264*G + 0.5*B;
// Cr = 128 + 0.5*R - 0.418688*G - 0.081312*B;
// R = Y + 1.402*(Cr - 128);
// G = Y - 0.344136*(Cb - 128) - 0.714136*(Cr - 128);
// B = Y + 1.772*(Cb - 128);

int cImageRGB::YFromRGB(int iR, int iG, int iB)
{
	double d = 0.299*iR + 0.587*iG + 0.114*iB;

	return Limiting_a_b(d, 0, 255);
}


int cImageRGB::CrFromRGB(int iR, int iG, int iB)
{
	double d = 128 + 0.5*iR - 0.418688*iG - 0.081312*iB;

	return Limiting_a_b(d, 0, 255);
}


int cImageRGB::CbFromRGB(int iR, int iG, int iB)
{
	double d = 128 - 0.168736*iR - 0.331264*iG + 0.5*iB;

	return Limiting_a_b(d, 0, 255);
}


int cImageRGB::Limiting_a_b(double d, int a, int b)
{
	int i = (int)round(d);

	if (i < a)
		i = a;
	else if (i > b)
		i = b;

	return i;
}


int cImageRGB::ceilTo(int number, int base, int remainder)
{
	return (number + base - 1 - remainder) / base*base + remainder;
}
/*
void cImage::Normalize(int iNumBands, int * pFilterLength, double ** pFilters, double * mult, int * add, int iEvenOdd)
{
	// Нормировка фильтров для одинакового квантования
	double dSum;
	int j;
	for (int k = 0; k < iNumBands; k++)
	{
		dSum = pFilters[iNumBands + k][0] * pFilters[iNumBands + k][0] * (iEvenOdd ? 1 : 2);
		for (j = 1; j < pFilterLength[iNumBands + k]; j++)
			dSum += 2 * pFilters[iNumBands + k][j] * pFilters[iNumBands + k][j];
		dSum = sqrt(dSum);
		pFilters[iNumBands + k][0] /= (dSum);
		for (j = 1; j < pFilterLength[iNumBands + k]; j++)
			pFilters[iNumBands + k][j] /= (dSum);
		pFilters[k][0] *= dSum;
		for (j = 1; j < pFilterLength[k]; j++)
			pFilters[k][j] *= dSum;
	}
	// Коэффициент нормирования низкочастотной области для отображения
	dSum = pFilters[0][0] * (iEvenOdd ? 1 : 2);
	for (int i = 1; i < pFilterLength[0]; i++)
		dSum += 2 * pFilters[0][i];
	mult[0] = 1.0 / (dSum*dSum);
	add[0] = 0;
}
*/
cImageRGB* cImageRGB::Copy()
{
	cImageRGB *copy = new cImageRGB;
	if (!copy)
		return nullptr;
	copy->iWidth = iWidth;
	copy->iHeight = iHeight;

	copy->pRGB = new unsigned char[iSizeRGB()];
	if (!copy->pRGB)
	{
		delete copy;
		copy = nullptr;
		return nullptr;
	}
	memcpy(copy->pRGB, this->pRGB, iSizeRGB());

	return copy;
}

int cImageRGB::WriteToBitmapFile(char * filename)
{
	cout << "Writing the picture to " << filename << "... ";
	FILE * fp;
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;
	assert(sizeof(bmfh)==14);
	assert(sizeof(bmih)==40);

	if (!pRGB)
		return 1;

	fp = fopen(filename, "wb");
	if (fp)
	{
		
		bmfh.bfType = 19778;
		bmfh.bfOffBits = 54;
		bmfh.bfReserved1 = 0;
		bmfh.bfReserved2 = 0;
		bmfh.bfSize = iSizeRGB() + bmfh.bfOffBits;
		if (fwrite(&bmfh, 14, 1, fp) < 1)
		{
			fclose(fp);
			return 3;
		}

		bmih.biSize = 40;
		bmih.biWidth = iWidth;
		bmih.biHeight = iHeight;
		bmih.biPlanes = 1;
		bmih.biBitCount = 24;
		bmih.biCompression = 0;
		bmih.biSizeImage = iSizeRGB();
		bmih.biXPelsPerMeter = 0;//3780;
		bmih.biYPelsPerMeter = 0;//3780;
		bmih.biClrUsed = 0;
		bmih.biClrImportant = 0;
		if (fwrite(&bmih, sizeof(bmih), 1, fp) < 1)
		{
			fclose(fp);
			return 4;
		}

		if (fwrite(pRGB, 1, iSizeRGB(), fp) < iSizeRGB())
		{
			fclose(fp);
			return 5;
		}

		fclose(fp);
	}
	else
		return 2;
	
	cout << "Success" << endl;
	return 0;
}


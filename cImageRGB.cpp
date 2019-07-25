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

			unsigned char * pBGR = new unsigned char[iSizeRGB()];
			if (!pBGR)
			{
				fclose(fp);
				return 5;
			}

			if ((fread(pBGR, 1, iSizeRGB(), fp)) < iSizeRGB())
			{
				fclose(fp);
				return 6;
			}

			fclose(fp);

			if (pRGB)
				delete[] pRGB;
			pRGB = getReverse(pBGR);
			delete[] pBGR;
			if (!pRGB)
				return 7;
		}
		else
		{
			cerr << "Unsupported set of .bmp parameters\n";
			fclose(fp);
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

#ifndef NOJPEG
// Creation from JPEG with standard settings
int cImageRGB::CreateFromJpegFile(const char * filename, int *picWidth, int *picHeight)
{
	cout << "Reading a picture from " << filename << "... ";
	FILE * fd = fopen(filename, "rb");
	if (!fd)
		return 1;

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

	jpeg_stdio_src(&cinfo, fd);

	jpeg_read_header(&cinfo, TRUE);

	// Change parameters?
/*	cinfo.image_width=iWidth;
	cinfo.image_height=iHeight;
	cinfo.input_components=3;
	cinfo.in_color_space=JCS_RGB;
*/

	jpeg_start_decompress(&cinfo);

	if (cinfo.out_color_components!=3 || cinfo.output_components!=3 ||
			cinfo.actual_number_of_colors!=0 || cinfo.out_color_space!=JCS_RGB)
	{
		cerr << "\nUnsupported set of .jpeg parameters:\n";
		if (cinfo.out_color_components!=3)
			cerr << "out_color_components = " << cinfo.out_color_components << endl;
		if (cinfo.output_components!=3)
			cerr << "output_components = " << cinfo.out_color_components << endl;
		if (cinfo.actual_number_of_colors!=0)
			cerr << "actual_number_of_colors = " << cinfo.actual_number_of_colors << endl;
		if (cinfo.out_color_space!=JCS_RGB)
			cerr << "out_color_space = " << cinfo.out_color_space << 
				" (RGB is " << JCS_RGB << ") " << endl;
		goto exit_jpeg;
	}

	iWidth = cinfo.output_width;
	iHeight = cinfo.output_height;

	if (pRGB)
		delete[] pRGB;
	pRGB = new unsigned char[iSizeRGB()];
	if (!pRGB)
		return 2;

    JSAMPROW row_pointer[1];
    int row_stride;

    row_stride = cinfo.image_width*3;

    while (cinfo.output_scanline < cinfo.output_height) {
        row_pointer[0]=(JSAMPLE *)(pRGB+cinfo.output_scanline*row_stride);
        jpeg_read_scanlines(&cinfo, row_pointer, 1);
	}

	cout << "Read an image " << iWidth << "x" << iHeight << endl;

exit_jpeg:
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	fclose(fd);
	return 0;	
}
#endif

#ifndef NOPNG
// Creation from PNG
int cImageRGB::CreateFromPngFile(const char * filename, int *picWidth, int *picHeight)
{
	cout << "Reading a picture from " << filename << "... ";

	FILE * fp = fopen(filename, "rb");
	if (!fp) 
		return 1;

	// Check it's PNG
	png_size_t number = 8;
	png_byte header[number];

	if (fread(header, 1, number, fp)!=number)
		return 2;
	bool is_png = !png_sig_cmp(header, (png_size_t)0, number);
	if(!is_png)
	{
		cerr << "Not a PNG file" << endl;
		return 3;
	}

	// Create png_struct
    png_structp png_ptr = png_create_read_struct
        (PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr)
       return 4;

	// Crete png_info
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
       png_destroy_read_struct(&png_ptr,
           (png_infopp)NULL, (png_infopp)NULL);
       return 5;
    }

	// Set jump to handler
    if (setjmp(png_jmpbuf(png_ptr)))
    {
       png_destroy_read_struct(&png_ptr, &info_ptr,
           /*&end_info*/(png_infopp)NULL);
       fclose(fp);
       return 6;
    }

	// Setting input
	png_init_io(png_ptr, fp);

	// Declare that bytes have been read
	png_set_sig_bytes(png_ptr, number);

	// Read
	int png_transforms = PNG_TRANSFORM_STRIP_ALPHA; //PNG_TRANSFORM_SCALE_16;
	png_read_png(png_ptr, info_ptr, png_transforms, NULL);

	// Retrieve data	// NORETURN 
	png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);

	// Extracting parameters
	png_uint_32 width, height;
   	int bit_depth, color_type, interlace_type, compression_type, filter_method;
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
		   &interlace_type, &compression_type, &filter_method);	

	if (bit_depth != 8 || color_type != PNG_COLOR_TYPE_RGB)
	{
		cerr << "\nUnsupported set of .png parameters:\n";
		if (bit_depth != 8)
			cerr << "bit_depth = " << bit_depth << endl;
		if (color_type != PNG_COLOR_TYPE_RGB)
			cerr << "color_type = " << color_type << 
				" (RGB is " << PNG_COLOR_TYPE_RGB << ") " << endl;
		goto exit_png;
	}
	iWidth = (signed) width;
	iHeight = (signed) height;

	// Creating intrinsic memory
	if (pRGB)
		delete[] pRGB;
	pRGB = new unsigned char[iSizeRGB()];
	if (!pRGB)
		return 2;

	// Copy
	for (int j = 0; j < iHeight; j++)
		for (int i = 0; i < iWidth*3; i++)
			pRGB[iWidthRGB()*j + i] = row_pointers[j][i];

	cout << "Read an image " << iWidth << "x" << iHeight << endl;

exit_png:
	fclose(fp);
	// Clean
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	return 0;	
}
#endif

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
	
	unsigned char * p;
	unsigned char * p1;
	unsigned char * p2;
	unsigned char * pp2; //= pRGB + iWidthRGB()*(iHeight - 2);
	unsigned char * pp1; //= pRGB + iWidthRGB()*(iHeight - 1);
	int i, j, k, l;
	for (	i = 0, pp1 = pRGB, pp2 = pRGB + iWidthRGB(), p = pY0, p1 = pCr0, p2 = pCb0;
			i < iHeight / 2 * 2; 
			i += 2, pp1 += iWidthRGB() * 2, pp2 += iWidthRGB() * 2, 
				p += iWidth * 2, p1 += iWidth2 * 2 / iSubH, p2 += iWidth2 * 2 / iSubH)
	{
		for (	j = 0, k = 0, l = 0; 
				j < iWidth / 2 * 2; 
				j += 2, k += 6, l += 2 / iSubW)
		{
			int R1 = pp1[k];
			int G1 = pp1[k + 1];
			int B1 = pp1[k + 2];
			int R2 = pp1[k + 3];
			int G2 = pp1[k + 4];
			int B2 = pp1[k + 5];
			int R3 = pp2[k];
			int G3 = pp2[k + 1];
			int B3 = pp2[k + 2];
			int R4 = pp2[k + 3];
			int G4 = pp2[k + 4];
			int B4 = pp2[k + 5];
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
			int R1 = pp1[k];
			int G1 = pp1[k + 1];
			int B1 = pp1[k + 2];
			int R3 = pp2[k];
			int G3 = pp2[k + 1];
			int B3 = pp2[k + 2];
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
			int R1 = pp1[k];
			int G1 = pp1[k + 1];
			int B1 = pp1[k + 2];
			int R2 = pp1[k + 3];
			int G2 = pp1[k + 4];
			int B2 = pp1[k + 5];
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
			int R1 = pp1[k];
			int G1 = pp1[k + 1];
			int B1 = pp1[k + 2];
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

	unsigned char * pBGR = getReverse(pRGB);

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

		if (fwrite(pBGR, 1, iSizeRGB(), fp) < iSizeRGB())
		{
			fclose(fp);
			return 5;
		}

		fclose(fp);
	}
	else
		return 2;
	
	delete[] pBGR;
	cout << "Success" << endl;
	return 0;
}

#ifndef NOJPEG
// Saving to JPEG with standard settings
int cImageRGB::WriteToJpegFile( char* filename, int quality )
{
	cout << "Writing the picture to " << filename << "... ";
	FILE * fd = fopen(filename, "wb");
	if (!fd)
		return 1;

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

	jpeg_stdio_dest(&cinfo, fd);

	cinfo.image_width=iWidth;
	cinfo.image_height=iHeight;
	cinfo.input_components=3;
	cinfo.in_color_space=JCS_RGB;

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE);
	jpeg_start_compress(&cinfo, TRUE);

    JSAMPROW row_pointer[1];
    int row_stride;

    row_stride = cinfo.image_width*3;

    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0]=(JSAMPLE *)(pRGB+cinfo.next_scanline*row_stride);
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	fclose(fd);
	cout << "Success" << endl;
	return 0;	
}
#endif

#ifndef NOPNG
// Saving to PNG with standard settings
int cImageRGB::WriteToPngFile( char* filename )
{
	cout << "Writing the picture to " << filename << "... ";
	FILE * fp = fopen(filename, "wb");
	if (!fp)
		return 1;

	// Create png_struct
    png_structp png_ptr = png_create_write_struct
        (PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr)
       return 4;

	// Crete png_info
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
       png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
       return 5;
    }

	// Set jump to handler
    if (setjmp(png_jmpbuf(png_ptr)))
    {
       png_destroy_write_struct(&png_ptr, &info_ptr);
       fclose(fp);
       return 6;
    }

	// Setting input
	png_init_io(png_ptr, fp);

	// Setting parameters
	png_uint_32 width=iWidth, height=iHeight;
   	int bit_depth = 8, color_type = PNG_COLOR_TYPE_RGB, 
		interlace_type = PNG_INTERLACE_NONE, 
		compression_type = PNG_COMPRESSION_TYPE_DEFAULT, 
		filter_method = PNG_FILTER_TYPE_DEFAULT;
    png_set_IHDR(png_ptr, info_ptr, width, height,
       bit_depth, color_type, interlace_type,
       compression_type, filter_method);
			
	// Creating row array
	// (Copying probably unnecessary)
	png_bytep row_pointers[iHeight];
	for (int i = 0; i<iHeight; i++)
		row_pointers[i] = pRGB+(iWidthRGB()*i);
	
	// Set data
	png_set_rows(png_ptr, info_ptr, row_pointers);


	// Write
	int png_transforms = 0; //PNG_TRANSFORM_SCALE_16;
	png_write_png(png_ptr, info_ptr, png_transforms, NULL);

	// Clean
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fp);

	cout << "Success" << endl;
	return 0;	
}
#endif

unsigned char* cImageRGB::getReverse(unsigned char* pRGB)
{
	if (!pRGB)
	{
		cerr << __func__ << ": Null argument passed" << endl;
		return nullptr;
	}
	bool flag;
	int iWidthRGB = this->iWidthRGB();
	unsigned char * new_pRGB = new unsigned char[iSizeRGB()];
	if (!new_pRGB)
	{
		perror(__func__);
		return nullptr;
	}
	for (int i = 0; i < iHeight; i++)
		for (int j = 0; j < iWidth; j++) {
			new_pRGB[(iHeight-1-i)*iWidthRGB + 3*j + 0] = pRGB[i*iWidthRGB + 3*j + 2];
			new_pRGB[(iHeight-1-i)*iWidthRGB + 3*j + 1] = pRGB[i*iWidthRGB + 3*j + 1];
			new_pRGB[(iHeight-1-i)*iWidthRGB + 3*j + 2] = pRGB[i*iWidthRGB + 3*j + 0];
			flag = (iHeight-1-i)*iWidthRGB + 3*j + 2 < iSizeRGB();
		}

	return new_pRGB;
}

extension_t cImageRGB::extract_extension(const char * filename)
{
	for (const char * p = filename+strlen(filename)-1; p>=filename; p--)
		if (*p == '.') {
			if (!strcasecmp(p+1, "bmp"))
				return BMP;
			else if (!strcasecmp(p+1, "jpeg") || !strcasecmp(p+1, "jpg"))
				return JPEG;
			else if (!strcasecmp(p+1, "png"))
				return PNG;
			else 
				return OTHER;
		}
	return NOTYPE;
}

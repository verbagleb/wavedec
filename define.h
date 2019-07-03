#ifndef DEFHEADER
#define DEFHEADER

//#define nullptr NULL
//#define fopen_s(a,b,c) a=fopen(b,c)
#include <string.h>
#include <limits.h>
#include <stdint.h>
#define PI 3.14159265358979323846

typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int32_t LONG;
	
typedef struct tagBITMAPFILEHEADER {
	WORD  bfType;
	DWORD bfSize;
	WORD  bfReserved1;
	WORD  bfReserved2;
	DWORD bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
	DWORD biSize;
	LONG  biWidth;
	LONG  biHeight;
	WORD  biPlanes;
	WORD  biBitCount;
	DWORD biCompression;
	DWORD biSizeImage;
	LONG  biXPelsPerMeter;
	LONG  biYPelsPerMeter;
	DWORD biClrUsed;
	DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

#endif

#ifndef DEFHEADER
#define DEFHEADER

//#define nullptr NULL
//#define fopen_s(a,b,c) a=fopen(b,c)
#include <string.h>
#include <limits.h>
#include <stdint.h>
#define PI 3.14159265358979323846

#ifndef WINDOWS // LINUX requires to determine the following

typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int32_t LONG;
	
// Turning alignment off
#pragma pack(push, 1)

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

// Turning alignment back
#pragma pack(pop)

#endif // !WINDOWS

// Error processing
#define ERR_MESSAGE_OUT { cerr << err_message << " " << strerror(errno) << endl; }
#define CHECK_ZERO(VAR) do { if((VAR)) ERR_MESSAGE_OUT } while(false)
#define CHECK_NZERO(VAR) do { if(!(VAR)) ERR_MESSAGE_OUT } while(false)
#define CHECK_VAL(VAR, VAL) do { if((VAR)!=(VAL)) ERR_MESSAGE_OUT } while(false)

// Windows mods
#ifdef WINDOWS
#include <windows.h>
#define MKDIR(A,B) mkdir(A)
#define error(A,B,C) do { \
	cerr << C << ": " << strerror(B) << "\nPress ENTER\n"; \
	getchar(); \
	return A; \
} while(false)
#define SYMLINK(A,B) do { \
	char command[256]; \
	sprintf(command, "mklink /j %s %s", B, A); \
	errno = system(command); \
} while (false)
#define RMLINK(A) rmdir(A)
#else
#define MKDIR(A,B) mkdir(A, B)
#define SYMLINK(A,B) symlink(A,B)
#define RMLINK(A) remove(A)
#endif

#endif

#pragma once
#include "cFilter.h"
#include "define.h"
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <assert.h>
#include <complex>
#include <unistd.h>

typedef std::complex<double> compd;

extern int bAbsDivision;

const char comp_name[3][3] = {"Y", "Cr", "Cb"};

int ceilTo(int number, int base, int remainder);
int readConfig(const char *config_name, int &nFilters, cFilter *& pFilter);
int readGrid(const char * grid_name, int &subW, int &subH, 
		char ** &images_array, int &images_number, double * &qs_array, int &qs_number);
int Limiting_a_b(double d, int a, int b);
short lowest(short * array, int size);
short highest(short * array, int size);
unsigned int * distribution(short * array, int size);
double entropy(short * array, int size);
void comp_entropy(short ** coeff_orig, short ** coeff_pred, int * coeff_pred_ln, short ** ref_sampl, int * ref_sampl_ln, short ** coeff_recon, int * skip_blk, int * total_blk, int * sub_width, int * sub_height, short ** pred_modes, short ** skip_flags, int ** split_blocks, int * split_depth, int * split_total, double * split_ent,  int BlockSize, int totalBands, FILE *log_p, FILE *log_short);
void comp_psnr(double * error, int totalBands, FILE * log_short);
int formOutput(char * output_dir_name, const char * bitmap_name, double quantStep, int totalBands, FILE ** log_this);
double psnr(short * original, short * distorted, int size);
short * difference(short * a, short * b, int size);
// GV - 30.05.18 //
void absDivide(short * src, short * abs_dest, short * sign_dest, int size);
void absUnite(short * abs_src, short * sign_src, short * dest, int size);
// ------------- //
int suppressValue(short * src, int size, short val, short * dest, bool * ind = nullptr);
short * getContext(short * data, int width, int height);
double entropy_cont(short * array, int width, int height, short supVal = SHRT_MAX);
double entropy(short * array, short * context, int size, short supVal = SHRT_MAX);
int getSign(int num);
// FFT //
compd * fft(compd * array, int n);
compd * ifft(compd * array, int n);

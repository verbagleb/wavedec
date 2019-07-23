// WaveletDecomposition.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include <stdio.h>
#include <cstdio>
#include <math.h>
#include <iostream>
#include <fstream>
#include <errno.h>
#include <error.h>

#include "define.h"
#include "cImageRGB.h"
#include "cImageYCbCr.h"
#include "userLib.h"
#include "decTree.h"
				
#ifdef WINDOWS
#define RETURN(A) do { \
	cout << "Press ENTER" << endl; \
	getchar(); \
	return A; \
} while(false)
#else
#define RETURN return
#endif

#define  INVSYNTAX do \
	{ \
	printf ("Invalid hierarchy syntax:\n\t%s\n\t",cline_array[cline_index]); \
	for (char * p = cline_array[cline_index]; p < c; p++) \
		printf(" "); \
	printf("^\n"); \
	RETURN(-1); } \
	while(false)

using namespace std;

// GV: Global parameters instead of defines
int bAllowSkipPred, bSplit, bFiltering, bAbsDivision;

// Components of prediction entropy to add into sum in short log
//bool bError = true;
//bool bModes = true;
//bool bReferenceSamples = true;
//bool bSplitFlags = true;
//bool bSigns = true;
//bool bSkipFlags = true;
bool bContext = false;		// defines whether the entropy uses context

int main(
	int argc,      // Number of strings in array argv
	char *argv[]
)
{
	if (argc != 1)
	{
		cerr << "No parameters are supported" << endl;
		RETURN(1);
	}

	const double large_qf = 20.0;

	const char config_name[] = "filters.cfg";
	const char grid_name[] = "parameters.cfg";
	const char image_dir[]="images/";
	char output_dir_name[128] = ""; // Let it find a new value

	int i;
	
	// reading config file
	int iSubH = 0, iSubW = 0;		//subsampling
	char ** images_array = nullptr, ** cline_array = nullptr;
	double * qs_array = nullptr;
	int images_number = 0, qs_number = 0, cline_number = 0;
	cFilter *pFilter;
	int nFilters;

	// Reads filter list
	i = readConfig(config_name, nFilters, pFilter);
	if (i)
		RETURN(2);
	for (int iFilter=0; iFilter<nFilters; iFilter++)
		pFilter[iFilter].normalize();
	// Reads decomposition parameters
	i = readGrid(grid_name, iSubW, iSubH, images_array, images_number, qs_array, qs_number, cline_array, cline_number);
	if (i)
		RETURN(3);

	for (int image_index=0; image_index < images_number; image_index++)
	{
		const char * image_name = images_array[image_index];
		char image_dir_name[128] = "";
		sprintf(image_dir_name, "%s%s", image_dir, image_name);

		cImageRGB * pImageRGB = new cImageRGB;
		if (!pImageRGB)
		{
			printf("Error creating cImage !\n");
			delete[] pFilter;
			RETURN(4);
		}

		int picWidth, picHeight;
		switch (cImageRGB::extract_extension(image_name))
		{
			case BMP:
				if (int i = pImageRGB->CreateFromBitmapFile(image_dir_name, &picWidth, &picHeight))
				{
					printf("Error reading bitmap file %s, err_num = %i !\n", image_dir_name, i);
					delete pImageRGB;
					pImageRGB = nullptr;
					delete[] pFilter;
					RETURN(4);
				}
				break;
			case JPEG:
				if (int i = pImageRGB->CreateFromJpegFile(image_dir_name, &picWidth, &picHeight))
				{
					printf("Error reading jpeg file %s, err_num = %i !\n", image_dir_name, i);
					delete pImageRGB;
					pImageRGB = nullptr;
					delete[] pFilter;
					RETURN(4);
				}
				break;
			case PNG:
				if (int i = pImageRGB->CreateFromPngFile(image_dir_name, &picWidth, &picHeight))
				{
					printf("Error reading png file %s, err_num = %i !\n", image_dir_name, i);
					delete pImageRGB;
					pImageRGB = nullptr;
					delete[] pFilter;
					RETURN(4);
				}
				break;
			case OTHER:
				error(1, 0, "Unknown extension");
			case NOTYPE:
				error(1, 0, "No extension");
		}

		cImageYCbCr *pImage_o = pImageRGB->CreateYCrCbFromRGB(iSubW, iSubH);
		if (!pImage_o)
		{
			printf("Error creating YCrCb, err_num = %i !\n", i);
			delete pImageRGB;
			pImageRGB = nullptr;
			delete[] pFilter;
			RETURN(6);
		}
		delete pImageRGB;
		// grey
		//pImage_o->paintComp(Cr,128);
		//pImage_o->paintComp(Cb,128);
		//
		//pImage_o->setGrey();

		decTree * pDecTree = new decTree;
		if (!pDecTree)
			RETURN(7);

		// Image is processed in a tree representation
		i = pDecTree->loadImage(pImage_o);
		if (i)
		{
			delete pImage_o;
			delete[] pFilter;
			RETURN(8);
		}

		// Making a copy of original
		decTree * pDecTree_original = new decTree;
		if (!pDecTree_original)
		{
			delete pImage_o;
			delete[] pFilter;
			RETURN(9);
		}

		pDecTree->copyTree(pDecTree_original);

		// Decomposition according to parameters.cfg
		for (int cline_index = 0; cline_index < cline_number; cline_index++)
		{
			char * c = cline_array[cline_index];
			char * end = c + strlen(c);
			char * aux;
			decTree * pBand = pDecTree;
			
			if (c == end)
				continue;

			while (*c == '(') 
			{
				if (++c >= end)
					INVSYNTAX;
				int xBand = strtol(c,&aux,10);
				if (!aux || aux == c || *aux != ',')
					INVSYNTAX;
				else
					c = aux + 1;
				int yBand = strtol(c,&aux,10);
				if (!aux || aux == c || *aux != ')')
					INVSYNTAX;
				else
					c = aux + 1;

				pBand = pBand->stepAt(xBand, yBand);
				if (!pBand)
					error(7, 0, "No such band in decomposition");
			}

			int iFilter = strtol(c, &aux, 10);
			if (!aux || aux == c || *aux != '(')
				INVSYNTAX;
			else 
				c = aux + 1;
			if (iFilter >= nFilters || iFilter < 0)
				error(8, 0, "Filter index exceeds the limits");

			bool byW, byH;
			if (*c != '0' && *c != '1')
				INVSYNTAX;
			else 
				byW = (*c == '1');
			if (*++c != ',')
				INVSYNTAX;
			if (*++c != '0' && *c != '1')
				INVSYNTAX;
			else 
				byH = (*c == '1');

			if (*++c != ')' || *++c != '\0')
				INVSYNTAX;

			if (byW && byH)
				pBand->analyseBandWH(pFilter+iFilter);
			else if (byW)
				pBand->analyseBandW(pFilter+iFilter);
			else if (byH)
				pBand->analyseBandH(pFilter+iFilter);
			cout << cline_array[cline_index] << endl;
		}

		/*	double commonMult = 20.0;
			for (int i=0; i<pDecTree->getNumH(); i++)
				for (int j=!i; j<pDecTree->getNumW(); j++)
					pDecTree->stepAt(i,j)->setMult(commonMult);
		*/

		// log files descriptors
		int totalBands = pDecTree->countBands();	//GV
		FILE * log_general[1] = {}; 
		FILE * log_energy[3] = {}, * log_entropy[3] = {}, * log_psnr[3] = {};

		// Getting band names for header generation
		char ** band_names = new char* [totalBands];
		if (!band_names)
			return 7;
		decTree::getAllNames(band_names, pDecTree);
		int maxlen = getMaxLength(band_names, totalBands);

		// Formation of all log outputs
		if (formOutput(output_dir_name, log_general, log_energy, log_entropy, log_psnr,
					totalBands, band_names))
			RETURN(10);

		// Memory liberation
		for (int i = 0; i < totalBands; i++)
			delete[] band_names[i];
		delete[] band_names;

		for (int qs_index = 0; qs_index < qs_number; qs_index++)
		{
			double quantStep = qs_array[qs_index];
			fprintf(log_general[0],"%s\t%.2f", image_name, quantStep);

			if (qs_index == 0)
			{
				double * energy = new double [totalBands];
				if (!energy)
					RETURN(11);
				for (int compnum = 0; compnum < 3; compnum++)
				{
					component comp = (component) compnum;
					double energy_original = decTree::getSubEnergy(pDecTree_original, comp);

					fprintf(log_energy[compnum], "%19s (rms)", image_name);
					decTree::getAllEnergy(energy, pDecTree, comp);
					double sum_sqrt = 0, sum = 0;
					for (int i=0; i<totalBands; i++)
					{
						sum_sqrt += sqrt(energy[i]/energy_original);
						fprintf(log_energy[compnum], "\t%*.5f", 
								maxlen, sqrt(energy[i]/energy_original));
					}
					fprintf(log_energy[compnum], "\n%20s (ms)","");
					for (int i=0; i<totalBands; i++)
					{
						sum += (energy[i]/energy_original);
						fprintf(log_energy[compnum], "\t%*.5f", 
								maxlen, energy[i]/energy_original);
					}
					fprintf(log_energy[compnum], "\n");
					fclose(log_energy[compnum]);
				}
				delete[] energy;
			}

			cout << image_name << " " << quantStep << endl;

			// For storing dequantized
			decTree * pDecTree_recon = new decTree;
			if (!pDecTree_recon)
				RETURN(13);
			pDecTree->copyTree(pDecTree_recon);

#ifdef PRINT_SEPARATE_BANDS
			char str_buffer[6][1024] = {};
#endif
			for (int compnum=0; compnum<3; compnum++)
			{
				component comp = (component) compnum;
				double extension = 1/quantStep;

				short ** coeff_orig = new short *[totalBands];
				int * sub_width = new int [totalBands];
				int * sub_height = new int [totalBands];
				if (!coeff_orig || !sub_width || !sub_height)
					RETURN(14);

				// Extracting the coeffs with quanization
				int f = pDecTree->getAllCoefs(coeff_orig, sub_width, sub_height, extension, comp);
				if (!f || f!=totalBands)
					RETURN(15);
				// Storing with dequantization
				pDecTree_recon->setAllCoefs(coeff_orig, extension, comp, 2);

				double ent0s, ent0_sum = 0;
				for (int i=0; i<totalBands; i++)
				{
					int size = sub_width[i]*sub_height[i];
					if (bContext)
						ent0s = entropy_cont(coeff_orig[i], sub_width[i], sub_height[i]);
					else
						ent0s = entropy(coeff_orig[i], size)*size;
					ent0_sum += ent0s;
#ifdef PRINT_SEPARATE_BANDS
					sprintf(str_buffer[compnum]+strlen(str_buffer[compnum]), "\t%*.f", 
							maxlen, ent0s);
#endif
				}
				fprintf(log_general[0],"\t%10.f", ent0_sum);

				delete[] sub_width;
				delete[] sub_height;
				for (int i=0; i<totalBands; i++)
					delete[] coeff_orig[i];
				delete[] coeff_orig;
			}

#ifdef PRINT_SEPARATE_BANDS
			double * psnr = new double [totalBands];
			if (!psnr)
				RETURN(16);
			for (int compnum = 0; compnum < 3; compnum++)
			{
				component comp = (component) compnum;
				decTree::getAllPSNR(psnr, pDecTree, pDecTree_recon, comp);
				for (int i=0; i<totalBands; i++)
					sprintf(str_buffer[3+compnum]+strlen(str_buffer[3+compnum]), "\t%*f", 
							maxlen, psnr[i]);
			}
			delete[] psnr;
#endif

#ifdef BAND_IMAGE_OUTPUT
			{
				// Decomposition image
				cImageYCbCr * pImage_b=pDecTree_recon->createImage(false);
				if (!pImage_b)
					RETURN(17);
				pImage_b->setSubW(iSubW);
				pImage_b->setSubH(iSubH);
			
				cImageRGB *pOut = pImage_b->CreateRGB24FromYCbCr();
				if (!pOut)
					RETURN(18);

				char bands_name[128], bands_dir[128];
				sprintf(bands_dir, "%s/bands", output_dir_name);
				i = MKDIR(bands_dir, 0777);
				if (i && errno!=EEXIST)
					error(19, errno, "Restored directory");
			   	sprintf(bands_name, "%s/%s_%.3f_bands.bmp", 
						bands_dir, image_name, quantStep);
				i = pOut->WriteToBitmapFile(bands_name);
				if (i)
					RETURN(200 + i);

				delete pImage_b;
				delete pOut;
			}
#endif

			// Composition
			if (	pDecTree_recon->synthesizeBand()	)
				RETURN(205);

			//output of whole-image PSNR
			for (int compnum = 0; compnum < 3; compnum++)
			{
				component comp = (component) compnum;
				double psnr = decTree::getSubPSNR(pDecTree_original, pDecTree_recon, comp);
				fprintf(log_general[0], "\t%.3f", psnr);
			}
			fprintf(log_general[0],"\n");

#ifdef PRINT_SEPARATE_BANDS 
			for (int compnum=0; compnum<3; compnum++)
				fprintf(log_entropy[compnum],"%25s\t%8.4f%s\n", 
						image_name, quantStep, str_buffer[compnum]); 
			for (int compnum=0; compnum<3; compnum++)
				fprintf(log_psnr[compnum],"%25s\t%8.4f%s\n", 
						image_name, quantStep, str_buffer[3+compnum]); 
#endif

#ifdef RESTORED_IMAGE_OUTPUT
			{	
				// Reconstruced image
				cImageYCbCr * pImage_r=pDecTree_recon->createImage(false);
				if (!pImage_r)
					RETURN(21);
				pImage_r->setSubW(iSubW);
				pImage_r->setSubH(iSubH);
				double multdif = 10.0;
				cImageYCbCr *pDiff = cImageYCbCr::difference(pImage_o, pImage_r, multdif);
				if (!pDiff)
					RETURN(22);
				cImageRGB *pOutR = pImage_r->CreateRGB24FromYCbCr();
				if (!pOutR)
					RETURN(23);

				char restored_name[128], restored_dir[128];
				sprintf(restored_dir, "%s/restored", output_dir_name);
				i = MKDIR(restored_dir, 0777);
				if (i && errno!=EEXIST)
					error(24, errno, "Restored directory");
#ifdef OUT_BMP
				// to bmp
			   	sprintf(restored_name, "%s/%s_%.3f.bmp", 
						restored_dir, image_name, quantStep);
				i = pOutR->WriteToBitmapFile(restored_name);
				if (i)
					RETURN(2510 + i);
#endif
#ifdef OUT_JPEG
				// to jpeg 
			   	sprintf(restored_name, "%s/%s_%.3f.jpeg", 
						restored_dir, image_name, quantStep);
				i = pOutR->WriteToJpegFile(restored_name, 100);
				if (i)
					RETURN(2520 + i);
#endif
#ifdef OUT_PNG
				// to png
			   	sprintf(restored_name, "%s/%s_%.3f.png", 
						restored_dir, image_name, quantStep);
				i = pOutR->WriteToPngFile(restored_name);
				if (i)
					RETURN(2530 + i);
#endif


				// Difference of images
				cImageRGB *pOutD = pDiff->CreateRGB24FromYCbCr();
				if (!pOutD)
					RETURN(26);
				char diff_name[128], diff_dir[128];
				sprintf(diff_dir, "%s/difference_x%.2f", output_dir_name, multdif);
				i = MKDIR(diff_dir, 0777);
				if (i && errno!=EEXIST)
					error(27, errno, "Restored directory");
			   	sprintf(diff_name, "%s/%s_%.2f.bmp", 
						diff_dir, image_name, quantStep);
				i = pOutD->WriteToBitmapFile(diff_name);
				if (i)
					RETURN(280 + i);
				delete pImage_r;
				delete pDiff;
				delete pOutR;
				delete pOutD;
			}
#endif

			delete pDecTree_recon;
		}

		for (int k=0; k<1; k++)
			fclose(log_general[k]);
#ifdef PRINT_SEPARATE_BANDS
		for (int k=0; k<3; k++)
		{
			fclose(log_entropy[k]);
			fclose(log_psnr[k]);
		}
#endif

		delete pImage_o;
		delete pDecTree;
		delete pDecTree_original;
	}

	delete[] pFilter; 

	for (int i = 0; i < images_number; i++)
		delete[] images_array[i];
	delete[] images_array;
	delete[] qs_array;
	for (int i = 0; i < cline_number; i++)
		delete[] cline_array[i];
	delete[] cline_array;
	
	RETURN(0);
}


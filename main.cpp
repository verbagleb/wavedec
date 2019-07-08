// WaveletDecomposition.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include <stdio.h>
#include <cstdio>
#include "define.h"
#include <math.h>
#include <iostream>
#include <fstream>

#include "cImageRGB.h"
#include "cImageYCbCr.h"
#include "userLib.h"
#include "decTree.h"
#include "errno.h"

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
		return -1;
	}

	const double large_qf = 20.0;

	const char config_name[] = "filters.cfg";
	const char grid_name[] = "parameters.cfg";
	const char bitmap_dir[]="images/";
	char output_dir_name[128] = ""; // Let it find a new value

	int i;
	
	// reading config file
	int iSubH = 0, iSubW = 0;		//subsampling
	char ** images_array = nullptr;
	double * qf_array = nullptr;
	int images_number = 0, qf_number = 0;
	cFilter *pFilter;
	int nFilters;

	readConfig(config_name, nFilters, pFilter);
	for (int iFilter=0; iFilter<nFilters; iFilter++)
		pFilter[iFilter].normalize();
	readGrid(grid_name, iSubW, iSubH, images_array, images_number, qf_array, qf_number);
	/*if (filterIndex >= nFilters || filterIndex < 0)
	{
		cerr << "Filter index exceeds the limits. Set to 0\n";
		filterIndex = 0;
	}*/

	for (int image_index=0; image_index < images_number; image_index++)
	{
		const char * bitmap_name = images_array[image_index];
		char bitmap_dir_name[128] = "";
		sprintf(bitmap_dir_name, "%s%s", bitmap_dir, bitmap_name);

		cImageRGB * pImageRGB = new cImageRGB;
		if (!pImageRGB)
		{
			printf("Error creating cImage !\n");
			delete[] pFilter;
			return 2;
		}

		int picWidth, picHeight;
		i = pImageRGB->CreateFromBitmapFile(bitmap_dir_name, &picWidth, &picHeight);
		if (i)
		{
			printf("Error reading bitmap file %s, err_num = %i !\n", bitmap_dir_name, i);
			delete pImageRGB;
			pImageRGB = nullptr;
			delete[] pFilter;
			return 3;
		}

		cImageYCbCr *pImage_o = pImageRGB->CreateYCrCb420FromRGB(iSubW, iSubH);
		if (i)
		{
			printf("Error creating YCrCb, err_num = %i !\n", i);
			delete pImageRGB;
			pImageRGB = nullptr;
			delete[] pFilter;
			return 4;
		}
		delete pImageRGB;
		// grey
		//pImage_o->paintComp(Cr,128);
		//pImage_o->paintComp(Cb,128);
		//
		//pImage_o->setGrey();

		decTree * pDecTree = new decTree;

		pDecTree->loadImage(pImage_o);
		if (!pDecTree)
			return 5;

		decTree * pDecTree_original = new decTree;
		pDecTree->copyTree(pDecTree_original);

		int xband=0;
		int yband=0;
		//double epsilon = 7.0;
		pDecTree->analyseBandWH(&pFilter[0]);
			//pDecTree->stepAt(xband,yband)->analyseBandWH(&pFilter[2]);
		/*	pDecTree->analyseBandWH(&pFilter[0]);
			pDecTree->stepAt(0,0)->analyseBandWH(&pFilter[0]);
			pDecTree->stepAt(1,0)->analyseBandH(&pFilter[0], true);
			pDecTree->stepAt(2,0)->analyseBandH(&pFilter[0], true);
			pDecTree->stepAt(0,1)->analyseBandW(&pFilter[0], true);
			pDecTree->stepAt(0,2)->analyseBandW(&pFilter[0], true);
			pDecTree->stepAt(1,1)->analyseBandWH(&pFilter[2], true);
			pDecTree->stepAt(2,1)->analyseBandH(&pFilter[2], true);
			pDecTree->stepAt(1,2)->analyseBandW(&pFilter[2], true);
			pDecTree->stepAt(2,2)->analyseBandWH(&pFilter[2], true);
			pDecTree->stepAt(0,0)->stepAt(0,0)->analyseBandWH(&pFilter[0], false);
		*/
		/*	double commonMult = 20.0;
			for (int i=0; i<pDecTree->getNumH(); i++)
				for (int j=!i; j<pDecTree->getNumW(); j++)
					pDecTree->stepAt(i,j)->setMult(commonMult);
		*/

		int totalBands = pDecTree->countBands();	//GV
		FILE * log_short[1]; // array for separate components

		for (int qf_index = 0; qf_index < qf_number; qf_index++)
		{
			double quantFactor = qf_array[qf_index];
			if (formOutput(output_dir_name, bitmap_name, quantFactor, totalBands, log_short))
				return -3;
			cout << bitmap_name << " " << quantFactor << endl;

			decTree * pDecTree_recon = new decTree;
			pDecTree->copyTree(pDecTree_recon);

			for (int compnum=0; compnum<3; compnum++)
			{
				component comp = (component) compnum;
				double extension = quantFactor;

				short ** coeff_orig = new short *[totalBands];
				int * sub_width = new int [totalBands];
				int * sub_height = new int [totalBands];

				int f = pDecTree->getAllCoefs(coeff_orig, sub_width, sub_height, extension, comp);
				if (!f || f!=totalBands)
					return 10;
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
					fprintf(log_short[0],"\t%.f", ent0s);
#endif
				}
				fprintf(log_short[0],"\t%.f", ent0_sum);
			}

#ifdef PRINT_SEPARATE_BANDS
			double * psnr = new double [totalBands];
			for (int compnum = 0; compnum < 3; compnum++)
			{
				component comp = (component) compnum;
				decTree::getAllPSNR(psnr, pDecTree, pDecTree_recon, comp);
				for (int i=0; i<totalBands; i++)
					fprintf(log_short[0], "\t%.3f", psnr[i]);
			}
			delete[] psnr;
#endif

#ifdef BAND_IMAGE_OUTPUT
			{
				cImageYCbCr * pImage_b=pDecTree->createImage(false);
				if (!pImage_b)
					return 6;
				pImage_b->setSubW(iSubW);
				pImage_b->setSubH(iSubH);
			
				cImageRGB *pOut = pImage_b->CreateRGB24FromYCbCr420();
				if (!pOut)
					return 9;

				char bands_name[128], bands_dir[128];
				sprintf(bands_dir, "%s/bands", output_dir_name);
				mkdir(bands_dir, 0777);
			   	sprintf(bands_name, "%s/%s_%.3f_bands.bmp", 
						bands_dir, bitmap_name, 1/quantFactor);
				i = pOut->WriteToBitmapFile(bands_name);
				if (i)
					return 100 + i;

				delete pImage_b;
				delete pOut;
			}
#endif

			pDecTree_recon->synthesizeBand();

			//output of whole-image PSNR
			for (int compnum = 0; compnum < 3; compnum++)
			{
				component comp = (component) compnum;
				double psnr = decTree::getSubPSNR(pDecTree_original, pDecTree_recon, comp);
				fprintf(log_short[0], "\t%.3f", psnr);
			}
			fprintf(log_short[0],"\n");

#ifdef RESTORED_IMAGE_OUTPUT
			{	
				cImageYCbCr * pImage_r=pDecTree_recon->createImage(false);
				if (!pImage_r)
					return 15;
				pImage_r->setSubW(iSubW);
				pImage_r->setSubH(iSubH);
				double multdif = 10.0;
				cImageYCbCr *pDiff = cImageYCbCr::difference(pImage_o, pImage_r, multdif);
				if (!pDiff)
					return 11;
				cImageRGB *pOutR = pImage_r->CreateRGB24FromYCbCr420();
				if (!pOutR)
					return 9;

				char restored_name[128], restored_dir[128];
				sprintf(restored_dir, "%s/restored", output_dir_name);
				mkdir(restored_dir, 0777);
			   	sprintf(restored_name, "%s/%s_%.3f.bmp", 
						restored_dir, bitmap_name, 1/quantFactor);
				i = pOutR->WriteToBitmapFile(restored_name);
				if (i)
					return 100 + i;
				cImageRGB *pOutD = pDiff->CreateRGB24FromYCbCr420();
				if (!pOutD)
					return 12;
				char diff_name[128], diff_dir[128];
				sprintf(diff_dir, "%s/difference_x%.2f", output_dir_name, multdif);
				mkdir(diff_dir, 0777);
			   	sprintf(diff_name, "%s/%s_%.2f.bmp", 
						diff_dir, bitmap_name, 1/quantFactor);
				i = pOutD->WriteToBitmapFile(diff_name);
				if (i)
					return 130 + i;
				delete pImage_r;
				delete pDiff;
				delete pOutR;
				delete pOutD;
			}
#endif

			for (int k=0; k<1; k++)
				fclose(log_short[k]);

			delete pDecTree_recon;
		}

		delete pImage_o;
		delete pDecTree;
		delete pDecTree_original;
	}

	delete[] pFilter; 

	for (int i = 0; i < images_number; i++)
		delete[] images_array[i];
	delete[] images_array;
	delete[] qf_array;
	
	return 0;
}


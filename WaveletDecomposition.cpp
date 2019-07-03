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
#include "intra.h"
#include "errno.h"

using namespace std;

// GV: Global parameters instead of defines
int bAllowSkipPred, bSplit, bFiltering, bAbsDivision, predScheme;

// Components of prediction entropy to add into sum in short log
bool bError = true;
bool bModes = true;
bool bReferenceSamples = true;
bool bSplitFlags = true;
bool bSigns = true;
bool bSkipFlags = true;
bool bContext = false;		// defines whether the entropy uses context

int main(
	int argc,      // Number of strings in array argv
	char *argv[]
)
{
	const double large_qf = 20.0;

	//GV: changed last input arguments //
	char config_name[64];
	char dir_name[64];
	if (argc == 8)
	{
		strcpy(config_name, argv[6]);
		strcpy(dir_name, argv[7]);
	}
	else if (argc == 7)
	{
		strcpy(config_name, argv[6]);
		dir_name[0]='\0';
	}
	else if (argc == 6)
	{
		strcpy(config_name, "config.cfg");
		dir_name[0]='\0';
	}
	else 
	{
		cout << "Syntax:\nwavedec file_name filter_index block_size pred_mode quant_factor [config_name [log_dir_name]]\n";
		return -1;
	}
	// ***************************** //

	char* bitmap_name = argv[1];
	int filterIndex = atoi(argv[2]);
	int blockSize = atoi(argv[3]);
	int predMode = atoi(argv[4]);
	double quantFactor = strtod(argv[5], nullptr);
	char bitmap_name_dir[128]="images/";
	strcat(bitmap_name_dir, bitmap_name);
	// in adaptive modes skip flags are included in mode array
	if (predMode == 10)
		bSkipFlags = false; 

	//if (blockSize!=4 && blockSize!=8 && 
	//	blockSize!=16 && blockSize!=32)
	if (blockSize<0 || blockSize%2!=0)
	{
		cout << "Incorrect block size" << endl;
		return -2;
	}

	int i;
	
	// reading config file
	int iSubH, iSubW;		//subsampling
	cFilter *pFilter;
	int nFilters;

	readConfig(config_name, bAllowSkipPred, bSplit, bFiltering, bAbsDivision, predScheme, iSubH, iSubW, nFilters, pFilter);
	for (int iFilter=0; iFilter<nFilters; iFilter++)
		pFilter[iFilter].normalize();
	if (filterIndex >= nFilters || filterIndex < 0)
	{
		cerr << "Filter index exceeds the limits. Set to 0\n";
		filterIndex = 0;
	}

	cImageRGB * pImageRGB = new cImageRGB;
	if (!pImageRGB)
	{
		printf("Error creating cImage !\n");
		delete[] pFilter;
		return 2;
	}

	int picWidth, picHeight;
	i = pImageRGB->CreateFromBitmapFile(bitmap_name_dir, &picWidth, &picHeight);
	if (i)
	{
		printf("Error reading bitmap file %s, err_num = %i !\n", bitmap_name_dir, i);
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

	int xband=0;
	int yband=0;
	//double epsilon = 7.0;
	#ifndef SKIP_ANALYSIS
		pDecTree->analyseBandWH(&pFilter[filterIndex]);
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
	#endif


	//decTree * pDT_part = pDecTree_pt;//->stepAt(xsub,ysub); //only half-band predicted
	decTree * pDT_part = pDecTree;//->stepAt(xband,yband); //only half-band predicted
	bool bRestriction = pDT_part != pDecTree;
	int totalBands = pDT_part->countBands();	//GV

	FILE * log;
	FILE * log_short[3]; // array for separate components

	if (blockSize)
	{
		if (formOutput(dir_name, bitmap_name, blockSize+bSplit*100, predMode, quantFactor, totalBands, &log, log_short))
		return -3;
	}
	else
	{
		if (formOutput(dir_name, bitmap_name, filterIndex, predMode, quantFactor, totalBands, &log, log_short))
		return -3;
	}

	fprintf(log, "image:\t%s\nfilter:\t%d(%s)\nblock size:\t%d\npred mode:\t%d\nquant factor:\t%f\n", bitmap_name, filterIndex, pFilter[filterIndex].sFilterName, blockSize, predMode, quantFactor);
	fprintf(log, "number of bands:\t%d\n\n", totalBands);

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
		i = pOut->WriteToBitmapFile("output/Bands.bmp");
		if (i)
			return 100 + i;

		delete pImage_b, pOut;
	}
	#endif

	decTree * pDT_pred = new decTree;
	decTree * pDT_recon = new decTree;

	#ifdef INTRA_PRED	

		decTree * pDT_wb = new decTree;
		pDT_part->copyStructure(pDT_wb);
		pDT_part->copyStructure(pDT_pred);
		pDT_part->copyTree(pDT_recon);
#ifdef SKIP_COLORS
		for (int compnum=0; compnum<1; compnum++)
#else
		for (int compnum=0; compnum<3; compnum++)
#endif
		{
			component comp = (component) compnum;
			double extension = quantFactor;

			short ** coeff_orig = new short *[totalBands];
			short ** coeff_pred = new short *[totalBands];
			short ** coeff_recon = new short *[totalBands];
			int * sub_width = new int [totalBands];
			int * sub_height = new int [totalBands];

			int f;
			switch (predScheme) {
				case 1:
					f = pDT_part->getAllCoefs(coeff_orig, sub_width, sub_height, extension, comp);
					if (!f || f!=totalBands)
						return 10;
					cout << f << " bands loaded" << endl;
					break;
				case 2:
					f = pDT_part->getAllCoefs(coeff_orig, sub_width, sub_height, large_qf, comp);
					if (!f || f!=totalBands)
						return 10;
					cout << f << " bands loaded" << endl;
					break;
				default:
					cout << predScheme << endl;
					return -100;
			}

			// GV - 30.05.18 //
			short ** coeff_orig_abs = new short *[totalBands];
			short ** coeff_orig_sgn = new short *[totalBands];
			short ** coeff_recon_abs = new short *[totalBands];
			short ** conc_pred_data = new short * [totalBands];
			// ------------- //
			
			int * skipped_blocks = new int [totalBands];
			int * total_blocks = new int [totalBands];
			INTRA_PRED_MODE ** pred_modes = new INTRA_PRED_MODE* [totalBands];
			short ** skip_flags = new short* [totalBands];
			short ** ref_sampl = new short *[totalBands];
			int * coeff_pred_ln = new int [totalBands];
			int * ref_sampl_ln = new int [totalBands];
			int ** split_blocks = new int *[totalBands];
			int * split_depth = new int [totalBands];
			double * split_ent = new double [totalBands];
			int * split_total = new int [totalBands];

			for (int i = 0; i<totalBands; i++)
			{
				int size = sub_width[i]*sub_height[i];
				//cout << i << ":" << lowest(coeff_orig[i], size) << " " << highest(coeff_orig[i], size) << endl;
				/* KB: Copy original DWT coeffs to new structs and making Intra Prediction within Subband */
				coeff_orig_abs[i] = new short[size];
				coeff_orig_sgn[i] = new short[size];
				coeff_recon_abs[i] = new short[size];
				coeff_recon[i] = new short[size];
				
				IntraPredParams * pred_params = create_intra_pred_params(sub_width[i], sub_height[i], blockSize, predMode, bSplit);
				if (bAbsDivision) {
					absDivide(coeff_orig[i], coeff_orig_abs[i], coeff_orig_sgn[i], size);
					switch (predScheme) {
						case 1:
							process_intra_prediction(pred_params, coeff_orig_abs[i], compnum, i, extension);
							break;
						case 2:
							process_intra_prediction(pred_params, coeff_orig_abs[i], compnum, i, extension/large_qf);
							break;
					}
				}
				else
					switch (predScheme) {
						case 1:
							process_intra_prediction(pred_params, coeff_orig[i], compnum, i, extension);
							break;
						case 2:
							process_intra_prediction(pred_params, coeff_orig[i], compnum, i, extension/large_qf);
							break;
					}
				skipped_blocks[i] = pred_params->skipped_blocks;
				total_blocks[i] = pred_params->total_blocks;
				split_depth[i] = pred_params->split_depth;
				split_blocks[i] = new int[split_depth[i] + 1];
				if (split_depth[i]){
					for (int t = 0; t < split_depth[i] + 1; t++)  {
						split_blocks[i][t] = pred_params->split_blocks[t];
					}
				} else {
					split_blocks[i][0] = total_blocks[i];
				}
				split_total[i] = pred_params->zero_count + pred_params->non_zero_count;
				double zero_prob = (split_total[i]) ? (double) pred_params->zero_count / (double) split_total[i]: 0;
				double non_zero_prob = (split_total[i]) ? (double) pred_params->non_zero_count / (double) split_total[i]: 0;
				split_ent[i]=  (zero_prob && non_zero_prob) ? - zero_prob * log2(zero_prob) - non_zero_prob * log2(non_zero_prob) : 0;
				/* KB: Copy intra prediction errors to new Bands which are used to estimate entropy changes */
				coeff_pred_ln[i] = pred_params->sub_pred_height * pred_params->sub_pred_width;
				coeff_pred[i] = new short [coeff_pred_ln[i]];
				if (pred_params->top_ref_sampl) {
					if (pred_params->left_ref_sampl) {
						ref_sampl_ln[i] = pred_params->sub_height + pred_params->sub_width - 1;
					} else {
						ref_sampl_ln[i] = pred_params->sub_width;
					}
				} else {
					ref_sampl_ln[i] = pred_params->sub_height;
				}
				if(!ref_sampl_ln[i]){
					return 200 + i;
				}
				ref_sampl[i] = new short [ref_sampl_ln[i]];
				//fill_recon_subband_data(pred_params, coeff_pred[i]);
				fill_pred_subband_data(pred_params, coeff_pred[i], ref_sampl[i]);
				pred_modes[i] = get_intra_modes(pred_params);
				skip_flags[i] = get_intra_skip_flags(pred_params);
				conc_pred_data[i] = new short[size];
				concatenate_pred_ref(pred_params, coeff_pred[i], ref_sampl[i], conc_pred_data[i],1);
				
				/* KB: Make Inverse Intra Prediction and copy to other Bands to make sure all right in Intra Prediction Process - then used in synthesis */
				// GV - 30.05.18 //
				/*if (bAbsDivision) {
					switch (predScheme) {
						case 1:
							process_inverse_intra_prediction(pred_params, coeff_recon_abs[i], compnum, i, extension);
							break;
						case 2:
							process_inverse_intra_prediction(pred_params, coeff_recon_abs[i], compnum, i, extension/large_qf);
							break;
					}
					absUnite(coeff_recon_abs[i], coeff_orig_sgn[i], coeff_recon[i], size);
				}
				else*/
					switch (predScheme) {
						case 1:
							process_inverse_intra_prediction(pred_params, coeff_recon[i], compnum, i, extension);
							break;
						case 2:
							process_inverse_intra_prediction(pred_params, coeff_recon[i], compnum, i, extension/large_qf);
							break;
					}

				// ------------- //

				/* KB: free needless data */
				free_intra_pred_params(pred_params);
			}

			switch (predScheme) {
				case 1:
					//int iW=0;
					//int iH=0;
					//int iN=3;
					pDT_pred->setAllCoefs(conc_pred_data, extension, comp, 1);
					//pDT_recon->stepAt(iW,iH)->setAllCoefs(&coeff_orig[iW*iN+iH], extension, comp, 2);
					pDT_recon->setAllCoefs(coeff_recon, extension, comp, 2);
					if (bAbsDivision)
						pDT_wb->setAllCoefs(coeff_orig_sgn, 1.0, comp, 1);
					break;
				case 2:
					pDT_pred->setAllCoefs(conc_pred_data, extension, comp, 1);
					pDT_recon->setAllCoefs(coeff_recon, large_qf, comp, 2);
					if (bAbsDivision)
						pDT_wb->setAllCoefs(coeff_orig_sgn, 1.0, comp, 1);
					break;
			}
			cout << ( (comp==Y)?"Y":(comp==Cr)?"Cr":"Cb" ) << " component" << endl;
			fprintf(log, ( (comp==Y)?"Y component:\n":(comp==Cr)?"Cr component:\n":"Cb component:\n" ) );
			comp_entropy(coeff_orig, coeff_pred, coeff_pred_ln, ref_sampl, ref_sampl_ln,  coeff_recon, skipped_blocks, total_blocks, sub_width, sub_height, pred_modes, skip_flags, split_blocks, split_depth, split_total, split_ent, blockSize, totalBands, log, log_short[compnum]);

			/* FILE * fdbn = fopen("distribution.txt", "w");
			for (int i = 0; i<totalBands; i++){
				GV
				int size = sub_width[i]*sub_height[i];
				unsigned int * dbn = distribution(coeff_pred[i], coeff_pred_ln[i]);
				short low = lowest(coeff_pred[i],coeff_pred_ln[i]);
				short high = highest(coeff_pred[i],coeff_pred_ln[i]);
				fprintf(fdbn,"%d %d", low, high);
				for (short j = low; j <= high; j++)
					fprintf(fdbn, " %f",(double)(dbn[j])/size);
				fprintf(fdbn, "\n");
			}
			fclose(fdbn);
			*/			

			for (int i = 0; i<totalBands; i++){
				delete [] coeff_orig[i], coeff_pred[i], coeff_recon[i], ref_sampl[i], split_blocks[i], coeff_orig_abs[i], coeff_orig_sgn[i], coeff_recon_abs[i];
			}
			delete[] coeff_orig, sub_width, sub_height, coeff_pred, coeff_recon, skipped_blocks, total_blocks, coeff_pred_ln, ref_sampl_ln, split_depth, split_ent, split_total, coeff_orig_abs, coeff_orig_sgn, coeff_recon_abs, pred_modes, skip_flags;
			#ifdef BAND_IMAGE_OUTPUT
			for (int k=0; k<totalBands; k++)
			       delete[] conc_pred_data[k];
			delete[] conc_pred_data;
			#endif
		}

		decTree * pDecTree_recon;
		decTree * pDecTree_pred;
		if (bRestriction)
		{
			// GV: put reconstructed part into a copy of original tree
			pDecTree_recon = new decTree;
			pDecTree->copyTree(pDecTree_recon);
			pDT_recon->copyData(pDecTree_recon->stepAt(xband,yband));//->stepAt(xsub,ysub));
			pDecTree_pred = new decTree;
			pDecTree->copyTree(pDecTree_pred);
			pDT_pred->copyData(pDecTree_pred->stepAt(xband,yband));//->stepAt(xsub,ysub));
		}
		else
		{
			pDecTree_recon = pDT_recon;
			pDecTree_pred = pDT_pred;
		}

		#ifdef BAND_IMAGE_OUTPUT
		{
			decTree * pDecTree_d = pDecTree_recon->substractTree(pDecTree_pred);
			cImageYCbCr * pImage_r=pDecTree_recon->createImage(false);
			cImageYCbCr * pImage_p=pDecTree_pred->createImage(true);
			cImageYCbCr * pImage_d=pDecTree_d->createImage(false);
			if (!pImage_r || !pImage_p)
				return 11;
			pImage_r->setSubW(iSubW);
			pImage_r->setSubH(iSubH);
			pImage_p->setSubW(iSubW);
			pImage_p->setSubH(iSubH);
			pImage_d->setSubW(iSubW);
			pImage_d->setSubH(iSubH);

			//cImageYCbCr * pImage_d=cImageYCbCr::difference(pImage_r, pImage_p, 1.0);	//GV

			cImageRGB *pOut_r = pImage_r->CreateRGB24FromYCbCr420();
			cImageRGB *pOut_p = pImage_p->CreateRGB24FromYCbCr420();
			cImageRGB *pOut_d = pImage_d->CreateRGB24FromYCbCr420();
			if (!pOut_r)
				return 12;

			i = pOut_r->WriteToBitmapFile("output/Reconstructed_bands.bmp");
			i |= pOut_p->WriteToBitmapFile("output/Pred_error_bands.bmp");
			i |= pOut_d->WriteToBitmapFile("output/Pred_values_bands.bmp");
			if (i)
				return 13;

			//int sizeW = 8;
			//int sizeH = 8;
			//int shiftW = 25;
			//int shiftH = 25;
			int sizeW = 64;
			int sizeH = 64;
			int shiftW = 1;
			int shiftH = 1;
			int numW = pDecTree_recon->getNumW();
			int numH = pDecTree_recon->getNumH();
			cImageYCbCr * pImage_comp = new cImageYCbCr;
			pImage_comp->setFullHeight(sizeH);
			pImage_comp->setFullWidth(sizeW*3+2*iSubW);
			pImage_comp->setFullHeight2(sizeH/iSubH);
			pImage_comp->setFullWidth2(sizeW*3/iSubW+2);
			pImage_comp->setSubW(iSubW);
			pImage_comp->setSubH(iSubH);
			pImage_comp->setWhite();
			for (int k=0; k<numW; k++)
				for (int l=0; l<numH; l++)
				{
					cImageYCbCr * pImage_r_p=pDecTree_recon->stepAt(l,k)->createImage(k||l);
					cImageYCbCr * pImage_p_p=pDecTree_pred->stepAt(l,k)->createImage(true);
					cImageYCbCr * pImage_d_p=pDecTree_d->stepAt(l,k)->createImage(k||l);
					pImage_r_p->setSubW(iSubW);
					pImage_r_p->setSubH(iSubH);
					pImage_p_p->setSubW(iSubW);
					pImage_p_p->setSubH(iSubH);
					pImage_d_p->setSubW(iSubW);
					pImage_d_p->setSubH(iSubH);
					pImage_r_p->blockBorders(blockSize);
					pImage_p_p->blockBorders(blockSize);
					pImage_d_p->blockBorders(blockSize);
				
					pImage_r_p->placeInto(Y,pImage_comp, 0, 0, shiftW, shiftW+sizeW-1, shiftH, shiftH+sizeH-1);
					pImage_r_p->placeInto(Cb,pImage_comp, 0, 0, shiftW/iSubW, shiftW/iSubW+sizeW/iSubW-1, shiftH/iSubH, shiftH/iSubH+sizeH/iSubH-1);
					pImage_r_p->placeInto(Cr,pImage_comp, 0, 0, shiftW/iSubW, shiftW/iSubW+sizeW/iSubW-1, shiftH/iSubH, shiftH/iSubH+sizeH/iSubH-1);
					pImage_d_p->placeInto(Y,pImage_comp, sizeW+iSubW, 0, shiftW, shiftW+sizeW-1, shiftH, shiftH+sizeH-1);
					pImage_d_p->placeInto(Cb,pImage_comp, sizeW/iSubW+1, 0, shiftW/iSubW, shiftW/iSubW+sizeW/iSubW-1, shiftH/iSubH, shiftH/iSubH+sizeH/iSubH-1);
					pImage_d_p->placeInto(Cr,pImage_comp, sizeW/iSubW+1, 0, shiftW/iSubW, shiftW/iSubW+sizeW/iSubW-1, shiftH/iSubH, shiftH/iSubH+sizeH/iSubH-1);
					pImage_p_p->placeInto(Y,pImage_comp, 2*(sizeW+iSubW), 0, shiftW, shiftW+sizeW-1, shiftH, shiftH+sizeH-1);
					pImage_p_p->placeInto(Cb,pImage_comp, 2*(sizeW/iSubW+1), 0, shiftW/iSubW, shiftW/iSubW+sizeW/iSubW-1, shiftH/iSubH, shiftH/iSubH+sizeH/iSubH-1);
					pImage_p_p->placeInto(Cr,pImage_comp, 2*(sizeW/iSubW+1), 0, shiftW/iSubW, shiftW/iSubW+sizeW/iSubW-1, shiftH/iSubH, shiftH/iSubH+sizeH/iSubH-1);

					cImageRGB * pOut_comp = pImage_comp->CreateRGB24FromYCbCr420();
					char fname[64];
					sprintf(fname,"output/Band_%d.bmp",numH*k+l+1);
					pOut_comp->WriteToBitmapFile(fname);
					delete pOut_comp, pImage_r_p, pImage_p_p, pImage_d_p;
				}
			delete pImage_comp;

			if (bAbsDivision)
				for (int compnum = 0; compnum < 3; compnum++)
				{
					char fname[64];
					cImageYCbCr * pImage_wb=pDT_wb->createImage(true);
					if (!pImage_wb)
						return 11;
					pImage_wb->setSubW(iSubW);
					pImage_wb->setSubH(iSubH);
					for (int icompnum = 0; icompnum < 3; icompnum++)
						if (icompnum!=compnum)
							pImage_wb->paintComp((component)icompnum,128);
					cImageRGB *pOut_wb = pImage_wb->CreateRGB24FromYCbCr420();
					if (!pOut_wb)
						return 12;
					sprintf(fname, "output/Sign_band_%s_%f.bmp", compnum==0?"Y":(compnum==1)?"Cr":"Cb", quantFactor);
					i = pOut_wb->WriteToBitmapFile(fname);
					if (i)
						return 13;
					delete pImage_wb, pOut_wb;
				}
			delete pImage_r, pImage_p, pImage_d,
			       pOut_r, pOut_p, pOut_d,
			       pDecTree_d;
		}
		#endif

		if (bAbsDivision)
		{
			if (bRestriction)
				pDecTree_recon->stepAt(xband,yband)->addSgn(pDT_wb);
			else
				pDecTree_recon->addSgn(pDT_wb);
		}
		delete pDT_wb;

	#else	//if prediction is skipped
		pDT_part->copyTree(pDT_recon);
		for (int compnum=0; compnum<3; compnum++)
		{
			component comp = (component) compnum;
			double extension = quantFactor;

			short ** coeff_orig = new short *[totalBands];
			int * sub_width = new int [totalBands];
			int * sub_height = new int [totalBands];

			int f;
			switch (predScheme) {
				case 1:
					f = pDT_part->getAllCoefs(coeff_orig, sub_width, sub_height, extension, comp);
					if (!f || f!=totalBands)
						return 10;
					//int iW=2;
					//int iH=2;
					//int iN=3;
					//pDT_recon->stepAt(iW,iH)->setAllCoefs(&coeff_orig[iW*iN+iH], extension, comp, 2);
					pDT_recon->setAllCoefs(coeff_orig, extension, comp, 2);
					break;
				case 2:
					f = pDT_part->getAllCoefs(coeff_orig, sub_width, sub_height, large_qf, comp);
					if (!f || f!=totalBands)
						return 10;
					pDT_recon->setAllCoefs(coeff_orig, large_qf, comp, 2);
					break;
			}
			//pDT_recon->setAllCoefs(coeff_orig, NULL, NULL, NULL, extension, comp, 2);

			double ent0s, ent0_sum = 0;
			for (int i=0; i<totalBands; i++)
			{
				int size = sub_width[i]*sub_height[i];
				if (bContext)
					ent0s = entropy_cont(coeff_orig[i], sub_width[i], sub_height[i]);
				else
					ent0s = entropy(coeff_orig[i], size)*size;
				ent0_sum += ent0s;
				fprintf(log_short[compnum],"%.f\tNaN\t", ent0s);
			}
			fprintf(log_short[compnum],"%.f\tNaN", ent0_sum);
		}

		decTree * pDecTree_recon;
		decTree * pDecTree_pred;
		if (bRestriction)
		{
			// GV: put reconstructed part into a copy of original tree
			pDecTree_recon = new decTree;
			pDecTree->copyTree(pDecTree_recon);
			pDT_recon->copyData(pDecTree_recon->stepAt(xband,yband));//->stepAt(xsub,ysub));
		}
		else
		{
			pDecTree_recon = pDT_recon;
		}

	#endif

	double * psnr = new double [totalBands];
	for (int compnum = 0; compnum < 3; compnum++)
	{
		component comp = (component) compnum;
		decTree::getAllPSNR(psnr, pDT_part, pDT_recon, comp);
		for (int i=0; i<totalBands; i++)
			fprintf(log_short[compnum], "\t%.3f", psnr[i]);
	}
	delete[] psnr;

	if (bRestriction)
	{
		pDecTree->stepAt(xband,yband)->synthesizeBand();
		pDecTree_recon->stepAt(xband,yband)->synthesizeBand();
		// !!
		//pDecTree_recon->stepAt(xband,yband)->addSgn(pDecTree_pt_sgn);
		//

		// output of band PSNR	
		for (int compnum = 0; compnum < 3; compnum++)
		{
			component comp = (component) compnum;
			double psnr_sb = decTree::getSubPSNR(pDecTree->stepAt(xband,yband), pDecTree_recon->stepAt(xband,yband), comp);
			fprintf(log_short[compnum], "\t%.3f", psnr_sb);
			/* // !!
			short ** coeff_sgn = new short *[1];
			int * sub_width_sgn = new int [1];
			int * sub_height_sgn = new int [1];
			int f = pDecTree_pt_sgn->getAllCoefs(coeff_sgn, sub_width_sgn, sub_height_sgn, 1.0, comp);
			for (int k=0; k<1; k++)
			{
				short * coeff_sgn_sup = new short [sub_width_sgn[k]*sub_height_sgn[k]];
				int sgnSize = suppressValue(coeff_sgn[k], sub_width_sgn[k]*sub_height_sgn[k], 0, coeff_sgn_sup);
				double ent_sgn = entropy(coeff_sgn_sup, sgnSize);
				fprintf(log_short[compnum], "\t%.3f\n", ent_sgn*sgnSize);
				delete [] coeff_sgn[k], coeff_sgn_sup;
			}
			delete [] coeff_sgn, sub_width_sgn, sub_height_sgn;
			// */
		}
	}
	
	pDecTree->synthesizeBand();
	pDecTree_recon->synthesizeBand();

	if (!bRestriction)
	{
		//output of whole-image PSNR
		for (int compnum = 0; compnum < 3; compnum++)
		{
			component comp = (component) compnum;
			double psnr = decTree::getSubPSNR(pDecTree, pDecTree_recon, comp);
			fprintf(log_short[compnum], "\t%.3f\n", psnr);
		}
	}		

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
		i = pOutR->WriteToBitmapFile("output/Restored.bmp");
		if (i)
			return 100 + i;
		cImageRGB *pOutD = pDiff->CreateRGB24FromYCbCr420();
		if (!pOutD)
			return 12;
		char diffname[64];
		sprintf(diffname, "output/Difference x%.2f.bmp", multdif);
		i = pOutD->WriteToBitmapFile(diffname);
		if (i)
			return 130 + i;
		delete pImage_r, pDiff, pOutR, pOutD;
	}
	#endif

	delete pDT_pred, pDT_recon;	

	fclose(log);

	for (int k=0; k<3; k++)
		fclose(log_short[k]);

	delete pImage_o, pFilter; 
	delete pDecTree; //, pDecTree_pt_sgn;
	if (bRestriction)
		delete pDecTree_recon, pDecTree_pred;
	
	return 0;
}


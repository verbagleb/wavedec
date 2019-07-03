#include "userLib.h"

using namespace std;

extern bool bError;
extern bool bModes;
extern bool bReferenceSamples;
extern bool bSplitFlags;
extern bool bSigns;
extern bool bSkipFlags;
extern bool bContext;		// defines whether the entropy uses context

int ceilTo(int number, int base, int remainder)
{
	return (number + base - 1 - remainder) / base*base + remainder;
}

int readConfig(const char *config_name, int &bAllowSkipPred, int &bSplit, int &bFiltering, int &bAbsDivision, int &predScheme, int &iSubH, int &iSubW, int &nFilters, cFilter*& pFilter)
{
	char Str[1000];
	double dCoeffs[1000];
	
	int i, j, k;
	int fl, temp;
	char *flg;
	FILE * fp = fopen(config_name, "rt");
	if (!fp)
	{
		printf("Error reading config file %s !\n", config_name);
		return 1;
	}
	fgets(Str, 1000, fp);
	fl = fscanf(fp, "%i", &bAllowSkipPred);
	flg = fgets(Str, 1000, fp);
	flg = fgets(Str, 1000, fp);
	fl = fscanf(fp, "%i", &bSplit);
	flg = fgets(Str, 1000, fp);
	flg = fgets(Str, 1000, fp);
	fl = fscanf(fp, "%i", &bFiltering);
	flg = fgets(Str, 1000, fp);
	flg = fgets(Str, 1000, fp);
	fl = fscanf(fp, "%i", &bAbsDivision);
	flg = fgets(Str, 1000, fp);
	flg = fgets(Str, 1000, fp);
	fl = fscanf(fp, "%i", &predScheme);
	flg = fgets(Str, 1000, fp);
	flg = fgets(Str, 1000, fp);
	fl = fscanf(fp, "%i %i", &iSubW, &iSubH);
	flg = fgets(Str, 1000, fp);
	flg = fgets(Str, 1000, fp);
	fl = fscanf(fp, "%i", &nFilters);
	flg = fgets(Str, 1000, fp);
	flg = fgets(Str, 1000, fp);

	pFilter = new cFilter[nFilters];
	for (int iFilter=0; iFilter<nFilters; iFilter++)
	{
		char *sFN = new char[128];
		flg = fgets(sFN, 128, fp);
		pFilter[iFilter].sFilterName = strtok(sFN, "\r\n");
		flg = fgets(Str, 1000, fp);
		fl = fscanf(fp, "%i", &(pFilter[iFilter].iNumBands));
		int iNumBands = pFilter[iFilter].iNumBands;
		flg = fgets(Str, 1000, fp);
		flg = fgets(Str, 1000, fp);
		
		pFilter[iFilter].mult = new double[iNumBands];
		//add = new int[iNumBands*iNumBands];
		pFilter[iFilter].pDFilterLength = new int[iNumBands];
		pFilter[iFilter].pIFilterLength = new int[iNumBands];

		int iEvenOdd;
		fscanf(fp, "%i", &iEvenOdd);
		pFilter[iFilter].bOdd = (iEvenOdd==1);
		fgets(Str, 1000, fp);
		fgets(Str, 1000, fp);
		for (i = 0; i < iNumBands; i++)
			fscanf(fp, "%lf", pFilter[iFilter].mult + i);
		//fgets(Str, 1000, fp);
		//fgets(Str, 1000, fp);
		//for (i = 0; i < iNumBands*iNumBands; i++)
		//	fscanf(fp, "%i", add + i);
		fgets(Str, 1000, fp);
		fgets(Str, 1000, fp);

		pFilter[iFilter].pDFilters = new double*[iNumBands];
		pFilter[iFilter].pIFilters = new double*[iNumBands];
		pFilter[iFilter].iEnhanceValue = 0;
		for (i = 0; i < iNumBands; i++)
		{
			int iNum;
			fscanf(fp, "%i", &iNum);
			pFilter[iFilter].pDFilterLength[i]=iNum;
			if (iNum + 1 > 	pFilter[iFilter].iEnhanceValue)
				pFilter[iFilter].iEnhanceValue = iNum + 1;

			pFilter[iFilter].pDFilters[i] = new double [iNum];
			for (j = 0; j < iNum; j++)
				fscanf(fp, "%lf", pFilter[iFilter].pDFilters[i]+j);
			fgets(Str, 1000, fp);
		}
		fgets(Str, 1000, fp);
		for (i = 0; i < iNumBands; i++)
		{
			int iNum;
			fscanf(fp, "%i", &iNum);
			pFilter[iFilter].pIFilterLength[i]=iNum;
			if (iNum + 1 > 	pFilter[iFilter].iEnhanceValue)
				pFilter[iFilter].iEnhanceValue = iNum + 1;

			pFilter[iFilter].pIFilters[i] = new double [iNum];
			for (j = 0; j < iNum; j++)
				fscanf(fp, "%lf", pFilter[iFilter].pIFilters[i]+j);
			fgets(Str, 1000, fp);
		}
		fgets(Str, 1000, fp);
		/**/
		pFilter[iFilter].iRemainder = iEvenOdd;		//add 1 pixel if mirror odd
		pFilter[iFilter].iAlternateShift =  (iNumBands == 2 && iEvenOdd);	//antiphase
		pFilter[iFilter].iInitialShift = 0;
	}
	fclose(fp);

	cout << "Read " << nFilters << " filters:" << endl;
	for (int iFilter;iFilter<nFilters;iFilter++)
		cout << iFilter << ": " << pFilter[iFilter].iNumBands << " bands: " << 
			pFilter[iFilter].sFilterName << endl;
	return 0;
}

int Limiting_a_b(double d, int a, int b)
{
	int i = (int)round(d);

	if (i < a)
		i = a;
	else if (i > b)
		i = b;

	return i;
}

short lowest(short * array, int size)
{
	if (!array)
		return SHRT_MAX;

	short low = SHRT_MAX;
	for (int i=0; i<size; i++)
		if (array[i] < low)
			low = array[i];
	
	return low;
}

short highest(short * array, int size)
{
	if (!array)
		return SHRT_MIN;

	short high = SHRT_MIN;
	for (int i=0; i<size; i++)
		if (array[i] > high)
			high = array[i];
	
	return high;
}

unsigned int * distribution(short * array, int size)
{
	if (!array)
		return nullptr;
	unsigned int *count0, *count;
	short low = lowest(array, size), high = highest(array, size);

	// cout << low << " " << high << endl;

	count0 = new unsigned int[high - low + 1];
	memset(count0, 0, (high-low+1)*sizeof(unsigned int));
	count = count0 - low;

	for (int i=0; i<size; i++)
		count[array[i]]++;
	
	return count;
}

double entropy(short * array, int size)
{
	if (!array)
		return -1.0;
	if (!size)
		return 0.0;

	short *pS, value, curHighest, curLowest;
	unsigned int *count0, *count;
	short low = lowest(array, size), high = highest(array, size);

	// cout << low << " " << high << endl;

	count0 = new unsigned int[high - low + 1];
	memset(count0, 0, (high-low+1)*sizeof(unsigned int));
	count = count0 - low;

	for (int i=0; i<size; i++)
		count[array[i]]++;
	
	double ent=0.0;
	for (int i = low; i <= high; i++)	// entropy compution
		ent += count[i] ? -((double)count[i] / size)*log2((double)count[i] / size) : 0.0;
	delete[] count0;

	return ent;
}

//Now uses abs and sgn instead of orig when bAbsDivision==true 
void comp_entropy(short ** coeff_orig, short ** coeff_pred, int * coeff_pred_ln, short ** ref_sampl, int * ref_sampl_ln, short ** coeff_recon, int * skip_blk, int * total_blk, int * sub_width, int * sub_height, short ** pred_modes, short ** skip_flags, int ** split_blocks, int * split_depth, int * split_total, double * split_ent, int blockSize, int totalBands, FILE *log, FILE *log_short)
{
	// Components of prediction entropy to add into sum in short log
	// -- moved to global in WaveletDecomposition.cpp --

	//cout << "Entered\n";

	double ent0_sum=0.0, ent0a_sum=0.0, ent1_sum=0.0, ent2_sum=0.0, ent_m_sum=0.0, ent1_ref_sum =0.0, ent_spl_sum = 0.0, ent_sgn_sum=0.0, ent_sf_sum=0.0;
	double percentage_av=0.0;
	fprintf(log, "\nEntropy for subband in bits/pixel:\n");
	fprintf(log, "j\tOrig[j]\tPred[j]\tRfSp[j]\tSgn[j]\tRecn[j]\tSkip\t(skp/ttl)\n");
	for (int i=0; i<totalBands; i++)
	{
		int size = sub_width[i]*sub_height[i];
		// GV - 30.05.18 //
		double ent_sgn = 0.0;
		short * coeff_abs = new short[size];
		if (bAbsDivision)
		{
			short * coeff_sgn = new short[size];
			short * coeff_sgn_sup = new short[size];
			absDivide(coeff_orig[i], coeff_abs, coeff_sgn, size);
			int sgnSize = suppressValue(coeff_sgn, size, 0, coeff_sgn_sup);
			ent_sgn = entropy(coeff_sgn_sup, sgnSize);
			delete [] coeff_sgn, coeff_sgn_sup;
		}
		// ------------- //

		double ent0a;
		if (bAbsDivision)
			ent0a = entropy(coeff_abs, size);
		double ent0 = entropy(coeff_orig[i], size);
		double ent1 = entropy(coeff_pred[i], coeff_pred_ln[i]);
		double ent1_ref = entropy(ref_sampl[i], ref_sampl_ln[i]);
		double ent2 = entropy(coeff_recon[i], size);
 		double percentage = (double)skip_blk[i] / (double)total_blk[i] * 100.0;
		fprintf(log,"%d\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.2f%%\t(%d/%d)", i, 
				bAbsDivision ? ent0a : ent0, 
				ent1, ent1_ref, ent_sgn, ent2, 
				percentage, skip_blk[i], total_blk[i]);
		for (int t = 0; t < split_depth[i] + 1; t++) {
			fprintf(log, " %dx%d num = %d;", blockSize >> t , blockSize >> t, split_blocks[i][t]);
		}
		fprintf(log, "\n");
		delete [] coeff_abs;
	}

	fprintf(log, "\nEntropy of modes vector in bits/block:\n");
	fprintf(log, "j\tMods[j]\n");
	for (int i=0; i<totalBands; i++)
	{
		double ent_m = entropy(pred_modes[i], total_blk[i]);
		fprintf(log,"%d\t%.5f\n", i, ent_m);
		ent_m_sum += ent_m*total_blk[i];
	}

	fprintf(log, "\nEntropy of skip_flags in bits/block:\n");
	fprintf(log, "j\tSkpFlg[j]\n");
	for (int i=0; i<totalBands; i++)
	{
		double ent_sf = entropy(skip_flags[i], total_blk[i]);
		fprintf(log,"%d\t%.5f\n", i, ent_sf);
		ent_sf_sum += ent_sf*total_blk[i];
	}

	fprintf(log, "\nEntropy of split_flags in bits/flag:\n");
	fprintf(log, "j\tSplFlg[j]\n");
	for (int i=0; i<totalBands; i++)
	{
		fprintf(log,"%d\t%.5f\n", i, split_ent[i]);
		ent_spl_sum += split_ent[i]*split_total[i];
	}

	fprintf(log, "\nEntropy for the whole subband in bits:\n");
	fprintf(log, "j\tOrig[j]\tPred[j]\t(Error\t+Modes\t+SkpFlg\t+RfSp\t+SplFlg\t+Sgn)\tRecn[j]\tSkip\t(skp/ttl)\n");
	for (int i=0; i<totalBands; i++)
	{
		int size = sub_width[i]*sub_height[i];
		// GV - 30.05.18 //
		double ent_sgn_s = 0.0;
		short * coeff_abs = new short[size];
		if (bAbsDivision)
		{
			short * coeff_sgn = new short[size];
			short * coeff_sgn_sup = new short[size];
			absDivide(coeff_orig[i], coeff_abs, coeff_sgn, size);
			if (bContext)
				ent_sgn_s = entropy_cont(coeff_sgn, sub_width[i], sub_height[i], 0);
			else
			{
				int sgnSize = suppressValue(coeff_sgn, size, 0, coeff_sgn_sup);
				ent_sgn_s = entropy(coeff_sgn_sup, sgnSize)*sgnSize;
			}

			delete [] coeff_sgn, coeff_sgn_sup;
		}
		// ------------- //
		
		double ent0as, ent0s, ent1s;
		if (bContext)
		{
			//cout << "Yes\n";
			if (bAbsDivision)
				ent0as = entropy_cont(coeff_abs, sub_width[i], sub_height[i]);
			ent0s = entropy_cont(coeff_orig[i], sub_width[i], sub_height[i]);
			ent1s = entropy_cont(coeff_pred[i], sub_width[i]-1, sub_height[i]-1);
		}
		else
		{
			//cout << "No\n";
			if (bAbsDivision)
				ent0as = entropy(coeff_abs, size)*size;
			ent0s = entropy(coeff_orig[i], size)*size;
			ent1s = entropy(coeff_pred[i], coeff_pred_ln[i])*coeff_pred_ln[i];
		}
		double ent1s_ref = entropy(ref_sampl[i], ref_sampl_ln[i]) * ref_sampl_ln[i];
		double ent2s = entropy(coeff_recon[i], size)*size;
 		double percentage = (double)skip_blk[i] / (double)total_blk[i] * 100.0;
		double ent_ms = entropy(pred_modes[i], total_blk[i])*total_blk[i];
		double ent_sfs = entropy(skip_flags[i], total_blk[i])*total_blk[i];
		double ent_sp = split_ent[i]*split_total[i];

		fprintf(log,"%d\t%.f\t%.f\t%.f\t%.f\t%.f\t%.f\t%.f\t%.f\t%.f\t%.2f%%\t(%d/%d)", i, 
				bAbsDivision ? ent0as+ent_sgn_s : ent0s, 
				ent1s+ent_ms+ent1s_ref+ent_sp+ent_sgn_s, 
				ent1s, ent_ms, ent_sfs, ent1s_ref, ent_sp, ent_sgn_s, 
				ent2s, percentage, skip_blk[i], total_blk[i]);
		for (int t = 0; t < split_depth[i] + 1; t++) {
			fprintf(log, " %dx%d num = %d;", blockSize >> t , blockSize >> t, split_blocks[i][t]);

		}
		fprintf(log, "\n");
		delete [] coeff_abs;

		ent0_sum += ent0s;
		ent0a_sum += ent0as;
		ent_sgn_sum += ent_sgn_s;
		ent1_sum += ent1s;
		ent1_ref_sum += ent1s_ref;
		ent2_sum += ent2s;
		percentage_av += percentage/totalBands;

		fprintf(log_short,"%.f\t%.f\t", 
				bAbsDivision ? ent0as + bSigns*ent_sgn_s : ent0s, 
				bError*ent1s+bModes*ent_ms+bSkipFlags*ent_sfs+bReferenceSamples*ent1s_ref+bSplitFlags*ent_sp+bSigns*ent_sgn_s);
	}

	fprintf(log, "\nTotal entropy for the frame in bits:\n");
	fprintf(log, "Orig_s\tPred_s\t(Error\t+Modes\t+SkpFl\t+RfSp\t+SplFlg\t+Sgn)\tRecon_s\tPerc_av:\n");
	fprintf(log, "%.f\t%.f\t%.f\t%.f\t%.f\t%.f\t%.f\t%.f\t%.f\t%.2f%%\n\n", 
			bAbsDivision ? ent0a_sum + ent_sgn_sum : ent0_sum, 
			ent1_sum+ent_m_sum+ent_sf_sum+ent1_ref_sum+ent_spl_sum+ent_sgn_sum, 
			ent1_sum, ent_m_sum, ent_sf_sum, ent1_ref_sum, ent_spl_sum, ent_sgn_sum, ent2_sum, 
			percentage_av);
	fprintf(log_short,"%.f\t%.f", 
			bAbsDivision ? ent0a_sum + bSigns*ent_sgn_sum : ent0_sum, 
			bError*ent1_sum+bModes*ent_m_sum+bSkipFlags*ent_sf_sum+bReferenceSamples*ent1_ref_sum+bSplitFlags*ent_spl_sum+bSigns*ent_sgn_sum);
}

void comp_psnr(double * error, int totalBands, FILE * log_short)
{
	for (int i=0; i<totalBands; i++)
		fprintf(log_short, "\t%.3f", 20.0*log10(USHRT_MAX/error[i]));
	fprintf(log_short, "\n");
}

int formOutput(char * dir_name, char * bitmap_name, int blockSize, int predMode, double quantFactor, int totalBands, FILE ** log_this, FILE ** log_short)
{
	mkdir("output", 0777); 	//This one is for image output
	mkdir("files", 0777); 	//This one is for file output
	
	const int NMAX=10000;
	if (dir_name[0]=='\0')
	{
		int n,flag;
		for (n=0, flag=-1; flag && n<NMAX; n++)
		{
			sprintf(dir_name, "log/log_%d", n);
			flag = mkdir(dir_name, 0777);
		}
		if (n==NMAX)
		{
			cout << "Can't create a directory\n";
			return 1;
		}
	}

	cout << "Directory: " << dir_name << endl;
	char log_name[128], log_short_name[128];
	sprintf(log_name, "./%s/log_%s_%d_%d_%.2f.txt", dir_name, bitmap_name, blockSize, predMode, quantFactor);
	*log_this = fopen(log_name, "w");
	if (!*log_this)
	{
		cout << "Can't open a file for log of this set\n";
		return 2;
	}

	if (!log_short)
	{
		cout << "Null pointers to log arrays\n";
		return 3;
	}
	for (int k=0; k<3; k++)
	{
		sprintf(log_short_name, "./%s/%s_log_short.txt", dir_name, (k==0)?"Y":( (k==1)? "Cr": "Cb") );
		log_short[k] = fopen(log_short_name, "r");
		if (log_short[k])
		{
			fclose(log_short[k]);
			log_short[k] = fopen(log_short_name, "a");
			if (!log_short[k])
			{
				cout << "Can't open a file for short log\n";
				return 4;
			}
		}
		else //if (errno == ENOENT) 
		{
			log_short[k] = fopen(log_short_name, "w");
			if (!log_short[k])
			{
				cout << "Can't open a file for short log\n";
				return 5;
			}
			fprintf(log_short[k],"bitmap_name\tsize\tmode\tqfactor");
			for (int j=0; j<totalBands; j++)
				fprintf(log_short[k],"\tOr_%d\tPr_%d",j,j);
			fprintf(log_short[k],"\tOr_S\tPr_S");
			for (int j=0; j<totalBands; j++)
				fprintf(log_short[k],"\tPSNR_%d",j);
			fprintf(log_short[k],"\tExtra\n");
		}
		fprintf(log_short[k],"%s\t%d\t%d\t%.5f\t", bitmap_name, blockSize, predMode, quantFactor);
	}
	return 0;
}

//GV: counts PSNR between an array and its distorted copy
double psnr(short * original, short * distorted, int size)
{
	int i;
	double sum = 0.0;

	short * diff = difference(original, distorted, size);
	for (i = 0; i < size; i++)
		sum += diff[i]*diff[i];
	double SNR = sqrt(sum / size);
	free(diff);

	short low = lowest(original, size);
	short high = highest(original, size);
	
	return (high-low-1)/SNR;
}

//GV: creates array which is the difference of arrays b and a
short * difference(short * a, short * b, int size)
{
	int i;
	short * ret = (short*)calloc(size, sizeof(short));
	if (!ret)
		return nullptr;
	for (i=0; i<size; i++)
		ret[i] = b[i] - a[i];
	return ret;
}

//GV: divide arrays into their modules and signs
void absDivide(short * src, short * abs_dest, short * sign_dest, int size)
{
	int cntrp = 0, cntrn = 0;
	for (int i = 0; i < size; i++)
	{
		abs_dest[i] = abs(src[i]);
		sign_dest[i] = (src[i]>0) ? 1 : ((src[i]==0) ? 0 : -1);
		if (sign_dest[i]>0)
			cntrp++;
		if (sign_dest[i]<0)
			cntrn++;
	}
	/*
	double ea = entropy(abs_dest,size);
	double es = entropy(sign_dest,size);
	cout << entropy(src,size) << " -> " << ea+es << " = " << ea << " + " << es;
	cout << " (" << cntrp << "/" << cntrn << "/" << size-cntrp-cntrn << ")" << endl;
	*/
}

//GV: reconstruct arrays from modules and signs
void absUnite(short * abs_src, short * sign_src, short * dest, int size)
{
	for (int i = 0; i < size; i++)
		dest[i] = abs_src[i]*sign_src[i];
}

//GV: creates a new array without values val
//Returns new size
int suppressValue(short * src, int size, short val, short * dest, bool * ind)
{
	int current = 0;
	for (int i=0; i<size;i++)
	{
		if (src[i]!=val)
		{
			dest[current]=src[i];
			current++;
		}
		if (ind)
			ind[i] = src[i]!=val;
	}
	return current;
}

short * getContext(short * data, int width, int height)
{
	if (!data)
		return nullptr;
	if (!width || !height)
		return nullptr;

	//short defval = lowest(data, width*height)*4 - 1; //must be different from any possible calculation result
	short defval = -2; //must be different from any possible calculation result
	short * context = new short[width*height];
	memset(context,0,width*height*sizeof(short));
	for (int j=0; j<height; j++)
		for (int i=0; i<width; i++)
			if (i==0 || j==0)
				context[j*width+i] = defval;
			else
			{
			/*	int sum = data[j*width+(i-1)]+
					data[(j-1)*width+(i-1)]+ 
					data[(j-1)*width+i]+ 
					data[(j-1)*width+(i+1)];
			*/	
				int sum = getSign(data[j*width+(i-1)])*27+
					getSign(data[(j-1)*width+(i-1)])*9 + 
					getSign(data[(j-1)*width+i])*3 + 
					getSign(data[(j-1)*width+(i+1)])*1;
				
				//context[j*width+i] = (sum>0)? 1: (sum<0)? -1 : 0;
				context[j*width+i] = sum;
			}
	return context;
}

double entropy_cont(short * array, int width, int height, short supVal)
{
	return entropy(array, getContext(array, width, height), width*height, supVal);
}

double entropy(short * array, short * context, int size, short supVal)
{
	if (!array || !context)
		return -1.0;
	if (!size)
		return 0.0;

	short low_c = lowest(context, size), high_c = highest(context, size);
	unsigned int **count00 = new unsigned int* [high_c-low_c+1];
	unsigned int **count0 = new unsigned int* [high_c-low_c+1];
	unsigned int **count = count0 - low_c;
	unsigned int *count_cont0 = new unsigned int [high_c-low_c+1];
	memset(count_cont0, 0, (high_c-low_c+1)*sizeof(unsigned int));
	unsigned int *count_cont = count_cont0 - low_c;

	short low_a = lowest(array, size), high_a=highest(array, size);

	for (int k=low_c; k<=high_c; k++)
	{
		// cout << low << " " << high << endl;

		count00[k-low_c] = new unsigned int[high_a - low_a + 1];
		memset(count00[k-low_c], 0, (high_a-low_a+1)*sizeof(unsigned int));
		count0[k-low_c]=count00[k-low_c]-low_a;
	}

	for (int i=0; i<size; i++)
		if (array[i]!=supVal)
		{
			count[context[i]][array[i]]++;
			count_cont[context[i]]++;
		}

	double ent_s=0.0;
	for (int k=low_c; k<=high_c; k++)
		if (count_cont[k])
			for (int i = low_a; i <= high_a; i++)	// entropy compution
				ent_s += count[k][i] ? - (double)count[k][i] * log2((double)count[k][i] / count_cont[k]) : 0.0;
	for (int k=low_c; k<high_c; k++)
		delete[] count00[k-low_c];
	delete[] count00, count0, count_cont0;

	return ent_s;
}

compd * fft(compd * array, int n)
{
	compd * retArray = new compd[n];
	memset(retArray, 0, sizeof(compd)*n);
	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
			retArray[i] += array[j]*exp(compd(0,-2.0*PI*((i*j)%n)/n))/sqrt(n);
	return retArray;
}

compd * ifft(compd * array, int n)
{
	compd * retArray = new compd[n];
	memset(retArray, 0, sizeof(compd)*n);
	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
			retArray[i] += array[j]*exp(compd(0,2.0*PI*((i*j)%n)/n))/sqrt(n);
	return retArray;
}

int getSign(int num)
{
	return num>0 ? 1: (num==0 ? 0: -1);
}

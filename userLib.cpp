#include "userLib.h"

using namespace std;

// Deprecated --------------
bool bError = false;
bool bModes = false;
bool bReferenceSamples = false;
bool bSplitFlags = false;
bool bSigns = false;
bool bSkipFlags = false;
// --------------------------

extern bool bContext;		// defines whether the entropy uses context

int ceilTo(int number, int base, int remainder)
{
	return (number + base - 1 - remainder) / base*base + remainder;
}

int readConfig(const char *config_name, int &nFilters, cFilter*& pFilter)
{
	const int BUFFSIZE = 1000;
	char str[BUFFSIZE];
	
	int i, j, k;
	int fl, temp;
	char *flg;
	FILE * fp = fopen(config_name, "rt");
	if (!fp)
	{
		printf("Error reading filter config file %s !\n", config_name);
		return 1;
	}
	fgets(str, BUFFSIZE, fp);
	fl = fscanf(fp, "%i", &nFilters);
	flg = fgets(str, BUFFSIZE, fp);
	flg = fgets(str, BUFFSIZE, fp);

	pFilter = new cFilter[nFilters];
	for (int iFilter=0; iFilter<nFilters; iFilter++)
	{
		char *sFN = new char[128];
		flg = fgets(sFN, 128, fp);
		pFilter[iFilter].sFilterName = strtok(sFN, "\r\n");
		flg = fgets(str, BUFFSIZE, fp);
		fl = fscanf(fp, "%i", &(pFilter[iFilter].iNumBands));
		int iNumBands = pFilter[iFilter].iNumBands;
		flg = fgets(str, BUFFSIZE, fp);
		flg = fgets(str, BUFFSIZE, fp);
		
		pFilter[iFilter].mult = new double[iNumBands];
		//add = new int[iNumBands*iNumBands];
		pFilter[iFilter].pDFilterLength = new int[iNumBands];
		pFilter[iFilter].pIFilterLength = new int[iNumBands];

		int iEvenOdd;
		fscanf(fp, "%i", &iEvenOdd);
		pFilter[iFilter].bOdd = (iEvenOdd==1);
		fgets(str, BUFFSIZE, fp);
		fgets(str, BUFFSIZE, fp);
		for (i = 0; i < iNumBands; i++)
			fscanf(fp, "%lf", pFilter[iFilter].mult + i);
		//fgets(str, BUFFSIZE, fp);
		//fgets(str, BUFFSIZE, fp);
		//for (i = 0; i < iNumBands*iNumBands; i++)
		//	fscanf(fp, "%i", add + i);
		fgets(str, BUFFSIZE, fp);
		fgets(str, BUFFSIZE, fp);

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
			fgets(str, BUFFSIZE, fp);
		}
		fgets(str, BUFFSIZE, fp);
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
			fgets(str, BUFFSIZE, fp);
		}
		fgets(str, BUFFSIZE, fp);
		/**/
		pFilter[iFilter].iRemainder = iEvenOdd;		//add 1 pixel if mirror odd
		pFilter[iFilter].iAlternateShift =  (iNumBands == 2 && iEvenOdd);	//antiphase
		pFilter[iFilter].iInitialShift = 0;
	}
	fclose(fp);

	cout << "Read " << nFilters << " filters:" << endl;
	for (int iFilter=0;iFilter<nFilters;iFilter++)
		cout << iFilter << ": " << pFilter[iFilter].iNumBands << " bands: " << 
			pFilter[iFilter].sFilterName << endl;
	return 0;
}

int readGrid(const char * grid_name, 
		int &subW, int &subH, 
		char ** &images_array, int &images_number, 
		double * &qs_array, int &qs_number,
		char ** &cline_array, int &cline_number)
{
	const int BUFFSIZE = 1000;
	char str[BUFFSIZE];
	
	const int ARRAY_MAX = 100;
	images_array = new char * [ARRAY_MAX];
	qs_array = new double [ARRAY_MAX];
	cline_array = new char * [ARRAY_MAX];

	int i, j, k;
	int fl, temp;
	char *flg;
	char * tok;

	FILE * fp = fopen(grid_name, "rt");
	if (!fp)
	{
		printf("Error reading grid config file %s !\n", grid_name);
		return 1;
	}

	fgets(str, BUFFSIZE, fp);
	fl = fscanf(fp, "%i %i", &subW, &subH);

	flg = fgets(str, BUFFSIZE, fp);
	flg = fgets(str, BUFFSIZE, fp);
	flg = fgets(str, BUFFSIZE, fp);
	for (tok = strtok(str," \t\r\n"), images_number = 0;
		tok && images_number < ARRAY_MAX;
		tok = strtok(nullptr, " \t\r\n"), images_number++)
	{
		images_array[images_number] = new char[strlen(tok)+1];
		flg = strcpy(images_array[images_number], tok);
	}
	if (tok && images_number == ARRAY_MAX)
		cerr << "The first " << ARRAY_MAX << " values for image name are loaded";

	flg = fgets(str, BUFFSIZE, fp);
	flg = fgets(str, BUFFSIZE, fp);
	for (tok = strtok(str," \t\r\n"), qs_number = 0;
		tok && qs_number < ARRAY_MAX;
		tok = strtok(nullptr, " \t\r\n"), qs_number++)
		qs_array[qs_number] = atof(tok);
	if (tok && qs_number == ARRAY_MAX)
		cerr << "The first " << ARRAY_MAX << " values for quant factor are loaded";

	flg = fgets(str, BUFFSIZE, fp);
	for (flg = fgets(str, BUFFSIZE, fp), cline_number = 0;
			flg;
			flg = fgets(str, BUFFSIZE, fp), cline_number++)
	{
		siftString(str);
		cline_array[cline_number] = new char[strlen(str)+1];
		strcpy(cline_array[cline_number], str);
	}

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
void comp_entropy(short ** coeff_orig, short ** coeff_pred, int * coeff_pred_ln, short ** ref_sampl, int * ref_sampl_ln, short ** coeff_recon, int * skip_blk, int * total_blk, int * sub_width, int * sub_height, short ** pred_modes, short ** skip_flags, int ** split_blocks, int * split_depth, int * split_total, double * split_ent, int blockSize, int totalBands, FILE *log_short)
{
	// Components of prediction entropy to add into sum in short log
	// -- moved to global in WaveletDecomposition.cpp --

	//cout << "Entered\n";

	double ent0_sum=0.0, ent0a_sum=0.0, ent1_sum=0.0, ent2_sum=0.0, ent_m_sum=0.0, ent1_ref_sum =0.0, ent_spl_sum = 0.0, ent_sgn_sum=0.0, ent_sf_sum=0.0;
	double percentage_av=0.0;
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
			delete [] coeff_sgn;
		  	delete [] coeff_sgn_sup;
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
		delete [] coeff_abs;
	}

	for (int i=0; i<totalBands; i++)
	{
		double ent_m = entropy(pred_modes[i], total_blk[i]);
		ent_m_sum += ent_m*total_blk[i];
	}

	for (int i=0; i<totalBands; i++)
	{
		double ent_sf = entropy(skip_flags[i], total_blk[i]);
		ent_sf_sum += ent_sf*total_blk[i];
	}

	for (int i=0; i<totalBands; i++)
	{
		ent_spl_sum += split_ent[i]*split_total[i];
	}

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

			delete [] coeff_sgn;
			delete [] coeff_sgn_sup;
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

int formOutput(char * output_dir_name, const char * bitmap_name, double quantStep, int totalBands, FILE ** log_short)
{
	mkdir("output", 0777); 	
	
	const int NMAX=10000;
	if (output_dir_name[0]=='\0')
	{
		char name[64], output_name[64];
		int n, flag;
		for (n=0, flag=-1; flag && n<NMAX; n++)
		{
			sprintf(output_dir_name, "output/output_%d", n);
			sprintf(output_name, "./output_%d", n);
			flag = mkdir(output_dir_name, 0777);
		}
		if (n==NMAX)
		{
			cout << "Can't create a directory\n";
			return 1;
		}
		cout << "Directory generated: " << output_dir_name << endl;

		remove("output/output_last");
		symlink(output_name, "output/output_last");

		char buf[BUFSIZ];
		char parameters_name[128];
		sprintf(parameters_name, "%s/parameters.cfg", output_dir_name);
		FILE* source = fopen("parameters.cfg", "rb");
		FILE* dest = fopen(parameters_name, "wb");
		while (size_t size = fread(buf, 1, BUFSIZ, source)) 
			fwrite(buf, 1, size, dest);
		fclose(source);
		fclose(dest);
	}

	char log_short_name[128];

	if (!log_short)
	{
		cout << "Null pointers to log arrays\n";
		return 3;
	}
	for (int k=0; k<1; k++)
	{
		//sprintf(log_short_name, "./%s/%s_log_short.txt", dir_name, (k==0)?"Y":( (k==1)? "Cr": "Cb") );
		sprintf(log_short_name, "./%s/log_short.txt", output_dir_name);
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
			fprintf(log_short[k],"bitmap_name\tqstep");
#ifdef PRINT_SEPARATE_BANDS
			for (int j=0; j<totalBands; j++)
				fprintf(log_short[k],"\tEnt_%d",j);
#endif
			fprintf(log_short[k],"\tEnt_Y");
			fprintf(log_short[k],"\tEnt_Cr");
			fprintf(log_short[k],"\tEnt_Cb");
#ifdef PRINT_SEPARATE_BANDS
			for (int j=0; j<totalBands; j++)
				fprintf(log_short[k],"\tPSNR_%d",j);
#endif
			fprintf(log_short[k],"\tPSNR_Y");
			fprintf(log_short[k],"\tPSNR_Cr");
			fprintf(log_short[k],"\tPSNR_Cb\n");
		}
		fprintf(log_short[k],"%s\t%.2f", bitmap_name, quantStep);
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
	delete[] count00;
	delete[] count0;
	delete[] count_cont0;

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

void siftString(char * line)
{
	char * end = line + strlen(line);
	char * dst = line;
	for (char * src = line;
			src < end;
			src++)
		if (*src == '(' || *src == ')' || (*src >= '0' && *src <= '9') || *src == ',')
			*(dst++) = *src;
	*dst = '\0';
}

#pragma once
#include <stdint.h>
#include <math.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_matrix.h>

#define CLAMP(x,a,b) ((x)<(a) ? (a) : ((x)>(b) ? (b) : (x)))
#define CLAMP_MAX(x,a,b) ((x)<(a) ? (b) : ((x)>(b) ? (b) : (x)))
#define CLAMP_MIN(x,a,b) ((x)<(a) ? (a) : ((x)>(b) ? (a) : (x)))
#define INV_MOD(a, b) ( (b != 0) ? ( (a)<(0) ? ((a % (-b)) - b) : (a % (-b)) ) : a )
#define MOD(a,b) ( (b > 0) ? ( (a)<(0) ? ((a % b) + b) : (a % b) ) : ( INV_MOD(a,b) ) ) 
#define MOD256(a) MOD(a, 256)
#define MULT(a) ( (a == 0) ? (1) : (-1) )

#define DDR_DIAG(ptr) ( ( ptr[0][1] +  (ptr[0][0] << 1) + ptr[1][0] + 2 ) >> 2 )
#define DDR_HORIZ(ptr, i, j) ( ( ptr[0][i - j - 1] +  (ptr[0][i - j] << 1) + ptr[0][i - j + 1] + 2 ) >> 2 )
#define DDR_VERT(ptr, i, j) ( ( ptr[j - i - 1][0] +  (ptr[j - i][0] << 1) + ptr[j - i + 1][0] + 2 ) >> 2 )
#define DDR(ptr, i, j) ( (i > j) ? DDR_HORIZ(ptr, i, j) : ( (i < j) ? DDR_VERT(ptr, i, j) : DDR_DIAG(ptr) ) )

#define VR1(ptr, i, j) ( (ptr[0][i - (j >> 1) - 1] + (ptr[0][i - (j >> 1)] << 1) + ptr[0][i - (j >> 1) + 1]  + 2 ) >> 2 )
#define VR2(ptr, i, j) ( (ptr[0][i - (j >> 1)] + ptr[0][i - (j >> 1) + 1]  + 1 ) >> 1 )
#define VR3(ptr, i, j) ( (ptr[j][0] + (ptr[j - 1][0] << 1) + ptr[j - 2][0]  + 2 ) >> 2 )
#define VR_HD(ptr, i, j, n) ( ( n < 0 ) ? ( (n == -1) ? DDR_DIAG(ptr) : VR3(ptr, i, j) ) : ( (n%2) ? VR1(ptr, i, j) : VR2(ptr, i, j) ) )

#define VL1(ptr, i, j) ( (ptr[i + (j >> 1)] + (ptr[i + (j >> 1) + 1] << 1) + ptr[i + (j >> 1) + 2] + 2) >> 2 )
#define VL2(ptr, i, j) ( (ptr[i + (j >> 1)] + ptr[i + (j >> 1) + 1] + 1) >> 1 )
#define VL(ptr, i, j, n) ( (n%2) ? VL1(ptr, i, j) : VL2(ptr, i, j) )

#define HU1(ptr, size1, size2) ( (ptr[size2] + 3*ptr[size1] + 2) >> 2)
#define HU2(ptr, size1) ( ptr[size1] )
#define HU(ptr, i, j, n, l1, l2, size1, size2) ( (n > l2) ? ( (n > l1) ? HU2(ptr, size1) : HU1(ptr, size1, size2)  ) : VL(ptr, j, i, n) )

typedef struct _IntraPredParams IntraPredParams;
typedef struct _IntraBlock IntraBlock;
typedef struct _RefValues RefValues;
typedef void (* pred) (IntraBlock*, int);
typedef short INTRA_PRED_MODE;		//GV: added common type

static double HADAMARD_4x4[4][4] = {
 										{1, 1, 1, 1},
 										{1, -1, 1, -1},
 										{1, 1, -1, -1},
 										{1, -1, -1, 1}
};

static double HADAMARD_8x8[8][8] = {
 										{1,  1,  1,  1,  1,  1,  1,  1},
 										{1, -1,  1, -1,  1, -1,  1, -1},
 										{1,  1, -1, -1,  1,  1, -1, -1},
 										{1, -1, -1,  1,  1, -1, -1,  1},
										{1,  1,  1,  1, -1, -1, -1, -1},
 										{1, -1,  1, -1, -1,  1, -1,  1},
 										{1,  1, -1, -1, -1, -1,  1,  1},
 										{1, -1, -1,  1, -1,  1,  1, -1}
};

static double HAAR_4x4[4][4] = {
 										{1,  1,  1,  1},
 										{1,  1, -1, -1},
 										{sqrt(2),  -sqrt(2), 0, 0},
 										{0, 0, sqrt(2),  -sqrt(2)}
};

typedef enum{
	IB_SIZE_4x4 = 4,
	IB_SIZE_8x8 = 8,
	IB_SIZE_16x16 = 16,
	IB_SIZE_32x32 = 32,
	IB_SIZE_ADAPTIVE, 
} INTRA_BLOCK_SIZE;

#define TYPE_NUMBER 10			//GV: only non-adaptive
typedef enum {
	I_PRED_4x4_V  = 0,
	I_PRED_4x4_H  = 1,
	I_PRED_4x4_DC = 2,
	I_PRED_4x4_DDL= 3,
	I_PRED_4x4_DDR= 4,
	I_PRED_4x4_VR = 5,
	I_PRED_4x4_HD = 6,
	I_PRED_4x4_VL = 7,
	I_PRED_4x4_HU = 8,
	I_PRED_4x4_PLANE = 9,
	I_PRED_4x4_ADAPTIVE = 10,
	I_PRED_4x4_NONE
} INTRA_4x4_PRED_MODE;

/*typedef enum {				//GV: added for 8x8
	I_PRED_8x8_V  = 0,
	I_PRED_8x8_H  = 1,
	I_PRED_8x8_DC = 2,
	I_PRED_8x8_DDL= 3,
	I_PRED_8x8_DDR= 4,
	I_PRED_8x8_VR = 5,
	I_PRED_8x8_HD = 6,
	I_PRED_8x8_VL = 7,
	I_PRED_8x8_HU = 8,
	I_PRED_8x8_ADAPTIVE = 9,
	I_PRED_8x8_NONE
} INTRA_8x8_PRED_MODE;*/

struct _IntraPredParams {
	INTRA_BLOCK_SIZE block_size;
	//INTRA_4x4_PRED_MODE pred_type;
	INTRA_PRED_MODE pred_type;	//GV
	int sub_width;
	int sub_height;
	int sub_pred_width;
	int sub_pred_height;
	void * data;
	int horiz_blocks;
	int vert_blocks;
	int right_filling;
	int bottom_filling;
	IntraBlock *** rep_order;
	IntraBlock **** child_order; //KB: 27.04.18
	IntraBlock ** pred_order;
	int16_t * top_ref_sampl;
	int16_t * left_ref_sampl;
	int skipped_blocks;
	int total_blocks;
	int * split_blocks;
	int zero_count;
	int non_zero_count;
	pred pred_func[TYPE_NUMBER+1];
	int split_flag; //KB: added for splitting IB into smaller IB in case of better RD for sum of smaller IBs;
	int split_depth; //KB: 27.04.18
};

struct _IntraBlock {
	IntraPredParams * intra_pred_params;
	//INTRA_4x4_PRED_MODE pred_mode;
	INTRA_PRED_MODE pred_mode;	//GV
	int block_size; //KB: right now it is template for future improvements - variable size and form of intra block within the same subband.
	int pos_x;
	int pos_y;
	int pred_index;
	int16_t ** sampl;
	int16_t *tmp_bottom;
	int16_t *tmp_right;
	int skip_flag;
	int split_flag; //KB: 27.04.18
	IntraBlock * root_ib; //KB: 27.04.18
	int depth; //KB: 27.04.18
	double RD0;
	double RD1;
	double RD[TYPE_NUMBER];
};

struct _RefValues{		//GV: structure of reference values
	int16_t * top;
	int16_t * left;
	int16_t * corner;
	int top_size;
	int left_size;
};

// GV - 31.05.18 //
extern int bAllowSkipPred;
extern int bFiltering;
extern int predScheme;
// ------------- //

#ifdef __cplusplus
extern "C"
{
#endif

/* KB: Utils functions */
void * malloc0(int size);
gsl_matrix * mul_matrix(gsl_matrix * A, const gsl_matrix * B);

/* KB: Intra prediction parameters and intra blocks creation and initialization */
IntraPredParams * create_intra_pred_params(int sub_width, int sub_height, int block_size, int pred_type, int split);
void init_intra_pred_params(IntraPredParams * intra_pred_params);
void create_intra_blocks(IntraPredParams * intra_pred_params);
void create_child_blocks(IntraPredParams * intra_pred_params, int depth);
IntraBlock * create_intra_block(IntraPredParams * intra_pred_params, int pos_x, int pos_y, int depth);
void create_prediction_order(IntraPredParams * intra_pred_params);

/* KB: Filling subband data in intra block  structures */
void fill_subband_data(IntraPredParams * intra_pred_params, short * data);
void fill_borders(IntraPredParams * intra_pred_params);
void fill_tmp(IntraPredParams * intra_pred_params);
void fill_tmp_block(IntraBlock * block);

/* KB: filling parent and child Intra Blocks during splitting process */
void fill_child_blocks(IntraPredParams * intra_pred_params);
void fill_parent_blocks(IntraPredParams *intra_pred_params);
void copy_smpl_ib (IntraBlock * dest, IntraBlock * src);

/* KB: Performs the whole forward intra prediction process - from taking data from initial DWT subband to intra predcition within all intra blocks */
void process_intra_prediction(IntraPredParams * intra_pred_params, short * data, int component, int sub_num, double quant_factor);
void count_skipped_blocks(IntraPredParams * intra_pred_params);
void count_split_blocks(IntraBlock * block,  int * total_count, int * split_blocks, int * skip_count);

/* KB: Rate-distortion estimation */
double estimate_ib_rd(IntraBlock * block);
double get_mse(IntraBlock * block);
int get_sad(IntraBlock * block);
double get_ssd(IntraBlock * block);
double get_rssd(IntraBlock * block);
double get_nrssd(IntraBlock * block);
double get_satd(IntraBlock * block);
double get_haar_rd(IntraBlock * block);
int get_bit_ib(IntraBlock * block);
double estimate_ent_ib(IntraBlock * block);
double etsimate_ent_whole(IntraBlock * block);
double estimate_ent_neig(IntraBlock * block);
double get_rd_split(IntraBlock * block);
INTRA_PRED_MODE *get_intra_modes(IntraPredParams * intra_pred_params);	//GV
short *get_intra_skip_flags(IntraPredParams * intra_pred_params);
void get_intra_modes_split(IntraBlock * block, INTRA_PRED_MODE * modes, int * count);
void get_intra_skip_flags_split(IntraBlock * block, short * skip_flags, int * count);

/* KB: Forward and Inverse Intra Prediction */
void make_intra_pred(IntraPredParams * intra_pred_params, int direct, double quant_factor);
void intra_pred_4x4_V(IntraBlock * block, int direct);
void intra_pred_4x4_H(IntraBlock * block, int direct);
void intra_pred_4x4_DC(IntraBlock * block, int direct);
void intra_pred_4x4_DDL(IntraBlock * block, int direct);
void intra_pred_4x4_DDR(IntraBlock * block, int direct);
void intra_pred_4x4_VR(IntraBlock * block, int direct);
void intra_pred_4x4_HD(IntraBlock * block, int direct);
void intra_pred_4x4_VL(IntraBlock * block, int direct);
void intra_pred_4x4_HU(IntraBlock * block, int direct);
void intra_pred_4x4_PLANE(IntraBlock * block, int direct);
void intra_pred_4x4_ADAPTIVE(IntraBlock * block, int direct);

//KB: added for splitting IB into smaller IB in case of better RD for sum of smaller IBs;
void intra_pred_split(IntraPredParams * intra_pred_params, int mult);
void make_split_ib(IntraBlock * block);
void pred_split_ib(IntraBlock * block, int mult);

/* KB: filling intra prediction errors array in input array "data" and reference samples of IntraPredParams struct into "ref_sampl" array */
void fill_pred_subband_data(IntraPredParams * intra_pred_params, short * data, short * ref_sampl);

/* KB: The whole process of subband reconstruction after intra prediction  */
void process_inverse_intra_prediction(IntraPredParams * intra_pred_params, short * data, int component, int sub_num, double quant_factor);
void fill_recon_subband_data(IntraPredParams * intra_pred_params, short * data);

/* KB: Printing subband and intra blocks samples values - for test issues  */
void show_ib_smpl(IntraPredParams * intra_pred_params, int stage, int component, int sub_num);
void show_ib_info(IntraPredParams * intra_pred_params, int pos_x, int pos_y, int stage, int component, int sub_num);

/* KB: free memory after complition of intra predcition process */
void free_intra_pred_params(IntraPredParams * intra_pred_params);
void free_intra_block(IntraBlock * intra_block);

/* GV: create RefValues structure */
RefValues * create_ref_values(IntraBlock * block);
void filter_ref_values(RefValues * ref_values);

/* GV: free memory from reference values */
void free_ref(RefValues * ref_values);

/* GV: concatenate prediction data and reference values */
void concatenate_pred_ref(IntraPredParams * intra_pred_params, short * pred_data, short * ref_sampl, short * conc_data, int force_zero_ref);

/* GV: quantize and copy data */
void quantize_block(IntraBlock * block, double quant_factor);
void quantize_ref_samples(IntraPredParams * intra_pred_params, double quant_factor);
IntraBlock * copy_intra_block(IntraBlock * block);

#ifdef __cplusplus
}
#endif

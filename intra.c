#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "intra.h"

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ KB: Utils functions ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

void * malloc0(int size){
    void *ptr;  
    ptr = malloc (size);
    memset (ptr, 0, size);

    return ptr;
}

/* KB: implements A*B matrix multiplication */
gsl_matrix * mul_matrix(gsl_matrix * A, const gsl_matrix * B){
	int i,j,k,l;
	double convl;

	if (!(A->size1 && B->size1 && A->size2 && B->size2)){
		return NULL;
	}

	if (A->size2 != B->size1){
		return NULL;
	}

	gsl_matrix * mul_mat = gsl_matrix_alloc (A->size1, B->size2);

	for (j = 0; j < A->size1; j++) {
		for (i = 0; i < B->size2; i++) {
			convl = 0;
			for (k = 0; k < A->size2; k++ ) {
				//convl += A->data[j * A->size2 + k] * B->data[i + k * B->size2];
				convl += gsl_matrix_get(A, j, k) * gsl_matrix_get(B, k, i);
			}
			//mul_mat->data[i + j * A->size2] = convl;
			gsl_matrix_set (mul_mat, j, i, convl);
		}
	}

	return mul_mat;
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */







/* +++++++++++++++++ KB: Intra prediction parameters and intra blocks creation and initialization ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

IntraPredParams * create_intra_pred_params(int sub_width, int sub_height, int block_size, int pred_type, int split) {
    
    IntraPredParams *intra_pred_params;
    intra_pred_params = malloc0 (sizeof (IntraPredParams));

    intra_pred_params->block_size = block_size;
    intra_pred_params->sub_width = sub_width;
    intra_pred_params->sub_height = sub_height;
    intra_pred_params->pred_type = CLAMP_MAX(pred_type, 0, TYPE_NUMBER);
	intra_pred_params->split_flag = split ? 1 : 0 ;

	/* KB: 28.04.18
		Intra Blocks are iteratively splitted from one IB of NxN sizes to 4 smaller child IBs with sizes (N/2)x(N/2). The following opeartion is repeated iteratively:
	    Splitting blocks NxN only in case when splitted blocks sizes (N/2)x(N/2) are equal or more than 4x4 and when N - is even. In other cases blocks are not splitted.
		If split_depth = 0 then there is no splitting during Intra Prediction process. 
	*/
	if (intra_pred_params->split_flag){
		int size = intra_pred_params->block_size;
		int depth = 0;
		while ((size >> 3) && !(size % 2) ) {
			size = size >> 1;
			depth++;
		}
		intra_pred_params->split_depth = depth;
		if (!intra_pred_params->split_depth) {
			intra_pred_params->split_flag = 0;
			intra_pred_params->split_depth = 0;
		}
	}

    return intra_pred_params;
}

void init_intra_pred_params(IntraPredParams * intra_pred_params){

	int16_t *top_ref_sampl, *left_ref_sampl;
	int right_reminder, bottom_reminder;

	switch (intra_pred_params->pred_type)
	{
	case 0:
	case 3:
	case 7:
		top_ref_sampl = malloc0(intra_pred_params->sub_width * sizeof(int16_t));
		intra_pred_params->top_ref_sampl = top_ref_sampl;
		left_ref_sampl = NULL;
		intra_pred_params->left_ref_sampl = left_ref_sampl;
		intra_pred_params->sub_pred_width = intra_pred_params->sub_width;
		intra_pred_params->sub_pred_height = intra_pred_params->sub_height - 1;
		break;
	case 1:
	case 8:
		top_ref_sampl = NULL;
		intra_pred_params->top_ref_sampl = top_ref_sampl;
		left_ref_sampl = malloc0(intra_pred_params->sub_height * sizeof(int16_t));
		intra_pred_params->left_ref_sampl = left_ref_sampl;
		intra_pred_params->sub_pred_width = intra_pred_params->sub_width - 1;
		intra_pred_params->sub_pred_height = intra_pred_params->sub_height;
		break;
	case 2:
	case 4:
	case 5:
	case 6:
	case 9:
	case 10:
	default:
		top_ref_sampl = malloc0(intra_pred_params->sub_width * sizeof(int16_t));
		intra_pred_params->top_ref_sampl = top_ref_sampl;
		left_ref_sampl = malloc0(intra_pred_params->sub_height * sizeof(int16_t));
		intra_pred_params->left_ref_sampl = left_ref_sampl;
		intra_pred_params->sub_pred_width = intra_pred_params->sub_width - 1;
		intra_pred_params->sub_pred_height = intra_pred_params->sub_height - 1;
		break;
	}
	right_reminder = (intra_pred_params->sub_pred_width % intra_pred_params->block_size);
	bottom_reminder = (intra_pred_params->sub_pred_height % intra_pred_params->block_size);
	intra_pred_params->right_filling = right_reminder ? intra_pred_params->block_size - right_reminder : 0;
	intra_pred_params->bottom_filling = bottom_reminder ? intra_pred_params->block_size - bottom_reminder : 0;
	intra_pred_params->horiz_blocks = (intra_pred_params->right_filling) ? (intra_pred_params->sub_pred_width / intra_pred_params->block_size) + 1 : (intra_pred_params->sub_pred_width / intra_pred_params->block_size);
	intra_pred_params->vert_blocks = (intra_pred_params->bottom_filling) ? (intra_pred_params->sub_pred_height / intra_pred_params->block_size) + 1 : (intra_pred_params->sub_pred_height / intra_pred_params->block_size);

	//if (intra_pred_params->block_size == 4 || intra_pred_params->block_size == 8 )
	{
		intra_pred_params->pred_func[0] = &intra_pred_4x4_V;
		intra_pred_params->pred_func[1] = &intra_pred_4x4_H;
		intra_pred_params->pred_func[2] = &intra_pred_4x4_DC;
		intra_pred_params->pred_func[3] = &intra_pred_4x4_DDL;
		intra_pred_params->pred_func[4] = &intra_pred_4x4_DDR;
		intra_pred_params->pred_func[5] = &intra_pred_4x4_VR;
		intra_pred_params->pred_func[6] = &intra_pred_4x4_HD;
		intra_pred_params->pred_func[7] = &intra_pred_4x4_VL;
		intra_pred_params->pred_func[8] = &intra_pred_4x4_HU;
		intra_pred_params->pred_func[9] = &intra_pred_4x4_PLANE;
		intra_pred_params->pred_func[10] = &intra_pred_4x4_ADAPTIVE;
	}

	if (intra_pred_params->split_flag){
		intra_pred_params->child_order = malloc0(intra_pred_params->split_depth * sizeof(IntraBlock ***));
	}
}

void create_intra_blocks(IntraPredParams * intra_pred_params) {
	int k, i, j, d;
	IntraBlock *** rep_ptr = malloc0(intra_pred_params->vert_blocks * sizeof(IntraBlock **));

	for (k = 0; k < intra_pred_params->vert_blocks; k++) {
		rep_ptr[k] = malloc0(intra_pred_params->horiz_blocks * sizeof(IntraBlock *));
	}

	intra_pred_params->rep_order = rep_ptr;

	for (j = 0; j < intra_pred_params->vert_blocks; j++) {
		for (i = 0; i < intra_pred_params->horiz_blocks; i++) {
			rep_ptr[j][i] = create_intra_block(intra_pred_params, i, j, 0);
		}
	}

	for (d = 0; d < intra_pred_params->split_depth; d++) {
		create_child_blocks(intra_pred_params, d+1);
	}
}

void create_child_blocks(IntraPredParams * intra_pred_params, int depth){
	int k, i, j;
	int vert_blocks = intra_pred_params->vert_blocks << depth;
	int horiz_blocks = intra_pred_params->horiz_blocks << depth;
	IntraBlock *** rep_ptr = malloc0(vert_blocks * sizeof(IntraBlock **));

	for (k = 0; k < vert_blocks; k++) {
		rep_ptr[k] = malloc0(horiz_blocks * sizeof(IntraBlock *));
	}

	intra_pred_params->child_order[depth - 1] = rep_ptr;

	for (j = 0; j < vert_blocks; j++) {
		for (i = 0; i < horiz_blocks; i++) {
			rep_ptr[j][i] = create_intra_block(intra_pred_params, i, j, depth);
		}
	}
}

IntraBlock * create_intra_block(IntraPredParams * intra_pred_params, int pos_x, int pos_y, int depth) {
	int i;
	IntraBlock * intra_block = malloc0( sizeof(IntraBlock) );
	intra_block->intra_pred_params = intra_pred_params;
	intra_block->pos_x = pos_x;
	intra_block->pos_y = pos_y;
	intra_block->pred_mode = intra_pred_params->pred_type;
	intra_block->block_size = intra_pred_params->block_size >> depth;
	intra_block->depth = depth;

	int16_t ** sampl_ptr = malloc0(intra_block->block_size * sizeof(int16_t *));
	intra_block->sampl = sampl_ptr;

	for (i = 0; i < intra_block->block_size; i++) {
		sampl_ptr[i] = malloc0(intra_block->block_size * sizeof(int16_t));
	}

	intra_block->tmp_bottom = malloc0(intra_block->block_size * sizeof(int16_t));
	intra_block->tmp_right = malloc0(intra_block->block_size * sizeof(int16_t));

	intra_block->skip_flag = 0;
	intra_block->split_flag = 0;

	return intra_block;
}


/* KB: right now it is not usefull but it shall be so when Intra Prediction has been modified in many variants for DWT coefficients */
void create_prediction_order(IntraPredParams * intra_pred_params){  
	IntraBlock ** pred_order = malloc0(intra_pred_params->horiz_blocks * intra_pred_params->vert_blocks * sizeof(IntraBlock *));
	intra_pred_params->pred_order = pred_order;
	int i, j;
	for (j = 0; j < intra_pred_params->vert_blocks; j++) {
		for (i = 0; i < intra_pred_params->horiz_blocks; i++) {
			pred_order[i + j * intra_pred_params->horiz_blocks] = intra_pred_params->rep_order[j][i];
			pred_order[i + j * intra_pred_params->horiz_blocks]->pred_index = i + j * intra_pred_params->horiz_blocks;
		}
	}
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */









/* ++++++++++++++++++++++++++++++++++++++++++ KB: Filling subband data in intra block  structures ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

/* KB: function for custom filling data in different projects; for each project should be modified */
void fill_subband_data(IntraPredParams * intra_pred_params, short * data){
	int i, j, w, h;
	intra_pred_params->data = data;
	/* KB: filling reference left and top samples in subband */
	if (intra_pred_params->top_ref_sampl == NULL) {

		for (j = 0; j < intra_pred_params->sub_height; j++) {
			intra_pred_params->left_ref_sampl[j] = data[j * intra_pred_params->sub_width];
		}

		for (j = 0; j < intra_pred_params->vert_blocks; j++) {
			for (i = 0; i < intra_pred_params->horiz_blocks; i++) {
				IntraBlock * block = intra_pred_params->rep_order[j][i];
				for (h = 0; h < intra_pred_params->block_size; h++) {
					for (w = 0; w < intra_pred_params->block_size; w++) {
						block->sampl[h][w] = data[(i * intra_pred_params->block_size + 1)
							+ ((j * intra_pred_params->block_size + h) * intra_pred_params->sub_width) + w];
					}
				}
			}
		}

	} else {

		for (i = 0; i < intra_pred_params->sub_width; i++) {
			intra_pred_params->top_ref_sampl[i] = data[i];
		}

		if (intra_pred_params->left_ref_sampl == NULL) {

			for (j = 0; j < intra_pred_params->vert_blocks; j++) {
				for (i = 0; i < intra_pred_params->horiz_blocks; i++) {
					IntraBlock * block = intra_pred_params->rep_order[j][i];
					for (h = 0; h < intra_pred_params->block_size; h++) {
						for (w = 0; w < intra_pred_params->block_size; w++) {
							block->sampl[h][w] = data[(i * intra_pred_params->block_size)
								+ ((j * intra_pred_params->block_size + h + 1) * intra_pred_params->sub_width) + w];
						}
					}
				}
			}

		} else {

			for (j = 0; j < intra_pred_params->sub_height; j++) {
				intra_pred_params->left_ref_sampl[j] = data[j * intra_pred_params->sub_width];
			}

			for (j = 0; j < intra_pred_params->vert_blocks; j++) {
				for (i = 0; i < intra_pred_params->horiz_blocks; i++) {
					IntraBlock * block = intra_pred_params->rep_order[j][i];
					for (h = 0; h < intra_pred_params->block_size; h++) {
						for (w = 0; w < intra_pred_params->block_size; w++) {
							block->sampl[h][w] = data[(i * intra_pred_params->block_size + 1) 
								+ ((j * intra_pred_params->block_size + h + 1) * intra_pred_params->sub_width) + w];
						}
					}
				}
			}
		}
	}

}

void fill_borders(IntraPredParams * intra_pred_params){
	int i, j, k;

	if (intra_pred_params->right_filling != 0) {

		if (intra_pred_params->bottom_filling != 0) {

			int last_col = intra_pred_params->horiz_blocks - 1;
			int right_sampl = intra_pred_params->block_size - intra_pred_params->right_filling - 1;
			for (j = 0; j < intra_pred_params->vert_blocks - 1; j++) {
				IntraBlock * block = intra_pred_params->rep_order[j][last_col];
				for (i = 0; i < intra_pred_params->block_size; i++) {
					for (k = 0; k < intra_pred_params->right_filling; k++) {
						block->sampl[i][right_sampl + k + 1] = block->sampl[i][right_sampl];
					}
				}
			}

			int last_row = intra_pred_params->vert_blocks - 1;
			int bottom_sampl = intra_pred_params->block_size - intra_pred_params->bottom_filling - 1;
			for (i = 0; i < intra_pred_params->horiz_blocks - 1; i++) {
				IntraBlock * block = intra_pred_params->rep_order[last_row][i];
				for (j = 0; j < intra_pred_params->block_size; j++) {
					for (k = 0; k < intra_pred_params->bottom_filling; k++) {
						block->sampl[bottom_sampl + k + 1][j] = block->sampl[bottom_sampl][j];
					}
				}
			}

			IntraBlock * block = intra_pred_params->rep_order[last_row][last_col];
			for (j = 0; j < bottom_sampl + 1; j++) {
				for (k = 0; k < intra_pred_params->right_filling; k++) {
					block->sampl[j][right_sampl + k + 1] = block->sampl[j][right_sampl];
				}
			}
			for (i = 0; i < intra_pred_params->block_size; i++) {
				for (k = 0; k < intra_pred_params->bottom_filling; k++) {
					block->sampl[bottom_sampl + k + 1][i] = block->sampl[bottom_sampl][i];
				}
			}


		} else {

			int last_col =  intra_pred_params->horiz_blocks - 1;
			int right_sampl = intra_pred_params->block_size - intra_pred_params->right_filling - 1;
			for (j = 0; j < intra_pred_params->vert_blocks; j++) {
				IntraBlock * block = intra_pred_params->rep_order[j][last_col];
				for (i = 0; i < intra_pred_params->block_size; i++) {
					for (k = 0; k < intra_pred_params->right_filling; k++) {
						 block->sampl[i][right_sampl + k + 1] = block->sampl[i][right_sampl];
					}
				}
			}

		}

	} else if (intra_pred_params->bottom_filling != 0) {

		int last_row = intra_pred_params->vert_blocks - 1;
		int bottom_sampl = intra_pred_params->block_size - intra_pred_params->bottom_filling - 1;
		for (i = 0; i < intra_pred_params->horiz_blocks; i++) {
			IntraBlock * block = intra_pred_params->rep_order[last_row][i];
			for (j = 0; j < intra_pred_params->block_size; j++) {
				for (k = 0; k < intra_pred_params->bottom_filling; k++) {
					block->sampl[bottom_sampl + k + 1][j] = block->sampl[bottom_sampl][j];
				}
			}
		}

	}

}

void fill_tmp(IntraPredParams * intra_pred_params){
	int i, j, w, h;
	for (j = 0; j < intra_pred_params->vert_blocks; j++) {
		for (i = 0; i < intra_pred_params->horiz_blocks; i++) {
			fill_tmp_block(intra_pred_params->rep_order[j][i]);
		}
	}
	/*
	if(intra_pred_params->split_flag) {
		int d, i, j;
		for (d = 0; d < intra_pred_params->split_depth; d++) {
			int vert_blocks = intra_pred_params->vert_blocks << (d+1);
        	int horiz_blocks = intra_pred_params->horiz_blocks << (d+1);
			for (j = 0; j < vert_blocks; j++){
				for(i = 0; i < horiz_blocks; i++){
					fill_tmp_block(intra_pred_params->child_order[d][j][i]);
				}
			}
		}
	}
	*/
}

void fill_tmp_block(IntraBlock * block){
	int w, h;
	int block_size = block->block_size - 1;
	for (h = 0; h < block->block_size; h++) {
		block->tmp_right[h] = block->sampl[h][block_size];
	}
	for (w = 0; w < block->block_size; w++) {
		block->tmp_bottom[w] = block->sampl[block_size][w];
	}
}
						
fill_tmp_block_from(IntraBlock * block_dest, IntraBlock * block_src){
	int w, h;
	int block_size = block_src->block_size - 1;
	for (h = 0; h < block_src->block_size; h++) {
		block_dest->tmp_right[h] = block_src->sampl[h][block_size];
	}
	for (w = 0; w < block_src->block_size; w++) {
		block_dest->tmp_bottom[w] = block_src->sampl[block_size][w];
	}
}

void fill_child_blocks(IntraPredParams * intra_pred_params){
	int d, i, j;
	for (d = 0; d < intra_pred_params->split_depth; d++){
		int vert_blocks = intra_pred_params->vert_blocks << (d+1);
		int horiz_blocks = intra_pred_params->horiz_blocks << (d+1);
		for (j = 0; j < vert_blocks; j++) {
			for (i = 0; i < horiz_blocks; i++) {
				IntraBlock * root_ib = intra_pred_params->rep_order[j >> (d+1)][i >> (d+1)];
				IntraBlock * child_ib = intra_pred_params->child_order[d][j][i];
				copy_smpl_ib(child_ib, root_ib);
				fill_tmp_block(child_ib);
			}
		}
	}
}

void fill_parent_blocks(IntraPredParams *intra_pred_params){
	int d, i, j;
	for (d = intra_pred_params->split_depth; d > 1; d--){
		int vert_blocks = intra_pred_params->vert_blocks << d;
		int horiz_blocks = intra_pred_params->horiz_blocks << d;
		for (j = 0; j < vert_blocks; j++) {
			for (i = 0; i < horiz_blocks; i++) {
				IntraBlock * root_ib = intra_pred_params->child_order[d-2][j >> d][i >> d];
				IntraBlock * child_ib = intra_pred_params->child_order[d-1][j][i];
				copy_smpl_ib(root_ib, child_ib);
				fill_tmp_block(root_ib);
			}
		}
	}
}

void copy_smpl_ib (IntraBlock * dest, IntraBlock * src){
	int dest_depth = dest->depth;
	int src_depth = src->depth;
	int sft_x, sft_y, add_x, add_y, depth; 
	int i, j;
	if (src_depth < dest_depth){
		depth = dest_depth - src_depth;
		sft_x = (dest->pos_x % (1 << depth));
		sft_y = (dest->pos_y % (1 << depth));
		add_x = sft_x * dest->block_size;
		add_y = sft_y * dest->block_size;
		for (j = 0; j < dest->block_size; j++) {
			for (i = 0; i < dest->block_size; i++) {
				dest->sampl[j][i] = src->sampl[j + add_y][i + add_x];
			}
		}
	} else if (src_depth > dest_depth) {
		depth = src_depth - dest_depth;
		sft_x = (src->pos_x % (1 << depth));
		sft_y = (src->pos_y % (1 << depth));
		add_x = sft_x * src->block_size;
		add_y = sft_y * src->block_size;
		for (j = 0; j < src->block_size; j++) {
			for (i = 0; i < src->block_size; i++) {
				dest->sampl[j + add_y][i + add_x] = src->sampl[j][i];
			}
		}
	} else if (src_depth = dest_depth) {
		for (j = 0; j < dest->block_size; j++) {
			for (i = 0; i < dest->block_size; i++) {
				dest->sampl[j][i] = src->sampl[j][i];
			}
		}
	}
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */








/* +++++++++++++++ KB: Performs the whole forward intar prediction process - from taking data from initial DWT subband to intra predcition within all intra blocks ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */


void process_intra_prediction(IntraPredParams * intra_pred_params, short * data, int component, int sub_num, double quant_factor){
	int x, y;

	init_intra_pred_params(intra_pred_params);

	create_intra_blocks(intra_pred_params);

	create_prediction_order(intra_pred_params);

	fill_subband_data(intra_pred_params, data);

	fill_borders(intra_pred_params);

	if(intra_pred_params->split_flag) {
		fill_child_blocks(intra_pred_params);
	}

	fill_tmp(intra_pred_params);

    #ifdef PRINT_TEST_FILES
	show_ib_smpl(intra_pred_params, 0, component, sub_num);

	for (y = 0; y < intra_pred_params->vert_blocks; y++) {
		for (x = 0; x < intra_pred_params->horiz_blocks; x++) {
			show_ib_info(intra_pred_params, x, y, 0, component, sub_num);
		}
	}
	#endif

	make_intra_pred(intra_pred_params, 0, quant_factor);

	if(intra_pred_params->split_flag) {
		fill_child_blocks(intra_pred_params);
	}


    #ifdef PRINT_TEST_FILES
	show_ib_smpl(intra_pred_params, 1, component, sub_num);

	for (y = 0; y < intra_pred_params->vert_blocks; y++) {
		for (x = 0; x < intra_pred_params->horiz_blocks; x++) {
			show_ib_info(intra_pred_params, x, y, 1, component, sub_num);
		}
	}
    #endif

	count_skipped_blocks(intra_pred_params);

}

void count_skipped_blocks(IntraPredParams * intra_pred_params)
{   
	int skip_count = 0;
	intra_pred_params->zero_count = 0;
	intra_pred_params->non_zero_count = 0;
	if (intra_pred_params->split_flag) {
		int total_count  = 0;
		int * split_blocks = malloc0((intra_pred_params->split_depth + 1) * sizeof(int));
		int i, j, d, x, y;
		for (j = 0; j < intra_pred_params->vert_blocks; j++) {
			for (i = 0; i < intra_pred_params->horiz_blocks; i++) {
				IntraBlock * block = intra_pred_params->rep_order[j][i];
				count_split_blocks(block, &total_count, split_blocks, &skip_count);
			}
		}
		intra_pred_params->total_blocks = total_count;
		intra_pred_params->split_blocks = split_blocks;
	} else {
		int i, j;
		for (j = 0; j < intra_pred_params->vert_blocks; j++) {
			for (i = 0; i < intra_pred_params->horiz_blocks; i++) {
				if (intra_pred_params->rep_order[j][i]->skip_flag) {
					skip_count++;
				}
			}
		}
		intra_pred_params->total_blocks = intra_pred_params->vert_blocks * intra_pred_params->horiz_blocks;
	}

	intra_pred_params->skipped_blocks = skip_count;
}

void count_split_blocks(IntraBlock * block, int * total_count, int * split_blocks, int * skip_count){
	if (block->split_flag) {
		block->intra_pred_params->non_zero_count++;
		int x, y;
		int pos_x = block->pos_x << 1;
		int pos_y = block->pos_y << 1;
		IntraBlock ** child_blocks = malloc0(4 * sizeof(IntraBlock *));
		for (y = 0; y < 2; y++) {
			for (x = 0; x < 2; x++) {
				child_blocks[x+2*y] = block->intra_pred_params->child_order[block->depth][pos_y + y][pos_x + x];
				count_split_blocks(child_blocks[x+2*y], total_count, split_blocks, skip_count);
			}
		}
		free(child_blocks);
	} else {
		(*total_count)++;
		split_blocks[block->depth]++;
		block->intra_pred_params->zero_count++;
		if (block->skip_flag) {
			(*skip_count)++;
		}
	}
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */




/* ++++++++++++++++++++++++++++++++++++++++++ KB: Rate-distortion estimation +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

double get_mse(IntraBlock * block)
{
	int i, j;
	int sum = 0;
	double denum = (double) (block->block_size * block->block_size);
	for (j = 0; j < block->block_size; j++) {
		for (i = 0; i < block->block_size; i++) {
			sum += block->sampl[j][i];
		}
	}
	double mean = (double) sum / denum;
	double variance = 0;
	for (j = 0; j < block->block_size; j++) {
		for (i = 0; i < block->block_size; i++) {
			double diff = block->sampl[j][i] - mean;
			variance += (diff * diff);
		}
	}

	double MSE = sqrt(variance / denum);

	return MSE;
}

int get_sad(IntraBlock * block){
	int i,j;
	int SAD = 0;
	for (j = 0; j < block->block_size; j++){
		for(i = 0; i < block->block_size; i++){
			SAD += abs(block->sampl[j][i]);
		}
	}

	return SAD;
}

double get_ssd(IntraBlock * block){
	int i,j;
	int SSD = 0;
	for (j = 0; j < block->block_size; j++){
		for(i = 0; i < block->block_size; i++){
			SSD += (block->sampl[j][i] * block->sampl[j][i]);
		}
	}

	return (double) SSD;
}

/* Square Root of SSD */
double get_rssd(IntraBlock * block){	
	double RSSD = sqrt(get_ssd(block));
	return RSSD;
}

/* Normilized Square Root of SSD */
double get_nrssd(IntraBlock * block){	
	double NRSSD = sqrt(get_ssd(block) / (block->block_size * block->block_size) );
	return NRSSD;
}

double get_satd(IntraBlock * block){
	int i,j;
	double SATD = 0;

	gsl_matrix * ib = gsl_matrix_alloc (block->block_size, block->block_size);
	gsl_matrix * had = gsl_matrix_alloc (block->block_size, block->block_size);
	gsl_matrix * had_inv = gsl_matrix_alloc (block->block_size, block->block_size);

    for (j = 0; j < block->block_size; j++){
		for(i = 0; i < block->block_size; i++){
			gsl_matrix_set (ib, j, i, block->sampl[j][i]);
			gsl_matrix_set (had, j, i, HADAMARD_4x4[j][i]);
		}
	}
    
	gsl_matrix_transpose_memcpy(had_inv, had);
	/*
    gsl_matrix_mul_elements(had, ib);
	gsl_matrix_mul_elements(had, had_inv);
	*/
    gsl_matrix * had_ib = mul_matrix(had, ib); 
	gsl_matrix * satd_mat = mul_matrix(had_ib, had_inv);

	for (j = 0; j < block->block_size; j++){
		for(i = 0; i < block->block_size; i++){
			SATD +=  abs(gsl_matrix_get(satd_mat, j, i));
		}
	}
    
	//SATD -= abs(gsl_matrix_get(satd_mat, 0, 0)); //KB; worked well for HEVC but not for our solution
	double mult = log2(block->block_size);
	double mult2 = powf(powf(0.5, (mult / 2.0) ) , 2.0) ;
	SATD = SATD * mult2 ;
	
    gsl_matrix_free (had_ib);
	gsl_matrix_free (satd_mat);

	gsl_matrix_free (ib);
	gsl_matrix_free (had);
	gsl_matrix_free (had_inv);
    
	return SATD;
}

double get_haar_rd(IntraBlock * block){
	int i,j;
	double haar_rd = 0;

	gsl_matrix * ib = gsl_matrix_alloc (block->block_size, block->block_size);
	gsl_matrix * haar = gsl_matrix_alloc (block->block_size, block->block_size);
	gsl_matrix * haar_inv = gsl_matrix_alloc (block->block_size, block->block_size);

    for (j = 0; j < block->block_size; j++){
		for(i = 0; i < block->block_size; i++){
			gsl_matrix_set (ib, j, i, block->sampl[j][i]);
			gsl_matrix_set (haar, j, i, HAAR_4x4[j][i]);
		}
	}
    
	gsl_matrix_transpose_memcpy(haar_inv, haar);
	/*
    gsl_matrix_mul_elements(had, ib);
	gsl_matrix_mul_elements(had, had_inv);
	*/
    gsl_matrix * haar_ib = mul_matrix(haar, ib); 
	gsl_matrix * haar_mat = mul_matrix(haar_ib, haar_inv);

	for (j = 0; j < block->block_size; j++){
		for(i = 0; i < block->block_size; i++){
			haar_rd +=  abs(gsl_matrix_get(haar_mat, j, i));
		}
	}
    
	double mult = log2(block->block_size);
	double mult2 = powf(powf(0.5, (mult / 2.0) ) , 2.0) ;
	haar_rd = haar_rd * mult2 ;
	
    gsl_matrix_free (haar_ib);
	gsl_matrix_free (haar_mat);

	gsl_matrix_free (ib);
	gsl_matrix_free (haar);
	gsl_matrix_free (haar_inv);
    
	return haar_rd;
}

int get_bit_ib(IntraBlock * block){
	int i,j;
	int bit_ib = 0;
	for (j = 0; j < block->block_size; j++){
		for(i = 0; i < block->block_size; i++){
			bit_ib += ceil(log2(abs(block->sampl[j][i]) + 1)) + 1;
		}
	}

	return bit_ib;
}

double estimate_ent_ib(IntraBlock * block){
	int i, j, l;
	double ib_ent = 0;
	int denum = block->block_size * block->block_size;
	int * count = malloc0(denum * sizeof(int));
	int16_t * vals = malloc0(denum * sizeof(int16_t));
	vals[0] = block->sampl[0][0];
	int symb = 1;
	for (j = 0; j < block->block_size; j++) {
		for (i = 0; i < block->block_size; i++) {
			int value = block->sampl[j][i];
			int t;
			int skip = 0;
			for (t = 0; t < symb; t++) {
				if (value == vals[t]) {
					count[t]++;
					skip = 1;
					break;
				}
			}
			if (!skip){
				vals[symb] = value;
				count[symb] = 1;
				symb++;
			}
		}
	}

	for (l = 0; l < symb; l++) {
		double prob = (double) count[l] / (double) denum;
		ib_ent -= prob * log2(prob);
	}

	free(count);
	free(vals);

	return ib_ent;
}

double etsimate_ent_whole(IntraBlock * block){
	int i, j, l;
	double ent = 0;
	int prev_enc_ib = block->pred_index + 1;
	int denum = prev_enc_ib * block->block_size * block->block_size;
	int * count = malloc0(denum * sizeof(int));
	int16_t * vals = malloc0(denum * sizeof(int16_t));
	vals[0] = block->intra_pred_params->pred_order[0]->sampl[0][0];
	int symb = 1;
	for (l = 0; l < prev_enc_ib; l++) {
	IntraBlock * prev_block = block->intra_pred_params->pred_order[l];
		for (j = 0; j < prev_block->block_size; j++) {
			for (i = 0; i < prev_block->block_size; i++) {
				int value = prev_block->sampl[j][i];
				int t;
				int skip = 0;
				for (t = 0; t < symb; t++) {
					if (value == vals[t]) {
						count[t]++;
						skip = 1;
						break;
					}
				}
				if (!skip){
					vals[symb] = value;
					count[symb] = 1;
					symb++;
				}
			}
		}
	}

	for (l = 0; l < symb; l++) {
		double prob = (double) count[l] / (double) denum;
		ent -= prob * log2(prob);
	}

	free(count);
	free(vals);

	return ent;
}

double estimate_ent_neig(IntraBlock * block){
	int i, j, l;
	double ent = 0;

	IntraBlock ** blocks;
	int block_num;

	if (block->pos_y == 0) {
		if (block->pos_x == 0) {
			block_num = 1;
			blocks = malloc0(sizeof(IntraBlock *));
			blocks[0] = block;
		} else {
			block_num = 2;
			blocks = malloc0(2 * sizeof(IntraBlock *));
			blocks[0] = block->intra_pred_params->rep_order[block->pos_y][block->pos_x - 1];
			blocks[1] = block;
		}
	} else if (block->pos_x == 0) {
		block_num = 3;
		blocks = malloc0(3 * sizeof(IntraBlock *));
		blocks[0] = block->intra_pred_params->rep_order[block->pos_y - 1][block->pos_x];
		blocks[1] = block->intra_pred_params->rep_order[block->pos_y - 1][block->pos_x + 1];
		blocks[2] = block;
	} else if (block->pos_x == block->intra_pred_params->horiz_blocks - 1){
		block_num = 4;
		blocks = malloc0(4 * sizeof(IntraBlock *));
		blocks[0] = block->intra_pred_params->rep_order[block->pos_y - 1][block->pos_x - 1];
		blocks[1] = block->intra_pred_params->rep_order[block->pos_y - 1][block->pos_x];
		blocks[2] = block->intra_pred_params->rep_order[block->pos_y][block->pos_x - 1];
		blocks[3] = block;
	} else {
		block_num = 5;
		blocks = malloc0(5 * sizeof(IntraBlock *));
		blocks[0] = block->intra_pred_params->rep_order[block->pos_y - 1][block->pos_x - 1];
		blocks[1] = block->intra_pred_params->rep_order[block->pos_y - 1][block->pos_x];
		blocks[2] = block->intra_pred_params->rep_order[block->pos_y - 1][block->pos_x + 1];
		blocks[3] = block->intra_pred_params->rep_order[block->pos_y][block->pos_x - 1];
		blocks[4] = block;
	}


	int denum = block_num * block->block_size * block->block_size;
	int * count = malloc0(denum * sizeof(int));
	int16_t * vals = malloc0(denum * sizeof(int16_t));
	vals[0] = blocks[0]->sampl[0][0];
	int symb = 1;
	for (l = 0; l < block_num; l++) {
	IntraBlock * prev_block = blocks[l];
		for (j = 0; j < prev_block->block_size; j++) {
			for (i = 0; i < prev_block->block_size; i++) {
				int value = prev_block->sampl[j][i];
				int t;
				int skip = 0;
				for (t = 0; t < symb; t++) {
					if (value == vals[t]) {
						count[t]++;
						skip = 1;
						break;
					}
				}
				if (!skip){
					vals[symb] = value;
					count[symb] = 1;
					symb++;
				}
			}
		}
	}

	for (l = 0; l < symb; l++) {
		double prob = (double) count[l] / (double) denum;
		ent -= prob * log2(prob);
	}

	free(count);
	free(vals);
	free(blocks);

	return ent;
}

double estimate_ib_rd(IntraBlock * block) {
	/*double MSE = get_mse(block);
	return MSE;*/
	double SAD = get_sad(block);
	return SAD;
	/*double SSD = get_ssd(block);
	return SSD;*/
	/*double RSSD = get_rssd(block);
	return RSSD;*/
	/*double NRSSD = get_nrssd(block);
	return NRSSD;*/
	/*double SATD = get_satd(block);
	return SATD;*/
	/*double bit_ib = get_bit_rd(block);
	return bit_rd;*/
	/*double ent_ib = estimate_ent_ib(block);
	return ent_ib;*/
	/*double ent_ib = estimate_ent_neig(block);
	return ent_ib;*/
	/*double haar_rd = get_haar_rd(block);
	return haar_rd;*/
}


//GV: extract modes from the structure
INTRA_PRED_MODE *get_intra_modes(IntraPredParams * intra_pred_params)
{
	INTRA_PRED_MODE * modes;
	if (intra_pred_params->split_flag){
		modes = malloc0(intra_pred_params->total_blocks * sizeof(INTRA_PRED_MODE));
		int i,j;
		int t = 0;
		for (j=0; j<intra_pred_params->vert_blocks; j++){
			for (i=0; i<intra_pred_params->horiz_blocks; i++){
				IntraBlock * root_block = intra_pred_params->rep_order[j][i];
				get_intra_modes_split(root_block, modes, &t);
			}
		}

	} else {
		modes = malloc0(intra_pred_params->vert_blocks * intra_pred_params->horiz_blocks * sizeof(INTRA_PRED_MODE));

		int i,j;
		for (j=0; j<intra_pred_params->vert_blocks; j++)
			for (i=0; i<intra_pred_params->horiz_blocks; i++)
				modes[j*intra_pred_params->horiz_blocks+i]=intra_pred_params->rep_order[j][i]->pred_mode;

	}

	return modes;
}

short *get_intra_skip_flags(IntraPredParams * intra_pred_params)
{
	short * skip_flags;
	if (intra_pred_params->split_flag){
		skip_flags = malloc0(intra_pred_params->total_blocks * sizeof(short));
		int i,j;
		int t = 0;
		for (j=0; j<intra_pred_params->vert_blocks; j++){
			for (i=0; i<intra_pred_params->horiz_blocks; i++){
				IntraBlock * root_block = intra_pred_params->rep_order[j][i];
				get_intra_modes_split(root_block, skip_flags, &t);
			}
		}

	} else {
		skip_flags = malloc0(intra_pred_params->vert_blocks * intra_pred_params->horiz_blocks * sizeof(short));

		int i,j;
		for (j=0; j<intra_pred_params->vert_blocks; j++)
			for (i=0; i<intra_pred_params->horiz_blocks; i++)
				skip_flags[j*intra_pred_params->horiz_blocks+i]=(short) intra_pred_params->rep_order[j][i]->skip_flag;

	}

	return skip_flags;
}

void get_intra_modes_split(IntraBlock * block, INTRA_PRED_MODE * modes, int * count){
	if (block->split_flag) {
		int x,y;
		int pos_x = block->pos_x << 1;
		int pos_y = block->pos_y << 1;
		IntraBlock ** child_blocks = malloc0(4 * sizeof(IntraBlock *) );
		for (y = 0; y < 2; y++) {
			for (x = 0; x < 2; x++) {
				child_blocks[x+2*y] = block->intra_pred_params->child_order[block->depth][pos_y + y][pos_x + x];
				get_intra_modes_split(child_blocks[x+2*y], modes, count);
			}
		}
		free(child_blocks);
	} else {
		modes[(*count)]=block->pred_mode;
		(*count)++;
	}
}

void get_intra_skip_flags_split(IntraBlock * block, short * skip_flags, int * count){
	if (block->split_flag) {
		int x,y;
		int pos_x = block->pos_x << 1;
		int pos_y = block->pos_y << 1;
		IntraBlock ** child_blocks = malloc0(4 * sizeof(IntraBlock *) );
		for (y = 0; y < 2; y++) {
			for (x = 0; x < 2; x++) {
				child_blocks[x+2*y] = block->intra_pred_params->child_order[block->depth][pos_y + y][pos_x + x];
				get_intra_skip_flags_split(child_blocks[x+2*y], skip_flags, count);
			}
		}
		free(child_blocks);
	} else {
		skip_flags[(*count)]=(short) block->skip_flag;
		(*count)++;
	}
}

double get_rd_split(IntraBlock * block){
	block->intra_pred_params->pred_func[block->intra_pred_params->pred_type](block, 1);
	block->RD1 = estimate_ib_rd(block);
	block->intra_pred_params->pred_func[block->intra_pred_params->pred_type](block, -1);
	double RD_root;
	if (bAllowSkipPred)
	{
		block->RD0 = estimate_ib_rd(block);
		RD_root = (block->RD1 > block->RD0) ? block->RD0 : block->RD1;
	}
	else
		RD_root = block->RD1;
	return RD_root;
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */








/* ++++++++++++++++++++++++++++++++++++++++++ KB: Forward and Inverse Intra Prediction +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

void make_intra_pred(IntraPredParams * intra_pred_params, int direct, double quant_factor){
	int i, j;
	int mult = MULT(direct);
	
	//GV 15.06.18
	int16_t * top_ref_sampl_mem, * left_ref_sampl_mem;
	if (predScheme == 2) {
		if (mult == 1) {
			quantize_ref_samples(intra_pred_params, quant_factor);
			if (intra_pred_params->top_ref_sampl) {
				top_ref_sampl_mem = malloc0(intra_pred_params->sub_width*sizeof(int16_t));
				memcpy(top_ref_sampl_mem, intra_pred_params->top_ref_sampl, intra_pred_params->sub_width*sizeof(int16_t));
			}
			if (intra_pred_params->left_ref_sampl) {
				left_ref_sampl_mem = malloc0(intra_pred_params->sub_height*sizeof(int16_t));
				memcpy(left_ref_sampl_mem, intra_pred_params->left_ref_sampl, intra_pred_params->sub_height*sizeof(int16_t));
			}
			quantize_ref_samples(intra_pred_params, 1.0/quant_factor);
		}
		else {
			quantize_ref_samples(intra_pred_params, 1.0/quant_factor);
		}
	}

	if (intra_pred_params->split_flag) { //KB: 28.04.18
		intra_pred_split(intra_pred_params, mult);
	} else {
		switch (intra_pred_params->pred_type){
		default:					//GV: compiled
			if (intra_pred_params->pred_type < 0 || intra_pred_params->pred_type > TYPE_NUMBER)
				break;
			for (j = 0; j < intra_pred_params->vert_blocks; j++) {
				for (i = 0; i < intra_pred_params->horiz_blocks; i++) {
					//IntraBlock * block = intra_pred_params->pred_order[i + j * intra_pred_params->sub_width]; //KB: right now it is spspended cause Representation and Prediction Orders are the same - through the picture raster. But it should be changed in the future (when some parallezation and optimization of prediction process). 
					IntraBlock * block = intra_pred_params->rep_order[j][i];
					if (mult == 1) {
						block->RD0 = estimate_ib_rd(block);
					}
					else if (predScheme == 2) {
						quantize_block(block, 1.0/quant_factor);
					}
					intra_pred_params->pred_func[intra_pred_params->pred_type](block, mult);
					if (mult == 1) {
						block->RD1 = estimate_ib_rd(block);
					}
					if (bAllowSkipPred && block->RD1 > block->RD0) {
						intra_pred_params->pred_func[intra_pred_params->pred_type](block, (-mult));
						block->skip_flag = 1;
						
						if (predScheme == 2 && mult == 1) {
							IntraBlock * restored_block;
							quantize_block(block, quant_factor);
							restored_block = copy_intra_block(block);
							quantize_block(restored_block, 1.0/quant_factor);
							fill_tmp_block_from(block, restored_block);
							free_intra_block(restored_block);
						}
					}
					else if (mult == 1) {
						// GV 15.06.18
						if (predScheme == 2) {
							IntraBlock * restored_block;
							quantize_block(block, quant_factor);
							restored_block = copy_intra_block(block);
							quantize_block(restored_block, 1.0/quant_factor);
							intra_pred_params->pred_func[intra_pred_params->pred_type](restored_block, (-mult));
							fill_tmp_block_from(block, restored_block);
							free_intra_block(restored_block);
						}
					}

					if (mult != 1) {
						fill_tmp_block(block);
					}
				}
			}
			break;
		case TYPE_NUMBER:
			for (j = 0; j < intra_pred_params->vert_blocks; j++) {
				for (i = 0; i < intra_pred_params->horiz_blocks; i++) {
					//IntraBlock * block = intra_pred_params->pred_order[i + j * intra_pred_params->sub_width]; //KB: right now it is spspended cause Representation and Prediction Orders are the same - through the picture raster. But it should be changed in the future (when some parallezation and optimization of prediction process). 
					IntraBlock * block = intra_pred_params->rep_order[j][i];
					if (mult == 1) {
						block->RD0 = estimate_ib_rd(block);
					}
					else if (predScheme == 2) {
						quantize_block(block, 1.0/quant_factor);
					}
					intra_pred_4x4_ADAPTIVE(block, mult);
					if (predScheme == 2 && mult == 1) {
						IntraBlock * restored_block;
						quantize_block(block, quant_factor);
						restored_block = copy_intra_block(block);
						quantize_block(restored_block, 1.0/quant_factor);
						intra_pred_4x4_ADAPTIVE(restored_block, (-mult));
						fill_tmp_block_from(block, restored_block);
						free_intra_block(restored_block);
					}
					if (mult != 1) {
						fill_tmp_block(block);
					}
				}
			}
			break;
		}
	}

	if (mult == 1) {
		fill_tmp(intra_pred_params);
		if (predScheme == 2 && intra_pred_params->top_ref_sampl) {
			free(intra_pred_params->top_ref_sampl);
			intra_pred_params->top_ref_sampl = top_ref_sampl_mem;
		}
		if (predScheme == 2 && intra_pred_params->left_ref_sampl) {
			free(intra_pred_params->left_ref_sampl);
			intra_pred_params->left_ref_sampl = left_ref_sampl_mem;
		}
	}
}

void intra_pred_4x4_V(IntraBlock * block, int mult){

	if (bAllowSkipPred && block->skip_flag) {
		return;
	}

	/*if ( (block->depth == 1) && (block->pos_x == 1) && (block->pos_y == 3) ){
		printf("stop");
	}*/

	int i, j;
	RefValues * ref_values = create_ref_values(block);

	/* KB: 08.05.2018 - for splitting blocks*/
	//int fl_right = block->pos_x == block->intra_pred_params->horiz_blocks - 1;
	int fl_right = block->pos_x == ( (block->intra_pred_params->horiz_blocks << block->depth) - 1 ); 
	
	//GV: requires any top reference points
	//if (ref_values->top_size < (fl_right ? block->block_size-block->intra_pred_params->right_filling : block->block_size)) /* KB: 08.05.2018 - for splitting blocks*/
	if (ref_values->top_size < (fl_right ? (block->block_size << block->block_size)-block->intra_pred_params->right_filling : block->block_size))
	{
		block->skip_flag=1;
		return;
	}

	if (fl_right) {
		for (j = 0; j < block->block_size; j++) {
			for (i = 0; i < block->block_size - block->intra_pred_params->right_filling; i++) {
				#ifdef USE_MOD
				block->sampl[j][i] = MOD256( (block->sampl[j][i] - mult * ref_values->top[i] );
				#else
				block->sampl[j][i] -= mult * ref_values->top[i];
				#endif
			}
			for ( ; i < block->block_size; i++) {
				block->sampl[j][i] = block->sampl[j][block->block_size - block->intra_pred_params->right_filling - 1];
			}
		}
	} else {
		for (j = 0; j < block->block_size; j++) {
			for (i = 0; i < block->block_size; i++) {
				#ifdef USE_MOD
				block->sampl[j][i] = MOD256( (block->sampl[j][i] - mult * ref_values->top[i] );
				#else
				block->sampl[j][i] -= mult * ref_values->top[i];
				#endif				
			}
		}
	}
	free_ref(ref_values);
}
void intra_pred_4x4_H(IntraBlock * block, int mult){

   	if (bAllowSkipPred && block->skip_flag) {
		return;
	}
	int i, j;
	RefValues * ref_values = create_ref_values(block);
	/* KB: 08.05.2018 - for splitting blocks*/
	//int fl_bottom = block->pos_y == block->intra_pred_params->vert_blocks - 1;
	int fl_bottom = block->pos_y == ( (block->intra_pred_params->vert_blocks << block->depth) - 1 );

	//GV: requires left reference points enough for this block
	//if (ref_values->left_size < (fl_bottom ? block->block_size-block->intra_pred_params->bottom_filling : block->block_size)) /* KB: 08.05.2018 - for splitting blocks*/
	if (ref_values->left_size < (fl_bottom ? (block->block_size << block->depth)-block->intra_pred_params->bottom_filling : block->block_size))
	{
		block->skip_flag=1;
		return;
	}

	if (fl_bottom) {
		for (i = 0; i < block->block_size; i++) {
			for (j = 0; j < block->block_size - block->intra_pred_params->bottom_filling; j++) {
				#ifdef USE_MOD
				block->sampl[j][i] = MOD256( (block->sampl[j][i] - mult * ref_values->left[j] );
				#else
				block->sampl[j][i] -= mult * ref_values->left[j];
				#endif
			}
			for ( ; j < block->block_size; j++) {
				block->sampl[j][i] = block->sampl[block->block_size - block->intra_pred_params->bottom_filling - 1][i];
			}
		}
	} else {
		for (j = 0; j < block->block_size; j++) {
			for (i = 0; i < block->block_size; i++) {
				#ifdef USE_MOD
				block->sampl[j][i] = MOD256( (block->sampl[j][i] - mult * ref_values->left[j] );
				#else
				block->sampl[j][i] -= mult * ref_values->left[j];
				#endif				
			}
		}
	}
	free_ref(ref_values);
}

void intra_pred_4x4_DC(IntraBlock * block, int mult){
	
	if (bAllowSkipPred && block->skip_flag) {
		return;
	}
	int i, j, l;
	RefValues * ref_values = create_ref_values(block);
	//int fl_bottom = block->pos_y == block->intra_pred_params->vert_blocks - 1;
	//int fl_right = block->pos_x == block->intra_pred_params->horiz_blocks - 1;
	
	// GV: doesn't require any reference point
	int fl_access_top = ref_values->top_size >= block->block_size;
	int fl_access_left = ref_values->left_size >= block->block_size;

	int sum = 0;
	int dc = 0;

	if (fl_access_top)
		for (l = 0; l < block->block_size; l++)
			sum += ref_values->top[l];

	if (fl_access_left)
		for (l = 0; l < block->block_size; l++)
			sum += ref_values->left[l];

	if (fl_access_top + fl_access_left == 1)
		dc = mult * (sum + (block->block_size >> 1)) / block->block_size;
	else if (fl_access_top && fl_access_left)
		dc = mult * (sum + block->block_size) / (block->block_size << 1);
	else
		dc = mult * 128;

	for (j = 0; j < block->block_size; j++) {
		for (i = 0; i < block->block_size; i++) {
			#ifdef USE_MOD
			block->sampl[j][i] = MOD256((block->sampl[j][i] - dc));
			#else			
			block->sampl[j][i] -= dc;
			#endif
		}
	}
	free_ref(ref_values);
}

void intra_pred_4x4_DDL(IntraBlock * block, int mult){

	if (bAllowSkipPred && block->skip_flag) {
		return;
	}

	RefValues * ref_values = create_ref_values(block);
	
	/* KB: 11.05.2018 - for appropriate decoding with splitting process for DDL and VL modes */
	int split_skip = 0;
	if(block->depth){
		int left_border = ( ( block->pos_x % (1 << block->depth) ) == (1 << block->depth) - 1 );
		int not_top_border = ( block->pos_y % (1 << block->depth) )  ;
		int d; 
		int prev_skip_flag = 0;
		for (d = block->depth - 1; d > 0; d--) {
			int prev_left_border = ( ( block->pos_x % (1 << d ) ) == (1 << d) - 1 );
			int prev_not_top_border = ( block->pos_y % (1 << d ) ) ;
			if ( prev_left_border && prev_not_top_border ) {
				prev_skip_flag = 1;
				break;
			}
		}
		split_skip = (  ( (block->pos_x % 2) && (block->pos_y % 2) ) || ( left_border && not_top_border ) || prev_skip_flag ) ? 1 : 0;
	}
	/*******************************************************************************************/

	//GV: requires 2x block size of top reference values
	if ( (ref_values->top_size < 2*block->block_size) || split_skip)
	{
		block->skip_flag = 1;
		return;
	}

	int i, j, l;
	int pred_value;

	/* KB: may be optimized later in amount of calculation operations */
	for (j = 0; j < block->block_size; j++) {
		for (i = 0; i < block->block_size; i++) {
			if (j == block->block_size - 1 &&
					i == block->block_size - 1)
				pred_value = mult * (ref_values->top[i+j] + 3*ref_values->top[i+j+1] + 2  >> 2);
			else 
				pred_value = mult * (ref_values->top[i+j] + 2*ref_values->top[i+j+1] + ref_values->top[i+j+2] + 2 >> 2);
			#ifdef USE_MOD
			block->sampl[j][i] = MOD256(block->sampl[j][i] - pred_value);
			#else			
			block->sampl[j][i] -= pred_value;
			#endif
		}
	}
	free_ref(ref_values);
}

void intra_pred_4x4_DDR(IntraBlock * block, int mult){

	if (bAllowSkipPred && block->skip_flag) {
		return;
	}

	int i, j, l, t;
	RefValues * ref_values = create_ref_values(block);
	
	//GV: requires block size of top reference values,
	//block size of left reference values and a corner
	if (ref_values->top_size < block->block_size || 
			ref_values->left_size < block->block_size ||
			!ref_values->corner)
	{
		block->skip_flag = 1;
		return;
	}

	int16_t ** ref_smpl = malloc0( (block->intra_pred_params->block_size + 1) * sizeof (int16_t *) );
	for (t = 0; t < (block->block_size + 1); t++) {
		ref_smpl[t] = malloc0( (block->block_size + 1) * sizeof(int16_t) );
	}
	ref_smpl[0][0] = *ref_values->corner;
	for (l = 1; l < block->block_size + 1; l++) {
		ref_smpl[0][l] = ref_values->top[l-1];
		ref_smpl[l][0] = ref_values->left[l-1];
	}
		
	for (j = 0; j < block->block_size; j++) {
		for (i = 0; i < block->block_size; i++) {
			#ifdef USE_MOD
			block->sampl[j][i] = MOD256(block->sampl[j][i] - mult * DDR(ref_smpl, i, j));
			#else			
			block->sampl[j][i] -= mult * DDR(ref_smpl, i, j);
			#endif
		}
	}

	for (t = 0; t < (block->block_size + 1); t++) {
		free(ref_smpl[t]);
	}
	free(ref_smpl);
	free_ref(ref_values);
}

void intra_pred_4x4_VR(IntraBlock * block, int mult){

	if (bAllowSkipPred && block->skip_flag) {
		return;
	}

	int i, j, l, t;
	RefValues * ref_values = create_ref_values(block);

	//GV: requires block size of top reference values,
	//block size of left reference values and a corner
	if (ref_values->top_size < block->block_size || 
			ref_values->left_size < block->block_size ||
			!ref_values->corner)
	{
		block->skip_flag = 1;
		return;
	}

	int16_t ** ref_smpl = malloc0((block->intra_pred_params->block_size + 1) * sizeof(int16_t *));
	for (t = 0; t < (block->block_size + 1); t++) {
		ref_smpl[t] = malloc0((block->block_size + 1) * sizeof(int16_t));
	}
	ref_smpl[0][0] = *ref_values->corner;
	for (l = 1; l < block->block_size + 1; l++) {
		ref_smpl[0][l] = ref_values->top[l-1];
		ref_smpl[l][0] = ref_values->left[l-1];
	}

	for (j = 0; j < block->block_size; j++) {
		for (i = 0; i < block->block_size; i++) {
			#ifdef USE_MOD
			block->sampl[j][i] = MOD256(block->sampl[j][i] - mult * VR_HD(ref_smpl, i, j, ( (i << 1) - j)));
			#else			
			block->sampl[j][i] -= mult * VR_HD(ref_smpl, i, j, ( (i << 1) - j));
			#endif
		}
	}

	for (t = 0; t < (block->block_size + 1); t++) {
		free(ref_smpl[t]);
	}
	free(ref_smpl);
	free_ref(ref_values);
}

//KB: the same as 4x4_VR but ref_sampl[y][x] are presented in different order: ref_sampl[x][y]
void intra_pred_4x4_HD(IntraBlock * block, int mult){

	if (bAllowSkipPred && block->skip_flag) {
		return;
	}

	int i, j, l, t;
	RefValues * ref_values = create_ref_values(block);

	//GV: requires block size of top reference values,
	//block size of left reference values and a corner
	if (ref_values->top_size < block->block_size || 
			ref_values->left_size < block->block_size ||
			!ref_values->corner)
	{
		block->skip_flag = 1;
		return;
	}

	int16_t ** ref_smpl = malloc0((block->intra_pred_params->block_size + 1) * sizeof(int16_t *));
	for (t = 0; t < (block->block_size + 1); t++) {
		ref_smpl[t] = malloc0((block->block_size + 1) * sizeof(int16_t));
	}
	ref_smpl[0][0] = *ref_values->corner;
	for (l = 1; l < block->block_size + 1; l++) {
		ref_smpl[l][0] = ref_values->top[l-1];
		ref_smpl[0][l] = ref_values->left[l-1];
	}

	for (j = 0; j < block->block_size; j++) {
		for (i = 0; i < block->block_size; i++) {
			#ifdef USE_MOD
			block->sampl[j][i] = MOD256(block->sampl[j][i] - mult * VR_HD(ref_smpl, i, j, ( (j << 1) - i)));
			#else			
			block->sampl[j][i] -= mult * VR_HD(ref_smpl, j, i, ( (j << 1) - i));
			#endif
		}
	}

	for (t = 0; t < (block->block_size + 1); t++) {
		free(ref_smpl[t]);
	}
	free(ref_smpl);
	free_ref(ref_values);
}

void intra_pred_4x4_VL(IntraBlock * block, int mult){

	if (bAllowSkipPred && block->skip_flag) {
		return;
	}

	RefValues * ref_values = create_ref_values(block);

	/* KB: 11.05.2018 - for appropriate decoding with splitting process for DDL and VL modes */
	int split_skip = 0;
	if(block->depth){
		int left_border = ( ( block->pos_x % (1 << block->depth) ) == (1 << block->depth) - 1 );
		int not_top_border = ( block->pos_y % (1 << block->depth) )  ;
		int d; 
		int prev_skip_flag = 0;
		for (d = block->depth - 1; d > 0; d--) {
			int prev_left_border = ( ( block->pos_x % (1 << d ) ) == (1 << d) - 1 );
			int prev_not_top_border = ( block->pos_y % (1 << d ) ) ;
			if ( prev_left_border && prev_not_top_border ) {
				prev_skip_flag = 1;
				break;
			}
		}
		split_skip = (  ( (block->pos_x % 2) && (block->pos_y % 2) ) || ( left_border && not_top_border ) || prev_skip_flag ) ? 1 : 0;
	}
	/*******************************************************************************************/
	
	//GV: requires 2x block size of top reference values
	if ( (ref_values->top_size < 2*block->block_size) || split_skip)
	{
		block->skip_flag = 1;
		return;
	}

	int i, j, l;
	/* KB: may be optimized later in amount of calculation operations */
	for (j = 0; j < block->block_size; j++) {
		for (i = 0; i < block->block_size; i++) {
			#ifdef USE_MOD
			block->sampl[j][i] = MOD256((block->sampl[j][i] - mult * VL(ref_values->top, i, j, j)));
			#else			
			block->sampl[j][i] -= mult * VL(ref_values->top, i, j, j);
			#endif
		}
	}

	free_ref(ref_values);
}

void intra_pred_4x4_HU(IntraBlock * block, int mult){

	if (bAllowSkipPred && block->skip_flag) {
		return;
	}

	RefValues * ref_values = create_ref_values(block);
	
	//GV: requires block size of left reference values
	if (ref_values->left_size < block->block_size)
	{
		block->skip_flag = 1;
		return;
	}

	int i, j, l;

	int l1 = (block->block_size << 1) - 3;
	int l2 = l1 - 1;
	int size1 = block->block_size - 1;
	int size2 = block->block_size - 2;

	/* KB: may be optimized later in amount of calculation operations */
	for (j = 0; j < block->block_size; j++) {
		for (i = 0; i < block->block_size; i++) {
			#ifdef USE_MOD
			block->sampl[j][i] = MOD256((block->sampl[j][i] - mult * HU(ref_values->left, i, j, (i + (j << 2)), l1, l2, size1, size2)));
			#else			
			block->sampl[j][i] -= mult * HU(ref_values->left, i, j, (i + (j << 2)), l1, l2, size1, size2);
			#endif
		}
	}

	free_ref(ref_values);
}

void intra_pred_4x4_PLANE(IntraBlock * block, int mult){

	if (bAllowSkipPred && block->skip_flag) {
		return;
	}

	int i, j, l, t;
	RefValues * ref_values = create_ref_values(block);

	//GV: requires block size of top reference values and
	//block size of left reference values and corner
	if (ref_values->top_size < block->block_size || 
			ref_values->left_size < block->block_size ||
			!ref_values->corner)
	{
		block->skip_flag = 1;
		return;
	}

	int16_t half = block->block_size>>1; 
	int16_t H = - *ref_values->corner*half,
		V = - *ref_values->corner*half;

	for (l=1; l<=block->block_size; l++)
	{
		H+=(l-half)*ref_values->top[l-1];
		V+=(l-half)*ref_values->left[l-1];
	}

	double A,B,C;
	A = (ref_values->top[block->block_size-1]+ref_values->left[block->block_size-1])/2.0;
	B =  12.0/block->block_size/(block->block_size+1)/(block->block_size+2)*H;
	C =  12.0/block->block_size/(block->block_size+1)/(block->block_size+2)*V;

	for (j = 0; j < block->block_size; j++) {
		for (i = 0; i < block->block_size; i++) {
			#ifdef USE_MOD
			block->sampl[j][i] = MOD256(block->sampl[j][i] - mult * (int16_t)(A + B*(i-half+1) + C*(j-half+1) + .5);
			#else
			block->sampl[j][i] -= mult*(int16_t)(A + B*(i-half+1) + C*(j-half+1) + .5);
			#endif
		}
	}

	free_ref(ref_values);
}

void intra_pred_4x4_ADAPTIVE(IntraBlock * block, int mult){
	int i;
	if (mult == 1) {
		for (i = 0; i < TYPE_NUMBER; i++) {
			block->intra_pred_params->pred_func[i](block, mult);
			if (block->skip_flag) {
				block->RD[i] = __DBL_MAX__;
			} else {
				block->RD[i] = estimate_ib_rd(block);
			}
			block->intra_pred_params->pred_func[i](block, (-mult));
			block->skip_flag = 0;
		}
		double minRD = __DBL_MAX__;
		block->pred_mode = I_PRED_4x4_NONE;
		if (bAllowSkipPred)
			minRD = block->RD0;
		for (i = 0; i < TYPE_NUMBER; i++) {
			if(block->RD[i] < minRD){
				minRD = block->RD[i];
				if (block->intra_pred_params->split_flag){
					block->RD1 = block->RD[i];
				}
				block->pred_mode = i;			
			}
		}
		if (block->pred_mode != I_PRED_4x4_NONE) {
			block->intra_pred_params->pred_func[block->pred_mode](block, mult);
		} else {
			block->skip_flag = 1;
			if (block->intra_pred_params->split_flag){
				block->RD1 = __DBL_MAX__;
			}
		}
	} else {
		if (block->pred_mode != I_PRED_4x4_NONE){
			block->intra_pred_params->pred_func[block->pred_mode](block, mult);
		}
	}
}

//KB: 28.04.18
void intra_pred_split(IntraPredParams * intra_pred_params, int mult){ 
	int i, j, d, y, x;
	switch (intra_pred_params->pred_type){
	default:					//GV: compiled
		if (intra_pred_params->pred_type < 0 || intra_pred_params->pred_type > TYPE_NUMBER)
			break;
		for (j = 0; j < intra_pred_params->vert_blocks; j++) {
			for (i = 0; i < intra_pred_params->horiz_blocks; i++) {
				IntraBlock * block = intra_pred_params->rep_order[j][i];
				if (mult == 1) {
					make_split_ib(block);
				}
				pred_split_ib(block, mult);
			}
		}
		break;
	case TYPE_NUMBER:
		for (j = 0; j < intra_pred_params->vert_blocks; j++) {
			for (i = 0; i < intra_pred_params->horiz_blocks; i++) {
				IntraBlock * block = intra_pred_params->rep_order[j][i];
				if (mult == 1) {
					make_split_ib(block);
				}
				pred_split_ib(block, mult);
			}
		}
		break;
	}
}

/* KB: works in appropriate way only with SAD as RD metric */
void make_split_ib(IntraBlock * block){
	int x, y;
	double RD_root = get_rd_split(block);
	int pos_x = block->pos_x << 1;
	int pos_y = block->pos_y << 1;
	double RD_child = 0;
	//printf("depth = %d, pred_index = %d, pos_y = %d, pos_x = %d \n", block->depth, block->pred_index, block->pos_y, block->pos_x);
	IntraBlock ** child_block = malloc0(4 * sizeof(IntraBlock *));
	for (y = 0; y < 2; y++) {
		for (x = 0; x < 2; x++){
			child_block[x+2*y] = block->intra_pred_params->child_order[block->depth][pos_y + y][pos_x + x];
			RD_child += get_rd_split(child_block[x+2*y]);
		}
	}
	if (RD_child > RD_root){
		block->split_flag = 0;
		free(child_block);
	} else {
		block->split_flag = 1;
		if (block->depth == block->intra_pred_params->split_depth - 1) {
			free(child_block);
			return;
		}
		for (y = 0; y < 2; y++) {
			for (x = 0; x < 2; x++){
				make_split_ib(child_block[x+2*y]);
			}
		}
		free(child_block);
	}
}

void pred_split_ib(IntraBlock * block, int mult){
	if (block->split_flag) {
		int x, y;
		int pos_x = block->pos_x << 1;
		int pos_y = block->pos_y << 1;
		IntraBlock ** child_block = malloc0(4 * sizeof(IntraBlock *));
		for (y = 0; y < 2; y++) {
			for (x = 0; x < 2; x++){
				child_block[x+2*y] = block->intra_pred_params->child_order[block->depth][pos_y + y][pos_x + x];
				pred_split_ib(child_block[x+2*y], mult);
				copy_smpl_ib (block, child_block[x+2*y]);
			}
		}
		if (mult != 1) {
			fill_tmp_block(block);
		}
		free(child_block);

	} else {
		
		if (bAllowSkipPred && block->RD1 < block->RD0)
			block->skip_flag = 1;
		else
			block->intra_pred_params->pred_func[block->intra_pred_params->pred_type](block, (mult));

		if(mult != 1){
			fill_tmp_block(block);
			if (block->depth != block->intra_pred_params->split_depth){
				int diff = block->intra_pred_params->split_depth - block->depth;
				int d, y, x;
				for (d = 0; d < diff; d++) {
					int step = 1 << (d+1);
					int pos_x = block->pos_x << (d+1);
					int pos_y = block->pos_y << (d+1);
					for (y = 0; y < step; y++) {
						for (x = 0; x < step; x++) {
							IntraBlock * child_ib = block->intra_pred_params->child_order[block->depth + d][pos_y + y][pos_x + x];
							copy_smpl_ib(child_ib, block);
							fill_tmp_block(child_ib);
						}
					}
				}
			}
		}
	}
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */





/* ++++++++++++++++++++++++++ KB: filling intra prediction errors array in input array "data" and reference samples of IntraPredParams struct into "ref_sampl" array +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

void fill_pred_subband_data(IntraPredParams * intra_pred_params, short * data, short * ref_sampl){
	int i, j, w, h;

	if (intra_pred_params->top_ref_sampl) {
		for (i = 0; i < intra_pred_params->sub_width; i++) {
			 ref_sampl[i] = intra_pred_params->top_ref_sampl[i];
		}
		if (intra_pred_params->left_ref_sampl) {
			for (j = 0; j < intra_pred_params->sub_height - 1; j++) {
				 ref_sampl[j + intra_pred_params->sub_width] = intra_pred_params->left_ref_sampl[j + 1];
			}
		}
	} else {
		for (j = 0; j < intra_pred_params->sub_height; j++) {
			 ref_sampl[j] = intra_pred_params->left_ref_sampl[j];
		}
	}

	for (j = 0; j < intra_pred_params->vert_blocks - 1; j++) {
		for (i = 0; i < intra_pred_params->horiz_blocks - 1; i++) {
			IntraBlock * block = intra_pred_params->rep_order[j][i];
			for (h = 0; h < intra_pred_params->block_size; h++) {
				for (w = 0; w < intra_pred_params->block_size; w++) {
					 data[(i * intra_pred_params->block_size) + ((j * intra_pred_params->block_size + h) * intra_pred_params->sub_pred_width) + w] = block->sampl[h][w];
				}
			}
		}
		IntraBlock * block = intra_pred_params->rep_order[j][intra_pred_params->horiz_blocks - 1];
		for (h = 0; h < intra_pred_params->block_size; h++) {
			for (w = 0; w < intra_pred_params->block_size - intra_pred_params->right_filling; w++) {
				data[( (intra_pred_params->horiz_blocks - 1) * intra_pred_params->block_size) + ((j * intra_pred_params->block_size + h) * intra_pred_params->sub_pred_width) + w] = block->sampl[h][w];
			}
		}
	}

	for (i = 0; i < intra_pred_params->horiz_blocks - 1; i++) {
		IntraBlock * block = intra_pred_params->rep_order[intra_pred_params->vert_blocks - 1][i];
		for (h = 0; h < intra_pred_params->block_size - intra_pred_params->bottom_filling; h++) {
			for (w = 0; w < intra_pred_params->block_size; w++) {
				data[(i * intra_pred_params->block_size) + (( (intra_pred_params->vert_blocks - 1) * intra_pred_params->block_size + h) * intra_pred_params->sub_pred_width) + w] = block->sampl[h][w];
			}
		}
	}

	IntraBlock * block = intra_pred_params->rep_order[intra_pred_params->vert_blocks - 1][intra_pred_params->horiz_blocks - 1];
	for (h = 0; h < intra_pred_params->block_size - intra_pred_params->bottom_filling; h++) {
		for (w = 0; w < intra_pred_params->block_size - intra_pred_params->right_filling; w++) {
			data[( (intra_pred_params->horiz_blocks - 1) * intra_pred_params->block_size) + (( (intra_pred_params->vert_blocks - 1) * intra_pred_params->block_size + h) * intra_pred_params->sub_pred_width) + w] = block->sampl[h][w];
		}
	}
	
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */




/* ++++++++++++++++++++++++++ KB: The whole process of subband reconstruction after intra prediction +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

void process_inverse_intra_prediction(IntraPredParams * intra_pred_params, short * data, int component, int sub_num, double quant_factor){
	int x, y;

	make_intra_pred(intra_pred_params, 1, quant_factor); // fill_tmp(intra_pred_params) within it

    #ifdef PRINT_TEST_FILES
	show_ib_smpl(intra_pred_params, 2, component, sub_num);

	for (y = 0; y < intra_pred_params->vert_blocks; y++) {
		for (x = 0; x < intra_pred_params->horiz_blocks; x++) {
			show_ib_info(intra_pred_params, x, y, 2, component, sub_num);
		}
	}
   #endif

	fill_recon_subband_data(intra_pred_params, data);

}

void fill_recon_subband_data(IntraPredParams * intra_pred_params, short * data){
	int i, j, w, h;
	//intra_pred_params->data = data;
	/* KB: filling reference left and top samples in subband */
	if (intra_pred_params->top_ref_sampl == NULL) {

		for (j = 0; j < intra_pred_params->sub_height; j++) {
			 data[j * intra_pred_params->sub_width] = intra_pred_params->left_ref_sampl[j];
		}

		for (j = 0; j < intra_pred_params->vert_blocks - 1; j++) {
			for (i = 0; i < intra_pred_params->horiz_blocks - 1; i++) {
				IntraBlock * block = intra_pred_params->rep_order[j][i];
				for (h = 0; h < intra_pred_params->block_size; h++) {
					for (w = 0; w < intra_pred_params->block_size; w++) {
						 data[(i * intra_pred_params->block_size + 1) + ((j * intra_pred_params->block_size + h) * intra_pred_params->sub_width) + w] = block->sampl[h][w];
					}
				}
			}
			IntraBlock * block = intra_pred_params->rep_order[j][intra_pred_params->horiz_blocks - 1];
			for (h = 0; h < intra_pred_params->block_size; h++) {
				for (w = 0; w < intra_pred_params->block_size - intra_pred_params->right_filling; w++) {
					data[( (intra_pred_params->horiz_blocks - 1) * intra_pred_params->block_size + 1) + ((j * intra_pred_params->block_size + h) * intra_pred_params->sub_width) + w] = block->sampl[h][w];
				}
			}
		}

		for (i = 0; i < intra_pred_params->horiz_blocks - 1; i++) {
			IntraBlock * block = intra_pred_params->rep_order[intra_pred_params->vert_blocks - 1][i];
			for (h = 0; h < intra_pred_params->block_size - intra_pred_params->bottom_filling; h++) {
				for (w = 0; w < intra_pred_params->block_size; w++) {
					data[(i * intra_pred_params->block_size + 1) + (( (intra_pred_params->vert_blocks - 1) * intra_pred_params->block_size + h) * intra_pred_params->sub_width) + w] = block->sampl[h][w];
				}
			}
		}

		IntraBlock * block = intra_pred_params->rep_order[intra_pred_params->vert_blocks - 1][intra_pred_params->horiz_blocks - 1];
		for (h = 0; h < intra_pred_params->block_size - intra_pred_params->bottom_filling; h++) {
			for (w = 0; w < intra_pred_params->block_size - intra_pred_params->right_filling; w++) {
				data[( (intra_pred_params->horiz_blocks - 1) * intra_pred_params->block_size + 1) + (( (intra_pred_params->vert_blocks - 1) * intra_pred_params->block_size + h) * intra_pred_params->sub_width) + w] = block->sampl[h][w];
			}
		}

	} else {

		for (i = 0; i < intra_pred_params->sub_width; i++) {
			 data[i] = intra_pred_params->top_ref_sampl[i];
		}

		if (intra_pred_params->left_ref_sampl == NULL) {

			for (j = 0; j < intra_pred_params->vert_blocks - 1; j++) {
				for (i = 0; i < intra_pred_params->horiz_blocks - 1; i++) {
					IntraBlock * block = intra_pred_params->rep_order[j][i];
					for (h = 0; h < intra_pred_params->block_size; h++) {
						for (w = 0; w < intra_pred_params->block_size; w++) {
							 data[(i * intra_pred_params->block_size) + ((j * intra_pred_params->block_size + h + 1) * intra_pred_params->sub_width) + w] = block->sampl[h][w];
						}
					}
				}
				IntraBlock * block = intra_pred_params->rep_order[j][intra_pred_params->horiz_blocks - 1];
				for (h = 0; h < intra_pred_params->block_size; h++) {
					for (w = 0; w < intra_pred_params->block_size - intra_pred_params->right_filling; w++) {
						data[( (intra_pred_params->horiz_blocks - 1) * intra_pred_params->block_size) + ((j * intra_pred_params->block_size + h + 1) * intra_pred_params->sub_width) + w] = block->sampl[h][w];
					}
				}
			}

			for (i = 0; i < intra_pred_params->horiz_blocks - 1; i++) {
				IntraBlock * block = intra_pred_params->rep_order[intra_pred_params->vert_blocks - 1][i];
				for (h = 0; h < intra_pred_params->block_size - intra_pred_params->bottom_filling; h++) {
					for (w = 0; w < intra_pred_params->block_size; w++) {
						data[(i * intra_pred_params->block_size) + (( (intra_pred_params->vert_blocks - 1) * intra_pred_params->block_size + h + 1) * intra_pred_params->sub_width) + w] = block->sampl[h][w];
					}
				}
			}

			IntraBlock * block = intra_pred_params->rep_order[intra_pred_params->vert_blocks - 1][intra_pred_params->horiz_blocks - 1];
			for (h = 0; h < intra_pred_params->block_size - intra_pred_params->bottom_filling; h++) {
				for (w = 0; w < intra_pred_params->block_size - intra_pred_params->right_filling; w++) {
					data[( (intra_pred_params->horiz_blocks - 1) * intra_pred_params->block_size) + (( (intra_pred_params->vert_blocks - 1) * intra_pred_params->block_size + h + 1) * intra_pred_params->sub_width) + w] = block->sampl[h][w];
				}
			}

		} else {

			for (j = 0; j < intra_pred_params->sub_height; j++) {
				 data[j * intra_pred_params->sub_width] = intra_pred_params->left_ref_sampl[j];
			}

			for (j = 0; j < intra_pred_params->vert_blocks - 1; j++) {
				for (i = 0; i < intra_pred_params->horiz_blocks - 1; i++) {
					IntraBlock * block = intra_pred_params->rep_order[j][i];
					for (h = 0; h < intra_pred_params->block_size; h++) {
						for (w = 0; w < intra_pred_params->block_size; w++) {
							 data[(i * intra_pred_params->block_size + 1) + ((j * intra_pred_params->block_size + h + 1) * intra_pred_params->sub_width) + w] = block->sampl[h][w];
						}
					}
				}
				IntraBlock * block = intra_pred_params->rep_order[j][intra_pred_params->horiz_blocks - 1];
				for (h = 0; h < intra_pred_params->block_size; h++) {
					for (w = 0; w < intra_pred_params->block_size - intra_pred_params->right_filling; w++) {
						data[( (intra_pred_params->horiz_blocks - 1) * intra_pred_params->block_size + 1) + ((j * intra_pred_params->block_size + h + 1) * intra_pred_params->sub_width) + w] = block->sampl[h][w];
					}
				}
			}

			for (i = 0; i < intra_pred_params->horiz_blocks - 1; i++) {
				IntraBlock * block = intra_pred_params->rep_order[intra_pred_params->vert_blocks - 1][i];
				for (h = 0; h < intra_pred_params->block_size - intra_pred_params->bottom_filling; h++) {
					for (w = 0; w < intra_pred_params->block_size; w++) {
						data[(i * intra_pred_params->block_size + 1) + (( (intra_pred_params->vert_blocks - 1) * intra_pred_params->block_size + h + 1) * intra_pred_params->sub_width) + w] = block->sampl[h][w];
					}
				}
			}

			IntraBlock * block = intra_pred_params->rep_order[intra_pred_params->vert_blocks - 1][intra_pred_params->horiz_blocks - 1];
			for (h = 0; h < intra_pred_params->block_size - intra_pred_params->bottom_filling; h++) {
				for (w = 0; w < intra_pred_params->block_size - intra_pred_params->right_filling; w++) {
					data[( (intra_pred_params->horiz_blocks - 1) * intra_pred_params->block_size + 1) + (( (intra_pred_params->vert_blocks - 1) * intra_pred_params->block_size + h + 1) * intra_pred_params->sub_width) + w] = block->sampl[h][w];
				}
			}
		}
	}
	
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */












/* ++++++++++++++++++++++++++++++++++++++ KB: Printing subband and intra blocks samples values - for test issues  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */


/* KB: only for test issues (not supported for pred->modes right now) */
void show_ib_smpl(IntraPredParams * intra_pred_params, int stage, int component, int sub_num){
	int i, j, w, h, t;
	FILE * ib_info;
	int comp = CLAMP_MIN(component, 0, 2);
	char * char_comp;
	switch (comp) {
	case 1:
		char_comp = "Cr";
		break;
	case 2:
		char_comp = "Cb";
		break;
	case 0:
	default:
		char_comp = "Y";
		break;
	}
	char FileName[700];
	if (stage == 0) {
		sprintf(FileName, "ib_info_%s_%d_orig.txt", char_comp, sub_num);
		ib_info = fopen( FileName, "w");
	} else if (stage == 1) {
		sprintf(FileName, "ib_info_%s_%d_pred.txt", char_comp, sub_num);
		ib_info = fopen(FileName, "w");
	} else {
		sprintf(FileName, "ib_info_%s_%d_recon.txt", char_comp, sub_num);
		ib_info = fopen(FileName, "w");
	}

	if (intra_pred_params->top_ref_sampl) {

		for (i = 0; i < intra_pred_params->sub_width; i++) {
			fprintf(ib_info, "%d ", intra_pred_params->top_ref_sampl[i]);
		}
		
		if (intra_pred_params->left_ref_sampl) {

			for (j = 0; j < intra_pred_params->sub_pred_height; j++) {
				fprintf(ib_info, "\n%d ", intra_pred_params->left_ref_sampl[j+1]);
				h = j / intra_pred_params->block_size;
				t = j % intra_pred_params->block_size;
				for (w = 0; w < intra_pred_params->horiz_blocks; w++) {
					IntraBlock * block = intra_pred_params->rep_order[h][w];
					for (i = 0; i < ((w == intra_pred_params->horiz_blocks - 1) ? (intra_pred_params->block_size - intra_pred_params->right_filling) : (intra_pred_params->block_size)); i++) {
						fprintf(ib_info, "%d ", block->sampl[t][i]);
					}
				}
			}

			fprintf(ib_info, "\n");

		} else {

			for (j = 0; j < intra_pred_params->sub_pred_height; j++) {
				//fprintf(ib_info, "\n%d ", intra_pred_params->left_ref_sampl[j+1]);
				fprintf(ib_info, "\n");
				h = j / intra_pred_params->block_size;
				t = j % intra_pred_params->block_size;
				for (w = 0; w < intra_pred_params->horiz_blocks; w++) {
					//fprintf(ib_info, "| ");
					IntraBlock * block = intra_pred_params->rep_order[h][w];
					for (i = 0; i < ((w == intra_pred_params->horiz_blocks - 1) ? (intra_pred_params->block_size - intra_pred_params->right_filling) : (intra_pred_params->block_size)); i++) {
						//for (i = 0; i < intra_pred_params->block_size; i++) {
						fprintf(ib_info, "%d ", block->sampl[t][i]);
					}
				}
			}

		}

	} else {

		for (j = 0; j < intra_pred_params->sub_height; j++) {
			fprintf(ib_info, "%d ", intra_pred_params->left_ref_sampl[j]);
			h = j / intra_pred_params->block_size;
			t = j % intra_pred_params->block_size;
			for (w = 0; w < intra_pred_params->horiz_blocks; w++) {
				IntraBlock * block = intra_pred_params->rep_order[h][w];
				for (i = 0; i < ((w == intra_pred_params->horiz_blocks - 1) ? (intra_pred_params->block_size - intra_pred_params->right_filling) : (intra_pred_params->block_size)); i++) {
					fprintf(ib_info, "%d ", block->sampl[t][i]);
				}
			}
			fprintf(ib_info, "\n");
		}

	}

	/*for (j = intra_pred_params->sub_pred_height; j < intra_pred_params->sub_pred_height + intra_pred_params->bottom_filling; j++) {
		fprintf(ib_info, "\n000 ");
		h = j / intra_pred_params->block_size;
		t = j % intra_pred_params->block_size;
		for (w = 0; w < intra_pred_params->horiz_blocks; w++) {
			fprintf(ib_info, "| ");
			IntraBlock * block = intra_pred_params->rep_order[h][w];
			for (i = 0; i < intra_pred_params->block_size; i++) {
				fprintf(ib_info, "%d ", block->sampl[t][i]);
			}
		}
	}*/

	//fprintf(ib_info, "\n");

	fclose(ib_info);
}

/* KB: only for test issues (not supported for pred->modes right now) */
void show_ib_info(IntraPredParams * intra_pred_params, int pos_x, int pos_y, int stage, int component, int sub_num){
	FILE * block_info;
	int i, j;
	int comp = CLAMP_MIN(component, 0, 2);
	char * char_comp;
	switch (comp) {
	case 1:
		char_comp = "Cr";
		break;
	case 2:
		char_comp = "Cb";
		break;
	case 0:
	default:
		char_comp = "Y";
		break;
	}
	char FileName[700];
	if (stage == 0) {
		sprintf(FileName, "block_info[%d][%d]_%s_%d_orig.txt", pos_x, pos_y, char_comp, sub_num);
		block_info = fopen( FileName, "w");
	} else if (stage == 1) {
		sprintf(FileName, "block_info[%d][%d]_%s_%d_pred.txt", pos_x, pos_y, char_comp, sub_num);
		block_info = fopen( FileName, "w");
	} else {
		sprintf(FileName, "block_info[%d][%d]_%s_%d_recon.txt", pos_x, pos_y, char_comp, sub_num);
		block_info = fopen( FileName, "w");
	}
	IntraBlock * block = intra_pred_params->rep_order[pos_y][pos_x];

	fprintf(block_info, "IntraBlock[%d][%d]: pred_index = %d; pred_mode = %d;\n\n", block->pos_x, block->pos_y, block->pred_index, block->pred_mode);
	fprintf(block_info, "Samples:\n");

	for (j = 0; j < intra_pred_params->block_size; j++) {
		for (i = 0; i < intra_pred_params->block_size; i++) {
			fprintf(block_info, "%d ", block->sampl[j][i]);
		}
		fprintf(block_info, "\n");
	}
	
	fprintf(block_info, "\n\nTmp bottom: ");

	for (i = 0; i < intra_pred_params->block_size; i++) {
		fprintf(block_info, "%d ", block->tmp_bottom[i]);
	}
	
	fprintf(block_info, "\n\nTmp right:\n");
	for (j = 0; j < intra_pred_params->block_size; j++) {
		fprintf(block_info, "%d\n", block->tmp_right[j]);
	}

	fclose(block_info);
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */













/* ++++++++++++++++++++++++++++++++++++++ KB: free memory after complition of intra predcition process  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

void free_intra_pred_params(IntraPredParams * intra_pred_params){   
	int i, j, k, d;

	free(intra_pred_params->top_ref_sampl);
	free(intra_pred_params->left_ref_sampl);

	if(intra_pred_params->split_flag){
		free(intra_pred_params->split_blocks);
	}

	for (d = 0; d < intra_pred_params->split_depth; d++) {
		int vert_blocks = intra_pred_params->vert_blocks << (d+1);
        int horiz_blocks = intra_pred_params->horiz_blocks << (d+1);
		int block_size = intra_pred_params->block_size >> (d+1);
		for (j = 0; j < vert_blocks; j++) {
			for (i = 0; i < horiz_blocks; i++) {
				free(intra_pred_params->child_order[d][j][i]->tmp_bottom);
				free(intra_pred_params->child_order[d][j][i]->tmp_right);
				for (k = 0; k < block_size; k++) {
					free(intra_pred_params->child_order[d][j][i]->sampl[k]);
				}
				free(intra_pred_params->child_order[d][j][i]->sampl);
				free(intra_pred_params->child_order[d][j][i]);
			}
		}
		for (i = 0; i < vert_blocks; i++) {
			free(intra_pred_params->child_order[d][i]);
		}
		free(intra_pred_params->child_order[d]);
	}

	for (j = 0; j < intra_pred_params->vert_blocks; j++) {
		for (i = 0; i < intra_pred_params->horiz_blocks; i++) {
			/* free_intra_block(IntraBlock * intra_block) */
			free(intra_pred_params->rep_order[j][i]->tmp_bottom);
			free(intra_pred_params->rep_order[j][i]->tmp_right);
			for (k = 0; k < intra_pred_params->block_size; k++) {
				free(intra_pred_params->rep_order[j][i]->sampl[k]);
			}
			free(intra_pred_params->rep_order[j][i]->sampl);
			free(intra_pred_params->rep_order[j][i]);
		}
	}

	for (i = 0; i < intra_pred_params->vert_blocks; i++) {
		free(intra_pred_params->rep_order[i]);
	}

	free(intra_pred_params->rep_order);
	free(intra_pred_params->pred_order);
	free(intra_pred_params);
}

void free_intra_block(IntraBlock * intra_block) {
	int k;
	free(intra_block->tmp_bottom);
	free(intra_block->tmp_right);
	for (k = 0; k < intra_block->intra_pred_params->block_size; k++) {
		free(intra_block->sampl[k]);
	}
	free(intra_block->sampl);
	free(intra_block);
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

// GV: create RefValues structure
RefValues * create_ref_values(IntraBlock * block)
{
	RefValues * pRV = malloc0(sizeof(RefValues));
	int16_t * top1, * top2, * left, * corner;

	/* KB: 04.05.2018  - for splitting blocks*/
	if (block->depth) {
		
		/* KB: how many splitted IBs are should be skipped because they are fully included into right/bottom fillings */
		int skip_x = block->intra_pred_params->right_filling/block->block_size;
		int skip_y = block->intra_pred_params->bottom_filling/block->block_size;
		/* KB: delta between IB's position and the last block in IntraPredParams across that dimension */
		int shift_x = ((block->intra_pred_params->horiz_blocks << block->depth) - 1) - block->pos_x;
		int shift_y = ((block->intra_pred_params->vert_blocks << block->depth) - 1) - block->pos_y;
		/* KB: All top reference samples are available */
		if ( shift_x > skip_x +1 ) {
			pRV->top_size = 2*block->block_size;
		/* KB: IB is fully in DWT band coeffs but the next IB is on the border */
		} else if ( shift_x == skip_x +1) {
			pRV->top_size = 2*block->block_size - (block->intra_pred_params->right_filling % block->block_size);
		/* KB: IB is on the border of image (partly inlcuded in right filling or precisely on the border with right filling) */
		} else if ( shift_x == skip_x) {
			pRV->top_size = block->block_size - (block->intra_pred_params->right_filling % block->block_size);
		/* KB: IB is fully included into right filling */
		} else {
			pRV->top_size = 0;
		}

		/* KB: for solving bug with  */
		switch(block->pred_mode)
		{
			case 0:
			case 2:
			case 4:
			case 5:
			case 9:
				if (pRV->top_size > block->block_size){
					pRV->top_size = block->block_size;
				}
				break;
			default:
				break;
		}
		/***********************************/
		/* KB: the same for bottom filling */
		if ( shift_y > skip_y ) {
			pRV->left_size = block->block_size;
		} else if ( shift_y == skip_y) {
			pRV->left_size = block->block_size - (block->intra_pred_params->bottom_filling % block->block_size);
		} else {
			pRV->left_size = 0;
		}

		//setting left
		if (block->pos_x == 0){
			if (block->intra_pred_params->left_ref_sampl){
				left = block->intra_pred_params->left_ref_sampl + block->pos_y * block->block_size + !!(block->intra_pred_params->top_ref_sampl);
			} else {
				pRV->left_size = 0;
			}
		} else {
			left = block->intra_pred_params->child_order[block->depth - 1][block->pos_y][block->pos_x-1]->tmp_right;
		}
		if (pRV->left_size){
			pRV->left = calloc(pRV->left_size,sizeof(int16_t));
			memcpy(pRV->left,left,pRV->left_size*sizeof(int16_t));
		}

		//setting top
		if (block->pos_y == 0){
			if (block->intra_pred_params->top_ref_sampl){
				top1 = block->intra_pred_params->top_ref_sampl + block->pos_x * block->block_size + !!(block->intra_pred_params->left_ref_sampl);
			} else {
				pRV->top_size = 0;
			}
		} else {
			if (pRV->top_size <= block->block_size) {
				top1 = block->intra_pred_params->child_order[block->depth - 1][block->pos_y-1][block->pos_x]->tmp_bottom;
			} else {
				top1 = block->intra_pred_params->child_order[block->depth - 1][block->pos_y-1][block->pos_x]->tmp_bottom;
				top2 = block->intra_pred_params->child_order[block->depth - 1][block->pos_y-1][block->pos_x+1]->tmp_bottom;
			}
		}	

		pRV->top = calloc(pRV->top_size, sizeof(int16_t));
		if (!pRV->top_size){
			;
		} else if (block->pos_y==0 || pRV->top_size <= block->block_size) {
			memcpy(pRV->top, top1, pRV->top_size*sizeof(int16_t));
		} else {
			memcpy(pRV->top, top1, block->block_size*sizeof(int16_t));
			memcpy(pRV->top + block->block_size, top2, (pRV->top_size - block->block_size)*sizeof(int16_t));
		}

		//setting corner
		if (block->pos_y == 0){
			if (!block->intra_pred_params->top_ref_sampl){
				pRV->corner = NULL;
			} else if (block->pos_x == 0 && !block->intra_pred_params->left_ref_sampl) {
				pRV->corner = NULL;
			} else {
				pRV->corner = malloc(sizeof(int16_t));
				*pRV->corner = block->intra_pred_params->top_ref_sampl[block->pos_x * block->block_size + !!(block->intra_pred_params->left_ref_sampl)-1];
			}
		} else if (block->pos_x == 0) {
			if (!block->intra_pred_params->left_ref_sampl){
				pRV->corner = NULL;
			} else {
				pRV->corner = malloc(sizeof(int16_t));
				*pRV->corner = block->intra_pred_params->left_ref_sampl[block->pos_y * block->block_size + !!(block->intra_pred_params->top_ref_sampl)-1];
			}
		} else {
			pRV->corner = malloc(sizeof(int16_t));
			*pRV->corner = block->intra_pred_params->child_order[block->depth - 1][block->pos_y-1][block->pos_x-1]->tmp_bottom[block->block_size-1];
		}

	} else {

    	   	if (block->pos_x == block->intra_pred_params->horiz_blocks-1)
			pRV->top_size = block->block_size - block->intra_pred_params->right_filling;
		else if (block->pos_x == block->intra_pred_params->horiz_blocks-2)
			pRV->top_size = 2*block->block_size - block->intra_pred_params->right_filling;
		else
			pRV->top_size = 2*block->block_size;
	
		if (block->pos_y == block->intra_pred_params->vert_blocks-1)
			pRV->left_size = block->block_size - block->intra_pred_params->bottom_filling;
		else 
			pRV->left_size = block->block_size;
	
		//setting left
		if (block->pos_x == 0)
		{
			if (block->intra_pred_params->left_ref_sampl)
				left = block->intra_pred_params->left_ref_sampl + block->pos_y * block->block_size + !!(block->intra_pred_params->top_ref_sampl);
			else
				pRV->left_size = 0;
		}
		else
			left = block->intra_pred_params->rep_order[block->pos_y][block->pos_x-1]->tmp_right;
		if (pRV->left_size)
		{
			pRV->left = calloc(pRV->left_size,sizeof(int16_t));
			memcpy(pRV->left,left,pRV->left_size*sizeof(int16_t));
		}
	
		//setting top
		if (block->pos_y == 0)
		{
			if (block->intra_pred_params->top_ref_sampl)
				top1 = block->intra_pred_params->top_ref_sampl + block->pos_x * block->block_size + !!(block->intra_pred_params->left_ref_sampl);
			else
				pRV->top_size = 0;
		}
		else
		{
			if (pRV->top_size <= block->block_size)
				top1 = block->intra_pred_params->rep_order[block->pos_y-1][block->pos_x]->tmp_bottom;
			else
			{
				top1 = block->intra_pred_params->rep_order[block->pos_y-1][block->pos_x]->tmp_bottom;
				top2 = block->intra_pred_params->rep_order[block->pos_y-1][block->pos_x+1]->tmp_bottom;
			}
		}
	
		pRV->top = calloc(pRV->top_size, sizeof(int16_t));
		if (!pRV->top_size)
			;
		else if (block->pos_y==0 || pRV->top_size <= block->block_size)
			memcpy(pRV->top, top1, pRV->top_size*sizeof(int16_t));
		else
		{
			memcpy(pRV->top, top1, block->block_size*sizeof(int16_t));
			memcpy(pRV->top + block->block_size, top2, (pRV->top_size - block->block_size)*sizeof(int16_t));
		}
	
		//setting corner
		if (block->pos_y == 0)
		{
			if (!block->intra_pred_params->top_ref_sampl)
				pRV->corner = NULL;
			else if (block->pos_x == 0 && !block->intra_pred_params->left_ref_sampl)
				pRV->corner = NULL;
			else
			{
				pRV->corner = malloc(sizeof(int16_t));
				*pRV->corner = block->intra_pred_params->top_ref_sampl[block->pos_x * block->block_size + !!(block->intra_pred_params->left_ref_sampl)-1];
			}
		}
		else if (block->pos_x == 0)
		{
			if (!block->intra_pred_params->left_ref_sampl)
				pRV->corner = NULL;
			else
			{
				pRV->corner = malloc(sizeof(int16_t));
				*pRV->corner = block->intra_pred_params->left_ref_sampl[block->pos_y * block->block_size + !!(block->intra_pred_params->top_ref_sampl)-1];
			}
		}
		else
		{
			pRV->corner = malloc(sizeof(int16_t));
			*pRV->corner = block->intra_pred_params->rep_order[block->pos_y-1][block->pos_x-1]->tmp_bottom[block->block_size-1];
		}
	
	}

	if (bFiltering) {
		filter_ref_values(pRV);
	}

	return pRV;
}

// GV: Filtering for 8x8 prediction
void filter_ref_values(RefValues * ref_values)
{
	int16_t * new_top, * new_left, * new_corner;
	int i;
	if (ref_values->top)
	{
		new_top = calloc(ref_values->top_size,sizeof(int16_t));
		
		if (ref_values->corner && ref_values->top_size>1)
			new_top[0]=(*ref_values->corner+2*ref_values->top[0]+ref_values->top[1]+2)>>2;
		else if (ref_values->corner)
			new_top[0]=(*ref_values->corner+3*ref_values->top[0]+2)>>2;
		else if (ref_values->top_size>1)
			new_top[0]=(3*ref_values->top[0]+ref_values->top[1]+2)>>2;
		else
			new_top[0]=ref_values->top[0];
			
		for (i=1; i<ref_values->top_size-1; i++)
			new_top[i]=(ref_values->top[i-1]+2*ref_values->top[i]+ref_values->top[i+1]+2)>>2;
		
		new_top[i]=(ref_values->top[i-1]+3*ref_values->top[i]+2)>>2;
	}

	if (ref_values->left)
	{
		new_left = calloc(ref_values->left_size,sizeof(int16_t));
		
		if (ref_values->corner && ref_values->left_size>1)
			new_left[0]=(*ref_values->corner+2*ref_values->left[0]+ref_values->left[1]+2)>>2;
		else if (ref_values->corner)
			new_left[0]=(*ref_values->corner+3*ref_values->left[0]+2)>>2;
		else if (ref_values->left_size>1)
			new_left[0]=(3*ref_values->left[0]+ref_values->left[1]+2)>>2;
		else
			new_left[0]=ref_values->left[0];
			
		for (i=1; i<ref_values->left_size-1; i++)
			new_left[i]=(ref_values->left[i-1]+2*ref_values->left[i]+ref_values->left[i+1]+2)>>2;
		
		new_left[i]=(ref_values->left[i-1]+3*ref_values->left[i]+2)>>2;
	}
	if (ref_values->corner)
	{
		if (ref_values->left && ref_values->top)
			*ref_values->corner = (*ref_values->left + 2* *ref_values->corner + *ref_values->top + 2) >> 2;
		else if (ref_values->left)
			*ref_values->corner = (*ref_values->left + 3* *ref_values->corner + 2) >> 2;
		else if (ref_values->top)
			*ref_values->corner = (3* *ref_values->corner + *ref_values->top + 2) >> 2;
		else
		       ;	
	}
	
	if (ref_values->top)
	{
		free(ref_values->top);
		ref_values->top = new_top;
	}
	if (ref_values->left)
	{
		free(ref_values->left);
		ref_values->left = new_left;
	}
}

// GV: Free memory from reference values
void free_ref(RefValues * ref_values)
{
	free(ref_values->top);
	free(ref_values->left);
	free(ref_values->corner);
}

// GV: concatenate prediction data and reference values
void concatenate_pred_ref(IntraPredParams * intra_pred_params, short * pred_data, short * ref_sampl, short * conc_data, int force_zero_ref)
{
	int i,j;
	int sub_height = intra_pred_params->sub_height;
	int sub_width = intra_pred_params->sub_width;
	int sub_pred_height = intra_pred_params->sub_pred_height;
	int sub_pred_width = intra_pred_params->sub_pred_width;
	if (intra_pred_params->top_ref_sampl) {
		for (i = 0; i < sub_width; i++)
			conc_data[i] = force_zero_ref?0:ref_sampl[i];
		if (intra_pred_params->left_ref_sampl)
			for (j = 1; j < sub_height; j++)
				conc_data[j*sub_width] = force_zero_ref?0:ref_sampl[sub_width+j-1];
	}
	else {
		for (j = 0; j < sub_height; j++)
			conc_data[j*sub_width] = force_zero_ref?0:ref_sampl[j];
	}
	
	int shiftH = !!intra_pred_params->top_ref_sampl;
	int shiftW = !!intra_pred_params->left_ref_sampl;
	for (j = 0; j < sub_pred_height; j++)
		for (i = 0; i < sub_pred_width; i++)
			conc_data[(j+shiftH)*sub_width+(i+shiftW)]=pred_data[j*sub_pred_width+i];
}

IntraBlock * copy_intra_block(IntraBlock * block)
{
	IntraBlock * new_intra_block = malloc0(sizeof(IntraBlock));
	memcpy(new_intra_block, block, sizeof(IntraBlock));

	int16_t ** sampl_ptr = malloc0(block->block_size * sizeof(int16_t *));
	new_intra_block->sampl = sampl_ptr;

	int i;
	for (i = 0; i < block->block_size; i++) {
		sampl_ptr[i] = malloc0(block->block_size * sizeof(int16_t));
		memcpy(sampl_ptr[i], block->sampl[i], block->block_size*sizeof(int16_t));
	}

	new_intra_block->tmp_bottom = malloc0(new_intra_block->block_size * sizeof(int16_t));
	new_intra_block->tmp_right = malloc0(new_intra_block->block_size * sizeof(int16_t));
//	memcpy(new_intra_block->tmp_bottom, intra_block->tmp_bottom, intra_block->block_size * sizeof(int16_t))
//	memcpy(new_intra_block->tmp_right, intra_block->tmp_right, intra_block->block_size * sizeof(int16_t))

	return new_intra_block;
}

void quantize_block(IntraBlock * block, double quant_factor)
{
	//printf ("up\n");
	int i, j;
	for (j = 0; j < block->block_size; j++) {
		for (i = 0; i < block->block_size; i++) {
			block->sampl[j][i] *= quant_factor;
		}
	}
}
		
void quantize_ref_samples(IntraPredParams * intra_pred_params, double quant_factor)
{
	int i;
	if (intra_pred_params->top_ref_sampl)
		for (i = 0; i < intra_pred_params->sub_width; i++) {
			intra_pred_params->top_ref_sampl[i] *= quant_factor;
		}
	if (intra_pred_params->left_ref_sampl)
		for (i = 0; i < intra_pred_params->sub_height; i++) {
			intra_pred_params->left_ref_sampl[i] *= quant_factor;
		}
}

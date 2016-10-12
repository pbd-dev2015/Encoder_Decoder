#ifndef JPEG_H
#define JPEG_H

/* This file handles Forward Discrete Cosine Transform (FDCT) and Inverse Discrete Cosine Transform (IDCT) 
 */

#include "endec_params.h"
#include "jpeg_tables.h"
#include "math.h"
#include "stdio.h"

typedef struct {
	unsigned char write_byte;
	int write_bits_available;
} WriteByteStruct;

void quantize_fdct(int fdct[][8], int QTable[][8]);

//Forward DCT, quantization, entropy_encoding of all 8x8 sized macroblocks.
int jpeg_encode(struct endec_params *pEndecParams);

int jpeg_encode_interleaved(struct endec_params *pEndecParams);

//Handles Forward Discrete Cosine Transform which is used during encoding JPEG.
void jpeg_forward_dct(int macroblock [][8], int fdct[][8]);

int encode_and_write_macroblock(int fdct_y[][8], int macroblock_type, int *prev_DC, WriteByteStruct *pWBS, FILE *fp);

void find_coefficient_code(int num, Code_T *c);

void write_coefficient_to_file(Code_T *c, WriteByteStruct *pWBS, FILE *fp);
#endif

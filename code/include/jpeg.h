#ifndef JPEG_H
#define JPEG_H

/* This file handles Forward Discrete Cosine Transform (FDCT) and Inverse Discrete Cosine Transform (IDCT) 
 */

#include "endec_params.h"

//Forward DCT, quantization, entropy_encoding of all 8x8 sized macroblocks.
int jpeg_encode(struct endec_params *pEndecParams);

//Handles Forward Discrete Cosine Transform which is used during encoding JPEG.
void jpeg_forward_dct(int macroblock [][8], int fdct[][8]);

#endif

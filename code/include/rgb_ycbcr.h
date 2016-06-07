#ifndef RGB_YCBCR_H
#define RGB_YCBCR_H

#include "endec_params.h"

/* This function takes in the files for the rgb channels and converts every pixel to YCbCr. The YCbCr channels are stored in the corresponding files
 * The function assumes all channels are of same length and width. Maybe call a sanity checker before calling this function.
 * Values of Y, Cb and Cr are all in the range [-128,127] after this function.
 * No subsampling is done in this function. 
 * Returns 0 on successful completion of function. -1 on error.
 */
int rgb_to_ycbcr(const char* r_filename, const char* g_filename, const char* b_filename, const char* y_filename, const char* cb_filename, const char* cr_filename);


/* This function will take in the cb and cr files that were created using rgb_to_ycbcr and then subsample them.
 * Write the subsampled data in new files. Copy those filenames to pEndecParams
 * Function returns 0 on success and -1 on error.
 */
int subsampleCbCr(const char* cb_filename, const char* cr_filename, struct endec_params* pEndecParams);
#endif

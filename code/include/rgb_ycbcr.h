#ifndef RGB_YCBCR_H
#define RGB_YCBCR_H

/* This function takes in the files for the rgb channels and converts every pixel to yuv. The yuv channels are stored in the corresponding files
*  The function assumes all channels are of same length and width. Maybe call a sanity checker before calling this function.
*/
void rgb_to_ycbcr(const char* r_filename, const char* g_filename, const char* b_filename, const char* y_filename, const char* cb_filename, const char* cr_filename);

#endif

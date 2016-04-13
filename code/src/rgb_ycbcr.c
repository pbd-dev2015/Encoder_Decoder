#include "rgb_ycbcr.h"
#include "stdio.h"

void rgb_to_ycbcr(const char* r_filename, const char* g_filename, const char* b_filename, const char* y_filename, const char* cb_filename, const char* cr_filename)
{
	//we will open the files and then just go through all the channels at once. If any channel ends, we stop the whole conversion. We hope for our sanity that all channels are of same length and width
	FILE *fp_red, *fp_grn, *fp_blu, *fp_y, *fp_cb, *fp_cr;
	fp_red = fopen(r_filename, "r");
	if( NULL == fp_red ) { printf("Error opening the red channel\n"); exit(0); }
	fp_grn = fopen(g_filename, "r");
	if( NULL == fp_grn ) { printf("Error opening the green channel\n"); exit(0); }
	fp_blu = fopen(b_filename, "r");
	if( NULL == fp_blu ) { printf("Error opening the blue channel\n"); exit(0); }

}

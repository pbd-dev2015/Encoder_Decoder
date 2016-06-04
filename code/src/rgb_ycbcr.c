#include "rgb_ycbcr.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define TXT_VALUES 0

void rgb_to_ycbcr(const char* r_filename, const char* g_filename, const char* b_filename, const char* y_filename, const char* cb_filename, const char* cr_filename)
{
	//can be called like this : rgb_to_ycbcr("lena.r", "len.grn", "lena.blu", "lena.y", "lena.cb", "lena.cr");
	//we will open the files and then just go through all the channels at once. If any channel ends, we stop the whole conversion. We hope for our sanity that all channels are of same length and width
	FILE *fp_red, *fp_grn, *fp_blu, *fp_y, *fp_cb, *fp_cr;
	fp_red = fopen(r_filename, "r");
	if( NULL == fp_red ) { printf("In file %s:%d:%s: Error opening the red channel\n",__FILE__,__LINE__,__FUNCTION__); exit(0); }
	fp_grn = fopen(g_filename, "r");
	if( NULL == fp_grn ) { printf("In file %s:%d:%s: Error opening the green channel\n",__FILE__,__LINE__,__FUNCTION__); exit(0); }
	fp_blu = fopen(b_filename, "r");
	if( NULL == fp_blu ) { printf("In file %s:%d:%s: Error opening the blue channel\n",__FILE__,__LINE__,__FUNCTION__); exit(0); }
	
	//truncate files if already present. create if not.
	fp_y = fopen(y_filename, "w+");
	if( NULL == fp_y  ) { printf("In file %s:%d:%s: Error opening the Y channel for writing\n",__FILE__,__LINE__,__FUNCTION__); exit(0); }
	fp_cb = fopen(cb_filename, "w+");
	if( NULL == fp_cb ) { printf("In file %s:%d:%s: Error opening the Cb channel for writing\n",__FILE__,__LINE__,__FUNCTION__); exit(0); }
	fp_cr = fopen(cr_filename, "w+");
	if( NULL == fp_cr ) { printf("In file %s:%d:%s: Error opening the Cr channel for writing\n",__FILE__,__LINE__,__FUNCTION__); exit(0); }

	int r = 0, g = 0, b = 0, y = 0, cb = 0, cr = 0;
	float t_y = 0, t_cb = 0, t_cr = 0;

#if TXT_VALUES
        //just for verifiying text.
        FILE *fp_y_txt, *fp_cb_txt, *fp_cr_txt;
        char *y_txt  = (char *)malloc(strlen(y_filename) + 4);  //for .txt
        strcpy(y_txt, y_filename);strcat(y_txt,".txt");
        char *cb_txt = (char *)malloc(strlen(cb_filename) + 4); //for .txt
        strcpy(cb_txt, cb_filename);strcat(cb_txt,".txt");
        char *cr_txt = (char *)malloc(strlen(cr_filename) + 4); //for .txt
        strcpy(cr_txt, cr_filename);strcat(cr_txt,".txt");
        fp_y_txt = fopen(y_txt, "w+"); fp_cb_txt = fopen(cb_txt, "w+"); fp_cr_txt = fopen(cr_txt, "w+");
#endif

	while(1)
	{
		if(feof(fp_red) || feof(fp_grn) || feof(fp_blu))
		{
			//if end of file of any one of the files is reached, then shut off the conversion.
			//According to our assumptions, this should happen when all channels are exhausted at the same time.
			fclose(fp_red); fclose(fp_grn); fclose(fp_blu);
			fclose(fp_y); fclose(fp_cb); fclose(fp_cr);
			#if TXT_VALUES
    			fclose(fp_y_txt); fclose(fp_cb_txt); fclose(fp_cr_txt);
			#endif
			return;
		}
		fread(&r, 1, 1, fp_red);
		fread(&g, 1, 1, fp_grn);
		fread(&b, 1, 1, fp_blu);

		//RGB to YCbCr conversion : https://msdn.microsoft.com/en-us/library/ff635643.aspx
		t_y  =  0.299 * r + 0.587 * g + 0.144 * b;
		t_cb = -0.159 * r - 0.332 * g + 0.050 * b;
		t_cr =  0.500 * r - 0.419 * g - 0.081 * b;

		y  = (int) t_y;	y -= 128; //level shift down the y component. This makes it symmetrical around 0 and thus in the range [-128,127]. This is good for DCT since cosine goes from -1 to 1
		cb = (int) t_cb;	//Already in the range [-128, 127]
		cr = (int) t_cr;	//Already in the range [-128, 127]

		fwrite(&y , 1, 1, fp_y);
		fwrite(&cb, 1, 1, fp_cb);
		fwrite(&cr, 1, 1, fp_cr);

#if TXT_VALUES
                fprintf(fp_y_txt, "%d ", y);
                fprintf(fp_cb_txt, "%d ", cb);
                fprintf(fp_cr_txt, "%d ", cr);
#endif
	}
}

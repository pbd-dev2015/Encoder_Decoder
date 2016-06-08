#include "rgb_ycbcr.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define TXT_VALUES 0	//it also creates text file with y cb cr values in decimal. [-128,127]

int rgb_to_ycbcr(const char* r_filename, const char* g_filename, const char* b_filename, const char* y_filename, const char* cb_filename, const char* cr_filename)
{
	//can be called like this : rgb_to_ycbcr("lena.r", "len.grn", "lena.blu", "lena.y", "lena.cb", "lena.cr");
	//we will open the files and then just go through all the channels at once. If any channel ends, we stop the whole conversion. We hope for our sanity that all channels are of same length and width
	FILE *fp_red, *fp_grn, *fp_blu, *fp_y, *fp_cb, *fp_cr;
	fp_red = fopen(r_filename, "r");
	if( NULL == fp_red ) { fprintf(stderr, "In file %s:%d:%s: Error opening the red channel\n",__FILE__,__LINE__,__FUNCTION__); goto l1; }
	fp_grn = fopen(g_filename, "r");
	if( NULL == fp_grn ) { fprintf(stderr, "In file %s:%d:%s: Error opening the green channel\n",__FILE__,__LINE__,__FUNCTION__); goto l2; }
	fp_blu = fopen(b_filename, "r");
	if( NULL == fp_blu ) { fprintf(stderr, "In file %s:%d:%s: Error opening the blue channel\n",__FILE__,__LINE__,__FUNCTION__); goto l3; }
	
	//truncate files if already present. create if not.
	fp_y = fopen(y_filename, "w+");
	if( NULL == fp_y  ) { fprintf(stderr, "In file %s:%d:%s: Error opening the Y channel for writing\n",__FILE__,__LINE__,__FUNCTION__); goto l4; }
	fp_cb = fopen(cb_filename, "w+");
	if( NULL == fp_cb ) { fprintf(stderr, "In file %s:%d:%s: Error opening the Cb channel for writing\n",__FILE__,__LINE__,__FUNCTION__); goto l5; }
	fp_cr = fopen(cr_filename, "w+");
	if( NULL == fp_cr ) { fprintf(stderr, "In file %s:%d:%s: Error opening the Cr channel for writing\n",__FILE__,__LINE__,__FUNCTION__); goto l6; }

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
			free(y_txt); free(cb_txt); free(cr_txt);
    			fclose(fp_y_txt); fclose(fp_cb_txt); fclose(fp_cr_txt);
			#endif
			return 0;	//this is the good exit.
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

	l6: fclose(fp_cb);
	l5: fclose(fp_y);
	l4: fclose(fp_blu);
	l3: fclose(fp_grn);
	l2: fclose(fp_red);
	l1: return -1;	//exit on error.
}

/* This function will take in the cb and cr files that were created using rgb_to_ycbcr and then subsample them.
 * Write the subsampled data in new files. Copy those filenames to pEndecParams
 * Function returns 0 on success and -1 on error.
 * Resources:
 * 1. https://en.wikipedia.org/wiki/Chroma_subsampling
 * 2. http://dougkerr.net/pumpkin/articles/Subsampling.pdf : Chrominance Subsampling in Digital Images - Douglas Kerr - Best
 * 3. http://www.poynton.com/PDFs/Chroma_subsampling_notation.pdf : Chroma Subsampling Notation - Charles Poynton
 * Important point to take away: In JPEG 420, there is centered alignment of (implied) chrominance pixel. The chrominance
 * pixel embrances a number of actual image pixels and thus is average of several chroma samples as per 2 - Douglas Kerr. 
 * The center of the chroma pixel doesn't spatially coincide with any original chroma sample.
 * In MPEG2 420, there is cosited alignment of (implied) chrominance pixel which means that the chrominance pixel coincides 
 * with one original chroma sample. I am not sure if this means average of chroma values is not taken. 
 * 
 * Problem : Whether average of chroma samples is taken or not.
 * I am inclined to think that average is taken. The reasoning is as follows.
 * I am inclined to think that average of chroma values is taken in both alignments, Centered and Cosited. The alignment
 * decides which samples to take the average of for a chroma pixel. Thus the alignment decides the scope of the chroma pixel.
 * Douglas Kerr mentions average for centered alignment but mentions nothing for cosited alignment.
 * Wikipedia says this : https://en.wikipedia.org/wiki/Chroma_subsampling#Compatibility_issues
 * The details of chroma subsampling implementation cause considerable confusion. Is the upper leftmost chroma value stored, 
 * or the rightmost, or is it the average of all the chroma values? This must be exactly specified in standards and followed 
 * by all implementors.
 */
int subsampleCbCr(const char* cb_filename, const char* cr_filename, struct endec_params* pEndecParams)
{
	int retval = -1;
	//Set the correct filenames in pEndecParams
	strcpy(pEndecParams->subsampledCb_filename, cb_filename);
	strcpy(pEndecParams->subsampledCr_filename, cr_filename);
	if(YUV422 == pEndecParams->subsampling){
		strcat(pEndecParams->subsampledCb_filename, ".YUV422");
		strcat(pEndecParams->subsampledCr_filename, ".YUV422");
	}
	else if(YUV420 == pEndecParams->subsampling){
		strcat(pEndecParams->subsampledCb_filename, ".YUV420");
		strcat(pEndecParams->subsampledCr_filename, ".YUV420");
	}

	FILE *fp_cb, *fp_cr, *fp_cb_subsampled, *fp_cr_subsampled;
	fp_cb = fopen(cb_filename, "r");
	if(NULL == fp_cb) {fprintf(stderr, "In file %s:%d:%s: Error opening Cb channel for subsampling\n",__FILE__,__LINE__,__FUNCTION__); goto l1;}
	fp_cr = fopen(cr_filename, "r");
	if(NULL == fp_cr) {fprintf(stderr, "In file %s:%d:%s: Error opening Cr channel for subsampling\n",__FILE__,__LINE__,__FUNCTION__); goto l2;}
	fp_cb_subsampled = fopen(pEndecParams->subsampledCb_filename, "w+");
	if(NULL == fp_cb_subsampled) {fprintf(stderr, "In file %s:%d:%s: Error opening file for writing subsampled Cb\n", __FILE__,__LINE__,__FUNCTION__); goto l3;}
	fp_cr_subsampled = fopen(pEndecParams->subsampledCr_filename, "w+");
	if(NULL == fp_cr_subsampled) {fprintf(stderr, "In file %s:%d:%s: Error opening file for writing subsampled Cr\n", __FILE__,__LINE__,__FUNCTION__); goto l4;}

	int row_increment = 0, column_increment = 0;
	FILE *fp_cb_2 = NULL, *fp_cr_2 = NULL;	//for 420. points to next row. we take average of the two values.
	if(YUV422 == pEndecParams->subsampling)
	{
		row_increment = 1;
		column_increment = 2;
	}
	else if(YUV420 == pEndecParams->subsampling)
	{
		row_increment = 2;
		column_increment = 2;
		fp_cb_2 = fopen(cb_filename, "r");
		if(NULL == fp_cb_2) {fprintf(stderr, "In file %s:%d:%s: Error opening Cb channel for subsampling\n",__FILE__,__LINE__,__FUNCTION__); goto l4;}
		fseek(fp_cb_2, pEndecParams->width, SEEK_SET);//goto next row
		if(feof(fp_cb_2))
		{
			rewind(fp_cb_2);
		}

		fp_cr_2 = fopen(cr_filename, "r");
		if(NULL == fp_cr_2) {fprintf(stderr, "In file %s:%d:%s: Error opening Cr channel for subsampling\n",__FILE__,__LINE__,__FUNCTION__); fclose(fp_cb_2); goto l4;}
		fseek(fp_cr_2, pEndecParams->width, SEEK_SET);//goto next row
		if(feof(fp_cr_2))
		{
			rewind(fp_cr_2);
		}
	}

	int i = 0, j = 0, num_rows = 0, num_columns = 0;
	num_rows    = pEndecParams->height;
	num_columns = pEndecParams->width;
	int cb_values[2][2], cr_values[2][2];//values on row and column.
	int average_cb_value = 0, average_cr_value = 0;	//the average of the cb_values that we are going to write to file.
	
	for(i = 1; i <= num_rows; i += row_increment)
	{
		//at this point, the file pointers are in valid positions.
		for(j = 1; j <= num_columns; j += column_increment)
		{
			//read two samples (j and j+1) in this row i.
			//read first sample j on row i.
			fread(&cb_values[0][0], 1, 1, fp_cb);
			fread(&cr_values[0][0], 1, 1, fp_cr);

			//read second sample j+1 on row i.
			if(j+1 <= num_columns)
			{
				fread(&cb_values[0][1], 1, 1, fp_cb);
				fread(&cr_values[0][1], 1, 1, fp_cr);
			}
			else
			{
				//col j+1 doesn't exist as num_columns is odd.
				cb_values[0][1] = cb_values[0][0];
				cr_values[0][1] = cr_values[0][0];
			}

			//for YUV420, read two samples (j and j+1) in row i+1.
			if(YUV420 == pEndecParams->subsampling)
			{
				if(i+1 <= num_rows)
				{
					//read first sample j on row i+1.
					fread(&cb_values[1][0], 1, 1, fp_cb_2);
					fread(&cr_values[1][0], 1, 1, fp_cr_2);

					//read second sample j+1 on row i+1.
					if(j+1 <= num_columns)
					{
						fread(&cb_values[1][1], 1, 1, fp_cb_2);
						fread(&cr_values[1][1], 1, 1, fp_cr_2);
					}
					else
					{
						//col j+1 doesn't exist as num_columns is odd.
						cb_values[1][1] = cb_values[0][0];
						cr_values[1][1] = cr_values[0][0];
					}
				}
				else
				{
					//copy values from row i as row i+1 doesn't exist. Which means num_rows is odd.
					cb_values[1][0] = cb_values[0][0];
					cb_values[1][1] = cb_values[0][1];

					cr_values[1][0] = cr_values[0][0];
					cr_values[1][1] = cr_values[0][1];
				}
			}

			//write the average (subsampled value) to the file.
			if(YUV422 == pEndecParams->subsampling)
			{
				average_cb_value = (cb_values[0][0] + cb_values[0][1]) / 2;
				average_cr_value = (cr_values[0][0] + cr_values[0][1]) / 2;
			}
			else if(YUV420 == pEndecParams->subsampling)
			{
				average_cb_value = (cb_values[0][0] + cb_values[0][1] + cb_values[1][0] + cb_values[1][1]) / 4;
				average_cr_value = (cr_values[0][0] + cr_values[0][1] + cr_values[1][0] + cr_values[1][1]) / 4;
			}
			fwrite(&average_cb_value, 1, 1, fp_cb_subsampled);
			fwrite(&average_cr_value, 1, 1, fp_cr_subsampled);
		}
		if(YUV420 == pEndecParams->subsampling)
		{
			//never mind EOF
			fseek(fp_cb, num_columns, SEEK_CUR);
			fseek(fp_cb_2, num_columns, SEEK_CUR);	
			
			fseek(fp_cr, num_columns, SEEK_CUR);
			fseek(fp_cr_2, num_columns, SEEK_CUR);	
		}
	}

	retval = 0;
	fclose(fp_cb_2);
	fclose(fp_cr_2);
	fclose(fp_cr_subsampled);
	l4: fclose(fp_cb_subsampled);
	l3: fclose(fp_cr);
	l2: fclose(fp_cb);
	l1: return retval;
}

#include "stdio.h"
#include <unistd.h>
#include "string.h"
#include <stdlib.h>
#include "endec_params.h"
#include "rgb_ycbcr.h"
#include "errno.h"
#include "jpeg.h"

//at the moment, use it like this. bin/endec -i lena.256x256

void help_and_usage()
{
	printf("\nEncoder Decoder\n");
	printf("Usage endec -i inputfileprefix [OPTIONS]\n\n");
	printf("-i inputfileprefix\t\tinputfileprefix is the filename of the input rgb file without the red, grn, blu suffixes. \n\t\t\t\tEx. lena.256x256 for lena.256x256.red/grn/blu.\n");
	printf("-r wxh\t\t\t\tfor specifying resolution of inputfile. w is width and h is height. Ex. -r 1920x1080\n");
	printf("-s subsampling\t\t\tsubsampling is one of these: YUV444, YUV422, YUV420\n");
	printf("-q quality\t\t\tQuality is one of these: low, medium, high. It is the quality of the resulting JPEG\n");
}

int extractResolution(struct endec_params *pEndecParams, char *optarg)
{
	char delim[2] = {0};
	delim[0] = 'x';
	char* ptr1 = strstr(optarg, delim);
	if(NULL == ptr1)
	{
		delim[0] = 'X';
		ptr1 = strstr(optarg, delim);
		if(NULL == ptr1)
			return -1;
	}

	char *ptr2 = NULL;
	errno = 0;
	pEndecParams->width = (int) strtol(optarg, &ptr2, 10);
	if(pEndecParams->width <= 0 || ptr2 != ptr1 || ERANGE == errno)
		return -1;

	ptr2 = NULL;
	ptr1++; //one character after 'x' or 'X'
	char *ptr3 = optarg + strlen(optarg);//'\0' at the end of optarg
	errno = 0;
	pEndecParams->height = (int) strtol(ptr1, &ptr2, 10);
	if(pEndecParams->height <= 0 || ptr2 != ptr3 || ERANGE == errno)
		return -1;

	return 0;
}

int main(int argc, char** argv)
{
	struct endec_params *pEndecParams;
	pEndecParams = (struct endec_params *)malloc(sizeof(struct endec_params));
	memset(pEndecParams, 0, sizeof(struct endec_params));
	int opt = 0, flag_i = 0, flag_r = 0;
	while((opt = getopt(argc, argv, "?hs:i:r:q:")) != -1)	//the ":" shows that the i option expects an argument.
	{
		switch(opt)
		{
			case 'i':
			flag_i  = 1;
			strcpy(pEndecParams->inputname, optarg);
			break;

			case 'r':
			flag_r = 1;
			if(-1 == extractResolution(pEndecParams, optarg))
			{
				fprintf(stderr, "Error -r: Resolution incorrect. Should be of the form -r wxh. Ex. -r 1920x1080\n");
				goto cleanup_endecparams;
			}
			break;

			case 's':
			if(!strcmp(optarg, "YUV444"))
			{
				pEndecParams->subsampling = YUV444;
			}
			else if(!strcmp(optarg, "YUV422"))
			{
				pEndecParams->subsampling = YUV422;
			}
			else if(!strcmp(optarg, "YUV420"))
			{
				pEndecParams->subsampling = YUV420;
			}
			else
			{
				fprintf(stderr, "Error -s: Subsampling should be YUV444, YUV422 or YUV420\n");
				goto cleanup_endecparams;
			}
			break;

			case 'q':
			if(!strcmp(optarg, "low"))
			{
				pEndecParams->qualityFactorQ = 25;
			}
			else if(!strcmp(optarg, "medium"))
			{
				pEndecParams->qualityFactorQ = 50;
			}
			else if(!strcmp(optarg, "high"))
			{
				pEndecParams->qualityFactorQ = 90;
			}
			else
			{
				fprintf(stderr, "Error -q: Quality must be low, medium or high\n");
				goto cleanup_endecparams;
			}
			break;

			case '?':
			case 'h':
			help_and_usage();
			goto cleanup_endecparams;
			break;

			default:
			help_and_usage();
			goto cleanup_endecparams;
			break;
		}
	}

	if(0 == flag_i || 0 == flag_r)
	{
		fprintf(stderr, "Bad command line. Need to have options -i and -r.\n");
		help_and_usage();
		goto cleanup_endecparams;
	}

	if(YUV_undefined == pEndecParams->subsampling)
	{
		pEndecParams->subsampling = YUV422;	//default. There is compression as well as good quality.
	}

	if(0 == pEndecParams->qualityFactorQ)
	{
		pEndecParams->qualityFactorQ = 50;
	}

	/*
	 * Conversion of input RGB data into YCbCr channels. 
	 */
	char r_fn[FILENAMELEN],    g_fn[FILENAMELEN], b_fn[FILENAMELEN];
	strcpy(r_fn, pEndecParams->inputname);    strcat(r_fn, ".red");
	strcpy(g_fn, pEndecParams->inputname);    strcat(g_fn, ".grn");
	strcpy(b_fn, pEndecParams->inputname);    strcat(b_fn, ".blu");

	char y_fn[FILENAMELEN],    cb_fn[FILENAMELEN], cr_fn[FILENAMELEN];
	strcpy(y_fn,  pEndecParams->inputname);   strcat(y_fn, ".y");
	strcpy(cb_fn, pEndecParams->inputname);   strcat(cb_fn, ".cb");
	strcpy(cr_fn, pEndecParams->inputname);   strcat(cr_fn, ".cr");

	if( -1 == rgb_to_ycbcr(r_fn, g_fn, b_fn, y_fn, cb_fn, cr_fn))	//ycbcr data is in files named y_fn,cb_fn, cr_fn
	{
		goto cleanup_files;
	}

	/*
	 * Subsampling of Y, Cb, Cr channels if needed.
	 */
	
	strcpy(pEndecParams->Y_filename, y_fn);	//copy the name of the file containing Y data.
	if(pEndecParams->subsampling != YUV444)
	{
		if(-1 == subsampleCbCr(cb_fn, cr_fn, pEndecParams))
		{
			goto cleanup_files;
		}
	}
	else
	{
		//Y filename already copied after rgb_to_ycbcr
		//copy cb and cr filenames.
		strcpy(pEndecParams->subsampledCb_filename, cb_fn);
		strcpy(pEndecParams->subsampledCr_filename, cr_fn);
	}

	//Forward DCT, quantization, entropy_encoding of all 8x8 sized macroblocks.
	jpeg_encode(pEndecParams);

cleanup_files:
	remove(pEndecParams->subsampledCb_filename);
	remove(pEndecParams->subsampledCr_filename);
	remove(y_fn); //= remove(pEndecParams->Y_filename)
	remove(cb_fn);//not subsampled
	remove(cr_fn);//not subsampled
cleanup_endecparams:
	free(pEndecParams);
	return 0;
}

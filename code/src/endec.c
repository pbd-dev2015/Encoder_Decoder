#include "stdio.h"
#include <unistd.h>
#include "string.h"
#include <stdlib.h>
#include "rgb_ycbcr.h"

//at the moment, use it like this. bin/endec -i lena.256x256

const int FILENAMELEN = 100;

void help_and_usage()
{
	printf("\nEncoder Decoder\n");
	printf("Usage endec -i inputfileprefix [OPTIONS]\n\n");
	printf("-i inputfileprefix\t\tinputfileprefix is the filename of the input rgb file without the red, grn, blu suffixes. Ex. lena.256x256 for lena.256x256.red/grn/blu.\n");
	printf("-s subsampling\t\t\tsubsampling is one of these: YUV444, YUV422, YUV420\n");
}

int main(int argc, char** argv)
{
	char cwd[FILENAMELEN];
	getcwd(cwd, FILENAMELEN);	//cwd now holds the directory path from which the executable was run.
	strcat(cwd, "/");
	char inputname[FILENAMELEN];	//cwd+inputname+extension gives the path to the red grn blu input files.
	
	int opt = 0, flag = 0;
	while((opt = getopt(argc, argv, "?hs:i:")) != -1)	//the ":" shows that the i option expects an argument.
	{
		switch(opt)
		{
			case 'i':
			flag  = 1;
			//strcpy(inputname, cwd);
			//strcat(inputname, optarg);
			strcpy(inputname, optarg);
			printf("Input file is %s\n", inputname);
			break;

			case '?':
			case 'h':
			help_and_usage();
			exit(0);
			break;

			default:
			help_and_usage();
			exit(0);
			break;
		}
	}

	if(0 == flag)
	{
		fprintf(stderr, "Bad command line. Need to have options.\n");
		help_and_usage();
		exit(0);
	}

	char r_fn[FILENAMELEN],    g_fn[FILENAMELEN], b_fn[FILENAMELEN];
	strcpy(r_fn, inputname);   strcat(r_fn, ".red");
	strcpy(g_fn, inputname);   strcat(g_fn, ".grn");
	strcpy(b_fn, inputname);   strcat(b_fn, ".blu");

	char y_fn[FILENAMELEN],    cb_fn[FILENAMELEN], cr_fn[FILENAMELEN];
	strcpy(y_fn, inputname);   strcat(y_fn, ".y");
	strcpy(cb_fn, inputname);  strcat(cb_fn, ".cb");
	strcpy(cr_fn, inputname);  strcat(cr_fn, ".cr");

	rgb_to_ycbcr(r_fn, g_fn, b_fn, y_fn, cb_fn, cr_fn);

	remove(y_fn);
	remove(cb_fn);
	remove(cr_fn);

	return 0;
}

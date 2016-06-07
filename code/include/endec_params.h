#ifndef ENDEC_PARAMS_H
#define ENDEC_PARAMS_H

//File for encoder decoder parameters.


#define FILENAMELEN 100

typedef enum {
	YUV_undefined,
	YUV444,
	YUV422,
	YUV420
}Subsampling_Type;

struct endec_params{
	char inputname[FILENAMELEN];	//name of the input file. Ex. lena.256x256. There is no other extension.
	int width;			//width of the input image
	int height;			//height of the input image
	Subsampling_Type subsampling;	//the subsampling level.
	char Y_filename[FILENAMELEN];	//name of file containing Y data
	char subsampledCb_filename[FILENAMELEN];	//name of file containing subsampled Cb data.
	char subsampledCr_filename[FILENAMELEN];	//name of file containing subsampled Cr data.
};

#endif

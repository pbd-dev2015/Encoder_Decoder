#include "jpeg.h"
#include "math.h"
#include "utils.h"

//Forward DCT, quantization, entropy_encoding of all 8x8 sized macroblocks.
int jpeg_encode(struct endec_params *pEndecParams)
{
	/* TODO: make this function multi-threaded by creating four threads to process the 8x8 blocks.
	 * We can have a boolean done[image_width/8][image_height/8] array. This array can hold true or false depending
	 * on whether that particular 8x8 block was processed or not. As soon as any array takes up an 8x8 block, we mark it 
	 * as done (true). We can have the threads process the 8x8 blocks sequentially (potentially harder to implement) or 
	 * have them process different parts of the image. For example, assume there are n blocks. Thread1 can start 
	 * processing blocks from block 1 and forward. Thread2 can process blocks n/2 and back to 1. Thread 3 can process
	 * from block n/2+1 and forward. Thread 4 can process blocks n and back to n/2+1. Threads should exit as soon as they
	 * find a done[i]j[] = true 8x8 block.
	 * Threads can write to different parts of output file.
	 */

	int retval = 0;
	//find number of macroblocks "horizontally and vertically" or "per row and per column".
	int num_macroblocks_horiz = 0, num_macroblocks_vert = 0;
	num_macroblocks_horiz = (pEndecParams->width  % 8 == 0) ? (pEndecParams->width  / 8) : (pEndecParams->width  / 8 + 1);
	num_macroblocks_vert  = (pEndecParams->height % 8 == 0) ? (pEndecParams->height / 8) : (pEndecParams->height / 8 + 1);
	FILE *fp = fopen(pEndecParams->Y_filename, "r");
	if(NULL == fp)
	{
		fprintf(stderr, "%s:%d:%s: Error opening file for reading macroblock\n", __FILE__,__LINE__,__FUNCTION__);
		retval = -1;
		goto l1;
	}
	//scan and handle y macroblocks left to right and top to bottom.
	int macroblock[8][8], row = 0, col = 0;
	for(row = 0; row < num_macroblocks_vert; row++)
	{
		for(col = 0; col < num_macroblocks_horiz; col++)
		{
			retval = read_macroblock(fp, pEndecParams->width, pEndecParams->height, row, col, macroblock);
			if ( -1 == retval)
			{
				goto l2;
			}
		}
	}
	l2: fclose(fp);
	l1: return retval;
}

int jpeg_forward_dct(struct endec_params *pEndecParams)
{
	return -1;
}

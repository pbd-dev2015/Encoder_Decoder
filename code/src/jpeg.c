#include "jpeg.h"
#include "utils.h"
#include "stdlib.h"

int jpeg_encode_interleaved(struct endec_params *pEndecParams)
{
	int retval = 0;

	generate_costable();	//this will generate the costable used to lookup cos values used in the DCT equation
	generate_quantization_table(pEndecParams->qualityFactorQ);

	FILE *fp_y = NULL;
	fp_y = fopen(pEndecParams->Y_filename, "r");
	if(NULL == fp_y)
	{
		fprintf(stderr, "%s:%d:%s: Error opening Y file for reading macroblock\n", __FILE__,__LINE__,__FUNCTION__);
		retval = -1;
		goto l1;
	}

	FILE *fp_cb = NULL;
	fp_cb = fopen(pEndecParams->subsampledCb_filename, "r");
	if(NULL == fp_cb)
	{
		fprintf(stderr, "%s:%d:%s: Error opening Cb file for reading macroblock\n", __FILE__,__LINE__,__FUNCTION__);
		retval = -1;
		goto l2;
	}

	FILE *fp_cr = NULL;
	fp_cr = fopen(pEndecParams->subsampledCr_filename, "r");
	if(NULL == fp_cr)
	{
		fprintf(stderr, "%s:%d:%s: Error opening Cr file for reading macroblock\n", __FILE__,__LINE__,__FUNCTION__);
		retval = -1;
		goto l3;
	}

	//this is the file we write the encoded data into.
	FILE *fp_jpeg = NULL;
	fp_jpeg = fopen(pEndecParams->outputname, "w");
	if(NULL == fp_jpeg)
	{
		fprintf(stderr, "%s:%d:%s: Error opening file for writing data\n", __FILE__,__LINE__,__FUNCTION__);
		retval = -1;
		goto l4;
	}

	/* Since this is interleaved encoding of luminance and chrominance AC and DC coefficients, we will have to determine
	 * which macroblocks should be interleaved together. That is, we find out which Y macroblocks are associated with
	 * a specific Cb(and Cr) macroblock. HxV define the macroblocks that are associated with one chroma.
	 * See Figure 13 – Interleaved order for components with different dimensions in ITU-T81.
	 *
	 * For example, if H = 2 and V = 2 as in case of YUV420, then macroblock_Y[0][0],macroblock_Y[0][1], macroblock_Y[1][0]
	 * and macroblock_Y[1][1] are associated with macroblock_Cb[0][0] and macroblock_Cr[0][0]. Hence the coefficient
	 * interleaving is done as follows: DC+AC coefficients of macroblock_Y[0][0],macroblock_Y[0][1], macroblock_Y[1][0]
	 * and macroblock_Y[1][1] and then DC+AC coefficients of macroblock_Cb[0][0] and DC+AC coefficients of
	 * macroblock_Cr[0][0].
	 *
	 * For 444, it will be macroblockY 0,0 for Cb and Cr 0,0.
	 * For 422, it will be macroblockY 0,0 and 0,1 for Cb and Cr 0,0.
	 * Also see https://books.google.com/books?id=AepB_PZ_WMkC&pg=PA101&lpg=PA101&dq=jpeg+mcu+interleaved&source=bl&ots=URJL8w5TtH&sig=XuZvFYvamTpKrxA0rGEwbfYUErA&hl=en&sa=X&ved=0ahUKEwjZ6bmFrtrOAhXLQyYKHeRkCeoQ6AEIHDAA#v=onepage&q=jpeg%20mcu%20interleaved&f=false
	 */

	//H goes to the right and V goes down. They denote the dimensions of the array which contains macroblocks.
	int H = 0, V = 0;

	switch (pEndecParams->subsampling)
	{
		case YUV444:
		H = 1;
		V = 1;
		break;

		case YUV422:
		H = 2;
		V = 1;
		break;

		case YUV420:
		H = 2;
		V = 2;
		break;

		case YUV_undefined:
		goto error_lbl;
		break;
	}

	//find number of macroblocks "horizontally and vertically" or "per row and per column".
	int num_macroblocks_horiz = 0, num_macroblocks_vert = 0;
	num_macroblocks_horiz = (pEndecParams->width  % 8 == 0) ? (pEndecParams->width  / 8) : (pEndecParams->width  / 8 + 1);
	num_macroblocks_vert  = (pEndecParams->height % 8 == 0) ? (pEndecParams->height / 8) : (pEndecParams->height / 8 + 1);

	/* we use a 2D array for the 8x8 macroblocks just so that if we need to interleave Y and chroma in funny ways,
	 * then this is handy. Otherwise just one 8x8 macroblock is enough.
	 */

	//row and col are defined over the whole image (complete macroblock grid)
	int macroblock_y[2][2][8][8], fdct_y[2][2][8][8], row = 0, col = 0;
	//macblockindex_row and macblockindex_col are defined over every HxV array of macroblocks.
	int macblockindex_row = 0, macblockindex_col = 0;
	int macroblock_cb[8][8], macroblock_cr[8][8], fdct_cb[8][8], fdct_cr[8][8];
	int prev_Y_DC = 0, prev_Cb_DC = 0, prev_Cr_DC = 0;

	WriteByteStruct wbs = {0x00, 8};	//write_byte = 0x00, write_bits_available = 8

	for(row = 0; row < num_macroblocks_vert; row+=V)
	{
		for(col = 0; col < num_macroblocks_horiz; col+=H)
		{
			//First let us work on Y.
			for(macblockindex_row = 0; macblockindex_row < V; macblockindex_row++)
			{
				for(macblockindex_col = 0; macblockindex_col < H; macblockindex_col++)
				{
						//read the macroblock
						retval = read_macroblock( fp_y, pEndecParams->width, pEndecParams->height,
									  row + macblockindex_row, col + macblockindex_col,
									  macroblock_y[macblockindex_row][macblockindex_col]
									);
						if ( -1 == retval)
						{
							goto error_lbl;
						}

						//take the forward dct of the macroblock and store it in 2d array fdct
						jpeg_forward_dct( macroblock_y[macblockindex_row][macblockindex_col],
								  fdct_y[macblockindex_row][macblockindex_col]
								);

						//Quantize the DCT output
						quantize_fdct( fdct_y[macblockindex_row][macblockindex_col],
							       QTableDerived_Luminance
							     );

						//Write Coefficients of this Macroblock to file using the Huffman encoding.
						encode_and_write_macroblock( fdct_y[macblockindex_row][macblockindex_col],
									     0,	//shows it is a Y macroblock
									     &prev_Y_DC,
									     &wbs,	//WriteByteStruct
									     fp_jpeg
									   );
				}
			}

			//now handle Cb
			//read the Cb macroblock
			retval = read_macroblock( fp_cb,
						  pEndecParams->width_subsampledchroma, pEndecParams->height_subsampledchroma,
						  row / V, col / H, //the index of the macroblock we want to read
						  macroblock_cb
						);
			if ( -1 == retval)
			{
				goto error_lbl;
			}

			//take the forward dct of the Cb macroblock and store it in 2d array fdct_cb
			jpeg_forward_dct(macroblock_cb, fdct_cb);

			//Quantize the DCT output
			quantize_fdct(fdct_cb, QTableDerived_Chrominance);

			//now handle Cr
			//read the Cr macroblock
			retval = read_macroblock( fp_cr,
						  pEndecParams->width_subsampledchroma, pEndecParams->height_subsampledchroma,
						  row / V, col / H, //the index of the macroblock we want to read
						  macroblock_cr
						);
			if ( -1 == retval)
			{
				goto error_lbl;
			}

			//take the forward dct of the Cr macroblock and store it in 2d array fdct_cr
			jpeg_forward_dct(macroblock_cr, fdct_cr);

			//Quantize the DCT output
			quantize_fdct(fdct_cr, QTableDerived_Chrominance);
		}
	}

	error_lbl:
	fclose(fp_jpeg);
	l4: fclose(fp_cr);
	l3: fclose(fp_cb);
	l2: fclose(fp_y);
	l1: return retval;
}

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
	FILE *fp_y = NULL;
	//First let us work on Y.
	num_macroblocks_horiz = (pEndecParams->width  % 8 == 0) ? (pEndecParams->width  / 8) : (pEndecParams->width  / 8 + 1);
	num_macroblocks_vert  = (pEndecParams->height % 8 == 0) ? (pEndecParams->height / 8) : (pEndecParams->height / 8 + 1);
	fp_y = fopen(pEndecParams->Y_filename, "r");
	if(NULL == fp_y)
	{
		fprintf(stderr, "%s:%d:%s: Error opening Y file for reading macroblock\n", __FILE__,__LINE__,__FUNCTION__);
		retval = -1;
		goto l1;
	}

	generate_costable();	//this will generate the costable used to lookup cos values used in the DCT equation
	generate_quantization_table(pEndecParams->qualityFactorQ);
	//scan and handle y macroblocks left to right and top to bottom.
	int macroblock[8][8], fdct[8][8], row = 0, col = 0;
	for(row = 0; row < num_macroblocks_vert; row++)
	{
		for(col = 0; col < num_macroblocks_horiz; col++)
		{
			//read the macroblock
			retval = read_macroblock(fp_y, pEndecParams->width, pEndecParams->height, row, col, macroblock);
			if ( -1 == retval)
			{
				goto l2;
			}

			//take the forward dct of the macroblock and store it in 2d array fdct
			jpeg_forward_dct(macroblock, fdct);

			//Quantize the DCT output
			quantize_fdct(fdct, QTableDerived_Luminance);
		}
	}

	//work on Cb and Cr.
	FILE *fp_cb = NULL, *fp_cr = NULL;
	fp_cb = fopen(pEndecParams->subsampledCb_filename, "r");
	if(NULL == fp_cb)
	{
		fprintf(stderr, "%s:%d:%s: Error opening Cb file for reading macroblock\n", __FILE__,__LINE__,__FUNCTION__);
		retval = -1;
		goto l2;
	}
	fp_cr = fopen(pEndecParams->subsampledCr_filename, "r");
	if(NULL == fp_cr)
	{
		fprintf(stderr, "%s:%d:%s: Error opening Cr file for reading macroblock\n", __FILE__,__LINE__,__FUNCTION__);
		retval = -1;
		goto l3;
	}

	int macroblock_cb[8][8], macroblock_cr[8][8], fdct_cb[8][8], fdct_cr[8][8];
	num_macroblocks_horiz = (pEndecParams->width_subsampledchroma  % 8 == 0) ? (pEndecParams->width_subsampledchroma  / 8) : (pEndecParams->width_subsampledchroma  / 8 + 1);
	num_macroblocks_vert  = (pEndecParams->height_subsampledchroma % 8 == 0) ? (pEndecParams->height_subsampledchroma / 8) : (pEndecParams->height_subsampledchroma / 8 + 1);

	for(row = 0; row < num_macroblocks_vert; row++)
	{
		for(col = 0; col < num_macroblocks_horiz; col++)
		{
			//read the Cb macroblock
			retval = read_macroblock(fp_cb, pEndecParams->width_subsampledchroma, pEndecParams->height_subsampledchroma, row, col, macroblock_cb);
			if ( -1 == retval)
			{
				goto l4;
			}

			//take the forward dct of the Cb macroblock and store it in 2d array fdct_cb
			jpeg_forward_dct(macroblock_cb, fdct_cb);

			//Quantize the DCT output
			quantize_fdct(fdct_cb, QTableDerived_Chrominance);

			//read the Cr macroblock
			retval = read_macroblock(fp_cr, pEndecParams->width_subsampledchroma, pEndecParams->height_subsampledchroma, row, col, macroblock_cr);
			if ( -1 == retval)
			{
				goto l4;
			}

			//take the forward dct of the Cr macroblock and store it in 2d array fdct_cr
			jpeg_forward_dct(macroblock_cr, fdct_cr);

			//Quantize the DCT output
			quantize_fdct(fdct_cr, QTableDerived_Chrominance);
		}
	}
	l4: fclose(fp_cr);
	l3: fclose(fp_cb);
	l2: fclose(fp_y);
	l1: return retval;
}


const double C_Table[8] = {M_SQRT1_2, 1 , 1, 1, 1, 1, 1, 1};	//M_SQRT1_2 = 1 / (square root of two) = sqrt(1/2)

void jpeg_forward_dct(int macroblock [][8], int fdct[][8])
{
	int x = 0, y = 0, u = 0, v = 0;
	double result = 0;

	for(u = 0; u < 8; u++)
	{
		for(v = 0; v < 8; v++)
		{
			result = 0;
			for(x = 0; x < 8; x++)
			{
				for(y = 0; y < 8; y++)
				{
					result += macroblock[x][y] * cos_table[x][u] * cos_table[y][v];
				}
			}
			result *= (C_Table[u] * C_Table[v]);
			result /= 4;
			fdct[u][v] = (int) result;
		}
	}
	return;
}


void quantize_fdct(int fdct[][8], int QTable[][8])
{
	int i = 0, j = 0;
	for(i = 0; i < 8; i++)
	{
		for(j = 0; j < 8; j++)
		{	
			//QTable is QTableDerived_Luminance/Chrominance
			fdct[i][j] /= QTable[i][j];
		}
	}	
}

int encode_and_write_macroblock(int fdct[][8], int macroblock_type, int *prev_DC, WriteByteStruct *pWBS, FILE *fp)
{
	int retval = 0;

	const Code_T *DC_Table;
	const Code_T (*AC_Table)[11];
	//macroblock_type = 0 =>  Y macroblock
	//macroblock_type = 1 => Cb macroblock
	//macroblock_type = 2 => Cr macroblock
	if(0 == macroblock_type)
	{
		DC_Table = DC_Luminance_Size_Table_K3;
		AC_Table = &AC_Luminance_Table_K5[0];
	}
	else
	{
		DC_Table = DC_Chrominance_Size_Table_K4;
		AC_Table = &AC_Chrominance_Table_K6[0];
	}

	Code_T DC_Diff_sym1;
	Code_T DC_Diff_sym2;
	//sym1 = size_of_sym2	sym2 = amplitude (= current DC coeff - previous DC coeff)
	//find the code for the DC_Diff which is symbol2. We find out symbol1 from the code of symbol2.
	find_coefficient_code(fdct[0][0] - (*prev_DC), &DC_Diff_sym2);

	//length of sym1. (which is not equal to length of sym2. length of sym2 is used as index into table)
	DC_Diff_sym1.code_length = DC_Table[DC_Diff_sym2.code_length].code_length;
	DC_Diff_sym1.code_word = DC_Table[DC_Diff_sym2.code_length].code_word;

	write_coefficient_to_file(&DC_Diff_sym1, pWBS, fp);
	if( DC_Diff_sym2.code_length != 0)
	{
		write_coefficient_to_file(&DC_Diff_sym2, pWBS, fp);
	}
	//else we don't write the symbol 2 for difference = 0. This is according to ITU_T81 F.1.2.1.1


	//now for the zigzag AC coefficients.
	//0,1	1,0
	//2,0	1,1   0,2
	//0,3	1,2   2,1   3,0
	//4,0	3,1   2,2   1,3   0,4
	//0,5	1,4   2,3   3,2   4,1   5,0
	//6,0	5,1   4,2   3,3   2,4   1,5   0,6
	//0,7	1,6   2,5   3,4   4,3   5,2   6,1   7,0

	//7,1	6,2   5,3   4,4   3,5   2,6   1,7
	//2,7   3,6   4,5   5,4   6,3   7,2
	//7,3   6,4   5,5   4,6   3,7
	//4,7   5,6   6,5   7,4
	//7,5   6,6   5,7
	//6,7   7,6
	//7,7
	enum {UP = -1, DOWN = 1};
	int sum_indexes = 1;	//sum goes from 1 to 14.
	int direction = DOWN;	//1 = down. -1 = up.
	int starting_i = 0, i = 0, j = 0;
	int runlength = 0;
	Code_T AC_sym1;
	Code_T AC_sym2;

	for(sum_indexes = 1; sum_indexes <= 14; sum_indexes++)
	{
		i = starting_i;
		while(1)
		{
			j = sum_indexes - i;
			if( i < 0 || i > 7 || j < 0 || j > 7 )
			{
				if(-1 == i)
					starting_i = 0;
				else if ( 8 == i)
					starting_i = 7;
				else
				{
					if(UP == direction)
						starting_i = i + 2;
					else
						starting_i = i;
				}

				direction = direction * (-1);	//reverse the direction.
				break;
			}

			if(0 == fdct[i][j])
			{
				runlength++;
			}
			else
			{
				//find the code for the symbol2 (AMPLITUDE)
				find_coefficient_code(fdct[i][j], &AC_sym2);
				//find the code for the symbol1 (runlength,size)
				AC_sym1.code_length = AC_Table[runlength][AC_Sym2.code_length].code_length;
				AC_sym1.code_word   = AC_Table[runlength][AC_Sym2.code_length].code_word;

				
				write_coefficient_to_file(&AC_sym1, pWBS, fp);
				write_coefficient_to_file(&AC_sym2, pWBS, fp);

				//reset runlength
				runlength = 0;
			}

			i += direction;
		}
	}

	if(runlength != 0)
	{
		AC_sym1.code_length = AC_Table[0][0].code_length;
		AC_sym1.code_word   = AC_Table[0][0].code_word;
		write_coefficient_to_file(&AC_sym1, pWBS, fp);
	}

	return retval;
}


void find_coefficient_code(int num, Code_T *c)
{
	//we find the code using Table F.1 "Difference magnitude categories for DC coding" of ITU_T81. Applies to AC as well.
	c->code_length = 0;
	c->code_word = 0;

	if(num == 0)
	{
		c->code_length = 0;
		return;
	}

	c->code_length = 1;
	int upper_limit = 2;
	c->code_word = abs(num);
	while(c->code_word >= upper_limit)
	{
		upper_limit *= 2;
		c->code_length++;
	}

	//now generate the code_word in case of negative num. In case, of positive num, the codeword is num and hence return.
	if(num < 0)
	{
		/* this is flipping the bits. This is 1's complement. This is not equal to taking the negative(-). 
		 * Negative numbers are stored as two's complement.
		*/
		c->code_word = ~(c->code_word);

		//Now we isolate just the  codeword by removing the extra MSB 1s (ones) that we got after flipping bits.
		c->code_word &= ( (unsigned int)(pow(2, c->code_length)) - 1);
	}
}

void write_coefficient_to_file(Code_T *c, WriteByteStruct *pWBS, FILE *fp)
{
	unsigned int tmp = 0, n = 0, count = 0;
	int code_bits_remaining = 0;		//number of bits of sym1 code or sym2 that are still remaining to write
	code_bits_remaining = c->code_length;
	while(code_bits_remaining > 0)
	{
		if(code_bits_remaining <= pWBS->write_bits_available)
		{
			count = code_bits_remaining;
		}
		else
		{
			count = pWBS->write_bits_available;
		}

		//make room at lsb by shifting to the left
		pWBS->write_byte = pWBS->write_byte << count;

		//now create the right value that remains to be pushed.
		n = (sizeof(unsigned int) * 8) - code_bits_remaining;
		tmp = c->code_word << n;

		/* Some explanation is in order here. 
		 * For the case where code_bits_remaining <= write_bits_available, then the bracket part doesn't matter.
		 * For the else case, we need to shift more to the right because remaining code bits don't fit into write_byte.
		 * The extra shifting to the right gives us only "count" number of MSB of the remaining code.
		*/
		tmp = tmp >> (n + (code_bits_remaining - count)); 
		tmp = tmp & 0xFF; //now tmp contains the code bits at lsb that need to be pushed.
		
		pWBS->write_byte = pWBS->write_byte | tmp;	//now we have pushed the code bits.
		
		code_bits_remaining -= count;
		pWBS->write_bits_available -= count;

		if(0 == pWBS->write_bits_available)
		{
			//TODO: Stuff byte. 0x00 after 0xFF
			fwrite(&(pWBS->write_byte), 1, 1, fp);
			pWBS->write_byte = 0;
			pWBS->write_bits_available = 8;
		}
	}
}


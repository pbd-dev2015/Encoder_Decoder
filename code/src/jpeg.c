#include "jpeg.h"
#include "math.h"
#include "utils.h"

/* we need this cos table according to the DCT and IDCT transform equations. Generating this table forehand makes it useful 
 * for all macroblocks and thus we avoid doing the same calculations again and again. The computations saved are huge since
 * a lot of math operations in floating point and cos() are used in the equation.
 * In this table, the first index is for the variable x(and y) and the second index is for u(and v).
 */
static double cos_table[8][8];
void generate_costable();
static const double C_Table[8] = {M_SQRT1_2, 1 , 1, 1, 1, 1, 1, 1};	//M_SQRT1_2 = 1 / (square root of two) = sqrt(1/2)

/* This is the IJG Standard Table for luminance. We use a quality factor to derive a quantization table QTableDerived_Luminance from this table*/
static const int QTableBase_Luminance[8][8] = {
	{16, 11, 10, 16,  24,  40,  51,  61},
	{12, 12, 14, 19,  26,  58,  60,  55},
	{14, 13, 16, 24,  40,  57,  69,  56},
	{14, 17, 22, 29,  51,  87,  80,  62},
	{18, 22, 37, 56,  68, 109, 103,  77},
	{24, 35, 55, 64,  81, 104, 113,  92},
	{49, 64, 78, 87, 103, 121, 120, 101},
	{72, 92, 95, 98, 112, 100, 103,  99}
};
//this is the quantization table derived from QTableBase_Luminance (IJG Standard Table)
static int QTableDerived_Luminance[8][8];

/* This is the IJG Standard Table for luminance. We use a quality factor to derive a quantization table QTableDerived_Luminance from this table*/
static const int QTableBase_Chrominance[8][8] = {
	{17, 18, 24, 47, 99, 99, 99, 99},
	{18, 21, 26, 66, 99, 99, 99, 99},
	{24, 26, 56, 99, 99, 99, 99, 99},
	{47, 66, 99, 99, 99, 99, 99, 99},
	{99, 99, 99, 99, 99, 99, 99, 99},
	{99, 99, 99, 99, 99, 99, 99, 99},
	{99, 99, 99, 99, 99, 99, 99, 99},
	{99, 99, 99, 99, 99, 99, 99, 99}
};
//this is the quantization table derived from QTableBase_Chrominance (IJG Standard Table)
static int QTableDerived_Chrominance[8][8];

void generate_quantization_table(int Q);
void quantize_fdct(int fdct[][8], int QTable[][8]);

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
		goto l3;
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
							goto l2;
						}

						//take the forward dct of the macroblock and store it in 2d array fdct
						jpeg_forward_dct( macroblock_y[macblockindex_row][macblockindex_col],
								  fdct_y[macblockindex_row][macblockindex_col]
								);

						//Quantize the DCT output
						quantize_fdct( fdct_y[macblockindex_row][macblockindex_col],
							       QTableDerived_Luminance
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
				goto l4;
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

void generate_costable()
{
	//This table contains the cosine values that are used in the DCT and IDCT equations.
	//this table is generated just once.
	/* In this table, the first index is for the variable x(and y) and the second index is for u(and v).*/
	int x = 0, u = 0;
	double angle = 0;

	for(x = 0; x < 8; x++)
	{
		for(u = 0; u < 8; u++)
		{
			angle = (2 * x + 1) * u * M_PI / 16;
			cos_table[x][u] = cos(angle);
		}
	}
}


void generate_quantization_table(int Q)
{
	//Q is the quality factor. 1 <= Q <= 100.
	// Q = 50 means that there is no quantization.
	//http://dfrws.org/2008/proceedings/p21-kornblum_pres.pdf : See page 13.
	//http://stackoverflow.com/a/29216609

	int S = (Q < 50) ? (5000 / Q) : (200 - 2 * Q);

	int i = 0 , j = 0;
	for(i = 0; i < 8; i++)
	{
		for(j = 0; j < 8; j++)
		{
			QTableDerived_Luminance[i][j]   = (S * QTableBase_Luminance[i][j]   + 50) / 100;
			QTableDerived_Chrominance[i][j] = (S * QTableBase_Chrominance[i][j] + 50) / 100;
		}
	}

	/* Some explanation on the math involved. 
	 * Tb = BaseTable. Ts = Derived Table.
	 * There are two cases : Q < 50 and Q >=50
	 * For Q < 50, the equation becomes Ts = 0.5 * { (100 * Tb / Q) + 1} = 50Tb/Q + 0.5
	 * Thus, as Q decreases, Ts increases. Ts is the actual quantizing value (the number we divide macroblock value by).
	 * Thus for low values of Q, we will be using greater divisors and hence there will be more compression.
	 * For Q = 1, Ts = 50*Tb + 0.5		For Q = 49, Ts = Tb + 0.5
	 *
	 * For Q >= 50, the equation becomes Ts = Tb*(100-Q)/50  + 0.5
	 * As Q increases towards 100, Ts decreases and hence compression reduces and quality increases.
	 *
	 * For Tb[0][0] = 16, we will calculate Ts[0][0] for different values of Q.
	 * For Q =  1, Ts[0][0] = 800.5
	 * For Q = 25, Ts[0][0] = 24.5
	 * For Q = 50, Ts[0][0] = 16.5
	 * For Q = 75, Ts[0][0] = 8.5
	 * For Q = 90, Ts[0][0] = 3.7
	 * For Q = 100, Ts[0][0] = 0.5
	 */
	return;
}

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

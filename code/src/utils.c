#include "utils.h"

//helper function to remove repition of code. Used in read_macroblock()
int read_valid_data(FILE *fp, int width, int height, int macroblock_row_num, int macroblock_col_num, int macroblock[][8], int num_valid_macroblockrows, int num_valid_macroblockcols)
{
	int i = 0;
	for(i = 0; i < num_valid_macroblockrows; i++)
	{
		//offset from beginning of file.
		fseek(fp, (macroblock_row_num * 8 + i) * width + (macroblock_col_num * 8), SEEK_SET);
		clearerr(fp);
		fread(macroblock[i], 1, num_valid_macroblockcols, fp);	//read 'num_valid_macroblockcols' bytes.
		if(ferror(fp))
		{
			fprintf(stderr, "%s:%d:%s: Error reading macroblock\n",__FILE__,__LINE__,__FUNCTION__);
			return -1;
		}
	}
	return 0;
}

int read_macroblock(FILE *fp, int width, int height, int macroblock_row_num, int macroblock_col_num, int macroblock[][8])
{
	//find row and column number of macroblock.
	//then use row and column numbers with actual width and height to read data using read_valid_data().
	int retval = 0;
	if(NULL == fp)
	{
		fprintf(stderr, "%s:%d:%s: Error opening file for reading macroblock\n", __FILE__,__LINE__,__FUNCTION__);
		return -1;
	}

	int right_flag = 0, bottom_flag = 0;	//to indicate if macroblock goes beyond right and bottom respectively.
	if(macroblock_col_num * 8 + 7 >= width)
		right_flag = 1;
	if(macroblock_row_num * 8 + 7 >= height)
		bottom_flag = 1;

	int i = 0, j = 0;
	if(0 == right_flag && 0 == bottom_flag)
	{
		//no need of padding.
		return read_valid_data(fp, width, height, macroblock_row_num, macroblock_col_num, macroblock, 8, 8);
	}
	else if(1 == right_flag && 1 == bottom_flag)
	{
		//need padding on right and bottom.
		//read only a small rectangular block and pad the rest.
		int num_valid_macroblockrows = height - (macroblock_row_num * 8);
		int num_valid_macroblockcols = width  - (macroblock_col_num * 8);
		retval = read_valid_data(fp, width, height, macroblock_row_num, macroblock_col_num, macroblock, num_valid_macroblockrows, num_valid_macroblockcols);
		if(-1 == retval)
			return -1;
		//now pad on the right
		for(i = 0; i < num_valid_macroblockrows; i++)
		{
			//for every row i, copy the last element in the macroblock to the remaining invalid cols of the row i.
			for( j = num_valid_macroblockcols; j < 8; j++)
				macroblock[i][j] = macroblock[i][num_valid_macroblockcols - 1];
		}

		//now pad on the bottom.
		for(i = num_valid_macroblockrows; i < 8; i++)
		{
			//copy the last valid row (including the right padded data) into the remaining rows.
			for(j = 0; j < 8; j++)
				macroblock[i][j] = macroblock[num_valid_macroblockrows - 1][j];
		}

	}
	else if(1 == right_flag)
	{
		//need padding on the right
		int num_valid_macroblockcols = width - (macroblock_col_num * 8);
		retval = read_valid_data(fp, width, height, macroblock_row_num, macroblock_col_num, macroblock, 8, num_valid_macroblockcols);
		if(-1 == retval)
			return -1;
		//now pad on the right
		for(i = 0; i < 8; i++)
		{
			//for every row i, copy the last element in the macroblock to the remaining invalid cols of the row i.
			for( j = num_valid_macroblockcols; j < 8; j++)
				macroblock[i][j] = macroblock[i][num_valid_macroblockcols - 1];
		}
	}
	else if (1 == bottom_flag)
	{
		//need padding on the bottom
		int num_valid_macroblockrows = height - (macroblock_row_num * 8);
		retval = read_valid_data(fp, width, height, macroblock_row_num, macroblock_col_num, macroblock, num_valid_macroblockrows, 8);
		if(-1 == retval)
			return -1;
		//now pad on the bottom.
		for(i = num_valid_macroblockrows; i < 8; i++)
		{
			//copy the last valid row into the remaining rows.
			for(j = 0; j < 8; j++)
				macroblock[i][j] = macroblock[num_valid_macroblockrows - 1][j];
		}
	}
	return 0;
}

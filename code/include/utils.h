#ifndef UTILS_H
#define UTILS_H

#include "stdio.h"
//this file is for utility functions used across all codecs.

int read_macroblock(FILE *fp, int width, int height, int macroblock_row_num, int macroblock_col_num, int macroblock[][8]);

#endif

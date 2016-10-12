#include "jpeg_tables.h"
#include "math.h"

/* This is the IJG Standard Table for luminance. We use a quality factor to derive a quantization table 
 * named QTableDerived_Luminance from this table.
 * This is table K.1 from ITU-T81
 */
const int QTableBase_Luminance[8][8] = {
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
int QTableDerived_Luminance[8][8];

/* This is the IJG Standard Table for luminance. We use a quality factor to derive a quantization table 
 * named QTableDerived_Luminance from this table.
 * This is table K.2 from ITU-T81.
 */
const int QTableBase_Chrominance[8][8] = {
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
int QTableDerived_Chrominance[8][8];

/* This is table K3. from ITU-T81
*/
const Code_T DC_Luminance_Size_Table_K3[12] = {
/* category 0*/		{2, 0x00},	//00
/* category 1*/		{3, 0x02},	//010
/* category 2*/		{3, 0x03},	//011
/* category 3*/		{3, 0x04},	//100
/* category 4*/		{3, 0x05},	//101
/* category 5*/		{3, 0x06},	//110
/* category 6*/		{4, 0x0E},	//1110
/* category 7*/		{5, 0x1E},	//11110
/* category 8*/		{6, 0x3E},	//111110
/* category 9*/		{7, 0x7E},	//1111110
/* category 10*/	{8, 0xFE},	//11111110
/* category 11*/	{9, 0x1FE}	//111111110
};

/* The AC table for Luminance is for symbol-1 (Runlength, Size). The values in the array are code_length of sym1 and
 * code of sym1.
 * Rows denote runlength of zeroes (0 to F). 
 * Columns denote size of amplitude (symbol-2).
 * So AC_Luminance_Table_K5[14][6] denotes the code for Runlength 14 (=0xE) and Size 6.
 * This is table K.5 from ITU-T81.
*/
const Code_T AC_Luminance_Table_K5[16][11] = {
	//	0	     1		   2		  3		4	     5		   6		7	     8		    9		  A
/*0*/	{   {4, 0x0A},    {2, 0x00},    {2, 0x01},    {3, 0x04},    {4, 0x0B},    {5, 0x1A},    {7, 0x78},    {8, 0xF8},  {10, 0x3F6}, {16, 0xFF82}, {16, 0xFF83} },
/*1*/	{  {-1, 0x00},    {4, 0x0C},    {5, 0x1B},    {7, 0x79},   {9, 0x1F6},  {11, 0x7F6}, {16, 0xFF84}, {16, 0xFF85}, {16, 0xFF86}, {16, 0xFF87}, {16, 0xFF88} },
/*2*/	{  {-1, 0x00},    {5, 0x1C},    {8, 0xF9},  {10, 0x3F7},  {12, 0xFF4}, {16, 0xFF89}, {16, 0xFF8A}, {16, 0xFF8B}, {16, 0xFF8C}, {16, 0xFF8D}, {16, 0xFF8E} },
/*3*/	{  {-1, 0x00},    {6, 0x3A},   {9, 0x1F7},  {12, 0xFF5}, {16, 0xFF8F}, {16, 0xFF90}, {16, 0xFF91}, {16, 0xFF92}, {16, 0xFF93}, {16, 0xFF94}, {16, 0xFF95} },
/*4*/	{  {-1, 0x00},    {6, 0x3B},  {10, 0x3F8}, {16, 0xFF96}, {16, 0xFF97}, {16, 0xFF98}, {16, 0xFF99}, {16, 0xFF9A}, {16, 0xFF9B}, {16, 0xFf9C}, {16, 0xFF9D} },
/*5*/	{  {-1, 0x00},    {7, 0x7A},  {11, 0x7F7}, {16, 0xFF9E}, {16, 0xFF9F}, {16, 0xFFA0}, {16, 0xFFA1}, {16, 0xFFA2}, {16, 0xFFA3}, {16, 0xFFA4}, {16, 0xFFA5} },
/*6*/	{  {-1, 0x00},    {7, 0x7B},  {12, 0xFF6}, {16, 0xFFA6}, {16, 0xFFA7}, {16, 0xFFA8}, {16, 0xFFA9}, {16, 0xFFAA}, {16, 0xFFAB}, {16, 0xFFAC}, {16, 0xFFAD} },
/*7*/	{  {-1, 0x00},    {8, 0xFA},  {12, 0xFF7}, {16, 0xFFAE}, {16, 0xFFAF}, {16, 0xFFB0}, {16, 0xFFB1}, {16, 0xFFB2}, {16, 0xFFB3}, {16, 0xFFB4}, {16, 0xFFB5} },
/*8*/	{  {-1, 0x00},   {9, 0x1F8}, {15, 0x7FC0}, {16, 0xFFB6}, {16, 0xFFB7}, {16, 0xFFB8}, {16, 0xFFB9}, {16, 0xFFBA}, {16, 0xFFBB}, {16, 0xFFBC}, {16, 0xFFBD} },
/*9*/	{  {-1, 0x00},   {9, 0x1F9}, {16, 0xFFBE}, {16, 0xFFBF}, {16, 0xFFC0}, {16, 0xFFC1}, {16, 0xFFC2}, {16, 0xFFC3}, {16, 0xFFC4}, {16, 0xFFC5}, {16, 0xFFC6} },
/*A*/	{  {-1, 0x00},   {9, 0x1FA}, {16, 0xFFC7}, {16, 0xFFC8}, {16, 0xFFC9}, {16, 0xFFCA}, {16, 0xFFCB}, {16, 0xFFCC}, {16, 0xFFCD}, {16, 0xFFCE}, {16, 0xFFCF} },
/*B*/	{  {-1, 0x00},  {10, 0x3F9}, {16, 0xFFD0}, {16, 0xFFD1}, {16, 0xFFD2}, {16, 0xFFD3}, {16, 0xFFD4}, {16, 0xFFD5}, {16, 0xFFD6}, {16, 0xFFD7}, {16, 0xFFD8} },
/*C*/	{  {-1, 0x00},  {10, 0x3FA}, {16, 0xFFD9}, {16, 0xFFDA}, {16, 0xFFDB}, {16, 0xFFDC}, {16, 0xFFDD}, {16, 0xFFDE}, {16, 0xFFDF}, {16, 0xFFE0}, {16, 0xFFE1} },
/*D*/	{  {-1, 0x00},  {11, 0x7F8}, {16, 0xFFE2}, {16, 0xFFE3}, {16, 0xFFE4}, {16, 0xFFE5}, {16, 0xFFE6}, {16, 0xFFE7}, {16, 0xFFE8}, {16, 0xFFE9}, {16, 0xFFEA} },
/*E*/	{  {-1, 0x00}, {16, 0xFFEB}, {16, 0xFFEC}, {16, 0xFFED}, {16, 0xFFEE}, {16, 0xFFEF}, {16, 0xFFF0}, {16, 0xFFF1}, {16, 0xFFF2}, {16, 0xFFF3}, {16, 0xFFF4} },
/*F*/	{ {11, 0x7F9}, {16, 0xFFF5}, {16, 0xFFF6}, {16, 0xFFF7}, {16, 0xFFF8}, {16, 0xFFF9}, {16, 0xFFFA}, {16, 0xFFFB}, {16, 0xFFFC}, {16, 0xFFFD}, {16, 0xFFFE} }
};

/* This is table K.4 from ITU-T81
*/
const Code_T DC_Chrominance_Size_Table_K4[12] = {
/* category 0*/		{2, 0x00},	//00
/* category 1*/		{2, 0x01},	//01
/* category 2*/		{2, 0x02},	//10
/* category 3*/		{3, 0x06},	//110
/* category 4*/		{4, 0x0E},	//1110
/* category 5*/		{5, 0x1E},	//11110
/* category 6*/		{6, 0x3E},	//111110
/* category 7*/		{7, 0x7E},	//1111110
/* category 8*/		{8, 0xFE},	//11111110
/* category 9*/		{9, 0x1FE},	//111111110
/* category 10*/	{10,0x3FE},	//1111111110
/* category 11*/	{11,0x7FE}	//11111111110
};

/* The AC table for Chrominance is for symbol-1 (Runlength, Size) The values in the array are code_length of sym1 and
 * code of sym1.
 * Rows denote runlength of zeroes (0 to F). 
 * Columns denote size of amplitude (symbol-2).
 * So AC_Chrominance_Table_K6[14][6] denotes the code for Runlength 14 (=0xE) and Size 6.
 * This is table K.6 from ITU-T81.
*/
const Code_T AC_Chrominance_Table_K6[16][11] = {
	//	0	      1		   2		 3	      4		    5		   6		7	      8		    9		 A
/*0*/	{   {2, 0x00},    {2, 0x01},    {3, 0x04},    {4, 0x0A}, {   5, 0x18},    {5, 0x19},    {6, 0x38},    {7, 0x78},   {9, 0x1F4},  {10, 0x3F6},  {12, 0xFF4} },
/*1*/	{  {-1, 0x00},    {4, 0x0B},    {6, 0x39},    {8, 0xF6},   {9, 0x1F5},  {11, 0x7F6},  {12, 0xFF5}, {16, 0xFF88}, {16, 0xFF89}, {16, 0xFF8A}, {16, 0xFF8B} },
/*2*/	{  {-1, 0x00},    {5, 0x1A},    {8, 0xF7},  {10, 0x3F7},  {12, 0xFF6}, {15, 0x7FC2}, {16, 0xFF8C}, {16, 0xFF8D}, {16, 0xFF8E}, {16, 0xFF8F}, {16, 0xFF90} },
/*3*/	{  {-1, 0x00},    {5, 0x1B},    {8, 0xF8},  {10, 0x3F8},  {12, 0xFF7}, {16, 0xFF91}, {16, 0xFF92}, {16, 0xFF93}, {16, 0xFF94}, {16, 0xFF95}, {16, 0xFF96} },
/*4*/	{  {-1, 0x00},    {6, 0x3A},   {9, 0x1F6}, {16, 0xFF97}, {16, 0xFF98}, {16, 0xFF99}, {16, 0xFF9A}, {16, 0xFF9B}, {16, 0xFF9C}, {16, 0xFF9D}, {16, 0xFF9E} },
/*5*/	{  {-1, 0x00},    {6, 0x3B},  {10, 0x3F9}, {16, 0xFF9F}, {16, 0xFFA0}, {16, 0xFFA1}, {16, 0xFFA2}, {16, 0xFFA3}, {16, 0xFFA4}, {16, 0xFFA5}, {16, 0xFFA6} },
/*6*/	{  {-1, 0x00},    {7, 0x79},  {11, 0x7F7}, {16, 0xFFA7}, {16, 0xFFA8}, {16, 0xFFA9}, {16, 0xFFAA}, {16, 0xFFAB}, {16, 0xFFAC}, {16, 0xFFAD}, {16, 0xFFAE} },
/*7*/	{  {-1, 0x00},    {7, 0x7A},  {11, 0x7F8}, {16, 0xFFAF}, {16, 0xFFB0}, {16, 0xFFB1}, {16, 0xFFB2}, {16, 0xFFB3}, {16, 0xFFB4}, {16, 0xFFB5}, {16, 0xFFB6} },
/*8*/	{  {-1, 0x00},    {8, 0xF9}, {16, 0xFFB7}, {16, 0xFFB8}, {16, 0xFFB9}, {16, 0xFFBA}, {16, 0xFFBB}, {16, 0xFFBC}, {16, 0xFFBD}, {16, 0xFFBE}, {16, 0xFFBF} },
/*9*/	{  {-1, 0x00},   {9, 0x1F7}, {16, 0xFFC0}, {16, 0xFFC1}, {16, 0xFFC2}, {16, 0xFFC3}, {16, 0xFFC4}, {16, 0xFFC5}, {16, 0xFFC6}, {16, 0xFFC7}, {16, 0xFFC8} },
/*A*/	{  {-1, 0x00},   {9, 0x1F8}, {16, 0xFFC9}, {16, 0xFFCA}, {16, 0xFFCB}, {16, 0xFFCC}, {16, 0xFFCD}, {16, 0xFFCE}, {16, 0xFFCF}, {16, 0xFFD0}, {16, 0xFFD1} },
/*B*/	{  {-1, 0x00},   {9, 0x1F9}, {16, 0xFFD2}, {16, 0xFFD3}, {16, 0xFFD4}, {16, 0xFFD5}, {16, 0xFFD6}, {16, 0xFFD7}, {16, 0xFFD8}, {16, 0xFFD9}, {16, 0xFFDA} },
/*C*/	{  {-1, 0x00},   {9, 0x1FA}, {16, 0xFFDB}, {16, 0xFFDC}, {16, 0xFFDD}, {16, 0xFFDE}, {16, 0xFFDF}, {16, 0xFFE0}, {16, 0xFFE1}, {16, 0xFFE2}, {16, 0xFFE3} },
/*D*/	{  {-1, 0x00},  {11, 0x7F9}, {16, 0xFFE4}, {16, 0xFFE5}, {16, 0xFFE6}, {16, 0xFFE7}, {16, 0xFFE8}, {16, 0xFFE9}, {16, 0xFFEA}, {16, 0xFFEB}, {16, 0xFFEC} },
/*E*/	{  {-1, 0x00}, {14, 0x3FE0}, {16, 0xFFED}, {16, 0xFFEE}, {16, 0xFFEF}, {16, 0xFFF0}, {16, 0xFFF1}, {16, 0xFFF2}, {16, 0xFFF3}, {16, 0xFFF4}, {16, 0xFFF5} },
/*F*/	{ {10, 0x3FA}, {15, 0x7FC3}, {16, 0xFFF6}, {16, 0xFFF7}, {16, 0xFFF8}, {16, 0xFFF9}, {16, 0xFFFA}, {16, 0xFFFB}, {16, 0xFFFC}, {16, 0xFFFD}, {16, 0xFFFE} }
};

/* we need this cos table according to the DCT and IDCT transform equations. Generating this table forehand makes it useful 
 * for all macroblocks and thus we avoid doing the same calculations again and again. The computations saved are huge since
 * a lot of math operations in floating point and cos() are used in the equation.
 * In this table, the first index is for the variable x(and y) and the second index is for u(and v).
 */
double cos_table[8][8];
void generate_costable();

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


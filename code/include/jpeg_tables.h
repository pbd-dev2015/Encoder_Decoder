typedef struct {
	int code_length;
	unsigned int code_word;
}Code_T;

extern const int QTableBase_Luminance[8][8];
extern int QTableDerived_Luminance[8][8];

extern const int QTableBase_Chrominance[8][8];
extern int QTableDerived_Chrominance[8][8];

extern const Code_T DC_Luminance_Size_Table_K3[12];
extern const Code_T AC_Luminance_Table_K5[16][11];

extern const Code_T DC_Chrominance_Size_Table_K4[12];
extern const Code_T AC_Chrominance_Table_K6[16][11];

extern double cos_table[8][8];
void generate_costable();
void generate_quantization_table(int Q);

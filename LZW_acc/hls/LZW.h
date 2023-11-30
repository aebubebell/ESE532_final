#ifndef LZW_HW_H
#define LZW_HW_H
#define CAPACITY 32768

extern "C"
{
//void LCS(unsigned char *Chunk, int *encode_array,unsigned int *chunk_length,int* compress_length);
void hardware_encoding(unsigned char *Chunk, int *encode_array,unsigned int *chunk_length, int* compress_length);
}
#endif

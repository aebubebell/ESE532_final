#ifndef LZW_HW_H
#define LZW_HW_H
#define CAPACITY 32768

extern "C"
{
void hardware_encoding(unsigned char *Chunk,int *encode_array,int* compress_length);
}
#endif

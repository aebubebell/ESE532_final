#ifndef UTILITY_H
#define UTILITY_H

#include <unordered_map>
#include <iostream>
#include <string>
using namespace std;
#define WIN_SIZE 16
#define PRIME 3
#define MODULUS 2048
#define TARGET 0
#define MAX_CHUNK 8192
#define MAX_NUM 256

int cdc(unsigned char *buff, unsigned int buff_size,  unsigned char ** chunk,unsigned int *chunk_length);
void cmd(unsigned char** chunk,uint32_t *header,int chunks_num);
void sha256_process_arm(uint32_t state[8], const uint8_t data[], uint32_t length);
std::string neon_sha(unsigned char *Chunk,int chunk_length);
void LZWencoding(unsigned char* Chunk, int* encode_array,int& compress_length);
#endif

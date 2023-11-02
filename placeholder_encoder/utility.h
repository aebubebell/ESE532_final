#pragma once
#include <unordered_map>
uint64_t hash_func(unsigned char *input, unsigned int pos);
int cdc(unsigned char *buff, unsigned int buff_size, bool * boundary);
void create_chunks(unsigned char ** chunk,bool* boundary,unsigned char *buff, unsigned int buff_size);
uint64_t bad_hash(unsigned char * chunk,int chunk_length);
uint32_t cmd(unsigned char* chunk,int chunk_length,std::unordered_map<uint64_t,uint32_t>& chunktable);


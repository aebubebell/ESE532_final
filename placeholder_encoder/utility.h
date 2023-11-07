#pragma once
#include <unordered_map>
#include <iostream>
#include <string>
uint64_t hash_func(unsigned char *input, unsigned int pos);
int cdc(unsigned char *buff, unsigned int buff_size,unsigned char ** chunk);
void create_chunks(unsigned char ** chunk,bool* boundary,unsigned char *buff, unsigned int buff_size);
uint64_t bad_hash(unsigned char * chunk,int chunk_length);
uint32_t cmd(unsigned char* chunk,int chunk_length,std::unordered_map<std::string,uint32_t>&chunktable);
void sha256_process(uint32_t state[8], const uint8_t data[], uint32_t length);


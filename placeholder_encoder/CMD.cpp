/*
Chunk matching with hash computing(currently using a bad hash)
Zhiye Zhang
11/2/2001
*/
//included library
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <iostream> 
#include <unordered_map>
#include "utility.h"
using namespace std;
#define WIN_SIZE 16
#define PRIME 3
#define MODULUS 256
#define TARGET 0
#define MAX_CHUNK 4096
#define MAX_NUM 256
uint64_t bad_hash(unsigned char * chunk,int chunk_length)
{
    uint64_t hash=0;
    for(int i=0;i<chunk_length;i++)
    {
        hash+=static_cast<uint64_t>(chunk[i]);
    }
    return hash;
}
uint32_t cmd(unsigned char* chunk,int chunk_length,std::unordered_map<string,uint32_t>& chunktable)
{
	string hash;
	 uint32_t state[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
		sha256_process(state,chunk,4096);
		for(int i =0;i<8;i++)
		{
			hash += to_string(state[i]);
		}
		//cout<<hash<<endl;
	auto it=chunktable.find(hash);
	if(it == chunktable.end())
	{
		uint32_t newIndex=(uint32_t)chunktable.size();
		chunktable[hash]=newIndex;
		//cout<<"chunk no match"<<endl;
		return newIndex << 1;
	}
	else
	{
		//cout<<"chunk match"<<endl;
		return (it->second << 1) |1u;

	}
}
void test_hash(unsigned char ** Chunks, int chunk_num,uint64_t* hash)
{
    int chunk_length;
    for (int i=0;i<chunk_num;i++)
    {
        chunk_length=static_cast<int>(sizeof(Chunks[i])/sizeof(unsigned char));
        hash[i]=bad_hash(Chunks[i],chunk_length);
        std::cout<< "The bad hash for Chunks ["<<i<<"] is"<<std::endl<<hash[i]<<std::endl;
    }
}
void test_cmd( const char* file )//test whether the cdc function works
{
	FILE* fp = fopen(file,"r" );
	if(fp == NULL ){
		perror("fopen error");
		return;
	}

	fseek(fp, 0, SEEK_END); // seek to end of file
	int file_size = ftell(fp); // get current file pointer
	fseek(fp, 0, SEEK_SET); // seek back to beginning of file

	unsigned char* buff = (unsigned char *)malloc((sizeof(unsigned char) * file_size ));	
	if(buff == NULL)
	{
		perror("not enough space");
		fclose(fp);
		return;
	}
	unsigned char* Chunk_array[MAX_NUM];
	int chunks_num = cdc(buff, file_size,Chunk_array);
    //test_hash(Chunk_array,chunks_num,hash);
	std::unordered_map<string,uint32_t> chunktable;
	for(int i=0;i<chunks_num;i++)
	{
		int chunk_length= 4096;
		//std::cout<<cmd(Chunk_array[i],chunk_length,chunktable)<<std::endl;
		uint32_t header=cmd(Chunk_array[i],chunk_length,chunktable);
		std::cout<<header<<std::endl;
	}
	free(buff);
    return;
}
// int main()
// {
//     test_cmd("LittlePrince.txt");
// 	return 0;
// }

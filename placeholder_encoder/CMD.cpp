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
uint32_t cmd(unsigned char* chunk,int chunk_length,std::unordered_map<uint64_t,uint32_t>& chunktable)
{
	uint64_t hash= bad_hash(chunk,chunk_length);
	//std::cout<< hash<<"	";
	auto it=chunktable.find(hash);
	if(it == chunktable.end())
	{
		uint32_t newIndex=(uint32_t)chunktable.size();
		chunktable[hash]=newIndex;
		return newIndex << 1;
	}
	else
	{
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

	int bytes_read = fread(&buff[0],sizeof(unsigned char),file_size,fp);
	bool* boundary = (bool*)malloc((sizeof(unsigned char)* file_size));
	unsigned char* Chunk_array[MAX_NUM];
	int chunks_num = cdc(buff, file_size, boundary);
	create_chunks(Chunk_array,boundary,buff,file_size);
    //uint64_t* hash= (uint64_t*)malloc(sizeof(uint64_t)*chunks_num);
    //test_hash(Chunk_array,chunks_num,hash);
	std::unordered_map<uint64_t,uint32_t> chunktable;
	unsigned char* Send_data[MAX_CHUNK];
	for(int i=0;i<chunks_num;i++)
	{
		int chunk_length= static_cast<int>(sizeof(Chunk_array[i])/sizeof(unsigned char));
		//std::cout<<cmd(Chunk_array[i],chunk_length,chunktable)<<std::endl;
		uint32_t header=cmd(Chunk_array[i],chunk_length,chunktable);
		if(header %2 ==0)
		{
			
		}
		else
		{
			memcpy(Send_data[i],&header,4);
		}
	}
    free(buff);
    return;
}
// int main()
// {
//     test_cmd("LittlePrince.txt");
// 	return 0;
// }
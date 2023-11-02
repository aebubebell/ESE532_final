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
#include<bits/stdc++.h>
using namespace std;

#define WIN_SIZE 16
#define PRIME 3
#define MODULUS 256
#define TARGET 0
#define MAX_CHUNK 4096
#define MAX_NUM 256

std::vector<int> LZWencoding(unsigned char* Chunk,int chunk_length)
{
   std::unordered_map <std::string, int> dictionary;
   for(int i =0;i<255;i++)
   {
        std::string ch="";
        ch+=static_cast<char>(i);
        dictionary[ch]=i;
   }
   std::string p="",c="";
   p+=static_cast<char>(Chunk[0]);
   int code =256;
   std::vector<int> output_code;
   for(int i=0;i<chunk_length;i++)
   {
        if(i != chunk_length-1 )
        {
            c+= static_cast<char>(Chunk[i+1]);
        }
        if(dictionary.find(p+c)!= dictionary.end())
        {
            p=p+c;
        }
        else
        {
            output_code.push_back(dictionary[p]);
            dictionary[p+c]=code;
            code ++;
            p=c;
        }
        c="";
   }
   output_code.push_back(dictionary[p]);
   return output_code;
}
void test_lzw( const char* file )//test whether the cdc function works
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
    uint64_t* hash= (uint64_t*)malloc(sizeof(uint64_t)*chunks_num);
   //test_hash(Chunk_array,chunks_num,hash);
	std::unordered_map<uint64_t,uint32_t> chunktable;
	unsigned char* Send_data[chunks_num];
	for(int i=0;i<chunks_num;i++)
	{
		int chunk_length= static_cast<int>(sizeof(Chunk_array[i])/sizeof(unsigned char));
		//std::cout<<cmd(Chunk_array[i],chunk_length,chunktable)<<std::endl;
		uint32_t header=cmd(Chunk_array[i],chunk_length,chunktable);
		if(header %2 ==0)
		{
			std::vector<int> LZW=LZWencoding(Chunk_array[i],static_cast<int>(sizeof(Chunk_array[i])/sizeof(unsigned char)));
            uint32_t encode_array[LZW.size()];
            for(int j =0;j<LZW.size();j++)
            {
                encode_array[j]=(uint32_t)LZW[j];
                header= sizeof(encode_array)<<1;
                memcpy(Send_data[i],&header,4);
                memcpy(Send_data[i]+4,&encode_array,sizeof(encode_array));
            }
		}
		else
		{
			memcpy(Send_data[i],&header,4);
		}
	}
    free(buff);
    return;
}
int main()
{

    test_lzw("LittlePrince.txt");
	return 0;
}

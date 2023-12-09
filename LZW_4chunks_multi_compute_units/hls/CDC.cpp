/*
Content defined chunking
Zhiye Zhang
11/1/2023
*/
//included library
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include<iostream>
#include "utility.h"

const uint32_t pow_table[17]={3,9,27,81,243,729,2187,6561,19683,59049,177147,531441,1594323,4782969,14348907,43046721,129140163};

uint64_t hash_func(unsigned char *input, unsigned int pos)//hash for
{
	uint64_t hash = 0;//initialize hash value
	for(int i = 0; i < WIN_SIZE; i++)//calculating hash value for window
	{
		hash += static_cast<uint64_t>(input[pos + WIN_SIZE - 1 - i]) * pow_table[i];
	}
	return hash;
}

int cdc(unsigned char *buff, unsigned int buff_size,  unsigned char ** chunk,unsigned int *chunk_length)
{
	uint64_t hash = 0;//initialize hash value
	int index=0;//the index of chunk
	unsigned int previous_i=0;//the previous positon of input data
	for(unsigned int i = WIN_SIZE; i < buff_size-WIN_SIZE; i++)//calculating cdc hash for input data, WIN_SIZE is 16
	{
		if(i == WIN_SIZE)//for the first iteration
		{
        	hash = hash_func(buff, i);//call hash_func
		}
		else//from the next iteration
		{
			hash = hash * PRIME - static_cast<uint64_t>(buff[i - 1]) * pow_table[WIN_SIZE] + static_cast<uint64_t>(buff[i - 1 + WIN_SIZE]) * PRIME;
			//next_hash = 3*(current_hash-input[pos]*3^16+input[pos+WIN_SIZE])
		}
        if((hash % MODULUS) == TARGET || (i == buff_size-WIN_SIZE-1))//MODULUS is 256, TARGET is 0
		// if hash%256 = 0 or i reach the end of buff
		{
           //chunk[index]=(unsigned char*)malloc(sizeof(unsigned char)* MAX_CHUNK);//create a chunk buffer for defined chunk, MAX_CHUNK is 8192
		   // since we don't know how big the chunk is, we just create a buffer at maxium size 8192
		   if(i<buff_size-WIN_SIZE-1)//if it is not the last buffer
			{
				memcpy(chunk[index],buff+previous_i,i-previous_i);//copy the data (from previous boundary to current boundary)
				chunk_length[index]=i-previous_i;
			}
			else
			{
				memcpy(chunk[index],buff+previous_i,buff_size-previous_i);//copy the data (from previous boundary to the end of buffer)
				chunk_length[index]=buff_size-previous_i;
			}
			index++;//chunk index increment
			previous_i=i;//save the end boundary of last chunk as the start boundary of next chunk.
        }
    }
    return index;
}
/*
In cdc function, I created a bool array with the same size of buffer,
whenver the cdc hash met target, the same index in arry is true,
otherwise is false.
*/
// void create_chunks(unsigned char ** chunk,bool* boundary,unsigned char *buff, unsigned int buff_size)
// {
// 	int index=0;
// 	unsigned int previous_i=0;
// 	for(unsigned int i = WIN_SIZE; i < buff_size-WIN_SIZE; i++)
// 	{
// 		if((boundary[i]==true)|| (i == buff_size-WIN_SIZE-1))
// 		{
// 			chunk[index]=(unsigned char*)malloc(sizeof(unsigned char)* MAX_CHUNK);
// 			if(i<buff_size-WIN_SIZE-1)
// 			{
// 				memcpy(chunk[index],buff+previous_i,i-previous_i);
// 			}
// 			else
// 			{
// 				memcpy(chunk[index],buff+previous_i,buff_size-previous_i);
// 			}
// 			index++;
// 			previous_i=i;
// 		}
// 	}
// }
// void test_boundary(bool * boundary,int buff_size)//testing whehter the boundary array works
// {
// 	for(int i = WIN_SIZE; i < buff_size-WIN_SIZE; i++)
// 	{
// 		if(boundary [i]== true)
// 		{
// 			std::cout<< i << std::endl;
// 		}
// 	}
// }
// void test_chunks(unsigned char ** chunks,int chunk_num)
// {
// 	for(int i=0;i<chunk_num;i++)
// 	{
// 		//std::cout<<"chunk["<<i<<"]is:"<<std::endl<<chunks[i]<<std::endl;
// 		std::cout<<chunks[i];
// 	}
// }

// void test_cdc( const char* file )//test whether the cdc function works
// {
// 	FILE* fp = fopen(file,"r" );
// 	if(fp == NULL ){
// 		perror("fopen error");
// 		return;
// 	}

// 	fseek(fp, 0, SEEK_END); // seek to end of file
// 	int file_size = ftell(fp); // get current file pointer
// 	fseek(fp, 0, SEEK_SET); // seek back to beginning of file

// 	unsigned char* buff = (unsigned char *)malloc((sizeof(unsigned char) * file_size ));
// 	if(buff == NULL)
// 	{
// 		perror("not enough space");
// 		fclose(fp);
// 		return;
// 	}

// 	fread(&buff[0],sizeof(unsigned char),file_size,fp);
// 	unsigned char* Chunk_array[MAX_NUM];
// 	int chunks_num=cdc(buff,file_size,Chunk_array);
// 	//test_chunks(Chunk_array,chunk_num);
//     free(buff);
//     return;
// }

// int main()//main function use to test whether the cdc function works.
// {
// 	test_cdc("LittlePrince.txt");
// 	return 0;
// }

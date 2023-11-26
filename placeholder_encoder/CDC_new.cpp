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
#include "utility.h"//all necessary function is here
using namespace std;
/*
hash calculation for CDC, using code from hw2, 5.1.
unsigned char *input:	input data
unsigned int pos:		positon of data 
*/
uint64_t hash_func(unsigned char *input, unsigned int pos)
{
	uint64_t hash = 0;//initialize hash value
	for(int i = 0; i < WIN_SIZE; i++)//WIN_SIZE is 16
	{
		hash += static_cast<uint64_t>(input[pos + WIN_SIZE - 1 - i]) * pow(PRIME, i + 1);//PRIME is 3
		/*
				input[pos]	input[pos+1]	input[pos+2]	...			input[pos+WIN_SIZE-1]
		hash =	*	   +	*		  +		*			+		+		*
				3^16		3^15			3^14			...			3
		*/
	}
	return hash;
}

/*
content defined chunking, using the code from hw2, 5.2 rolling hash
unsigned char *buff:	input data
unsigned int buff_size:	size of the input data
unsigned char **chunk:	array of pointer of chunks 
*/
int cdc(unsigned char *buff, unsigned int buff_size,  unsigned char ** chunk, uint64_t* hash, int* index, int previous_i)
{

	for(unsigned int i = previous_i; i < buff_size-WIN_SIZE; i++)//calculating cdc hash for input data, WIN_SIZE is 16
	{
		if(i == WIN_SIZE)//for the first iteration
		{
        	*hash = hash_func(buff, i);//call hash_func
		}
		else//from the next iteration
		{
			(*hash) = (*hash) * PRIME - static_cast<uint64_t>(buff[i - 1]) * pow(PRIME, WIN_SIZE + 1) + static_cast<uint64_t>(buff[i - 1 + WIN_SIZE]) * PRIME;
			//next_hash = 3*(current_hash-input[pos]*3^16+input[pos+WIN_SIZE])
		}
        if(((*hash) % MODULUS) == TARGET || (i == buff_size-WIN_SIZE-1))//MODULUS is 256, TARGET is 0
		// if hash%256 = 0 or i reach the end of buff
		{

			chunk[*index]=(unsigned char*)malloc(sizeof(unsigned char)* MAX_CHUNK);//create a chunk buffer for defined chunk, MAX_CHUNK is 8192
		   // since we don't know how big the chunk is, we just create a buffer at maxium size 8192
		   if(i<buff_size-WIN_SIZE-1)//if it is not the last buffer
			{
				memcpy(chunk[*index],buff+previous_i,i-previous_i);//copy the data (from previous boundary to current boundary)
			}
			else
			{
				memcpy(chunk[*index],buff+previous_i,buff_size-previous_i);//copy the data (from previous boundary to the end of buffer)
			}
			(*index)++;//chunk index increment
			return i;;//save the end boundary of last chunk as the start boundary of next chunk.
        }
    }
    //return index;//return how many chunk we defined
}

void whole_cdc(unsigned char *buff, unsigned int buff_size,  unsigned char ** chunk, int*index)
{
	uint64_t* hash;
	int previous_i=0;

	 while(previous_i<buff_size-WIN_SIZE)
	 {
		previous_i=cdc(buff, buff_size,chunk,hash,index, previous_i);
	 }

}
void test_chunks(unsigned char ** chunks,int chunk_num)//print out every defined chunk to check whether it works
{
	for(int i=0;i<chunk_num;i++)
	{
		std::cout<<"chunk["<<i<<"]is:"<<std::endl<<chunks[i]<<std::endl;
		//std::cout<<chunks[i];
	}
}

void test_cdc( const char* file )//Read the file, run the cdc, and print the defined chunk
{

	//-------------------------File Read-------------------------
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

	fread(&buff[0],sizeof(unsigned char),file_size,fp);
	//-----------------------------------------------------------
	unsigned char* Chunk_array[MAX_NUM];
	int *index;
	whole_cdc(buff,file_size,Chunk_array,index);
	test_chunks(Chunk_array,*index);//test cdc here
    free(buff);
    return;
}

int main()
{
	test_cdc("LittlePrince.txt");
	return 0;
}

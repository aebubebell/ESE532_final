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
#include<iostream> 
//parameter
#define WIN_SIZE 16
#define PRIME 3
#define MODULUS 256
#define TARGET 0

uint64_t hash_func(unsigned char *input, unsigned int pos)//hash for cdc
{
	uint64_t hash = 0;//initialize hash value
	for(int i = 0; i < WIN_SIZE; i++)//calculating hash value for window
	{
		hash += static_cast<uint64_t>(input[pos + WIN_SIZE - 1 - i]) * pow(PRIME, i + 1);
	}
	return hash;
}

void cdc(unsigned char *buff, unsigned int buff_size, bool * boundary)//content defined chunking
{
	uint64_t hash = 0;//initialize hash value

	for(unsigned int i = WIN_SIZE; i < buff_size-WIN_SIZE; i++)//calculating cdc hash for buffer
	{
		if(i == WIN_SIZE)//for the first time
		{
        	hash = hash_func(buff, i);
		}
		else//for other iteration
		{
			hash = hash * PRIME - static_cast<uint64_t>(buff[i - 1]) * pow(PRIME, WIN_SIZE + 1) + static_cast<uint64_t>(buff[i - 1 + WIN_SIZE]) * PRIME;//rolling hash
		}	
        if((hash % MODULUS) == TARGET)//if hash met particular value
		{
            boundary[i]=true;
        }
        else
        {
            boundary[i]=false;
        }
    }
}
/*
In cdc function, I created a bool array with the same size of buffer, 
whenver the cdc hash met target, the same index in arry is true,
otherwise is false. 
*/
void test_boundary(bool * boundary,int buff_size)//testing whehter the boundary array works
{
	for(unsigned int i = WIN_SIZE; i < buff_size-WIN_SIZE; i++)
	{
		if(boundary [i]== true)
		{
			std::cout<< i << std::endl;
		}
	}
}

void test_cdc( const char* file )//test whether the cdc function works
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
	cdc(buff, file_size,boundary);
	test_boundary(boundary,file_size);
    free(buff);
    return;
}

int main()//main function use to test whether the cdc function works.
{
	test_cdc("prince.txt");
	return 0;
}
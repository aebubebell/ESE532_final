/*
LZW encoding
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
#include<fstream>
using namespace std;

#define WIN_SIZE 16
#define PRIME 3
#define MODULUS 256
#define TARGET 0
#define MAX_CHUNK 4096
#define MAX_NUM 256

void LZWencoding(unsigned char* Chunk, int* encode_array,int& compress_length)
{
       std::map<std::string, int> dictionary;
    int nextCode = 256;
    int index = 0;

    // Initialize the dictionary with single character strings
    for (int i = 0; i < 256; i++) {
        dictionary[std::string(1, char(i))] = i;
    }

    std::string P = "";
    while (*Chunk != '\0') {
        char C = *Chunk;
        std::string PC = P + C;
        if (dictionary.find(PC) != dictionary.end()) {
            P = PC;
        } else {
            // Store the code for P in the compressed array
            encode_array[index++] = dictionary[P];

            // Add P + C to the dictionary
            dictionary[PC] = nextCode++;
            P = C;
        }

        Chunk++;
    }

    // Store the code for the last character in the compressed array
    encode_array[index++] = dictionary[P];
    compress_length = index;
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

	fread(&buff[0],sizeof(unsigned char),file_size,fp);
	unsigned char* Chunk_array[MAX_NUM];
	int chunks_num = cdc(buff, file_size,Chunk_array);
    //uint64_t* hash= (uint64_t*)malloc(sizeof(uint64_t)*chunks_num);
   //test_hash(Chunk_array,chunks_num,hash);
	std::unordered_map<string,uint32_t> chunktable;
    ofstream outputFile("compress.bin"); // Open a file named "output.txt" for writing
    if (outputFile.is_open())
    {
        //std::cout << "File open" << std::endl;
    }
    else
    {
        std::cerr << "Failed to open the file." << std::endl;
    }
	for(int i=0;i<chunks_num;i++)
	{
		int chunk_length= 4096;
        uint32_t header;
		header=cmd(Chunk_array[i],chunk_length,chunktable);
		if(header%2 ==0)
		{
            int* encode_array= (int*)malloc(sizeof(int)*MAX_NUM);
            int compress_length;
            LZWencoding(Chunk_array[0],encode_array,compress_length);
            outputFile << header;
            for(int j=0;j<compress_length;j+=2)
             {
                 uint8_t send=0;
                // cout <<"The encode data1 is:"<<*(encode_array+j)<<endl;
                // cout <<"The encode data2 is:"<<*(encode_array+j+1)<<endl;
                send = ((uint8_t)*(encode_array+j))>>4;
                outputFile << send;
                // cout <<"The first Byte is:"<<std::hex<<(int)send<<endl;
                send = (uint8_t)*(encode_array+j)<<4;
                send |=(uint8_t)*(encode_array+j+1)>>8;
                outputFile<<send;
                // cout<<"The second Byte is:"<<std::hex<<(int)send<<endl;
                send = (uint8_t)*(encode_array+j+1);
                // cout<<"The third Byte is:"<<std::hex<<(int)send<<endl;
             }
		}
		else
		{
            outputFile << header;
		}

	}
    free(buff);
    outputFile.close(); // Close the file when done
    return;
}
int main()
{
    test_lzw("LittlePrince.txt");
	return 0;
}

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
#define MAX_CHUNK 8192
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
        cout<<endl;
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
	std::unordered_map<string,uint32_t> chunktable;
    unsigned char* DRAM;
    int offset=0;
    DRAM = (unsigned char*)malloc(MAX_CHUNK*MAX_NUM);
	for(int i=0;i<chunks_num;i++)
	{
		int chunk_length=MAX_CHUNK;
        uint32_t header=0;
		header=cmd(Chunk_array[i],chunk_length,chunktable);
		if(header%2 ==0)
		{
            if(i==0)
            {
                cout<<"The chunk is:"<<endl<<Chunk_array[0]<<endl;
                chunk_length=0;
                int j=0;
                while(*(Chunk_array[0]+j) !='\0')
                {
                    chunk_length++;
                    j++;
                }
                cout<<"The chunks_length is:"<<chunk_length<<endl;
            }
            int* encode_array= (int*)malloc(sizeof(int)*MAX_CHUNK);
            int compress_length=0;
            LZWencoding(Chunk_array[i],encode_array,compress_length);
            if(i==0)
            {
                cout<<"The compress length is:"<<compress_length<<endl;
                cout<<"The encode_array is:"<<endl;
                for(int j=0;j<compress_length;j++)
                {
                    cout<<*encode_array+j<<" ";
                }
                cout<<endl;
            }
            header=(uint32_t)compress_length<<1;
            //cout<< "Write header:	"<<header<<"	to file"<<endl;
            memcpy(&DRAM[offset],&header,sizeof(uint32_t));
            offset +=sizeof(uint32_t);
            for(int j=0;j<compress_length;j+=2)
             {
                uint8_t send=0;
                send = *encode_array+j>>4;
                memcpy(&DRAM[offset],&send,sizeof(uint8_t));
                if(i==0)
                {
                    cout<<(int)send<<" ";
                }
                offset +=sizeof(uint8_t);
                send = *encode_array+j<<4;
                send |= *encode_array+j+1>>8;
                memcpy(&DRAM[offset],&send,sizeof(uint8_t));
                 if(i==0)
                {
                    cout<<(int)send<<" ";
                }
                offset +=sizeof(uint8_t);
                send = *encode_array+j+1;
                memcpy(&DRAM[offset],&send,sizeof(uint8_t));
                 if(i==0)
                {
                    cout<<(int)send<<" ";
                }
                offset +=sizeof(uint8_t);
             }
		}
		else
		{
            //cout<< "Write header:	"<<header<<"	to file"<<endl;
            memcpy(&DRAM[offset],&header,sizeof(uint32_t));
            offset +=sizeof(uint32_t);
		}
        
	}
    FILE *outfd = fopen("compress.bin", "wb");
	int bytes_written = fwrite(&DRAM[0], 1, offset, outfd);
	fclose(outfd);
    for(int i=0;i<chunks_num;i++)
    {
        free(Chunk_array[i]);
    }
    free(buff);
    free(DRAM);
  // Close the file when done
    return;
}
int main()
{
    test_lzw("LittlePrince.txt");
	return 0;
}

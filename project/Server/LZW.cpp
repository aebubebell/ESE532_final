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

/*
Lempel–Ziv–Welch encoding for unqiue chunk, code from geek_to_geek, 
URL:https://www.geeksforgeeks.org/lzw-lempel-ziv-welch-compression-technique/
unsigned char* Chunk:   the content of unique chunk
int *encode_array:      the encoded data output
int &compress_lengh:    the number of element in encode array
*/
void LZWencoding(unsigned char* Chunk, int* encode_array,int& compress_length)
{
    std::map<std::string, int> dictionary;//dictionary for encoding
    int nextCode = 256;//next code start from 256
    int index = 0;//the index of encode array

    // Initialize the dictionary with single character strings
    for (int i = 0; i < 256; i++) {
        dictionary[std::string(1, char(i))] = i;
        //every character is 8-bit, letting 256 unique character
    }
    std::string P = "";//initialize string P is empty string
    while (*Chunk != '\0') //if there is something in current Chunk pointer
    {
        char C = *Chunk;//char C equal to current pointer 
        std::string PC = P + C;//PC equals to P+C
        if (dictionary.find(PC) != dictionary.end()) //if PC is in the dictionary 
        {
            P = PC;//P equal to PC
        } 
        else //if PC is not in the dictionary
        {
            // Store the code for P in the compressed array
            encode_array[index++] = dictionary[P];

            // Add P + C to the dictionary
            dictionary[PC] = nextCode++;
            P = C;//P equal to C
        }
        Chunk++;

    }
    // Store the code for the last character in the compressed array
    encode_array[index++] = dictionary[P];
    compress_length = index;//compress length is the current index
}
void test_lzw( const char* file )//test lzw function
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
            int* encode_array= (int*)malloc(sizeof(int)*MAX_CHUNK);
            int compress_length=0;
            LZWencoding(Chunk_array[i],encode_array,compress_length);
            uint32_t compress_byte=ceil((12*(float)compress_length)/8);
            header=(uint32_t)compress_byte<<1;
            memcpy(&DRAM[offset],&header,sizeof(uint32_t));
            offset +=sizeof(uint32_t);
            for(int j=0;j<compress_length;j+=2)
            {
                if(compress_length-j == 1)
                {
                    uint8_t send=0;
                    send = *(encode_array+compress_length-1)>>4;
                    memcpy(&DRAM[offset],&send,sizeof(uint8_t));
                    offset +=sizeof(uint8_t);
                    send = *(encode_array+compress_length-1)<<4;
                    memcpy(&DRAM[offset],&send,sizeof(uint8_t));
                    offset +=sizeof(uint8_t);
                }
                else
                {
                    uint8_t send=0;
                    send = *(encode_array+j)>>4;
                    memcpy(&DRAM[offset],&send,sizeof(uint8_t));
                    offset +=sizeof(uint8_t);
                    send = *(encode_array+j)<<4;
                    send |= *(encode_array+j+1)>>8;
                    memcpy(&DRAM[offset],&send,sizeof(uint8_t));
                    offset +=sizeof(uint8_t);
                    send = *(encode_array+j+1);
                    memcpy(&DRAM[offset],&send,sizeof(uint8_t));
                    offset +=sizeof(uint8_t);
                }
            }
		}
		else
		{
            memcpy(&DRAM[offset],&header,sizeof(uint32_t));
            offset +=sizeof(uint32_t);
		}
        
	}
    FILE *outfd = fopen("compress.bin", "wb");
	int bytes_written = fwrite(&DRAM[0], 1, offset, outfd);
	fclose(outfd);
    cout<<"write "<<bytes_written<<" to file"<<endl;
    for(int i=0;i<chunks_num;i++)
    {
        free(Chunk_array[i]);
    }
    free(buff);
    free(DRAM);
  // Close the file when done
    return;
}
// int main()
// {
//     test_lzw("LittlePrince.txt");
// 	return 0;
// }

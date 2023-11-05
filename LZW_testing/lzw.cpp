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

void LZWencoding(unsigned char* Chunk, unsigned char* encode_array,int& compress_length)
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
void decompress(const unsigned char* compressedData, int compressedLength, unsigned char* decompressedOutput, int& decompressedLength) {
    std::vector<std::string> dictionary;
    int nextCode = 256;

    // Initialize the dictionary with single character strings
    for (int i = 0; i < 256; i++) {
        dictionary.push_back(std::string(1, char(i)));
    }

    int old = static_cast<int>(compressedData[0]);
    int outputIndex = 0;
    decompressedOutput[outputIndex++] = static_cast<unsigned char>(old);

    for (int i = 1; i < compressedLength; i++) {
        int newValue = static_cast<int>(compressedData[i]);
        std::string s;

        if (newValue >= dictionary.size()) {
            s = dictionary[old] + dictionary[old][0];
        } else {
            s = dictionary[newValue];
        }

        for (char c : s) {
            decompressedOutput[outputIndex++] = static_cast<unsigned char>(c);
        }

        // Add the new entry to the dictionary
        dictionary.push_back(dictionary[old] + s[0]);
        old = newValue;
    }

    decompressedLength = outputIndex;
}
void test_lzw( const char* file )//test whether the lzw function works
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
	bool* boundary = (bool*)malloc((sizeof(unsigned char)* file_size));
	unsigned char* Chunk_array[MAX_NUM];
	int chunks_num = cdc(buff, file_size, boundary);
	create_chunks(Chunk_array,boundary,buff,file_size);
	std::unordered_map<uint64_t,uint32_t> chunktable;
	int chunk_length= static_cast<int>(sizeof(Chunk_array[0])/sizeof(unsigned char));
    uint32_t header;
	header=cmd(Chunk_array[0],chunk_length,chunktable);
    unsigned char* encode_array= (unsigned char*)malloc(sizeof(unsigned char)*MAX_NUM);
    int compress_length;
    LZWencoding(Chunk_array[0],encode_array,compress_length);
    int decompress_length;
    decompress(encode_array,compress_length,Chunk_array[1],decompress_length);
    cout<<"The chunk content is:" <<Chunk_array[0]<<endl;
    cout<<"The compress length is"<< compress_length<<endl;
    cout<< "The Lzw encoding for this chunk is:";
    for(int i=0;i<compress_length;i++)
    {
        cout<<(int)encode_array[i]<<"   ";
    }
    cout << endl;
    cout << "The decompress data is:"<<endl<<Chunk_array[1];
    free(buff);
    return;
}
int main()
{
    test_lzw("LittlePrince.txt");
	return 0;
}

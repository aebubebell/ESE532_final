#include <iostream>
#include <vector>
#include <unordered_map>
#include <stdlib.h>
#include <string>
#include "LZW.h"
#include<bits/stdc++.h>
#include <hls_stream.h>
using namespace std;

#define WIN_SIZE 16
#define PRIME 3
#define MODULUS 256
#define TARGET 0
#define MAX_CHUNK 8192
#define MAX_NUM 256

uint64_t hash_func(unsigned char *input, unsigned int pos)//hash for cdc
{
	uint64_t hash = 0;//initialize hash value
	for(int i = 0; i < WIN_SIZE; i++)//calculating hash value for window
	{
		hash += static_cast<uint64_t>(input[pos + WIN_SIZE - 1 - i]) * pow(PRIME, i + 1);
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
			hash = hash * PRIME - static_cast<uint64_t>(buff[i - 1]) * pow(PRIME, WIN_SIZE + 1) + static_cast<uint64_t>(buff[i - 1 + WIN_SIZE]) * PRIME;
			//next_hash = 3*(current_hash-input[pos]*3^16+input[pos+WIN_SIZE])
		}
        if((hash % MODULUS) == TARGET || (i == buff_size-WIN_SIZE-1))//MODULUS is 256, TARGET is 0
		// if hash%256 = 0 or i reach the end of buff
		{
           chunk[index]=(unsigned char*)malloc(MAX_CHUNK);//create a chunk buffer for defined chunk, MAX_CHUNK is 8192
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
    return index;//return how many chunk we defined
}

static const uint32_t K256[] =
{
    0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5,
    0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5,
    0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3,
    0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174,
    0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC,
    0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
    0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7,
    0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967,
    0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13,
    0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
    0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3,
    0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
    0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5,
    0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3,
    0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208,
    0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2
};

#define ROTATE(x,y)  (((x)>>(y)) | ((x)<<(32-(y))))
#define Sigma0(x)    (ROTATE((x), 2) ^ ROTATE((x),13) ^ ROTATE((x),22))
#define Sigma1(x)    (ROTATE((x), 6) ^ ROTATE((x),11) ^ ROTATE((x),25))
#define sigma0(x)    (ROTATE((x), 7) ^ ROTATE((x),18) ^ ((x)>> 3))
#define sigma1(x)    (ROTATE((x),17) ^ ROTATE((x),19) ^ ((x)>>10))

#define Ch(x,y,z)    (((x) & (y)) ^ ((~(x)) & (z)))
#define Maj(x,y,z)   (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

/* Avoid undefined behavior                    */
/* https://stackoverflow.com/q/29538935/608639 */
uint32_t B2U32(uint8_t val, uint8_t sh)
{
    return ((uint32_t)val) << sh;
}

/* Process multiple blocks. The caller is responsible for setting the initial */
/*  state, and the caller is responsible for padding the final block.        */
void sha256_process(uint32_t state[8], const uint8_t data[], uint32_t length)
{
    uint32_t a, b, c, d, e, f, g, h, s0, s1, T1, T2;
    uint32_t X[16], i;

    size_t blocks = length / 64;
    while (blocks--)
    {
        a = state[0];
        b = state[1];
        c = state[2];
        d = state[3];
        e = state[4];
        f = state[5];
        g = state[6];
        h = state[7];

        for (i = 0; i < 16; i++)
        {
            X[i] = B2U32(data[0], 24) | B2U32(data[1], 16) | B2U32(data[2], 8) | B2U32(data[3], 0);
            data += 4;

            T1 = h;
            T1 += Sigma1(e);
            T1 += Ch(e, f, g);
            T1 += K256[i];
            T1 += X[i];

            T2 = Sigma0(a);
            T2 += Maj(a, b, c);

            h = g;
            g = f;
            f = e;
            e = d + T1;
            d = c;
            c = b;
            b = a;
            a = T1 + T2;
        }

        for (; i < 64; i++)
        {
            s0 = X[(i + 1) & 0x0f];
            s0 = sigma0(s0);
            s1 = X[(i + 14) & 0x0f];
            s1 = sigma1(s1);

            T1 = X[i & 0xf] += s0 + s1 + X[(i + 9) & 0xf];
            T1 += h + Sigma1(e) + Ch(e, f, g) + K256[i];
            T2 = Sigma0(a) + Maj(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + T1;
            d = c;
            c = b;
            b = a;
            a = T1 + T2;
        }

        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;
        state[4] += e;
        state[5] += f;
        state[6] += g;
        state[7] += h;
    }
}
uint32_t cmd(unsigned char* chunk,std::unordered_map<string,uint32_t>& chunktable)
{
	string hash;//create a string to store 256 bit hash value
	//initialize the state,which will carry the output later
	uint32_t state[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
	sha256_process(state,chunk,64);//call the sha256 function
	//stack all the state to hash string
	for(int i =0;i<8;i++)
	{
		hash += to_string(state[i]);
	}
	//std::cout<< "The hash for Chunks is"<<std::endl<<hash<<std::endl;
	/*
	keyword "auto", it can automatcially infer the type of it base on the type of the expression on the right
	in this case, it is the iterator type of chunktable map, used to access and operate elements in that map
	chunktable.find(hash) will return an iterator pointing to the element in the chunktable whose key is hash
	if it is found, it will point to it
	if not, it will be equal to chunktable.end(),pointing at the end of map
	*/
	/*
	unorder map chunktable looks like:
	--------------------------
	|	key		|	value	|
	--------------------------
	|	hash0	|	size0	|
	--------------------------
	|	hash1	|	size1	|
	*/
	auto it=chunktable.find(hash);
	if(it == chunktable.end())//if the hash key is not found in chunktable
	{
		uint32_t newIndex=(uint32_t)chunktable.size();//use the current map size as index,which increase from 0 to number of unique chunk,
		chunktable[hash]=newIndex;//insert that hash as key and the index as value into hashtable
		return newIndex << 1;//return that value and left shift so that the first bit is 0
	}
	else//if the hash key is found in chunktable
	{
		return (it->second << 1) |1u;//access the value of the element through iterator,which is the index of the match chunk in chunktable
		//left shift it then or with 1, which make the first bit is 1
	}
}
//void LZWencoding(unsigned char* Chunk, int* encode_array,int& compress_length)
//{
//       std::map<std::string, int> dictionary;
//    int nextCode = 256;
//    int index = 0;
//
//    // Initialize the dictionary with single character strings
//    for (int i = 0; i < 256; i++) {
//        dictionary[std::string(1, char(i))] = i;
//    }
//
//    std::string P = "";
//    while (*Chunk != '\0') {
//        char C = *Chunk;
//        std::string PC = P + C;
//        if (dictionary.find(PC) != dictionary.end()) {
//            P = PC;
//        } else {
//            // Store the code for P in the compressed array
//            encode_array[index++] = dictionary[P];
//
//            // Add P + C to the dictionary
//            dictionary[PC] = nextCode++;
//            P = C;
//        }
//
//        Chunk++;
//    }
//
//    // Store the code for the last character in the compressed array
//    encode_array[index++] = dictionary[P];
//    compress_length = index;
//}

//****************************************************************************************************************
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
	int offset=0;
	unsigned char* DRAM;
	DRAM = (unsigned char*)malloc(MAX_CHUNK*MAX_NUM);
	unsigned char* Chunk_array[MAX_NUM];
	unsigned int *chunk_length=(unsigned int*)malloc(sizeof(int)*MAX_NUM);
	int chunks_num = cdc(buff, file_size,Chunk_array,chunk_length);
	std::unordered_map<string,uint32_t> chunktable;
	for(int i=0;i<chunks_num;i++)
	{
        uint32_t header=0;
		header=cmd(Chunk_array[i],chunktable);
		if(header%2 ==0)
		{
            int* encode_array= (int*)malloc(sizeof(int)*MAX_CHUNK);
               for(int i = 0; i < MAX_CHUNK; i++)
               {
                   encode_array[i] = 0;
               }
            int *compress_len=(int*)malloc(sizeof(int));
            //cout<<"Are you serious ?"<<endl;
            hardware_encoding(Chunk_array[i],encode_array,chunk_length+i,compress_len);
            //LCS(Chunk_array[i],encode_array,chunk_length+i,compress_len);
            int compress_length = compress_len[0];
            cout<< "the encode array is :";
            			for(int j =0;j<compress_length;j++)
            			{
            				cout<<*(encode_array+j)<<" ";
            			}
            			cout<< endl;
            cout<<"The compress length is: "<<compress_length<<endl;
            //LZWencoding(Chunk_array[i],encode_array,compress_leng);
			uint32_t compress_byte=ceil((12*(float)compress_length)/8);
            header=(uint32_t)compress_byte<<1;
            memcpy(&DRAM[offset],&header,sizeof(uint32_t));
            offset +=sizeof(uint32_t);

			for(int j=0;j<compress_length;j+=2)
            {
                if(compress_length-j == 1)
                {
                    uint8_t send=0;
                    //cout<< "encode array:"<<*encode_array+compress_length-1<<" to mem"<<endl;
                    send = *(encode_array+compress_length-1)>>4;
                    //cout<< "send data:"<<(int)send<<" to mem"<<endl;
                    memcpy(&DRAM[offset],&send,sizeof(uint8_t));
                    offset +=sizeof(uint8_t);
                    send = *(encode_array+compress_length-1)<<4;
                    //cout<< "send data:"<<(int)send<<" to mem"<<endl;
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
			compress_length=0;
			free(encode_array);
	}
		else
		{
            memcpy(&DRAM[offset],&header,sizeof(uint32_t));
            offset +=sizeof(uint32_t);
		}

	}
	FILE *outfd = fopen("output_cpu.bin", "wb");
	int compress_bytes_written = fwrite(&DRAM[0], 1, offset, outfd);
		printf("write file with %d\n", compress_bytes_written);
		fclose(outfd);
}
int main()
 {
    //test_lzw("LittlePrince.txt");
	const char *content= "WSDSFHDSHFUSDHFSDUSI";
	unsigned char *chunk=(unsigned char*)malloc(sizeof(unsigned char)*100);

	//unsigned char chunk[100];
	int* encode_array= (int*)malloc(sizeof(int)*MAX_CHUNK);
	memcpy(chunk,content,100);
	int *compress_len=(int*)malloc(sizeof(int));
	unsigned int *chunk_length= (unsigned int*)malloc(sizeof(unsigned int));
	*chunk_length=10;
	hardware_encoding(chunk,encode_array,chunk_length,compress_len);
	int compress_length = compress_len[0];
	            cout<< "the encode array is :";
	            			for(int j =0;j<compress_length;j++)
	            			{
	            				cout<<*(encode_array+j)<<" ";
	            			}
	            			cout<< endl;
	            cout<<"The compress length is: "<<compress_length<<endl;
 	return 0;
 }

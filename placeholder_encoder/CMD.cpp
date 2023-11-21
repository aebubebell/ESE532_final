/*
Chunk matching for deduplication
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
#include "stopwatch.h"
using namespace std;

/*
Chunk matching for deduplication, write the code myself
unsigned char *chunk:	chunk content
int chunk_length:		the length of chunk, since evey character in chunk is 1 byte, it is also the size of chunk
std::unordered_map<string,uint32_t>& chunktable:	chunktable that store all the unqiue chunk 
*/
uint32_t cmd(unsigned char* chunk,int chunk_length,std::unordered_map<string,uint32_t>& chunktable)
{
	string hash;//create a string to store 256 bit hash value
	//initialize the state,which will carry the output later
	uint32_t state[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
	sha256_process(state,chunk,8192);//call the sha256 function
	//stack all the state to hash string
	for(int i =0;i<8;i++)
	{
		hash += to_string(state[i]);
	}
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
void test_hash(unsigned char ** Chunks, int chunk_num)//test the sha256 function
{
    for (int i=0;i<chunk_num;i++)
    {
       string hash;
	   uint32_t state[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
	   sha256_process(state,Chunks[i],8192);
	   for(int j =0;j<8;j++)
	   {
		hash += to_string(state[j]);
	   }
        std::cout<< "The hash for Chunks ["<<i<<"] is"<<std::endl<<hash<<std::endl;
    }
}
void test_cmd( const char* file )//test the cmd function
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
	int chunks_num = cdc(buff,file_size,Chunk_array);
    test_hash(Chunk_array,chunks_num);
	std::unordered_map<string,uint32_t> chunktable;
	for(int i=0;i<chunks_num;i++)
	{
		int chunk_length= MAX_CHUNK;
		uint32_t header=cmd(Chunk_array[i],chunk_length,chunktable);
		if(header%2==0)
		{
			std::cout<<"Chunk ["<<i<<"] is an unique chunk"<<std::endl;
		}
		else
		{
			uint32_t val = header>>1;
			std::cout<<"Chunk ["<<i<<"] is the same as Chunk ["<<val<<"]"<<std::endl;
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

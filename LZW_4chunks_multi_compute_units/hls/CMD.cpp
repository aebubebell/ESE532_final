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
#include <thread>
#include <vector>
#include "utility.h"
#include "Utilities.h"
using namespace std;
uint64_t bad_hash(unsigned char * chunk,int chunk_length)
{
    uint64_t hash=0;
    for(int i=0;i<chunk_length;i++)
    {
        hash+=static_cast<uint64_t>(chunk[i]);
    }
    return hash;
}
// void SHA256(unsigned char**chunk,string &hash[MAX_NUM],int chunks_num)
// {
// 	for(int i=0;i<chunks_num;i++)
// 	{
// 		uint32_t state[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
// 		sha256_process_arm(state,chunk,64);
// 		for(int j =0;j<8;j++)
// 		{
// 			hash[i] += to_string(state[j]);
// 		}
// 	}
// }
// void dedup(string &hash[MAX_NUM],int chunks_num,uint32_t &header[MAX_NUM])
// {
// 	unordered_map<string,uint32_t> chunk_table;
// 	for(int i=0;i<chunks_num;i++)
// 	{
// 		auto it=chunktable.find(hash[i]);
// 		if(it == chunktable.end())//if the hash key is not found in chunktable
// 		{
// 			uint32_t newIndex=(uint32_t)chunktable.size();//use the current map size as index,which increase from 0 to number of unique chunk,
// 			chunktable[hash[i]]=newIndex;//insert that hash as key and the index as value into hashtable
// 			header[i]=newIndex << 1;//return that value and left shift so that the first bit is 0
// 		}
// 		else//if the hash key is found in chunktable
// 		{
// 			header[i]= (it->second << 1) |1u;//access the value of the element through iterator,which is the index of the match chunk in chunktable
// 			//left shift it then or with 1, which make the first bit is 1
// 		}
// 	}
// }
void SHA256(unsigned char*content,string &hash)
{
	uint32_t state[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
	sha256_process_arm(state,content,256);//call the sha256 function
	for(int i =0;i<8;i++)
		{
			hash += to_string(state[i]);
		}
}
void cmd(unsigned char** chunk,uint32_t *header,int chunks_num)
{
	string hash[chunks_num];
	//vector<thread>ths;
	for(int i=0;i<chunks_num;i++)
	{
		// ths.push_back(thread(&SHA256,chunk[i],ref(hash[i])));
		// ths.push_back(thread(&SHA256,chunk[i+1],ref(hash[i+1])));
		// ths.push_back(thread(&SHA256,chunk[i+2],ref(hash[i+2])));
		// ths.push_back(thread(&SHA256,chunk[i+3],ref(hash[i+3])));
		// pin_thread_to_cpu(ths[i],0);
		// pin_thread_to_cpu(ths[i+1],1);
		// pin_thread_to_cpu(ths[i+2],2);
		// pin_thread_to_cpu(ths[i+3],3);
		// ths[i].join();
		// ths[i+1].join();
		// ths[i+2].join();
		// ths[i+3].join();
		// for(auto &th:ths)
		// {
		// 	th.join();
		// }
		SHA256(chunk[i],hash[i]);
		// uint32_t state[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
		// sha256_process_arm(state,chunk[i],64);//call the sha256 function
		// for(int j =0;j<8;j++)
		// {
		// 	hash[i] += to_string(state[j]);
		// }
	}
	std::unordered_map<string,uint32_t> chunktable;
	for(int i=0;i<chunks_num;i++)
	{
		auto it=chunktable.find(hash[i]);
		if(it == chunktable.end())//if the hash key is not found in chunktable
		{
			uint32_t newIndex=(uint32_t)chunktable.size();//use the current map size as index,which increase from 0 to number of unique chunk,
			chunktable[hash[i]]=newIndex;//insert that hash as key and the index as value into hashtable
			*(header+i)=newIndex << 1;//return that value and left shift so that the first bit is 0
		}
		else//if the hash key is found in chunktable
		{
			*(header+i)=(it->second << 1) |1u;//access the value of the element through iterator,which is the index of the match chunk in chunktable
			//left shift it then or with 1, which make the first bit is 1
		}
	}
	//string hash;//create a string to store 256 bit hash value
	//initialize the state,which will carry the output later
	//uint32_t state[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
	//sha256_process_arm(state,chunk,64);//call the sha256 function
	//hash = neon_sha(chunk,chunk_length+1);
	//stack all the state to hash string
	// for(int i =0;i<8;i++)
	// {
	// 	hash += to_string(state[i]);
	// }
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
	// auto it=chunktable.find(hash);
	// if(it == chunktable.end())//if the hash key is not found in chunktable
	// {
	// 	uint32_t newIndex=(uint32_t)chunktable.size();//use the current map size as index,which increase from 0 to number of unique chunk,
	// 	chunktable[hash]=newIndex;//insert that hash as key and the index as value into hashtable
	// 	return newIndex << 1;//return that value and left shift so that the first bit is 0
	// }
	// else//if the hash key is found in chunktable
	// {
	// 	return (it->second << 1) |1u;//access the value of the element through iterator,which is the index of the match chunk in chunktable
	// 	//left shift it then or with 1, which make the first bit is 1
	// }
}
// void test_hash(unsigned char ** Chunks, int chunk_num,uint64_t* hash)
// {
//     int chunk_length;
//     for (int i=0;i<chunk_num;i++)
//     {
//         chunk_length=static_cast<int>(sizeof(Chunks[i])/sizeof(unsigned char));
//         hash[i]=bad_hash(Chunks[i],chunk_length);
//         std::cout<< "The bad hash for Chunks ["<<i<<"] is"<<std::endl<<hash[i]<<std::endl;
//     }
// }
// void test_cmd( const char* file )//test whether the cdc function works
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
// 	unsigned char* Chunk_array[MAX_NUM];
// 	int chunks_num = cdc(buff, file_size,Chunk_array);
//     //test_hash(Chunk_array,chunks_num,hash);
// 	std::unordered_map<string,uint32_t> chunktable;
// 	for(int i=0;i<chunks_num;i++)
// 	{
// 		int chunk_length= 4096;
// 		//std::cout<<cmd(Chunk_array[i],chunk_length,chunktable)<<std::endl;
// 		uint32_t header=cmd(Chunk_array[i],chunk_length,chunktable);
// 		std::cout<<header<<std::endl;
// 	}
// 	free(buff);
//     return;
// }
// int main()
// {
//     test_cmd("LittlePrince.txt");
// 	return 0;
// }

#include "LZW.h"
#include <iostream>
#include <hls_stream.h>
unsigned int my_hash(unsigned long key)
{
    key &= 0xFFFFF; // make sure the key is only 20 bits

    unsigned int hashed = 0;

hash_loop:for(int i = 0; i < 20; i++)
    {

        hashed += (key >> i)&0x01;
        hashed += hashed << 10;
        hashed ^= hashed >> 6;
    }
    hashed += hashed << 3;
    hashed ^= hashed >> 11;
    hashed += hashed << 15;
    return hashed & 0x7FFF;          // hash output is 15 bits
    //return hashed & 0xFFF;
}

void hash_lookup(unsigned long* hash_table, unsigned int key, bool* hit, unsigned int* result)
{
    //std::cout << "hash_lookup():" << std::endl;
    key &= 0xFFFFF; // make sure key is only 20 bits

    unsigned long lookup = hash_table[my_hash(key)];

    // [valid][value][key]
    unsigned int stored_key = lookup&0xFFFFF;       // stored key is 20 bits
    unsigned int value = (lookup >> 20)&0xFFF;      // value is 12 bits
    unsigned int valid = (lookup >> (20 + 12))&0x1; // valid is 1 bit

    if(valid && (key == stored_key))
    {
        *hit = 1;
        *result = value;
        //std::cout << "\thit the hash" << std::endl;
        //std::cout << "\t(k,v,h) = " << key << " " << value << " " << my_hash(key) << std::endl;
    }
    else
    {
        *hit = 0;
        *result = 0;
        //std::cout << "\tmissed the hash" << std::endl;
    }
}

void hash_insert(unsigned long* hash_table, unsigned int key, unsigned int value, bool* collision)
{
    //std::cout << "hash_insert():" << std::endl;
    key &= 0xFFFFF;   // make sure key is only 20 bits
    value &= 0xFFF;   // value is only 12 bits

    unsigned long lookup = hash_table[my_hash(key)];
    unsigned int valid = (lookup >> (20 + 12))&0x1;

    if(valid)
    {
        *collision = 1;
        //std::cout << "\tcollision in the hash" << std::endl;
    }
    else
    {
        hash_table[my_hash(key)] = (1UL << (20 + 12)) | (value << 20) | key;
        *collision = 0;
        //std::cout << "\tinserted into the hash table" << std::endl;
        //std::cout << "\t(k,v,h) = " << key << " " << value << " " << my_hash(key) << std::endl;
    }
}
//****************************************************************************************************************
typedef struct
{
    // Each key_mem has a 9 bit address (so capacity = 2^9 = 512)
    // and the key is 20 bits, so we need to use 3 key_mems to cover all the key bits.
    // The output width of each of these memories is 64 bits, so we can only store 64 key
    // value pairs in our associative memory map.

    unsigned long upper_key_mem[512]; // the output of these  will be 64 bits wide (size of unsigned long).
    unsigned long middle_key_mem[512];
    unsigned long lower_key_mem[512];
    unsigned int value[64];    // value store is 64 deep, because the lookup mems are 64 bits wide
    unsigned int fill;         // tells us how many entries we've currently stored
} assoc_mem;

// cast to struct and use ap types to pull out various feilds.

void assoc_insert(assoc_mem* mem,  unsigned int key, unsigned int value, bool* collision)
{
    //std::cout << "assoc_insert():" << std::endl;
    key &= 0xFFFFF; // make sure key is only 20 bits
    value &= 0xFFF;   // value is only 12 bits

    if(mem->fill < 64)
    {
        mem->upper_key_mem[(key >> 18)%512] |= (1 << mem->fill);  // set the fill'th bit to 1, while preserving everything else
        mem->middle_key_mem[(key >> 9)%512] |= (1 << mem->fill);  // set the fill'th bit to 1, while preserving everything else
        mem->lower_key_mem[(key >> 0)%512] |= (1 << mem->fill);   // set the fill'th bit to 1, while preserving everything else
        mem->value[mem->fill] = value;
        mem->fill++;
        *collision = 0;
        //std::cout << "\tinserted into the assoc mem" << std::endl;
        //std::cout << "\t(k,v) = " << key << " " << value << std::endl;
    }
    else
    {
        *collision = 1;
        //std::cout << "\tcollision in the assoc mem" << std::endl;
    }
}

void assoc_lookup(assoc_mem* mem, unsigned int key, bool* hit, unsigned int* result)
{
    //std::cout << "assoc_lookup():" << std::endl;
    key &= 0xFFFFF; // make sure key is only 20 bits

    unsigned int match_high = mem->upper_key_mem[(key >> 18)%512];
    unsigned int match_middle = mem->middle_key_mem[(key >> 9)%512];
    unsigned int match_low  = mem->lower_key_mem[(key >> 0)%512];

    unsigned int match = match_high & match_middle & match_low;

    unsigned int address = 0;
binary_encode_loop:for(; address < 64; address++)
    {
#pragma HLS unroll
        if((match >> address) & 0x1)
        {
            break;
        }
    }
    if(address != 64)
    {
        *result = mem->value[address];
        *hit = 1;
        //std::cout << "\thit the assoc" << std::endl;
        //std::cout << "\t(k,v) = " << key << " " << *result << std::endl;
    }
    else
    {
        *hit = 0;
        //std::cout << "\tmissed the assoc" << std::endl;
    }
}
//****************************************************************************************************************
void insert(unsigned long* hash_table, assoc_mem* mem, unsigned int key, unsigned int value, bool* collision)
{
    hash_insert(hash_table, key, value, collision);
    if(*collision)
    {
        assoc_insert(mem, key, value, collision);
    }
}

void lookup(unsigned long* hash_table, assoc_mem* mem, unsigned int key, bool* hit, unsigned int* result)
{
    hash_lookup(hash_table, key, hit, result);
    if(!*hit)
    {
        assoc_lookup(mem, key, hit, result);
    }
}
//****************************************************************************************************************
void hardware_encoding(unsigned char *Chunk, int *encode_array,unsigned int *chunk_length, int* compress_length)
{
#pragma HLS INTERFACE m_axi depth=64 port=Chunk bundle=p0
#pragma HLS INTERFACE m_axi depth=64 port=encode_array bundle=p1
#pragma HLS INTERFACE m_axi port=compress_length bundle=p2
#pragma HLS INTERFACE m_axi port=chunk_length bundle=p3
    // create hash table and assoc mem
    unsigned long hash_table[CAPACITY];
    assoc_mem my_assoc_mem;
    int index =0;

    // make sure the memories are clear
Clear_hashtable_loop:for(int i = 0; i < CAPACITY; i++)
    {
#pragma HLS unroll factor=4
        hash_table[i] = 0;
    }
    my_assoc_mem.fill = 0;
Clear_assoc_mem_loop:for(int i = 0; i < 512; i++)
    {
#pragma HLS unroll
        my_assoc_mem.upper_key_mem[i] = 0;
        my_assoc_mem.middle_key_mem[i] = 0;
        my_assoc_mem.lower_key_mem[i] = 0;
    }

    int next_code = 256;


    unsigned int prefix_code = *Chunk;
    unsigned int code = 0;
    unsigned char next_char = 0;

Encoding_loop:for(unsigned int i =0;i<*chunk_length;i++)
    {

		if(i != *chunk_length - 1)
		{
			next_char = *(Chunk+i+1);
			bool hit = 0;
			lookup(hash_table, &my_assoc_mem, (prefix_code << 8) + next_char, &hit, &code);
			if(!hit)
			{
				encode_array[index++]=prefix_code;
				bool collision = 0;
			    insert(hash_table, &my_assoc_mem, (prefix_code << 8) + next_char, next_code, &collision);
			    next_code += 1;
			    prefix_code = next_char;
			}
			else
			{
				prefix_code = code;
			}
		}
		else
		{
			encode_array[index++]=prefix_code;
			*compress_length=index;
		}
    }
}
//void hardware_encoding_stream(hls::stream<unsigned char>&chunkStream,hls::stream<int>&encodeStream,unsigned int *chunk_length,int* compress_length)
//{
////#pragma HLS INTERFACE m_axi port= Chunk bundle=p0
////#pragma HLS INTERFACE m_axi port= encode_array bundle=p1
////#pragma HLS INTERFACE m_axi port= compress_length bundle=p2
//    // create hash table and assoc mem
//    unsigned long hash_table[CAPACITY];
//    assoc_mem my_assoc_mem;
//    int index =0;
//
//    //unsigned int chunk_length = chunk_lengthStream.read();
//    // make sure the memories are clear
//Clear_hashtable_loop:for(int i = 0; i < CAPACITY; i++)
//    {
//#pragma HLS unroll factor=4
//        hash_table[i] = 0;
//    }
//    my_assoc_mem.fill = 0;
//Clear_assoc_mem_loop:for(int i = 0; i < 512; i++)
//    {
//#pragma HLS unroll
//        my_assoc_mem.upper_key_mem[i] = 0;
//        my_assoc_mem.middle_key_mem[i] = 0;
//        my_assoc_mem.lower_key_mem[i] = 0;
//    }
//
//    // init the memories with the first 256 codes
//Init_mem_loop:for(unsigned long i = 0; i < 256; i++)
//    {
//#pragma HLS pipeline II=1
//        bool collision = 0;
//        unsigned int key = (i << 8) + 0UL; // lower 8 bits are the next char, the upper bits are the prefix code
//        insert(hash_table, &my_assoc_mem, key, i, &collision);
//    }
//    int next_code = 256;
//
//
//    unsigned int prefix_code = chunkStream.read();
//
//    unsigned int code = 0;
//    unsigned char next_char = 0;
//
//Encoding_loop:for(unsigned int i =0;i<*(chunk_length)-1;i++)
//    {
//        next_char = chunkStream.read();
//        bool hit = 0;
//        lookup(hash_table, &my_assoc_mem, (prefix_code << 8) + next_char, &hit, &code);
//        if(!hit)
//        {
//        	//encode_array[index++]=prefix_code;
//        	encodeStream.write(prefix_code);
//        	index++;
//            bool collision = 0;
//            insert(hash_table, &my_assoc_mem, (prefix_code << 8) + next_char, next_code, &collision);
//            if(collision)
//            {
//                return;
//            }
//            next_code += 1;
//
//            prefix_code = next_char;
//        }
//        else
//        {
//            prefix_code = code;
//        }
//    }
//	//encode_array[index++]=prefix_code;
//	index++;
//	encodeStream.write(prefix_code);
//    *compress_length=index;
//	//compress_lengthStream.write(index);
//}
//void Load(unsigned char *Chunk,unsigned int *chunk_length,hls::stream<unsigned char>&chunkStream,hls::stream<unsigned char>&chunk_lengthStream)
//{
//	for(int i =0;Chunk[i]!='\0';i++)
//	{
//		chunkStream.write(Chunk[i]);
//	}
//	chunk_lengthStream.write(*chunk_length);
//}
//void Store(int *encode_array,hls::stream<int>&encodeStream)
//{
//
//	for(int i =0;encode_array[i-1]!='\0';i++)
//	{
//		encode_array[i]=encodeStream.read();
//	}
//}
//void LCS(unsigned char *Chunk, int *encode_array,unsigned int *chunk_length,int* compress_length)
//{
//#pragma HLS INTERFACE m_axi port=Chunk bundle=p0
//#pragma HLS INTERFACE m_axi port=encode_array bundle=p1
//#pragma HLS INTERFACE m_axi port=compress_length bundle=p2
//#pragma HLS INTERFACE m_axi port=chunk_length bundle=p3
//
//	hls::stream<unsigned char>chunkStream("Chunk_Stream");
//	hls::stream<unsigned int>chunk_lengthStream("Chunk_Length_Stream");
//	hls::stream<int>encodeStream("encode_Stream");
//	hls::stream<int>compress_lengthStream("Compress_Length_Stream");
//
//#pragma HLS STREAM variable=chunkStream depth=64
//#pragma HLS STREAM variable=chunk_lengthStream depth=1
//#pragma HLS STREAM variable=encodeStream depth=64
//#pragma HLS STREAM variable=compress_lengthStream depth=1
//
//#pragma HLS dataflow
//	//unsigned int chu_length = *chunk_length;
//	Load(Chunk,chunk_length,chunkStream,);
//	hardware_encoding_stream(chunkStream,encodeStream,chunk_length,compress_length);
//	//int com_length = *(compress_length);
//	Store(encode_array,encodeStream);
//}

/*
SHA hash computing(currently using a bad hash)
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
#include "utility.h"

uint64_t bad_hash(unsigned char * chunk,int chunk_length)
{
    uint64_t hash=0;
    for(int i=0;i<chunk_length;i++)
    {
        hash+=static_cast<uint64_t>chunk[i];
    }
    return hash
}
void test_hash(unsigned char ** Chunks, int chunk_num,uint64_t* hash)
{
    int chunk_length;
    for (int i=0;i<chunk_num;i++)
    {
        chunk_length=static_cast<int>(sizeof(Chunks[i])/sizeof(unsigned char));
        hash[i]=bad_hash(Chunks[i],chunk_length);
        std::cout<< "The bad hash for Chunks ["<<i<<"] is"<<std::endl<<hash[i]<<std::endl;
    }
}
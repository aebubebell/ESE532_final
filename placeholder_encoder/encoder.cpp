/*
placeholder_encoder for software implementation
from file to file
Zhiye Zhang
11/20/2001
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
#include<bits/stdc++.h>
using namespace std;

int main()
{
    //-------------------------File Read-------------------------
	FILE* fp = fopen("LittlePrince.txt","r" );
	if(fp == NULL ){
		perror("fopen error");
		return 0;
	}

	fseek(fp, 0, SEEK_END); // seek to end of file
	int file_size = ftell(fp); // get current file pointer
	fseek(fp, 0, SEEK_SET); // seek back to beginning of file

	unsigned char* buff = (unsigned char *)malloc((sizeof(unsigned char) * file_size ));	
	if(buff == NULL)
	{
		perror("not enough space");
		fclose(fp);
		return 0;
	}

	fread(&buff[0],sizeof(unsigned char),file_size,fp);
    //-----------------------------------------------------------
    stopwatch CDC_timer, CMD_timer, LZW_timer,send_data_timer,total_timer;
    total_timer.start();
    unsigned char* Chunk_array[MAX_NUM];
    CDC_timer.start();
	int chunks_num = cdc(buff, file_size,Chunk_array);
    CDC_timer.stop();
	std::unordered_map<string,uint32_t> chunktable;
    unsigned char* DRAM;
    int offset=0;
    DRAM = (unsigned char*)malloc(MAX_CHUNK*MAX_NUM);
	for(int i=0;i<chunks_num;i++)
	{
		int chunk_length=MAX_CHUNK;
        uint32_t header=0;
        CMD_timer.start();
		header=cmd(Chunk_array[i],chunk_length,chunktable);
        CMD_timer.stop();
		if(header%2 ==0)
		{
            int* encode_array= (int*)malloc(sizeof(int)*MAX_CHUNK);
            int compress_length=0;
            LZW_timer.start();
            LZWencoding(Chunk_array[i],encode_array,compress_length);
            LZW_timer.stop();
            uint32_t compress_byte=ceil((12*(float)compress_length)/8);
            header=(uint32_t)compress_byte<<1;
            memcpy(&DRAM[offset],&header,sizeof(uint32_t));
            offset +=sizeof(uint32_t);
            send_data_timer.start();
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
            send_data_timer.stop();
		}
		else
		{
            send_data_timer.start();
            memcpy(&DRAM[offset],&header,sizeof(uint32_t));
            offset +=sizeof(uint32_t);
            send_data_timer.stop();
		}
        
	}
    total_timer.stop();
    FILE *outfd = fopen("compress.bin", "wb");
    int bytes_written = fwrite(&DRAM[0], 1, offset, outfd);
    fclose(outfd);
    std::cout << "Total latency of CDC is: " << CDC_timer.latency() << " ns." << std::endl;
    std::cout << "Total latency of CMD is: " << CMD_timer.latency() << " ns." << std::endl;
    std::cout << "Total latency of LZW is: " << LZW_timer.latency() << " ns." << std::endl;
    std::cout << "Total latency of send data is: " <<send_data_timer.latency() << " ns." << std::endl;
    std::cout << "Total time is: " << total_timer.latency() << " ns." << std::endl;
    float total_latency = total_timer.latency()/1000.0;
    float output_throughput = (file_size * 8 / 1000000.0) / total_latency;
    std::cout << "Output Throughput to DRAM: " << output_throughput << " Mb/s."<< " (Latency: " << total_latency << "s)." << std::endl;
    std::cout << "---------------------------------------------------------------" << std::endl;
    cout<<"write "<<bytes_written<<" to file"<<endl;
    for(int i=0;i<chunks_num;i++)
    {
        free(Chunk_array[i]);
    }
    free(buff);
    free(DRAM);
  // Close the file when done
    return 0;
}

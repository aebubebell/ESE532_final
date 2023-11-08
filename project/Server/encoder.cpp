#include "encoder.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "server.h"
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <fstream>
#include "stopwatch.h"
#include "utility.h"
using namespace std;
#define NUM_PACKETS 8
#define pipe_depth 4
#define DONE_BIT_L (1 << 7)
#define DONE_BIT_H (1 << 15)

int offset = 0;
unsigned char* file;

void handle_input(int argc, char* argv[], int* blocksize) {
	int x;
	extern char *optarg;

	while ((x = getopt(argc, argv, ":b:")) != -1) {
		switch (x) {
		case 'b':
			*blocksize = atoi(optarg);
			printf("blocksize is set to %d optarg\n", *blocksize);
			break;
		case ':':
			printf("-%c without parameter\n", optopt);
			break;
		}
	}
}

int main(int argc, char* argv[]) {
	stopwatch ethernet_timer;
	unsigned char* input[NUM_PACKETS];//8
	int writer = 0;
	int done = 0;
	int length = 0;
	int count = 0;
	ESE532_Server server;

	// default is 2k
	int blocksize = BLOCKSIZE;

	// set blocksize if decalred through command line
	handle_input(argc, argv, &blocksize);

	file = (unsigned char*) malloc(sizeof(unsigned char) * 70000000);
	if (file == NULL) {
		printf("help\n");
	}

	for (int i = 0; i < NUM_PACKETS; i++) {
		input[i] = (unsigned char*) malloc(
				sizeof(unsigned char) * (NUM_ELEMENTS + HEADER));
		if (input[i] == NULL) {
			std::cout << "aborting " << std::endl;
			return 1;
		}
	}

	server.setup_server(blocksize);

	writer = pipe_depth;
	server.get_packet(input[writer]);
	count++;

	// get packet
	unsigned char* buffer = input[writer];

	// decode
	done = buffer[1] & DONE_BIT_L;
	length = buffer[0] | (buffer[1] << 8);
	length &= ~DONE_BIT_H;
	// printing takes time so be weary of transfer rate
	//printf("length: %d offset %d\n",length,offset);

	// we are just memcpy'ing here, but you should call your
	// top function here.
	memcpy(&file[offset], &buffer[HEADER], length);

	offset += length;
	writer++;

	//last message
	while (!done) {
		// reset ring buffer
		if (writer == NUM_PACKETS) {
			writer = 0;
		}

		ethernet_timer.start();
		server.get_packet(input[writer]);
		ethernet_timer.stop();

		count++;

		// get packet
		unsigned char* buffer = input[writer];

		// decode
		done = buffer[1] & DONE_BIT_L;
		length = buffer[0] | (buffer[1] << 8);
		length &= ~DONE_BIT_H;
		//printf("length: %d offset %d\n",length,offset);
		memcpy(&file[offset], &buffer[HEADER], length);

		offset += length;
		writer++;
	}
	unsigned char* Chunk_array[MAX_NUM];
	int chunks_num=cdc(file,offset,Chunk_array);
	std::unordered_map<string,uint32_t> chunktable;
	unsigned char* DRAM;
    offset=0;
    DRAM = (unsigned char*)malloc(MAX_CHUNK*MAX_NUM);
	for(int i=0;i<chunks_num;i++)
	{
		int chunk_length= MAX_CHUNK;
        uint32_t header;
		header=cmd(Chunk_array[i],chunk_length,chunktable);
		if(header%2 ==0)
		{
            int* encode_array= (int*)malloc(sizeof(int)*MAX_CHUNK);
            int compress_length;
            LZWencoding(Chunk_array[i],encode_array,compress_length);
			header=compress_length<<1;
			memcpy(&DRAM[offset],&header,sizeof(uint32_t));
            offset +=sizeof(uint32_t);
			//cout<< "Write header:"<<header<<" to file"<<endl;
            for(int j=0;j<compress_length;j+=2)
             {
                 uint8_t send=0;
                //  cout <<"The encode data1 is:"<<*(encode_array+j)<<endl;
                //  cout <<"The encode data2 is:"<<*(encode_array+j+1)<<endl;
                send = *encode_array+j>>4;
				memcpy(&DRAM[offset],&send,sizeof(uint8_t));
                offset +=sizeof(uint8_t);
                //  cout <<"The first Byte is:"<<std::hex<<(int)send<<endl;
                send = *encode_array+j<<4;
                send |=*encode_array+j+1>>8;
                memcpy(&DRAM[offset],&send,sizeof(uint8_t));
                offset +=sizeof(uint8_t);
                //  cout<<"The second Byte is:"<<std::hex<<(int)send<<endl;
                send = *encode_array+j+1 &0xFF;
                //  cout<<"The third Byte is:"<<std::hex<<(int)send<<endl;
				memcpy(&DRAM[offset],&send,sizeof(uint8_t));
                offset +=sizeof(uint8_t);
             }
			if(compress_length%2 != 0)
			{
				uint8_t send;
				send = (uint8_t)*(encode_array+compress_length-1)<<4;
				memcpy(&DRAM[offset],&send,sizeof(uint8_t));
                offset +=sizeof(uint8_t);
			}
			free(encode_array);
		}
		else
		{
			// cout<< "Write header:	"<<header<<"	to file"<<endl;
            memcpy(&DRAM[offset],&header,sizeof(uint32_t));
            offset +=sizeof(uint32_t);
		}

	}
	
	// write file to root and you can use diff tool on board
	FILE *outfd = fopen("output_cpu.bin", "wb");
	int bytes_written = fwrite(&DRAM[0], 1, offset, outfd);
	printf("write file with %d\n", bytes_written);
	fclose(outfd);

	for (int i = 0; i < NUM_PACKETS; i++) {
		free(input[i]);
	}

	free(DRAM);
	free(file);
	std::cout << "--------------- Key Throughputs ---------------" << std::endl;
	float ethernet_latency = ethernet_timer.latency() / 1000.0;
	float input_throughput = (bytes_written * 8 / 1000000.0) / ethernet_latency; // Mb/s
	std::cout << "Input Throughput to Encoder: " << input_throughput << " Mb/s."
			<< " (Latency: " << ethernet_latency << "s)." << std::endl;

	return 0;
}


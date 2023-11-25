#include "encoder.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include "server.h"
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
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
	unsigned char* input[NUM_PACKETS];
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
	int bytes_input = offset;
	stopwatch CDC_timer, CMD_timer, LZW_timer,send_data_timer,total_timer;
	total_timer.start();
    unsigned char* Chunk_array[MAX_NUM];
    CDC_timer.start();
	int chunks_num = cdc(file,bytes_input,Chunk_array);
    CDC_timer.stop();
	std::unordered_map<string,uint32_t> chunktable;
    unsigned char* DRAM;
    offset=0;
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
                    offset +=sizeof(uinTotal latency of CMD is: 71.5353 ns.t8_t);
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
    std::cout << "Total latency of CDC is: " << CDC_timer.latency() << " ns." << std::endl;
    std::cout << "Total latency of CMD is: " << CMD_timer.latency() << " ns." << std::endl;
    std::cout << "Total latency of LZW is: " << LZW_timer.latency() << " ns." << std::endl;
    std::cout << "Total latency of send data is: " <<send_data_timer.latency() << " ns." << std::endl;
    std::cout << "---------------------------------------------------------------" << std::endl;
    FILE *outfd = fopen("compress.bin", "wb");
	int bytes_output = fwrite(&DRAM[0], 1, offset, outfd);
	fclose(outfd);
    cout<<"write "<<bytes_output<<" to file"<<endl;
    for(int i=0;i<chunks_num;i++)
    {
        free(Chunk_array[i]);
    }
	for (int i = 0; i < NUM_PACKETS; i++) {
		free(input[i]);
	}

	free(file);
	std::cout << "--------------- Key Throughputs ---------------" << std::endl;
	float ethernet_latency = ethernet_timer.latency() / 1000.0;
	float input_throughput = (bytes_input * 8 / 1000000.0) / ethernet_latency; // Mb/s
	std::cout << "Input Throughput to Encoder: " << input_throughput << " Mb/s."
			<< " (Latency: " << ethernet_latency << "s)." << std::endl;
	float total_latency = total_timer.latency()/1000.0;
	float output_throughput = (bytes_input * 8 / 1000000.0) / total_latency;
	std::cout << "Output Throughput to DRAM: " << output_throughput << " Mb/s."
			<< " (Latency: " << total_latency << "s)." << std::endl;
	return 0;
}

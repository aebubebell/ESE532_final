#define CL_HPP_CL_1_2_DEFAULT_BUILD
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY 1
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <CL/cl2.hpp>
#include "../Server/server.h"
#include <unistd.h>
#include <math.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <fstream>
#include "utility.h"
#include <unordered_map>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include "../common/Utilities.h"
#include "stopwatch.h"

#define NUM_PACKETS 8
#define pipe_depth 4
#define DONE_BIT_L (1 << 7)
#define DONE_BIT_H (1 << 15)
#define NUM_ELEMENTS 16384
#define HEADER 2

using namespace std;
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


int main(int argc, char* argv[])
{
    // ------------------------------------------------------------------------------------
    // Step 1: Initialize the OpenCL environment
    // ------------------------------------------------------------------------------------
    cl_int err;
    std::string binaryFile = argv[1];
    unsigned fileBufSize;
    std::vector<cl::Device> devices = get_xilinx_devices();
    devices.resize(1);
    cl::Device device = devices[0];
    cl::Context context(device,NULL,NULL,NULL,&err);
    char *fileBuf = read_binary_file(binaryFile,fileBufSize);
    cl::Program::Binaries bins{{fileBuf,fileBufSize}};
    cl::Program program(context, devices, bins, NULL, &err);
    cl::CommandQueue q(context,device,CL_QUEUE_PROFILING_ENABLE, &err);
    cl::Kernel LZW_fpga(program,"hardware_encoding",&err);
    // ------------------------------------------------------------------------------------
    // Step 2: Create buffers and initialize test values
    // ------------------------------------------------------------------------------------
    cl::Buffer chunk_in[15];
    cl::Buffer encode_out[15];
	cl::Buffer compress[15];
	// // chunk_in = cl::Buffer(context, NULL,MAX_CHUNK, NULL, &err);
    // // encode_out = cl::Buffer(context,NULL, sizeof(int)*MAX_CHUNK, NULL, &err);
	// // LZW_fpga.setArg(0,chunk_in);
    // // LZW_fpga.setArg(1,encode_out);
    // unsigned char *Chunk_in = (unsigned char*)q.enqueueMapBuffer(chunk_in,CL_TRUE,CL_MAP_WRITE,0,MAX_CHUNK);
    // int* encode_array = (int*)q.enqueueMapBuffer(encode_out,CL_TRUE,CL_MAP_READ,0,sizeof(int)*MAX_CHUNK);
	// int* compress_len = (int*)q.enqueueMapBuffer(compress,CL_TRUE,CL_MAP_READ,0,sizeof(int));
    // ------------------------------------------------------------------------------------
    // Step 3: Run the kernel
    // ------------------------------------------------------------------------------------
    unsigned char* input[NUM_PACKETS];//8
	int writer = 0;
	int done = 0;
	int length = 0;
	int count = 0;
	ESE532_Server server; 
	stopwatch throughput_timer;

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

		server.get_packet(input[writer]);

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
	//std::vector<cl::Event> read_events;
	throughput_timer.start();
    unsigned char* Chunk_array[MAX_NUM];
	int chunks_num=cdc(file,offset,Chunk_array);
	std::unordered_map<string,uint32_t> chunktable;
	unsigned char* DRAM;
    offset=0;
    DRAM = (unsigned char*)malloc(MAX_CHUNK*MAX_NUM);
	int buffer_index = 0;
	for(int i=0;i<chunks_num;i++)

	{
		int chunk_length= MAX_CHUNK;
        uint32_t header;
		header=cmd(Chunk_array[i],chunk_length,chunktable);

		if(header%2 ==0)
		{
			chunk_in[buffer_index]= cl::Buffer(context,CL_MEM_ALLOC_HOST_PTR|CL_MEM_READ_ONLY,MAX_CHUNK,NULL,&err);
    		encode_out[buffer_index]=cl::Buffer(context,CL_MEM_ALLOC_HOST_PTR,sizeof(int)*MAX_CHUNK,NULL,&err);
			compress[buffer_index]=cl::Buffer(context,CL_MEM_ALLOC_HOST_PTR,sizeof(int)*MAX_CHUNK,NULL,&err);
			unsigned char *Chunk_in = (unsigned char*)q.enqueueMapBuffer(chunk_in[buffer_index],CL_TRUE,CL_MAP_WRITE,0,MAX_CHUNK);
    		int* encode_array = (int*)q.enqueueMapBuffer(encode_out[buffer_index],CL_TRUE,CL_MAP_READ,0,sizeof(int)*MAX_CHUNK);
			int* compress_len = (int*)q.enqueueMapBuffer(compress[buffer_index],CL_TRUE,CL_MAP_READ,0,sizeof(int));
            //int* encode= (int*)malloc(sizeof(int)*MAX_CHUNK);
            int compress_length=0;
			//cout<<"chunk "<<i<<" is unique arry"<<endl;
            //Chunk_in = Chunk_array[i];
			//cout<<Chunk_in<<endl;
			// Chunk_in = (unsigned char*)q.enqueueMapBuffer(chunk_in,CL_TRUE,CL_MAP_WRITE,0,MAX_CHUNK);
			// encode_array= (int*)q.enqueueMapBuffer(encode_out,CL_TRUE,CL_MAP_READ,0,sizeof(int)*MAX_CHUNK);
            std::vector<cl::Event> exec_events, write_events,read_events;
            cl::Event write_ev;
            cl::Event exec_ev;
            cl::Event read_ev;
			Chunk_in = (unsigned char*)q.enqueueMapBuffer(chunk_in[buffer_index],CL_TRUE,CL_MAP_WRITE,0,MAX_CHUNK);
			//memcpy(&Chunk_in,&Chunk_array[i],10);
			// cout<<"------------------Chunk_array ["<<i<<"] -------------------" <<endl;
			// cout<<Chunk_array[i]<<endl;
			// cout<<"------------------------------------------------------------"<<endl;
			for(int j =0;*(Chunk_array[i]+j)!='\0';j++)
			{
				Chunk_in[j]=Chunk_array[i][j];
				//cout <<Chunk_array[i][j];
			}
			// cout<<endl;
			// cout<<"------------------------------------------------------------"<<endl;
			//encode_array = (int*)q.enqueueMapBuffer(encode_out,CL_TRUE,CL_MAP_READ,0,sizeof(int)*MAX_CHUNK);
            LZW_fpga.setArg(0,chunk_in[buffer_index]);
            LZW_fpga.setArg(1,encode_out[buffer_index]);
			LZW_fpga.setArg(2,compress[buffer_index]);
            if(i==0)
            {
                q.enqueueMigrateMemObjects({chunk_in[buffer_index]}, 0 /* 0 means from host*/, NULL, &write_ev);
            }
            else
            {
                q.enqueueMigrateMemObjects({chunk_in[buffer_index]}, 0 /* 0 means from host*/, &read_events, &write_ev);
            }
            write_events.push_back(write_ev);
			//clWaitForEvents(1, (const cl_event *)&write_ev);

            q.enqueueTask(LZW_fpga,&write_events,&exec_ev);
			exec_events.push_back(exec_ev);
			//clWaitForEvents(1, (const cl_event *)&exec_ev);
			cout<<"before map"<<*compress_len<<endl;
			encode_array= (int*)q.enqueueMapBuffer(encode_out[buffer_index],CL_TRUE,CL_MAP_READ,0,sizeof(int)*MAX_CHUNK);
			compress_len = (int*)q.enqueueMapBuffer(compress[buffer_index],CL_TRUE,CL_MAP_READ,0,sizeof(int));
            q.enqueueMigrateMemObjects({encode_out[buffer_index],compress[buffer_index]},CL_MIGRATE_MEM_OBJECT_HOST,&exec_events,&read_ev);
			cout<<"after migrate"<<*(compress_len)<<endl;
            // read_events.push_back(read_ev);
			// clWaitForEvents(1, (const cl_event *)&read_ev);
			//cout<<*encode_array<<endl;
			compress_length=*compress_len;
			// cout<< "the encode array is :";
			// for(int j =0;j<compress_length;j++)
			// {
			// 	cout<<*(encode_array+j)<<" ";
			// }
			// cout<< endl;
			// cout<<"The compress length is: "<<compress_length<<endl;
			uint32_t compress_byte=ceil((12*(float)compress_length)/8);
            //cout<<"The number of compress length is:"<<compress_length<<endl;
            header=(uint32_t)compress_byte<<1;
            //cout<< "Write header:	"<<header<<"	to file"<<endl;
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
					//cout<< "send data:"<<(int)send<<" to mem"<<endl;
                    offset +=sizeof(uint8_t);
                    send = *(encode_array+j)<<4;
                    send |= *(encode_array+j+1)>>8;
                    memcpy(&DRAM[offset],&send,sizeof(uint8_t));
					//cout<< "send data:"<<(int)send<<" to mem"<<endl;
                    offset +=sizeof(uint8_t);
                    send = *(encode_array+j+1);
                    memcpy(&DRAM[offset],&send,sizeof(uint8_t));
					//cout<< "send data:"<<(int)send<<" to mem"<<endl;
                    offset +=sizeof(uint8_t);
                }
            }
			q.enqueueUnmapMemObject(chunk_in[buffer_index], Chunk_in);
			q.enqueueUnmapMemObject(encode_out[buffer_index], encode_array);
			q.enqueueUnmapMemObject(compress[buffer_index], compress_len);
			cout<<"after unmap "<<*(compress_len)<<endl;
			buffer_index++;
		}
		else
		{
			//cout<< "Write header:	"<<header<<"	to file"<<endl;
            memcpy(&DRAM[offset],&header,sizeof(uint32_t));
            offset +=sizeof(uint32_t);
		}

	}
    q.finish();
	throughput_timer.stop();
	// write file to root and you can use diff tool on board
	FILE *outfd = fopen("output_cpu.bin", "wb");
	int compress_bytes_written = fwrite(&DRAM[0], 1, offset, outfd);
	printf("write file with %d\n", compress_bytes_written);
	fclose(outfd);

	for (int i = 0; i < NUM_PACKETS; i++) {
		free(input[i]);
	}
	float throughput_latency = throughput_timer.latency()/1000.0;
	float output_throughput = (compress_bytes_written * 8 / 1000000.0) / throughput_latency;
	std::cout << "Output Throughput to Encoder: " << output_throughput << " Mb/s."
			<< " (Latency: " << throughput_latency << "s)." << std::endl;
	free(DRAM);
	free(file);
    return 0;
}
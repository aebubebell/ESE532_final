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
#include "../common/EventTimer.h"

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
// Main program
// ------------------------------------------------------------------------------------
// Initialize an event timer we'll use for monitoring the application
    EventTimer timer;
// ------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------
// Step 1: Initialize the OpenCL environment
// ------------------------------------------------------------------------------------
	timer.add("OpenCL Initialization");
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
	timer.add("create 15 buffer for chunk and compress_length, one for encode data");
    cl::Buffer chunk_in;
    cl::Buffer encode_out;
	cl::Buffer compress;
	cl::Buffer chunk_len;
	for(int i=0;i<15;i++)
	{
		//chunk_in[i]= cl::Buffer(context, NULL,MAX_CHUNK, NULL, &err);
		//compress[i]= cl::Buffer(context, NULL,sizeof(int)*11, NULL, &err);
	}
	chunk_in = cl::Buffer(context, NULL,MAX_CHUNK, NULL, &err);
    encode_out = cl::Buffer(context,NULL, sizeof(int)*MAX_CHUNK, NULL, &err);
	compress = cl::Buffer(context, NULL,sizeof(int)*11, NULL, &err);
	chunk_len = cl::Buffer(context, NULL,sizeof(unsigned int)*11, NULL, &err);
	unsigned char * Chunk_in;
	int *compress_len;
	int *encode_array;
	unsigned int *Chunk_len;
	for(int i=0;i<15;i++)
	{
		//Chunk_in[i] = (unsigned char*)q.enqueueMapBuffer(chunk_in[i],CL_TRUE,CL_MAP_WRITE,0,MAX_CHUNK);
		//compress_len[i] = (int*)q.enqueueMapBuffer(compress[i],CL_TRUE,CL_MAP_READ,0,sizeof(int));
	}
	encode_array = (int*)q.enqueueMapBuffer(encode_out,CL_TRUE,CL_MAP_READ,0,sizeof(int)*MAX_CHUNK);
	Chunk_in = (unsigned char*)q.enqueueMapBuffer(chunk_in,CL_TRUE,CL_MAP_WRITE,0,MAX_CHUNK);
	Chunk_len = (unsigned int*)q.enqueueMapBuffer(chunk_len,CL_TRUE,CL_MAP_WRITE,0,11*sizeof(unsigned int));
	compress_len = (int*)q.enqueueMapBuffer(compress,CL_TRUE,CL_MAP_READ,0,11*sizeof(int));
	// LZW_fpga.setArg(0,chunk_in);
    // LZW_fpga.setArg(1,encode_out);
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
	stopwatch ethernet_timer,CDC_timer,CMD_timer,LZW_timer,send_data_timer,total_timer;
	stopwatch buffer_map,read,kernel_ex,write;

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
	std::unordered_map<string,uint32_t> chunktable;
	int bytes_input= offset;
	//std::vector<cl::Event> read_events;
    unsigned char* Chunk_array[MAX_NUM];
	unsigned int *chunk_length=(unsigned int*)malloc(sizeof(int)*MAX_NUM);
	total_timer.start();
	CDC_timer.start();
	int chunks_num=cdc(file,offset,Chunk_array,chunk_length);
	CDC_timer.stop();

	unsigned char* DRAM;
    offset=0;
    DRAM = (unsigned char*)malloc(MAX_CHUNK*MAX_NUM);
	int buffer_index = 0;
	for(int i=0;i<chunks_num;i++)

	{
        uint32_t header=0;
		CMD_timer.start();
		header=cmd(Chunk_array[i],chunktable);
		CMD_timer.stop();
		if(header%2 ==0)
		{
			LZW_timer.start();
			buffer_map.start();
			//Chunk_array[i] = (unsigned char*)q.enqueueMapBuffer(chunk_in,CL_TRUE,CL_MAP_WRITE,0,MAX_CHUNK);
			// timer.add("Allocate contiguous OpenCl buffer");
			// chunk_in[buffer_index]= cl::Buffer(context,CL_MEM_ALLOC_HOST_PTR|CL_MEM_READ_ONLY,MAX_CHUNK,NULL,&err);
			// compress[buffer_index]=cl::Buffer(context,CL_MEM_ALLOC_HOST_PTR,sizeof(int)*11,NULL,&err);
			// //encode_out[buffer_index]=cl::Buffer(context,CL_MEM_ALLOC_HOST_PTR,sizeof(int)*MAX_CHUNK,NULL,&err);
			// unsigned char *Chunk_in = (unsigned char*)q.enqueueMapBuffer(chunk_in[buffer_index],CL_TRUE,CL_MAP_WRITE,0,MAX_CHUNK);
    		// int* encode_array = (int*)q.enqueueMapBuffer(encode_out,CL_TRUE,CL_MAP_READ,0,sizeof(int)*MAX_CHUNK);
			// int* compress_len = (int*)q.enqueueMapBuffer(compress[buffer_index],CL_TRUE,CL_MAP_READ,0,sizeof(int));
            //int* encode= (int*)malloc(sizeof(int)*MAX_CHUNK);
            int compress_length=0;
			//cout<<"chunk "<<i<<" is unique arry"<<endl;
            //Chunk_in = Chunk_array[i];uncompress_byte
			//cout<<Chunk_in<<endl;
			// Chunk_in = (unsigned char*)q.enqueueMapBuffer(chunk_in,CL_TRUE,CL_MAP_WRITE,0,MAX_CHUNK);
			// encode_array= (int*)q.enqueueMapBuffer(encode_out,CL_TRUE,CL_MAP_READ,0,sizeof(int)*MAX_CHUNK);
            std::vector<cl::Event> exec_events(1);
			std::vector<cl::Event> write_events(1);
			std::vector<cl::Event> read_events(1);
			//Chunk_in = (unsigned char*)q.enqueueMapBuffer(chunk_in[buffer_index],CL_TRUE,CL_MAP_WRITE,0,MAX_CHUNK);
			//memcpy(&Chunk_in,&Chunk_array[i],10);
			// cout<<"------------------Chunk_array ["<<i<<"] -------------------" <<endl;
			// cout<<Chunk_array[i]<<endl;
			cout<<"Chunk "<< i <<" is: "<<endl;
			cout<<"------------------------------------------------------------"<<endl;
			for(int j =0; *(Chunk_in+j)!='\0';j++)
			{
				Chunk_in[j]= NULL;
			}
			for(int j =0;*(Chunk_array[i]+j)!='\0';j++)
			{
				Chunk_in[j]=Chunk_array[i][j];
				//cout <<Chunk_in[j];
			}
			cout<<endl;
			*Chunk_len = 0;
			*Chunk_len = *(chunk_length+i);
			cout<<"------------------------------------------------------------"<<endl;
			cout<<"Chunk length for Chunk "<<i<<"  is:"<<*Chunk_len<<endl;
			cout<<"------------------------------------------------------------"<<endl;
			// cout<<endl;
			//encode_array = (int*)q.enqueueMapBuffer(encode_out,CL_TRUE,CL_MAP_READ,0,sizeof(int)*MAX_CHUNK);
			buffer_map.stop();
			read.start();
			//timer.add("Set kernel arguements");
            LZW_fpga.setArg(0,chunk_in);
            LZW_fpga.setArg(1,encode_out);
			LZW_fpga.setArg(2,chunk_len);
			LZW_fpga.setArg(3,compress);
			//timer.add("Memory object migration enqueue host->device");
            // if(i==0)
            // {
            //     q.enqueueMigrateMemObjects({chunk_in[buffer_index]}, 0 /* 0 means from host*/, NULL, &write_ev);
            // }
            // else
            // {
            //     q.enqueueMigrateMemObjects({chunk_in[buffer_index]}, 0 /* 0 means from host*/, &read_events, &write_ev);
            // }
			q.enqueueMigrateMemObjects({chunk_in,chunk_len}, 0 /* 0 means from host*/, NULL,&write_events[0]);
            //write_events.push_back(write_ev);
			//clWaitForEvents(1, (const cl_event *)&write_ev);
			read.stop();
			kernel_ex.start();
			//timer.add("Launch mmult kernel");
            q.enqueueTask(LZW_fpga,&write_events,&exec_events[0]);
			kernel_ex.stop();
			//exec_events.push_back(exec_ev);
			//clWaitForEvents(1, (const cl_event *)&exec_ev);
			//cout<<"before map"<<*compress_len<<endl;
			write.start();
			//timer.add("Read back computation results (implicit device->host migration)");
			//encode_array= (int*)q.enqueueMapBuffer(encode_out,CL_TRUE,CL_MAP_READ,0,sizeof(int)*MAX_CHUNK);
			//compress_len[buffer_index] = (int*)q.enqueueMapBuffer(compress[buffer_index],CL_TRUE,CL_MAP_READ,0,sizeof(int));
            q.enqueueMigrateMemObjects({encode_out,compress},CL_MIGRATE_MEM_OBJECT_HOST,&exec_events,&read_events[0]);
			clWaitForEvents(1, (const cl_event *)&read_events[0]);
			write.stop();
			LZW_timer.stop();
			//cout<<"The compress length is: "<<*(compress_len[buffer_index])<<endl;
            // read_events.push_back(read_ev);
			// clWaitForEvents(1, (const cl_event *)&read_ev);
			//cout<<*encode_array<<endl;
			compress_length=*compress_len;
			cout<<"------------------------------------------------------------"<<endl;
			cout<< "the encode array is :"<<endl;
			for(int j =0;*(encode_array+j)!='\0';j++)
			{
				cout<<*(encode_array+j)<<" ";
			}
			cout<< endl;
			cout<<"------------------------------------------------------------"<<endl;
			cout<<"The compress length is: "<<compress_length<<endl;
			cout<<"------------------------------------------------------------"<<endl;
			uint32_t compress_byte=ceil((12*(float)compress_length)/8);
            //cout<<"The number of compress length is:"<<compress_length<<endl;
			send_data_timer.start();
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
			//q.enqueueUnmapMemObject(chunk_in[buffer_index], Chunk_in[buffer_index]);
			//q.enqueueUnmapMemObject(encode_out[buffer_index], encode_array);
			//q.enqueueUnmapMemObject(compress[buffer_index], compress_len[buffer_index]);
			//cout<<"after unmap "<<*(compress_len)<<endl;
			buffer_index++;
			send_data_timer.stop();
		}
		else
		{
			send_data_timer.start();
			//cout<< "Write header:	"<<header<<"	to file"<<endl;
            memcpy(&DRAM[offset],&header,sizeof(uint32_t));
            offset +=sizeof(uint32_t);
			send_data_timer.stop();
		}

	}
    q.finish();
	//timer.finish();
	total_timer.stop();
	// write file to root and you can use diff tool on board
	FILE *outfd = fopen("output_cpu.bin", "wb");
	int compress_bytes_written = fwrite(&DRAM[0], 1, offset, outfd);
	printf("write file with %d\n", compress_bytes_written);
	fclose(outfd);

	std::cout << "Total latency of ethernet is: " << ethernet_timer.latency() << " ms." << std::endl;
	std::cout << "Total latency of CDC is: " << CDC_timer.latency() << " ms." << std::endl;
    std::cout << "Total latency of CMD is: " << CMD_timer.latency() << " ms." << std::endl;
	std::cout << "Total latency of map buffer is: " << buffer_map.latency() << " ms." << std::endl;
	std::cout << "Total latency of read is: " << read.latency() << " ms." << std::endl;
	std::cout << "Total latency of kernel is: " << kernel_ex.latency() << " ms." << std::endl;
	std::cout << "Total latency of write is: " << write.latency() << " ms." << std::endl;
    std::cout << "Total latency of LZW is: " << LZW_timer.latency() << " ms." << std::endl;
    std::cout << "Total latency of send data is: " <<send_data_timer.latency() << " ms." << std::endl;
    std::cout << "Total time is: " << total_timer.latency() << " ms." << std::endl;
	// std::cout << "--------------- Key execution times ---------------" << std::endl;
    // timer.print();
	std::cout << "--------------- Key Throughputs ---------------" << std::endl;
	float ethernet_latency = ethernet_timer.latency() / 1000.0;
	float input_throughput = (bytes_input * 8 / 1000000.0) / ethernet_latency; // Mb/s
	std::cout << "Input Throughput to Encoder: " << input_throughput << " Mb/s."
			<< " (Latency: " << ethernet_latency << "s)." << std::endl;
	float CDC_latency = CDC_timer.latency()/1000.0;
	float CDC_throughput = (bytes_input * 8 / 1000000.0) / CDC_latency;
	std::cout << "data rate of CDC: " << CDC_throughput << " Mb/s."
			<< " (Latency: " << CDC_latency << "s)." << std::endl;
	float CMD_latency = CMD_timer.latency()/1000.0;
	float CMD_throughput = (bytes_input * 8 / 1000000.0) / CMD_latency;
	std::cout << "data rate of CMD: " << CMD_throughput << " Mb/s."
			<< " (Latency: " << CMD_latency << "s)." << std::endl;
	float LZW_latency = LZW_timer.latency()/1000.0;
	float LZW_throughput = (bytes_input * 8 / 1000000.0) / LZW_latency;
	std::cout << "data rate of LZW: " << LZW_throughput << " Mb/s."
			<< " (Latency: " << LZW_latency << "s)." << std::endl;
	float total_latency = total_timer.latency()/1000.0;
    float output_throughput = (bytes_input* 8 / 1000000.0) / total_latency;
    std::cout << "Output Throughput to DRAM: " << output_throughput << " Mb/s."<< " (Latency: " << total_latency << "s)." << std::endl;
    std::cout << "---------------------------------------------------------------" << std::endl;

	for (int i = 0; i < NUM_PACKETS; i++) {
		free(input[i]);
	}

	free(DRAM);
	free(file);
    return 0;
}

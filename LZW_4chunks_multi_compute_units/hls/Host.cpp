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
// ------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------
// Step 1: Initialize the OpenCL environment
// ------------------------------------------------------------------------------------
	//timer.add("OpenCL Initialization");
	//pin_main_thread_to_cpu0();
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
    cl::CommandQueue q(context,device,CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,&err);
    //cl::Kernel LZW_fpga(program,"encoding_4chunks",&err);
	vector<cl::Kernel> LZW_fpga(4);
	for(int i=0;i<4;i++)
	{
		LZW_fpga[i]=cl::Kernel(program,"encoding_4chunks",&err);
	}
    // ------------------------------------------------------------------------------------
    // Step 2: Create buffers and initialize test values
    // ------------------------------------------------------------------------------------
    cl::Buffer chunk_in[14];
    cl::Buffer encode_out[14];
	cl::Buffer compress[14];
	cl::Buffer chunk_len[14];
	for(int i=0;i<14;i++)
	{
		chunk_in[i]= cl::Buffer(context, NULL,MAX_CHUNK*4, NULL, &err);
		encode_out[i]=cl::Buffer(context, NULL,sizeof(int)*MAX_CHUNK*4, NULL, &err);
		chunk_len[i]=cl::Buffer(context, NULL,sizeof(unsigned int)*44, NULL, &err);
		compress[i]= cl::Buffer(context, NULL,sizeof(int)*44, NULL, &err);
	}
	// chunk_in = cl::Buffer(context, NULL,MAX_CHUNK*4, NULL, &err);
    // encode_out = cl::Buffer(context,NULL, sizeof(int)*MAX_CHUNK*4, NULL, &err);
	// compress = cl::Buffer(context, NULL,sizeof(int)*44, NULL, &err);
	// chunk_len = cl::Buffer(context, NULL,sizeof(unsigned int)*44, NULL, &err);
	unsigned char * Chunk_in[14];
	int *compress_len[14];
	int *encode_array[14];
	unsigned int *Chunk_len[14];
	for(int i=0;i<14;i++)
	{
		Chunk_in[i] = (unsigned char*)q.enqueueMapBuffer(chunk_in[i],CL_TRUE,CL_MAP_WRITE,0,MAX_CHUNK*4);
		Chunk_len[i] = (unsigned int*)q.enqueueMapBuffer(chunk_len[i],CL_TRUE,CL_MAP_WRITE,0,sizeof(unsigned int)*44);
		compress_len[i] = (int*)q.enqueueMapBuffer(compress[i],CL_TRUE,CL_MAP_READ,0,sizeof(int)*44);
		encode_array[i] = (int*)q.enqueueMapBuffer(encode_out[i],CL_TRUE,CL_MAP_READ,0,sizeof(int)*MAX_CHUNK*4);
	}
	// encode_array = (int*)q.enqueueMapBuffer(encode_out,CL_TRUE,CL_MAP_READ,0,sizeof(int)*MAX_CHUNK*4);
	// Chunk_in = (unsigned char*)q.enqueueMapBuffer(chunk_in,CL_TRUE,CL_MAP_WRITE,0,MAX_CHUNK*4);
	// Chunk_len = (unsigned int*)q.enqueueMapBuffer(chunk_len,CL_TRUE,CL_MAP_WRITE,0,44*sizeof(unsigned int));
	// compress_len = (int*)q.enqueueMapBuffer(compress,CL_TRUE,CL_MAP_READ,0,44*sizeof(int));
    // ------------------------------------------------------------------------------------
    // Step 3: Run the kernel
    // ------------------------------------------------------------------------------------
	std::vector<cl::Event> read_done(4);
	std::vector<cl::Event> execute_done(4);
	std::vector<cl::Event> write_done(4);
	std::vector<cl::Event> write_waitlist;
	std::vector<std::vector<cl::Event>> execute_waitlists(4);
	std::vector<std::vector<cl::Event>> read_waitlists(4);

    unsigned char* input[NUM_PACKETS];//8
	int writer = 0;
	int done = 0;
	int length = 0;
	int count = 0;
	ESE532_Server server; 
	stopwatch ethernet_timer,CDC_timer,CMD_timer,LZW_timer,send_data_timer,total_timer;
	stopwatch fill_buffer,migrate_to_fpga,migrate_to_host,kernel_execution;

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

	//std::unordered_map<string,uint32_t> chunktable;
	int bytes_input= offset;

	int counter=0;
	int compress_length=0;
	int encode_index=0;
	int previous_chunk_length=0;
	int chunks_index[60];
	int kernel_index=0;
	
    unsigned char* Chunk_array[MAX_NUM];
	for(int i =0;i<MAX_NUM;i++)
	{
		Chunk_array[i]=(unsigned char*)malloc(sizeof(unsigned char)* MAX_CHUNK);
	}
	unsigned int *chunk_length=(unsigned int*)malloc(sizeof(int)*MAX_NUM);
	unsigned char* DRAM;
	int DRAM_offset=0;

    DRAM = (unsigned char*)malloc(MAX_CHUNK*MAX_NUM);
	int compress_length_array[MAX_NUM];
	int *encode_data[MAX_NUM];
	uint32_t *header=(uint32_t*)malloc(sizeof(uint32_t)*MAX_NUM);
	for(int i=0;i<MAX_NUM;i++)
	{
		encode_data[i]=(int*)malloc(sizeof(int)*MAX_CHUNK);
	}
	//server.get_packet(input[writer]);
	//count++;

	// get packet
	//unsigned char* buffer = input[writer];
	// decode
	// done = buffer[1] & DONE_BIT_L;
	// length = buffer[0] | (buffer[1] << 8);
	// length &= ~DONE_BIT_H;
	// printing takes time so be weary of transfer rate
	//printf("length: %d offset %d\n",length,offset);

	// we are just memcpy'ing here, but you should call your
	// top function here.
	// memcpy(&file[offset], &buffer[HEADER], length);

	// offset += length;
	// writer++;

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
		if(count ==2)
		{
			bytes_input=offset;
			total_timer.start();
			CDC_timer.start();
			int chunks_num=cdc(file,offset,Chunk_array,chunk_length);
			CDC_timer.stop();
			CMD_timer.start();
			cmd(Chunk_array,header,chunks_num);
			CMD_timer.stop();
			LZW_timer.start();
			for(int i=0;i<chunks_num+3;i++)
			{
				if(header[i]%2 ==0)
				{
					if(i<chunks_num)
					{
						fill_buffer.start();
						memcpy(Chunk_in[kernel_index]+previous_chunk_length,&Chunk_array[i][0],*(chunk_length+i));
						previous_chunk_length+= *(chunk_length+i);
						Chunk_len[kernel_index][counter]=*(chunk_length+i);
						chunks_index[kernel_index*4+counter]=i;
						//cout<<"Put Chunk "<< i<<" into buffer"<<endl;
						fill_buffer.stop();
					}
					else
					{
						Chunk_len[kernel_index][counter]=0;
					}
					if(counter == 3)
					{
						std::vector<cl::Event> exec_events(1);
						std::vector<cl::Event> write_events(1);
						std::vector<cl::Event> read_events(1);
						LZW_fpga[kernel_index].setArg(0,chunk_in[kernel_index]);
            			LZW_fpga[kernel_index].setArg(1,encode_out[kernel_index]);
						LZW_fpga[kernel_index].setArg(2,chunk_len[kernel_index]);
						LZW_fpga[kernel_index].setArg(3,compress[kernel_index]);
						//cout<<"Call Kernel"<<endl;
						migrate_to_fpga.start();
						//q.enqueueMigrateMemObjects({chunk_in[kernel_index],chunk_len[kernel_index]}, 0 /* 0 means from host*/, NULL,&write_events[0]);
						q.enqueueMigrateMemObjects({chunk_in[kernel_index],chunk_len[kernel_index]}, 0 /* 0 means from host*/, NULL,&write_done[kernel_index]);
						write_waitlist.push_back(write_done[kernel_index]);
						migrate_to_fpga.stop();
						kernel_execution.start();
						execute_waitlists[kernel_index].push_back(write_done[kernel_index]);
						//q.enqueueTask(LZW_fpga[kernel_index],&write_events,&exec_events[0]);
						q.enqueueTask(LZW_fpga[kernel_index],&execute_waitlists[kernel_index],&execute_done[kernel_index]);
						kernel_execution.stop();
						migrate_to_host.start();
						read_waitlists[kernel_index].push_back(execute_done[kernel_index]);
						//q.enqueueMigrateMemObjects({encode_out[kernel_index],compress[kernel_index]},CL_MIGRATE_MEM_OBJECT_HOST,&exec_events,&read_events[0]);
						q.enqueueMigrateMemObjects({encode_out[kernel_index],compress[kernel_index]},CL_MIGRATE_MEM_OBJECT_HOST,&read_waitlists[kernel_index],&read_done[kernel_index]);
						//clWaitForEvents(1, (const cl_event *)&read_events[0]);
						migrate_to_host.stop();
						for(int j =0;j<4;j++)
						{
						  	compress_length=*(compress_len[kernel_index]+j);
						  	//cout<<"The compress_length is "<<compress_length<<endl;
							if(compress_length != 0)
							{
								uint32_t compress_byte=ceil((12*(float)compress_length)/8);
								header[chunks_index[j]]=(uint32_t)compress_byte<<1;
								compress_length_array[chunks_index[j]]=compress_length;
								cout<<"The encode array for chunk["<<chunks_index[kernel_index*4+j]<<"] is"<<endl;
								for(int k=0;k<compress_length;k++)
								{
									cout<<*(encode_array[kernel_index]+encode_index+k)<<" ";
									*(encode_data[chunks_index[j]]+k)=*(encode_array[kernel_index]+encode_index+k);
								}
								cout<<endl;
								cout<<"----------------------------------------------"<<endl;
							}
							encode_index+=compress_length;
						 	compress_length=0;
						}
						counter=0;
						previous_chunk_length=0;
						encode_index=0;
						kernel_index++;
					}
					else
					{
					counter++;
					}
				}	
			}
			// for(int i=0;i<kernel_index;i++)
			// {
			// 	LZW_fpga[i].setArg(0,chunk_in[i]);
            // 	LZW_fpga[i].setArg(1,encode_out[i]);
			// 	LZW_fpga[i].setArg(2,chunk_len[i]);
			// 	LZW_fpga[i].setArg(3,compress[i]);
			// 	q.enqueueMigrateMemObjects({chunk_in[i],chunk_len[i]}, 0 /* 0 means from host*/);
			// }
			// q.finish();
			// for(int i=0;i<kernel_index;i++)
			// {
			// 	q.enqueueTask(LZW_fpga[i]);
			// }
			// q.finish();
			// for(int i=0;i<kernel_index;i++)
			// {
			// 	q.enqueueMigrateMemObjects({encode_out[i],compress[i]},CL_MIGRATE_MEM_OBJECT_HOST);
			// }
			q.finish();
			LZW_timer.stop();
			//cout<<"------------------------------------------------------------"<<endl;
			send_data_timer.start();
			for(int i =0;i<kernel_index;i++)
			{
				encode_index=0;
				for(int j =0;j<4;j++)
				{
					compress_length=*(compress_len[i]+j);
			 		//cout<<"The compress_length is "<<compress_length<<endl;
					if(compress_length != 0)
					{
						uint32_t compress_byte=ceil((12*(float)compress_length)/8);
						header[chunks_index[i*4+j]]=(uint32_t)compress_byte<<1;
						compress_length_array[chunks_index[i*4+j]]=compress_length;
						//cout<<"The encode array for chunk["<<chunks_index[i*4+j]<<"] is"<<endl;
						for(int k=0;k<compress_length;k++)
						{
							//cout<<*(encode_array[i]+encode_index+k)<<" ";
							*(encode_data[chunks_index[i*4+j]]+k)=*(encode_array[i]+encode_index+k);
						}
						//cout<<endl;
						//cout<<"----------------------------------------------"<<endl;
					}
					encode_index+=compress_length;
					compress_length=0;
				}
			}
			for(int i=0;i<chunks_num;i++)
			{
				if(header[i]%2 ==0)
				{
					//cout<< "write header"<<header[i]<<" to file"<<endl;
					memcpy(&DRAM[DRAM_offset],&header[i],sizeof(uint32_t));
					DRAM_offset +=sizeof(uint32_t);
					//cout<<"DRAM_offset is "<<DRAM_offset<<endl;
					// cout<<"----------------------------------------------"<<endl;
					// cout<<"The compress length is "<<compress_length_array[i]<<endl;
					// cout<<"The encode array for chunk["<<i<<"] is"<<endl;
					for(int j=0;j<compress_length_array[i];j+=2)
					{
						if(compress_length_array[i]-j == 1)
						{
							uint8_t send=0;
							//cout<<*(encode_data[i]+compress_length_array[i]-1)<<" ";
							send = *(encode_data[i]+compress_length_array[i]-1)>>4;
							//cout<< "write "<<(int)send<<" to file"<<endl;
							memcpy(&DRAM[DRAM_offset],&send,sizeof(uint8_t));
							DRAM_offset +=sizeof(uint8_t);
							send = *(encode_data[i]+compress_length_array[i]-1)<<4;
							//cout<< "write "<<(int)send<<" to file"<<endl;
							memcpy(&DRAM[DRAM_offset],&send,sizeof(uint8_t));
							DRAM_offset +=sizeof(uint8_t);
						}
						else
						{
							uint8_t send=0;
							//cout<<*(encode_data[i]+j)<<" ";
							send = *(encode_data[i]+j)>>4;
							//cout<< "write "<<(int)send<<" to file"<<endl;
							memcpy(&DRAM[DRAM_offset],&send,sizeof(uint8_t));
							DRAM_offset +=sizeof(uint8_t);
							send = *(encode_data[i]+j)<<4;
							//cout<<*(encode_data[i]+j+1)<<" ";
							send |= *(encode_data[i]+j+1)>>8;
							//cout<< "write "<<(int)send<<" to file"<<endl;
							memcpy(&DRAM[DRAM_offset],&send,sizeof(uint8_t));
							DRAM_offset +=sizeof(uint8_t);
							send = *(encode_data[i]+j+1);
							//cout<< "write "<<(int)send<<" to file"<<endl;
							memcpy(&DRAM[DRAM_offset],&send,sizeof(uint8_t));
							DRAM_offset +=sizeof(uint8_t);
						}
					}
					// cout<<endl;
					// cout<<"----------------------------------------------"<<endl;
				}
				else
				{
					//cout<< "write header"<<header[i]<<" to file"<<endl;
					memcpy(&DRAM[DRAM_offset],&header[i],sizeof(uint32_t));
					DRAM_offset +=sizeof(uint32_t);
				}
			}
			send_data_timer.stop();
			count=0;
		}
	}
	total_timer.stop();
	//cout<<"DRAM_offset is "<<DRAM_offset<<endl;
	FILE *outfd = fopen("output_cpu.bin", "wb");
	int compress_bytes_written = fwrite(&DRAM[0], 1, DRAM_offset, outfd);
	printf("write file with %d\n", compress_bytes_written);
	fclose(outfd);

	std::cout << "Total latency of ethernet is: " << ethernet_timer.latency() << " ms." << std::endl;
	std::cout << "Total latency of CDC is: " << CDC_timer.latency() << " ms." << std::endl;
    std::cout << "Total latency of CMD is: " << CMD_timer.latency() << " ms." << std::endl;
	std::cout << "Total latency of fill buffer is: " << fill_buffer.latency() << " ms." << std::endl;
	std::cout << "Total latency of migrate buffer to fpga is: " << migrate_to_fpga.latency() << " ms." << std::endl;
	std::cout << "Total average latency of migrate buffer to fpga is: " << migrate_to_fpga.avg_latency() << " ms." << std::endl;
	std::cout << "Total latency of kernel execution is: " << kernel_execution.latency() << " ms." << std::endl;
	std::cout << "Total average latency of kernel execution is: " << kernel_execution.avg_latency() << " ms." << std::endl;
	std::cout << "Total latency of migrate buffer to host is: " << migrate_to_host.latency() << " ms." << std::endl;
	std::cout << "Total average latency of migrate buffer to host is: " << migrate_to_host.avg_latency() << " ms." << std::endl;
    std::cout << "Total latency of LZW is: " << LZW_timer.latency() << " ms." << std::endl;
	std::cout << "Total latency of send data is: " << send_data_timer.latency() << " ms." << std::endl;
    std::cout << "Total time is: " << total_timer.latency() << " ms." << std::endl;
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

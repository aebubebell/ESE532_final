# ESE532_final

[repo]: https://github.com/aebubebell/ESE532_final	"github repo for this  project"



## 1.Software implementation latency

### (1) run in host machine, file to file 

Total latency of CDC is: 0.656716 ms.

Total latency of CMD is: 3.28696 ms.

Total latency of LZW is: 4.27539 ms.

Total latency of send data is: 0.008129 ms.

Total time is: 8.49791 ms.

Output Throughput to DRAM: 13.4122 Mb/s. (Latency: 0.00849791s).

\---------------------------------------------------------------

write 3510 to file

### (2) run on board, ethernet to file

setting up sever...

server setup complete!

Total latency of CDC is: 0.72279 ms.

Total latency of CMD is: 71.4183 ms.

Total latency of LZW is: 7.66038 ms.

Total latency of send data is: 0.01733 ms.

\---------------------------------------------------------------

write 3510 to file

--------------- Key Throughputs ---------------

Input Throughput to Encoder: 6197.72 Mb/s. (Latency: 1.839e-05s).

Output Throughput to DRAM: 1.41892 Mb/s. (Latency: 0.0803261s).

### (3) run on board , file to file, change ordinary sha into neon sha

Total latency of CDC is: 0.60857 ms.

Total latency of CMD is: 1.87715 ms.

Total latency of LZW is: 7.66404 ms.

Total latency of send data is: 0.01815 ms.

Total time is: 10.6679 ms.

Output Throughput to DRAM: 10.684 Mb/s. (Latency: 0.0106679s).



### (4)run on board, LZW on neon,ethernet to file, neon sha

Total latency of CDC is: 0.70946 ms.

Total latency of CMD is:  1.42086 ms.

Total latency of map buffer is: 26.8229 ms.

Total latency of read is: 8.23301 ms.

Total latency of kernel is: 4.58577 ms.

Total latency of write is: 24.7353 ms.

Total latency of LZW is: 66.2817 ms.

Total latency of send data is: 4.69682 ms.

Total time is: 77.7869 ms.

Output Throughput to DRAM: 1.46523 Mb/s. (Latency: 0.0777869s).

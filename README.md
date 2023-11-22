# ESE532_final

[repo]: https://github.com/aebubebell/ESE532_final	"github repo for this  project"



## 1.Software implementation latency

### (1) run in host machine, file to file 

Total latency of CDC is: 0.656716 ns.

Total latency of CMD is: 3.28696 ns.

Total latency of LZW is: 4.27539 ns.

Total latency of send data is: 0.008129 ns.

Total time is: 8.49791 ns.

Output Throughput to DRAM: 13.4122 Mb/s. (Latency: 0.00849791s).

\---------------------------------------------------------------

write 3510 to file

### (2) run on board, ethernet to file

setting up sever...

server setup complete!

Total latency of CDC is: 0.72279 ns.

Total latency of CMD is: 71.4183 ns.

Total latency of LZW is: 7.66038 ns.

Total latency of send data is: 0.01733 ns.

\---------------------------------------------------------------

write 3510 to file

--------------- Key Throughputs ---------------

Input Throughput to Encoder: 6197.72 Mb/s. (Latency: 1.839e-05s).

Output Throughput to DRAM: 1.41892 Mb/s. (Latency: 0.0803261s).

### (3) run on board , file to file, change ordinary sha into neon sha

Total latency of CDC is: 0.60857 ns.
Total latency of CMD is: 1.87715 ns.
Total latency of LZW is: 7.66404 ns.
Total latency of send data is: 0.01815 ns.
Total time is: 10.6679 ns.
Output Throughput to DRAM: 10.684 Mb/s. (Latency: 0.0106679s).

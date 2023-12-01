# ESE532_final

[repo]: https://github.com/aebubebell/ESE532_final	"github repo for this  project"



## 1.Software/hardware implementation latency

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



### (5) run on board, LZW on FPGA using for loop and single buffer for each pointer,ethernet to file, neon sha 

Total latency of CDC is: 0.78844 ms.

Total latency of CMD is:  0.71536 ms.

Total latency of map buffer is: 1.26869 ms.

Total latency of read is: 11.8567 ms.

Total latency of kernel is: 4.67049 ms.

Total latency of write is: 10.2695 ms.

Total latency of LZW is: 28.0767 ms.

Total latency of send data is: 0.03089ms.

Total time is: 37.1131 ms.

Output Throughput to DRAM: 3.07105 Mb/s. (Latency: 0.0777869s).

### （6）run on board, LZW on FPGA using for loop and single buffer, encode 4 chunks together, ethernet to file, neon sha

Total latency of ethernet is: 0.02101 ms.

Total latency of CDC is: 0.69341 ms.

Total latency of CMD is: 0.58388 ms.

Total latency of LZW is: 10.4269 ms.

Total time is: 11.7956 ms.

--------------- Key Throughputs ---------------

Input Throughput to Encoder: 5424.85 Mb/s. (Latency: 2.101e-05

s).

data rate of CDC: 164.37 Mb/s. (Latency: 0.00069341s).

data rate of CMD: 195.204 Mb/s. (Latency: 0.00058388s).

data rate of LZW: 10.9309 Mb/s. (Latency: 0.0104269s).

Output Throughput to DRAM: 9.66259 Mb/s. (Latency: 0.0117956s).

\--------------------------------------------------------------

### (6) newest throughput

write file with 3510

Total latency of ethernet is: 0.01531 ms.

Total latency of CDC is: 0.69324 ms.

Total latency of CMD is: 0.57167 ms.

Total latency of LZW is: 3.55424 ms.

Total latency of send data is: 0.01338 ms.

Total time is: 5.09881 ms.

--------------- Key Throughputs ---------------

Input Throughput to Encoder: 7444.55 Mb/s. (Latency: 1.531e-05s).

data rate of CDC: 164.411 Mb/s. (Latency: 0.00069324s).

data rate of CMD: 199.374 Mb/s. (Latency: 0.00057167s).

data rate of LZW: 32.0676 Mb/s. (Latency: 0.00355424s).

Output Throughput to DRAM: 22.3534 Mb/s. (Latency: 0.00509881s).

\---------------------------------------------------------------

# 2. correct output

## 2.1 compress length

compress_length is: 210
compress_length is: 58
compress_length is: 1
compress_length is: 307
compress_length is: 124
compress_length is: 190
compress_length is: 506
compress_length is: 113
compress_length is: 252
compress_length is: 46
compress_length is: 36
compress_length is: 60
compress_length is: 226
compress_length is: 47

## 2.2 header

write header 630 to file
write header 174 to file
write header 4 to file
write header 922 to file
write header 372 to file
write header 570 to file
write header 1518 to file
write header 340 to file
write header 756 to file
write header 138 to file
write header 108 to file
write header 180 to file
write header 678 to file
write header 3 to file
write header 5 to file
write header 7 to file
write header 9 to file
write header 11 to file
write header 13 to file
write header 15 to file
write header 17 to file
write header 19 to file
write header 21 to file
write header 23 to file
write header 25 to file
write header 3 to file
write header 5 to file
write header 7 to file
write header 9 to file
write header 11 to file
write header 13 to file
write header 15 to file
write header 17 to file
write header 19 to file
write header 21 to file
write header 23 to file
write header 25 to file
write header 3 to file
write header 5 to file
write header 7 to file
write header 9 to file
write header 11 to file
write header 13 to file
write header 15 to file
write header 17 to file
write header 19 to file
write header 21 to file
write header 23 to file
write header 25 to file
write header 3 to file
write header 5 to file
write header 7 to file
write header 9 to file
write header 11 to file
write header 13 to file
write header 15 to file
write header 17 to file
write header 19 to file
write header 21 to file
write header 23 to file
write header 142 to file

## 2.3 encoder array

-------------------------------------------
The encode array for chunk [0] is :

84 104 101 32 76 105 116 116 108 258 80 114 105 110 99 258 67 104 97 112 116 101 114 32 73 10 79 269 258 119 257 110 279 32 119 97 115 32 115 105 120 32 121 101 97 114 292 111 108 100 288 115 97 119 32 97 32 109 97 103 110 105 102 105 270 110 116 32 112 319 116 117 114 258 268 310 32 98 111 111 107 44 32 99 97 108 264 305 84 114 117 258 83 116 111 267 101 292 102 114 111 109 32 78 97 326 328 337 97 333 117 322 116 257 323 267 109 101 118 340 32 102 350 352 116 46 279 322 290 292 311 324 99 361 258 111 102 331 333 311 99 111 110 115 116 267 388 350 32 330 368 258 97 388 32 391 293 290 341 111 119 268 103 310 287 97 316 313 108 381 72 277 329 385 338 111 112 121 410 392 406 32 100 114 308 417 46 10 66 111 97 10 73 287 436 333 335 404 322 307 105 100 58 32 34 444 395 397 399 267

-------------------------------------------
The encode array for chunk [1] is :

99 116 111 114 115 32 115 119 97 108 108 111 119 32 116 104 101 105 114 32 112 114 101 121 32 119 104 111 108 101 44 280 105 270 111 117 116 32 99 271 119 105 110 103 32 288 46 32 65 102 116 101 274 270 97 292 270 278

-------------------------------------------
The encode array for chunk [2] is :

32

-------------------------------------------
The encode array for chunk [3] is :

97 114 101 32 110 111 116 32 97 98 108 258 116 111 32 109 111 118 101 44 263 110 100 32 116 104 101 121 32 115 266 101 112 279 104 114 111 117 103 104 289 258 115 105 120 270 111 110 280 115 289 97 262 280 282 259 101 101 278 102 111 114 32 100 105 103 101 115 116 105 302 46 34 10 73 32 112 302 100 101 257 278 334 287 108 121 275 309 110 275 272 335 296 263 100 273 303 117 257 305 111 102 348 106 117 110 103 266 46 32 65 277 263 102 116 347 115 111 109 258 119 316 107 32 119 105 280 263 32 99 111 108 316 313 331 101 110 99 105 108 32 330 115 117 99 99 312 334 278 105 110 270 97 107 405 103 270 283 102 105 114 323 318 114 97 380 361 364 77 283 68 419 421 411 78 117 109 98 347 79 110 101 364 73 262 387 111 107 389 372 374 280 410 32 108 105 443 289 105 115 58 10 72 307 329 284 104 111 119 389 109 283 109 97 323 335 112 105 101 401 279 269 309 32 103 291 119 110 45 117 112 115 275 97 367 469 443 278 309 109 379 281 309 317 478 100 427 448 102 114 320 104 370 436 493 281 109 46 10 66 117 308 281 283 489 115 464 336 58 32 34 70 505 294 508 63 32 87 104 283 115 462 117 108 278 489 283 302 258 433 32 504 506 508 389 98 519 32 104 307 63 328 424 418 420 448 119

-------------------------------------------
The encode array for chunk [4] is :

97 115 32 110 111 116 32 97 32 112 105 99 116 117 114 101 32 111 102 262 32 104 97 116 46 32 73 261 119 256 275 265 267 269 271 273 275 98 111 263 99 111 110 115 116 114 266 116 111 114 32 100 105 103 101 299 105 110 103 262 110 32 101 108 101 112 277 110 279 32 66 117 261 115 312 99 271 116 104 271 103 114 111 119 110 45 117 112 257 119 101 270 258 260 262 98 319 32 303 32 117 110 100 346 299 97 357 32 105 116 44 281 32 109 97 358 315 260 334 305 100 114 97 119

-------------------------------------------
The encode array for chunk [5] is :

105 110 103 58 32 73 32 100 114 101 119 32 116 104 101 32 256 115 105 100 270 111 102 32 97 32 98 111 280 99 111 110 115 116 114 105 99 116 111 114 44 32 115 111 267 104 97 116 300 270 103 114 111 119 110 45 117 112 115 32 285 117 108 100 297 101 270 105 303 99 108 101 97 114 108 121 46 32 84 269 121 279 108 119 97 121 314 110 321 319 293 32 301 118 270 268 256 103 314 101 120 112 108 97 256 101 100 332 77 336 68 114 97 119 352 32 78 117 109 98 101 114 333 119 299 108 111 111 107 361 32 108 105 384 300 105 115 58 10 69 326 112 301 110 303 272 274 350 269 281 283 10 334 305 307 309 311 313 39 32 264 115 112 286 115 101 296 351 314 116 105 109 421 32 339 424 299 97 100 118 391 270 427 267 380 340 279 273 275 32

-------------------------------------------
The encode array for chunk [6] is :

109 121 32 100 114 97 119 105 110 103 115 32 111 102 32 98 111 97 32 99 111 110 115 116 114 105 99 116 111 114 115 44 32 119 104 101 116 290 114 32 102 114 111 109 32 292 101 32 263 115 105 100 302 284 300 290 267 117 116 305 307 287 97 110 100 258 101 118 111 116 302 256 115 101 108 269 304 325 97 320 283 32 103 101 111 103 260 112 104 121 287 104 105 278 284 345 32 97 280 292 109 291 281 317 319 337 260 109 109 353 46 32 84 104 97 116 303 266 289 351 370 310 302 97 338 267 269 305 120 287 73 337 97 118 302 117 112 288 369 371 109 105 103 104 371 369 389 270 101 101 110 352 32 364 103 110 105 102 281 405 371 99 353 404 294 97 266 273 112 97 263 325 114 366 386 369 320 98 404 406 100 348 290 353 325 110 101 432 257 301 295 425 108 117 114 308 269 256 32 68 260 262 264 32 78 117 109 433 294 79 441 352 360 453 455 261 263 103 459 461 463 367 119 111 366 71 297 119 110 45 391 266 441 389 294 117 319 101 285 116 318 320 318 121 292 472 270 444 290 109 328 108 389 286 467 320 105 371 348 300 105 450 115 298 302 102 309 99 347 108 259 405 300 111 403 352 108 119 97 121 422 360 521 450 489 32 101 120 112 108 425 411 458 500 264 266 336 301 109 46 10 83 529 301 406 386 523 111 328 467 324 293 32 112 297 102 101 115 305 276 359 320 108 101 353 441 335 529 112 105 108 324 352 516 544 318 572 429 32 401 520 586 483 407 108 512 116 578 267 540 97 108 108 568 439 266 268 377 288 284 525 59 510 303 513 551 114 117 302 292 376 338 340 342 344 594 421 403 527 489 257 117 328 102 117 608 336 356 366 65 371 273 103 545 110 99 302 561 318 258 348 116 472 117 437 32 67 347 110 273 296 298 32 65 280 122 276 97 429 269 276 302 338 314 32 586 278 303 406 445 411 398 116 287 115 117 523 32 107 110 111 119 578 100 380 514 118 606 117 97 98 578 555 73 684 311 275 449 564 612 500 266 600 571 32 430 388 302 431 407 341 579 395 498 541 650 111 491 427 373 512 104 725 450 376 364 110 257 112 339 544 302 289 529 595 633 406 275 650 493 581 288 736 408 370 325

-------------------------------------------
The encode array for chunk [7] is :

114 115 32 111 102 32 99 111 110 115 101 113 117 101 110 99 101 46 32 73 32 104 97 118 101 32 108 105 279 100 32 97 32 103 114 101 97 116 32 100 291 108 286 109 263 103 288 114 111 119 110 45 117 112 115 273 275 277 279 32 265 269 32 116 104 101 109 32 105 110 116 105 109 292 101 108 121 44 261 108 111 265 286 293 277 110 100 273 65 341 318 277 339 97 115 110 39 293 109 117 99 104 323 109 112 303 284 32 109 121 258 112 105

-------------------------------------------
The encode array for chunk [8] is :

110 105 111 110 32 111 102 32 116 104 101 109 46 10 87 265 110 101 118 101 114 32 73 32 109 101 116 260 272 260 262 264 266 32 119 104 111 32 115 101 266 101 100 263 292 280 32 97 282 97 108 108 32 99 108 101 97 114 45 115 105 103 104 116 297 44 277 263 114 105 297 263 265 32 101 120 112 275 105 280 110 282 261 293 291 119 105 110 103 32 104 334 279 121 32 68 114 97 341 343 32 78 117 109 98 275 32 79 272 321 290 105 99 104 322 104 97 274 302 108 119 97 121 115 32 107 101 112 116 46 322 119 111 117 108 298 116 114 349 116 292 102 342 298 388 116 321 115 111 321 105 286 346 379 376 379 97 32 332 114 403 259 338 392 117 101 32 117 110 100 275 115 116 97 424 342 103 385 66 117 401 289 291 273 361 105 282 410 321 265 321 111 276 115 445 289 388 390 374 376 378 293 377 58 10 34 84 371 282 105 411 345 303 46 34 10 462 101 259 278 387 389 298 272 274 276 428 108 107 299 327 303 413 426 258 302 98 400 32 492 412 99 258 427 324 99 395 415 446 276 112 324

-------------------------------------------
The encode array for chunk [9] is :

109 101 118 97 108 32 102 111 114 101 115 116 115 44 32 263 32 266 97 114 115 46 32 73 32 119 111 117 108 100 32 98 114 105 110 103 32 109 121 115 101 108 102 32 100 111

-------------------------------------------
The encode array for chunk [10] is :

119 110 32 116 111 32 104 105 115 32 108 101 118 101 108 46 32 73 32 119 111 117 108 100 258 97 108 107 258 260 262 109 32 97 98 111

-------------------------------------------
The encode array for chunk [11] is :

117 116 32 98 114 105 100 103 101 44 32 97 110 100 32 103 111 108 102 265 267 269 112 272 105 116 105 99 115 275 268 32 110 101 99 107 281 101 115 46 32 65 286 116 104 101 270 114 111 119 110 45 117 112 32 119 111 117 108 269

-------------------------------------------
The encode array for chunk [12] is :

98 101 32 103 114 101 97 116 108 121 32 112 108 261 115 101 100 32 116 111 32 104 97 118 257 109 101 116 32 115 117 99 104 32 97 284 101 110 115 105 98 268 32 109 97 110 46 10 79 110 99 257 119 104 292 32 73 32 119 97 115 284 105 120 32 121 261 114 316 111 108 272 312 115 97 119 289 298 97 103 110 105 102 105 306 110 283 112 339 116 117 260 32 105 110 332 98 111 111 107 44 32 99 97 108 268 272 84 114 117 257 83 274 114 105 101 316 102 114 111 109 32 78 262 346 101 356 97 352 117 283 116 309 266 369 281 118 359 32 102 111 260 115 116 46 311 283 314 316 290 343 99 345 347 111 102 351 111 290 99 111 293 116 369 407 396 348 350 387 257 97 407 32 410 284 314 360 111 119 349 103 289 350 300 105 299 108 400 72 101 347 105 404 357 111 112 265 429 424 32 100 114 330 435 302 66 413 10 73 423 388 352 354 348 283 329 105 100 58 32 34 461 414 416 398 369

-------------------------------------------
The encode array for chunk [60] is :

98 101 32 103 114 101 97 116 108 121 32 112 108 261 115 101 100 32 116 111 32 104 97 118 257 109 101 116 32 115 117 99 104 32 97 284 101 110 115 105 98 268 32 109 97 110 46

## 记录一个4chunk一起处理的BUG

对于LittlePrince.txt文件，最后一次调用LZW是处理chunk[12]和chunk[60],所以chunk[12]会被送到chunk[59]上，而原来应该在的位置则由chunk[13]替代，目前没有解决方案。

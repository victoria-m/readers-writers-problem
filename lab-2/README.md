#Summary
This lab addresses the first readers-writers problem. The objective of this lab is to fill in the code so that the program will
be able to create multiple multiple reader and writer threads. The writer threads will first write account information to a new file and the reader threads will read that account information. Mutex locks are used to lock sections of code so that both types of threads do not read and write to it at once.

#Files
* **rw.c** contains the main() function as well as additional functions
* **rw.h** is a custom header file containing the number of reader/writer thread iterations, max balance, and account struct

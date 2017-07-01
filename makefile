#! /bin/bash
Gunz_UDP: Gunz_UDP.c
	gcc -o Gunz_UDP Gunz_UDP.c   
clean:
	rm -f * .o

/**
 *  LZW Decompression
 */
 
#ifndef LZW_H
#define LZW_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef uint8_t byte;

struct lzw_params{
	
	bool left;
	byte prev_bytes[2];
	byte* dict[4096];
	int dict_entry_sizes[4096];												//Sizes needed for fwrite
};

void free_dict (byte* dict[], int from, int to);							//Free all entries in dict between chosen range (inclusive) 

void load_dict_defaults (void* param, byte* dict[], int sizes[]);			//Load first 256 ASCII characters into dict

int read_twelve_bits(FILE* source);												//Read 12 bits of source file returning int value

int decompress (void* param, FILE* source, FILE* dest);						//Decompress source file and ouput to dest returns 0 on sucess 

#endif // LZW_H
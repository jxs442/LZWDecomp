/**  
 *  
 *  LZW decompression algorithm implementation for 12-bit fixed-width compression format.
 *
 */

#include "lzw.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>


void free_dict (byte* dict[], int from, int to){							//Dealloc all entries in dict between chosen range (inclusive)
	
    for (int i = from; i < to+1; i++){
        free(dict[i]);
    }
}

void load_dict_defaults (void* param, byte* dict[], int sizes[]){			//Load first 256 ASCII characters into dict

	lzw_params* p = (lzw_params*)param;
    byte* tmp;
    byte x = 0;
    for (int i = 0; i < 256; i++){
		
        tmp = (byte*) malloc(1);
        tmp[0] = x++;
        dict[i] = tmp;
        p->dict_entry_sizes[i] = 1;
    }
}

int read_twelve_bits(FILE* source){											//Read 12 bits of source file returning int value
    
	int val;
    static bool left = true;												//LHS or RHS of byte pair
    static byte prev_bytes[2];

    byte buffer[3];

    if (left){
		
        if (fread(buffer, 1, 2, source) != 2){	return 0; }					//EOF, even # entries

        if (fread(buffer+2, 1, 1, source) != 1){							//EOF, buffer[0] & buffer[1] are padded last entry
            val = (buffer[0] << 8) | buffer[1];
        }
        else
        {
            val = (buffer[0] << 4) | (buffer[1] >> 4);
            left = false;
            prev_bytes[0] = buffer[1];
            prev_bytes[1] = buffer[2];
        }
    }
    else
    {
        val = ((prev_bytes[0] & 0x0F) << 8) | prev_bytes[1];				//0x0F in binary 0000111115; takes only RHS of prev_bytes[0]
        left = true;
    }
    return val;
}


int decompress (void* param, FILE* source, FILE* dest){						//Decompress source file and ouput to dest returns 0 on sucess 

	lzw_params* p = (lzw_params*)param;
    load_dict_defaults(p, p->dict, p->dict_entry_sizes);

    int dict_pos = 256;

    int prev_code, curr_code;
    byte* prev_string, *curr_string;

    // First character
    curr_code = read_twelve_bits(source);
    size_t size = p->dict_entry_sizes[curr_code];
    curr_string = (byte*) malloc(size + 1);									//+1 as appending another char

    if (curr_string == NULL){
		
        std::cerr << "LZWDecomp: Error curr_string is NULL" << std::endl;
        return 1;
    }

    memcpy(curr_string, p->dict[curr_code], size);
    fwrite(curr_string, size, 1, dest);
    prev_string = curr_string;
	prev_code = curr_code;


    // Other characters
    size_t size_old;
    while (curr_code){

		curr_code = read_twelve_bits(source);
		size_old = p->dict_entry_sizes[prev_code];
		
        if (curr_code > dict_pos){
			
            std::cerr << "LZWDecomp: Error curr_code out of dictionary range" << std::endl;
            return 1;
        }
        
        if (curr_code < dict_pos){															//In dict
        
            size = p->dict_entry_sizes[curr_code];
            curr_string = (byte*) malloc(size + 1); 
            if (curr_string == NULL){
            
                std::cerr << "LZWDecomp: Error assigning curr_string." << std::endl;
                return 1;
            }
            memcpy(curr_string, p->dict[curr_code], size);
            fwrite(curr_string, size, 1, dest);

            prev_string[size_old] = curr_string[0];
            p->dict[dict_pos] = prev_string;
            p->dict_entry_sizes[dict_pos++] = size_old + 1;

            // reset dict if full 
            if (dict_pos >= 4096){
				
                free_dict(p->dict, 256, 4095);
                dict_pos = 256;
            }
        }
        else{																				//Not in dict
        
            memmove(prev_string + size_old, prev_string, 1);								//memmove allows overlap
            size = size_old + 1;
            fwrite(prev_string, size, 1, dest);
            p->dict[dict_pos] = prev_string;
            p->dict_entry_sizes[dict_pos++] = size; 
            curr_string = (byte*) malloc(size + 1);  
			
            if (curr_string == NULL){
				
                std::cerr << "LZWDecomp: Error curr_string is NULL" << std::endl;
                return 1;
            }
            memcpy(curr_string, prev_string, size);
        }
        prev_code = curr_code;
        prev_string = curr_string;
    }

    free(prev_string);       
    free_dict(p->dict, 0, dict_pos-1);

    return 0;
}
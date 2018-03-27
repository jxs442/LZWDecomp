/**
 *	LZW Deomp Task
 *  usage: LZWdecompression.exe <source> <output>
 */

#include "lzw.h"
#include <stdio.h>
#include <iostream>

using std::cout;
using std::endl;

int main (int argc, char* argv[])
{   
    // check for correct number of args
    if (argc != 3){
		
        cout << "Error: Incorrect numbr of arguments \nFormat: LZWdecompression.exe <source> <output>" << endl;
        return 1;
    }

    // open compressed file
    FILE* source = fopen(argv[1], "rb");
    if (source == NULL){
		
        cout << "Error: Cannot open " << argv[1] << endl;
        return 1;
    }

    // open destination file
    FILE* dest = fopen(argv[2], "wb");
    if (dest == NULL){
		
        cout << "Error: Cannot open " << argv[2] << endl;
        return 1;
    }

    // decompress
	lzw_params params;
    if (decompress(&params,source, dest) == 1){
		
        cout << "Error: Decompression failed\n" <<endl;
        fclose(source);
        fclose(dest);
        return 1;
    }
    else
    {
        fclose(source);
        fclose(dest);
        return 0;
    }
}
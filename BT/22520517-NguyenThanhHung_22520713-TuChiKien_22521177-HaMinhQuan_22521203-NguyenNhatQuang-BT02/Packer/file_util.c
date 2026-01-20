/*
Code check PE header: https://gist.github.com/huytohl/42af52763491083b1f28
Định nghĩa các struct của windows.h: https://learn.microsoft.com/en-us/archive/msdn-magazine/2002/february/inside-windows-win32-portable-executable-file-format-in-detail
*/

#include "file_util.h"
#include <stdio.h>

int check_validPEfile(const char* filename){
    FILE* fptr; // Pointer for file operation
    IMAGE_DOS_HEADER dosheader; // Dos header variable to save all dos headers info
    DWORD Signature; // Variable to save the pe signature

    fptr = fopen(filename, "rb");
    // Check if file can be open
    if(fptr == NULL){
        printf("Error: File can't be opened.\n");
        return 1;
    }
    // Get DOS header info 
    fread(&dosheader, sizeof(dosheader), 1, fptr);
    // Check if the file is a PE file via "MZ" signature
    if(dosheader.e_magic != IMAGE_DOS_SIGNATURE){ // IMAGE_DOS_SIGNATURE = 0x5A4D = MZ
        printf("Can't find MZ in DOS header, not a PE file.\n");
        fclose(fptr);
        return 1;
    }
    // Move to nt headers
    fseek(fptr, dosheader.e_lfanew, SEEK_SET);
    // Get PE signature 
    fread(&Signature, sizeof(DWORD), 1, fptr);
    if(Signature != IMAGE_NT_SIGNATURE){ // IMAGE_NT_SIGNATURE = 0x00004550 = PE00
        printf("Can't find PE signature.\n");
        fclose(fptr);
        return 1;
    }

    fclose(fptr);
    return 0;
}
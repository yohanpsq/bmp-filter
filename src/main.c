#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "filters.h"

#pragma pack(push, 1)       // avoid adding empty bytes to align structures during compilation

typedef struct {            // structure of the file Header of the bmp file
    uint16_t signature;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
} FileHeader;

typedef struct {            // structure of the information Header of the bmp file
    uint32_t headerSize;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bitCount;
    uint32_t compression;
    uint32_t sizeImage;
    uint32_t hRes;
    uint32_t vRes;
    uint32_t clrUsed;
    uint32_t clrImportant;
} InfoHeader;

#pragma pack(pop)


int main(int argc, char *argv[]) {
    if (argc < 4) {
            printf("Usage: %s <input.bmp> <output.bmp> <filter_flag> [intensity]\n", argv[0]);
            printf("Filters: --inverse, --grayscale, --sepia, --blur, --threshold\n");
            return EXIT_FAILURE;
        }
    
    const char *inputPath = argv[1];
        const char *outputPath = argv[2];
        const char *filterTag = argv[3];
    
    int intensity = 1; // default value
        if (argc >= 5) {
            intensity = atoi(argv[4]); // convert the string argument into an int
        }
    
    // READING
    FILE *f = fopen(inputPath, "rb");  // open the .bmp file in binary read mode
    if (!f) {
        perror("fopen input");
        return EXIT_FAILURE;
    }
    
    // creation of the headers to store the file values
    FileHeader fileH;
    InfoHeader infoH;
    
    // BMP check up
    if (fread(&fileH, sizeof(fileH), 1, f) != 1) {
        fclose(f);
        return EXIT_FAILURE;
    };
    if (fileH.signature != 0x4D42) {
        fclose(f);
        printf("Not a BMP file (Signature error)\n");
        return EXIT_FAILURE;
    }
    if (fread(&infoH, sizeof(infoH), 1, f) != 1) {
        fclose(f);
        printf("Failed to load InfoHeader. \n");
        return EXIT_FAILURE;
    }
    
    int height = abs(infoH.height); // the height can be negative, so we use abs()
    int width = infoH.width;
    
    printf("Image successfully loaded: %d x %d pixels \n", width, height);
    
    // calculing sizes
    int rowSize = width * infoH.bitCount / 8; // Real numbers of pixels in a line
    int padding = (4 - (rowSize % 4)) % 4;    // Empty bytes at the end of line
    
    // memory allocation the store all the pixel values
    unsigned char* lines = malloc(rowSize * height);
    if (!lines) {
        fclose(f);
        perror("malloc");
        return EXIT_FAILURE;
    }
    // place the cursor at the beginning of the pixels
    fseek(f, fileH.offset, SEEK_SET);
    // Reading file
    for (int line = 0; line < height; line++) {
        unsigned char* destination = lines + (line * rowSize);
        fread(destination, rowSize, 1, f); // reading pixels
        fseek(f, padding, SEEK_CUR);       // ignoring the padding
    }
    
    // FILTERING
    if (strcmp(filterTag, "--inverse") == 0) {
            inverse(lines, rowSize * height);
        }
        else if (strcmp(filterTag, "--grayscale") == 0) {
            grayscale(lines, rowSize * height);
        }
        else if (strcmp(filterTag, "--blur") == 0) {
            // Calling the function multiple times for the intensity
            printf("Applying blur with intensity %d...\n", intensity);
            for(int i = 0; i < intensity; i++) {
                blur(lines, infoH.width, height);
            }
        }
        else if (strcmp(filterTag, "--sepia") == 0) {
            sepia(lines, rowSize * height);
        }
        else if (strcmp(filterTag, "--threshold") == 0) {
            threshold(lines, rowSize * height);
        }
        else {
            printf("Unknown filter: %s\n", filterTag);
        }

    
    // Header updates
    fileH.offset = sizeof(FileHeader) + sizeof(InfoHeader);
    infoH.headerSize = sizeof(InfoHeader);
    fileH.size = fileH.offset + (rowSize + padding) * height;
    infoH.sizeImage = (rowSize + padding) * height;
    infoH.compression = 0;

    
    // WRITING
    FILE *output = fopen(outputPath, "wb"); // creation of the output file
    if (!output) {
        perror("fopen output");
        free(lines);
        fclose(f);
        return EXIT_FAILURE;
    }
    // writing the updated headers
    fwrite(&fileH, sizeof(FileHeader), 1, output);
    fwrite(&infoH, sizeof(InfoHeader), 1, output);
    
    uint8_t zero = 0;
    for (int line = 0; line < height; line++) {
        unsigned char *rowPtr = &lines[line * rowSize];
        
        // Writing the pixels
        if (fwrite(rowPtr, rowSize, 1, output) != 1) {
            perror("fwrite row");
            // Nettoyage avant de quitter
            fclose(output); fclose(f); free(lines);
            return EXIT_FAILURE;
        }
        
        // Writing the padding ('zero' bytes)
        for (int i = 0; i < padding; i++) {
            fwrite(&zero, 1, 1, output);
        }
    }
    
    printf("%s file successfully generated!\n", outputPath);
    
    // free the memory
    free(lines);
    fclose(output);
    fclose(f);
    return EXIT_SUCCESS;
}

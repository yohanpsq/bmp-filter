#include "filters.h"
#include <string.h>
#include <stdlib.h>

int clamp(int value) {
    if (value < 0) return 0;
    if (value > 255) return 255;
    return value;
}

void inverse(unsigned char* lines, int size) {
    for (int car = 0; car < size; car++) {
        lines[car] = 255 - lines[car];
    }
}

void grayscale(unsigned char* lines, int size) {
    for (int car = 0; car < size; car += 3) {
        float blue = lines[car];
        float green = lines[car+1];
        float red = lines[car+2];
        
        int mean = (int)(0.114 * blue + 0.587 * green + 0.299 * red);
        
        lines[car]   = (unsigned char)mean;
        lines[car+1] = (unsigned char)mean;
        lines[car+2] = (unsigned char)mean;
    }
}

void threshold(unsigned char* lines, int size) {
    grayscale(lines, size);
    for (int car = 0; car < size; car++) {
        if (lines[car] < 128) {
            lines[car] = 0;
        }
        else {
            lines[car] = 255;
        }
    }
}

void sepia(unsigned char* lines, int size) {
    for (int car = 0; car < size; car += 3) {

        float blue = lines[car];
        float green = lines[car+1];
        float red = lines[car+2];

        int newRed = (int)(0.189 * blue + 0.769 * green + 0.393 * red);
        int newGreen = (int)(0.168 * blue + 0.686 * green + 0.349 * red);
        int newBlue = (int)(0.131 * blue + 0.534 * green + 0.272 * red);
        
        lines[car]   = (unsigned char)clamp(newBlue);
        lines[car+1] = (unsigned char)clamp(newGreen);
        lines[car+2] = (unsigned char)clamp(newRed);
    }
}

void blur(unsigned char* lines, int width, int height) {
    int totalSize = width * height * 3;
    unsigned char* temp = malloc(totalSize);
    if (!temp) return;

    memcpy(temp, lines, totalSize);
    
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            for (int c = 0; c < 3; c++) {
                int sum = 0;
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        int neighborIndex = ((y + dy) * width + (x + dx)) * 3 + c;
                        sum += lines[neighborIndex];
                    }
                }
                int targetIndex = (y * width + x) * 3 + c;
                temp[targetIndex] = (unsigned char)(sum / 9);
            }
        }
    }

    memcpy(lines, temp, totalSize);

    free(temp);
}


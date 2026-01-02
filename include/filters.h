#ifndef FILTERS_H
#define FILTERS_H

int clamp(int value);

void inverse(unsigned char* lines, int size);

void grayscale(unsigned char* lines, int size);

void threshold(unsigned char* lines, int size);

void sepia(unsigned char* lines, int size);

void blur(unsigned char* lines, int width, int height);


#endif // !FILTERS_H

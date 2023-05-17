#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <malloc.h>

void uint32ToBytes(unsigned char *buf,unsigned int val ,bool isLE);
FILE *getFileSizeAndPointer(const char *path, unsigned int *fileSize);
unsigned char *fileToBytes(const char *path, unsigned int *fileSize);
bool bytesToFile(unsigned char *buf,unsigned int fileSize, const char *path);
#endif //_UTILS_H_
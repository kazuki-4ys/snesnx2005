#include "utils.h"

void uint32ToBytes(unsigned char *buf,unsigned int val ,bool isLE){
    unsigned char tmpU8;
    for(int i = 0;i < 4;i++){
        if(isLE){
            tmpU8 = (unsigned char)((val >> (i * 8)) & 0xFF);
        }else{
            tmpU8 = (unsigned char)((val >> ((3 - i) * 8)) & 0xFF);
        }
        *(buf + i) = tmpU8;
    }
}

FILE *getFileSizeAndPointer(const char* path, unsigned int *fileSize){
    int fd = open(path,O_RDONLY);
    struct stat fileStat;
    if(fd < 0)return NULL;
    if(fstat(fd, &fileStat) == -1){
        close(fd);
        return NULL;
    }
    if(fileSize)*fileSize = (unsigned int)fileStat.st_size;
    FILE*f = fdopen(fd, "rb");
    if(!f){
        close(fd);
        return NULL;
    }
    return f;
}

unsigned char *fileToBytes(const char* path, unsigned int *fileSize){
    FILE *f = getFileSizeAndPointer(path, fileSize);
    if(!f)return NULL;
    unsigned char *buf = (unsigned char*)calloc(*fileSize, sizeof(unsigned char));
    if(!buf)return NULL;
    fread(buf, sizeof(unsigned char), *fileSize, f);
    fclose(f);
    return buf;
}

bool bytesToFile(unsigned char *buf,unsigned int fileSize, const char *path){
    FILE*f = fopen(path,"wb");
    if(!f)return false;
    fwrite(buf,sizeof(char),fileSize,f);
    fclose(f);
    return true;
}
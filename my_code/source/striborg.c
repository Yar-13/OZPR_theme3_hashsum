#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "striborg.h"

#define FILE_BUFFER_SIZE 4096
#define DEFAULT_HASH_SIZE 512

TContext *CTX;

static void HashPrint(TContext *CTX)
{
    printf("%d bit hash sum: \n", CTX->hash_size);
    if (CTX->hash_size == 256)
        for(int i = 32; i < 64; i++)
            printf("%02x", CTX->hash[i]);
    else
        for(int i = 0; i < 64; i++)
            printf("%02x", CTX->hash[i]);
    printf("\n");
}

static void GetHashString(const char *str, int hash_size)
{
    uint8_t *buffer;
    buffer = malloc(strlen(str));
    memcpy(buffer, str, strlen(str));
    Init(CTX, hash_size);
    Update(CTX, buffer, strlen(str));
    Final(CTX);
    printf("\"Stribog\"\nString: %s\n", str);
    HashPrint(CTX);
}

static void GetHashFile(const char *file_name, int hash_size)
{
    FILE *file;
    uint8_t *buffer;
    size_t len;
    Init(CTX, hash_size);
    if ((file = fopen(file_name, "rb")) != NULL)
    {
        buffer = malloc((size_t) FILE_BUFFER_SIZE);
        while ((len = fread(buffer, (size_t) 1, (size_t) FILE_BUFFER_SIZE, file)))
            Update(CTX, buffer, len);
        free(buffer);
        Final(CTX);
        fclose(file);
        printf("\"Stribog\"\nFile name: %s\n", file_name);
        HashPrint(CTX);
    }
    else
        printf("File error: %s\n", file_name);
}

int main(int argc, char *argv[])
{
    CTX = (TContext*)(malloc(sizeof(TContext)));

    int hash_size = DEFAULT_HASH_SIZE;
    int opt;
    while ((opt = getopt(argc, argv, "hf:s:d:")) != -1)
    {
        switch (opt)
        {
            case 'd':
                if (strcmp(optarg, "256") == 0)
                    hash_size = 256;
                if (strcmp(optarg, "512") == 0)
                    hash_size = 512;
            break;
            case 'h':
                printf("\"Stribog\"\n./stribog [-d <256 or 512>] [-s <string>] [-f <file>]\n");
            break;
            case 'f':
                GetHashFile(optarg, hash_size);
            break;
            case 's':
                GetHashString(optarg, hash_size);
            break;
        }
    }
    return 0;
}



/*
    // -s=size
    // -f=file
    char **ptr;
    char filename[50];
    uint16_t hash_size;
    bool fl_file = false, fl_size=false;
    for(ptr=argv; *ptr != NULL; ptr++)
    {
        if(ptr[0][1] == 'f' && is_flag(ptr[0]))
        {
            int i=3;
            for(i; ptr[0][i] != '\0'; i++) filename[i-3] = ptr[0][i];
            filename[i-3] = '\0';
            fl_file = true;
            //printf("%s\n", filename);
        }
        if(ptr[0][1] == 's' && is_flag(ptr[0]))
        {
            char size[4];
            int i=3;
            for(i; ptr[0][i] != '\0'; i++) size[i-3] = ptr[0][i];
            size[i-3] = '\0';
            hash_size = atoi(size);
            fl_size = true;
            //printf("%d\n", hash_size);
        }
        if(argc == 1 || (ptr[0][1] == 'h'))
            printf("help message\n");
    }
    uint8_t *hash;
    if(fl_file && fl_size)
        hash = Hash(filename, hash_size);
    else
        printf("Error arguments.\n Use -f and -s.\n");

*/

/*int is_flag(char *ptr)
{
    return ((ptr[0]=='-')&&(ptr[2]=='=')) ? 1 : 0 ;
}*/
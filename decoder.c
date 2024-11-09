#include <stdio.h>
#include <string.h>
#include <stdlib.h>

FILE* read_file(char *path) {
    FILE* file = fopen(path, "r");
    return file;
}

void reverse_byte_order(unsigned int *number) {
    unsigned char *bytes = (unsigned char*)number;
    unsigned char byte = bytes[0];
    bytes[0] = bytes[3];
    bytes[3] = byte;
    byte = bytes[1];
    bytes[1] = bytes[2];
    bytes[2] = byte;
}

unsigned char header[] = {
 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A
};

typedef struct {
    unsigned int length;
    char type[5];
    void* data;
    unsigned int crc;
} Chunk;

typedef struct {
    unsigned int width;
    unsigned int height;
    char depth;
    char color;
    char compression;
    char filter;
    char interlace;
} Image;

int decode_header(FILE *file) {
    unsigned char bytes[8];
    int i;
    if ((i = fread(bytes, 1, 8, file)) != 8) {
        //printf("Couldn't load read header %d<8.", i);
        return 1;
    }

    if ((i = memcmp(bytes, header, 8)) != 0) {
        //printf("Header doesn't match %d.", i);
        return 1;
    }

    //printf("Decoded header\n");
    return 0;
}


int decode_chunk_length(FILE *file, Chunk *chunk) {
    unsigned int length;
    int i;
    if ((i = fread(&length, 1, 4, file)) != 4) {
        printf("Couldn't load chunk length %d<4.\n", i);
        return 1;
    }
    reverse_byte_order(&length);
    chunk->length = length; 
    //printf("Decoded length %u\n", length);

    return 0;
}

int decode_chunk_type(FILE *file, Chunk *chunk) {
    int i;
    if ((i = fread(chunk->type, 1, 4, file)) != 4) {
        printf("Couldn't load chunk type %d<4.\n", i);
        return 1;
    }

    chunk->type[4] = '\0';

    //printf("Decoded type %s\n", chunk->type);
    return 0;
}

int decode_chunk_data(FILE *file, Chunk *chunk) {
    chunk->data = malloc(chunk->length);

    unsigned int i;
    if ((i = (unsigned int) fread(chunk->data, 1, chunk->length, file)) != chunk->length) {
        printf("Couldn't load chunk data %d<%d.\n", i, chunk->length);
        return 1;
    }

    //printf("Decoded data %c\n", *((char*)chunk->data));
    return 0;
}

int decode_chunk_crc(FILE *file, Chunk *chunk) {
    unsigned int crc;
    int i;
    if ((i = fread(&crc, 1, 4, file)) != 4) {
        printf("Couldn't load chunk crc %d<4.\n", i);
        return 1;
    }

    reverse_byte_order(&crc);
    chunk->crc = crc;

    //printf("Decoded crc %d\n", crc);
    return 0;
}

void read_IHDR(Image *image, Chunk *chunk) {
    char *data = chunk->data;

    unsigned int width = *((unsigned int*) data); data += 4;
    reverse_byte_order(&width);

    unsigned int height = *((unsigned int*) data); data += 4;
    reverse_byte_order(&height);

    char depth = *((char*) data); data++;
    char color = *((char*) data); data++;
    char compression = *((char*) data); data++;
    char filter = *((char*) data); data++;
    char interlace = *((char*) data);

    image->width = width;
    image->height = height;
    image->depth = depth;
    image->color = color;
    image->compression = compression;
    image->filter = filter;
    image->interlace = interlace;
    
    printf("width=%u\nheight=%u\ndepth=%d\ncolor=%d\ncompression=%d\nfilter=%d\ninterlace=%d\n",
           width, height, depth, color, compression, filter, interlace);
}

Chunk* decode_chunk(Image *image, FILE *file) {
    Chunk *chunk = malloc(sizeof(Chunk));
    if (decode_chunk_length(file, chunk)) return NULL;
    if (decode_chunk_type(file, chunk)) return NULL;
    if (decode_chunk_data(file, chunk)) return NULL;
    if (decode_chunk_crc(file, chunk)) return NULL;

    if (strcmp("IHDR", chunk->type) == 0) {
        read_IHDR(image, chunk);
    }

    return chunk;
}

void decode_file(Image *image, FILE *file) {
    if (decode_header(file) != 0) {
        return;
    }
    
    Chunk *chunk;
    while ((chunk = decode_chunk(image, file)) != NULL && strcmp(chunk->type, "IEND") != 0);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        //printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    char *fileName = argv[1];
    FILE *file = read_file(fileName);

    Image image;
    decode_file(&image, file);
    
    return 0;
}

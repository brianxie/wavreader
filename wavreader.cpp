// Basic WAV file reader for WAVE_FORMAT_PCM
// compiled with Visual C++ Build Tools 2015
// not tested extensively (read: at all), so may be broken

// compile: cl wavreader.cpp
// usage: wavreader wavfile.wav
// output: wavfile.pcm


#include <windows.h>
#include <stdio.h>


int main(int argc, char* argv[]) {
    FILE* fp = NULL;
    FILE* dp = NULL;
    long fsize = 0;
    long dsize = 0;
    char* fbuf = NULL;
    char* dbuf = NULL;
    int rv = 0;

    // Loading the file into fbuf
    if (argc != 2) {
        printf("Invalid number of files specified\n");
        return 0;
    }

    fp = fopen(argv[1], "rb");
    if (fp == NULL) {
        printf("File %s could not be found\n", argv[1]);
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    printf("Size of %s is %d bytes\n", argv[1], fsize);

    fbuf = (char*) malloc(fsize * sizeof(char));
    if (fbuf == NULL) {
        printf("MALLOC failure\n");
        return 0;
    }

    rv = fread(fbuf, sizeof(char), fsize, fp);
    if (rv == fsize) {
        printf("%d bytes successfully read\n", fsize);
    } else {
        printf("ERROR: %d bytes read, %d bytes expected\n", rv, fsize);
        return 0;
    }
    fclose(fp);

    printf("\n");

    // Processing WAV header, loading PCM data into dbuf

    // Bytes [0,3] should be "RIFF"
    if (*((DWORD*) (fbuf + 0)) != 0x46464952) {
        printf("'RIFF' tag not found\n");
        return 0;
    } else {
        printf("'RIFF' tag found\n");
    }

    // Bytes [4,7] should be filesize - 8 (little endian)
    int size_tag = *((DWORD*) (fbuf + 4));
    if (size_tag != (fsize - 8)) {
        printf("Size tag incorrect\n");
        return 0;
    } else {
        printf("Size tag found: %d bytes\n", size_tag);
    }

    // Bytes [8,11] should be "WAVE"
    if (*((DWORD*) (fbuf + 8)) != 0x45564157) {
        printf("'WAVE' tag not found\n");
        return 0;
    } else {
        printf("'WAVE' tag found\n");
    }

    // fmt block
    printf("\n");

    // Bytes [12,15] should be "fmt "
    if (*((DWORD*) (fbuf + 12)) != 0x20746d66) {
        printf("'fmt ' tag not found\n");
        return 0;
    } else {
        printf("'fmt ' tag found\n");
    }

    // Bytes [16,19] is the size of this subchunk
    // This should be 16 for PCM data, but is (40?) for EXTENSIBLE data
    int pcm_tag = *((DWORD*) (fbuf + 16));
    if (pcm_tag != 16) {
        printf("PCM tag incorrect\n");
    } else {
        printf("PCM tag found\n");
    }

    // Bytes [20,21] should be 1 for WAVE_FORMAT_PCM, but may also be WAVE_FORMAT_EXTENSIBLE
    int compression_tag = *((WORD*) (fbuf + 20));
    if (compression_tag != 1) {
        printf("Compression tag incorrect\n");
    } else {
        printf("Compression tag found\n");
    }

    // Bytes [22,23] indicate number of channels
    int num_channels = *((WORD*) (fbuf + 22));
    printf("Number of channels: %d\n", num_channels);

    // Bytes [24,27] indicate sample rate (more accurately, frame rate)
    int sample_rate = *((DWORD*) (fbuf + 24));
    printf("Sample rate: %d hz\n", sample_rate);

    // Bytes [28,31] indicate byte rate (samplerate * numchannels * bytesperchannel)
    int byte_rate = *((DWORD*) (fbuf + 28));
    printf("Byte rate: %d bytes/s\n", byte_rate);

    // Bytes [32,33] indicate block alignment (more accurately, frame size) (numchannels * bytespersample)
    int block_alignment = *((WORD*) (fbuf + 32));
    printf("Block alignment: %d bytes\n", block_alignment);

    // Bytes [34,35] indicate bits per sample
    int bits_per_sample = *((WORD*) (fbuf + 34));
    printf("Bits per sample: %d bits\n", bits_per_sample);

    // data block
    printf("\n");

    // Bytes [36,39] should be "data"
    if (*((DWORD*) (fbuf + 36)) != 0x61746164) {
        printf("'data' tag not found\n");
        return 0;
    } else {
        printf("'data' tag found\n");
    }

    // Bytes [40,43] indicate total number of data bytes
    int data_bytes = *((DWORD*) (fbuf + 40));
    printf("Data bytes: %d bytes\n", data_bytes);


    // real data processing
    printf("\n");

    // Remaining bytes are raw audio data

    dsize = data_bytes;
    dbuf = (char*) malloc(dsize * sizeof(char));    
    memcpy(dbuf, fbuf + 44, dsize);
    if (dbuf == NULL) {
        printf("MALLOC failure\n");
        return 0;
    }

    // write to file

    char* filename = argv[1];
    int old_filename_len = strlen(filename);
    char* ident = strtok(filename, ".");
    int ident_len = strlen(ident);

    char* new_file_name = (char*) malloc((old_filename_len + 1) * sizeof(char));
    strcpy(new_file_name, ident);
    strcpy(new_file_name + ident_len, ".pcm");

    dp = fopen(new_file_name, "wb");
    if (dp == NULL) {
        printf("Failed to create output file\n");
        return 0;
    }
    rv = fwrite(dbuf, sizeof(char), dsize, dp);
    if (rv == dsize) {
        printf("%d bytes successfully written\n", dsize);
    } else {
        printf("ERROR: %d bytes written, %d bytes expected\n", rv, dsize);
        return 0;
    }
    fclose(dp);

    printf("Wrote to file: %s\n", new_file_name);


    // Cleaning up
    free(fbuf);
    free(dbuf);
    free(new_file_name);
    return 0;
}

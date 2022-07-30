/*
g++ text2bin.cpp -o text2bin
./text2bin [infile] [outfile]


*/

#include <string.h>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>

using namespace std;

uint64_t fsize(const string& fname)
{
    struct stat st;
    if (0 == stat(fname.c_str(), &st)) {
        return st.st_size;
    }
    perror("stat issue");
    return -1L;
}

int main(int argc, char *argv[])
{
    // char fname_in[] = "/home/hpy/dataset/Kron21/kron21_16.txt";
    // char fname_out[] = "/home/hpy/dataset/Kron21-bin/kron21_16.bin";
    assert (argc == 3);
    char *fname_in = argv[1];
    char *fname_out = argv[2];

    FILE * fin = fopen(fname_in, "rb");
    assert(fin);
    FILE * fout = fopen(fname_out, "wb");
    assert(fout);

    uint64_t size = fsize(fname_in);
    char * buf = (char *)malloc(size * sizeof(char));
    assert (size == fread(buf, sizeof(char), size, fin));
    printf("infile size = %llu\n", size);
 
    char* start = 0;
    char* end = 0;
    char  sss[512];
    char* line = sss;
    uint32_t linecount = 0, validcount = 0;
 
    start = buf;
    buf = strchr(start, '\n');
    
    while (buf) 
    {
        end = buf;
        memcpy(sss, start, end - start);
        sss[end-start] = '\0';
        line = sss;
        if (line[0] != '%' && line[0]!='#')
        {
            validcount++;
            uint32_t src, dst;

            const char* del = " \t\n";
            char* token = 0;
            
            token = strtok_r(line, del, &line);
            sscanf(token, "%u", &src);
            token = strtok_r(line, del, &line);
            sscanf(token, "%u", &dst);
            
            fwrite(&src, sizeof(uint32_t), 1, fout);
            fwrite(&dst, sizeof(uint32_t), 1, fout);
        }

        start = buf + 1;
        buf = strchr(start, '\n');
        linecount++;
    }

    printf("linecount = %u, validcount = %u, outfile size = %llu\n", linecount, validcount, fsize(fname_out));
    fclose(fin);
    fclose(fout);

    return 0;
}


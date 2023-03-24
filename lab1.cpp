#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#define MAX_FILE_SIZE 1000

struct params {
    size_t a;
    size_t c;
    size_t m;
    size_t x;
    const char* input;
    const char* output;
    size_t file_size;
}; 

void* get_lkg_seq (params& prms) {
   
   size_t sz = prms.file_size/sizeof(int)-1; 
   int* psp = new int [sz];

   size_t a = prms.a;
   size_t b = prms.c;
   size_t m = prms.m;
   size_t x = prms.x;
   psp[0] = x;

   for (size_t i = 0, i < sz - 1, ++i) {
      psp[i+1]
   }
}; 
int main(int argc, char* argv[]) {

    int arg = 0;
    params prms;


    if (argc != 13) {
        perror("Invalid parameters");
    }; // проверка на количество аргемнтов

    while ((arg = getopt(argc, argv, "i:o:x:a:c:m:")) != -1){
        switch (arg) {
             case 'i': {
                params.input = optarg;
                break;
            } 
             case 'o': {
                params.output = optarg;
                break;
             }
             case 'x': {
                params.x = atoi(optarg);
                break;
             }
                
             case 'a': {
                params.a = atoi(optarg);
                break;
             }
               
             case 'c': {
                params.c = atoi(optarg);
                break;
             }

             case 'm':{
                params.m = atoi(optarg);
                break;
             }
             default:
                perror("Invalid parameters");
                break;
        }      
    }

    printf(" Parameters:\n Input file: %s\n Output file: %s\n x: %u\n a: %u\n c: %u\n m: %u\n", params.input, params.output, params.x, params.a, params.c, params.m);
    return 0;
}

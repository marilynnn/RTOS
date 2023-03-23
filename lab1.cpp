#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#define MAX_FILE_SIZE 1000

struct cmd_params {
    size_t a;
    size_t c;
    size_t m;
    size_t x;
    const char* input;
    const char* output;
}; //струкутура с аргументами командной строки

struct chunk {

}; // стурктура с данными для обработки

int main(int argc, char* argv[]) {

    int arg = 0;
    cmd_params params;


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

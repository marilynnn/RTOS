#include <pthread.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>


#define MAX_FILE_SIZE 1000

struct cmd_params {
    size_t a;
    size_t c;
    size_t m;
    size_t x;
    const char* input;
    const char* output;
}; 

struct chunck {
   const char* in;
   int* psp;
   char* out;
   pthread_barrier_t barr;
   size_t begin;   //начало
   size_t end; //конец 
};

void* get_lkg_seq (params& prms, int file_size) {
   
   size_t sz = file_size; 
   int* psp = new int [sz];

   size_t a = prms.a;
   size_t c = prms.c;
   size_t m = prms.m;
   size_t x = prms.x;
   psp[0] = x;

   for (size_t i = 0, i < sz - 1, ++i) 
   {
      psp[i+1] = (a * psp[i] + c) % m;
   }

   return psp; 
}; 

//void* encrypt (int* psp, )



int main(int argc, char* argv[]) {

    int arg = 0;
    cmd_params prms;

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


   int fd_in = open(file_name, O_RDONLY);
	if (fd_in == -1) 
   {
		perror("File can`t be opened");
      exit(-1);  
   }
   
   int file_size = lseek(fd_in, SEEK_END);
   if (file_size == -1)
   {
      perror("Unable to get file`s size"); 
      exit(-1);
   }
   lseek(fd_in,SEEK_SET);

   if (file_size == 0)
   {
      perror("File is empty");
      exit(-1);
   }
   if(file_size > MAX_FILE_SIZE)
   {
      perror("File`s size is too big")
      exit(-1);
   }

   int cpu_count = get_nprocs ();
   pthread_barrier_t barrier;
 

    printf(" Parameters:\n Input file: %s\n Output file: %s\n x: %u\n a: %u\n c: %u\n m: %u\n", params.input, params.output, params.x, params.a, params.c, params.m);
    return 0;
}

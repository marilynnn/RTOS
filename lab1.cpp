#include <pthread.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>


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

struct chunk {
   const char* in;
   int* psp;
   char* out;
   pthread_barrier_t* barr;
   size_t begin;   //начало
   size_t end; //конец 
};

void* get_lkg_seq (void* prms_) {
   params* prms = reinterpret_cast<params*>(prms_);
   
   size_t sz = prms->file_size; 
   int* psp = new int [sz];

   size_t a = prms->a;
   size_t c = prms->c;
   size_t m = prms->m;
   size_t x = prms->x;
   psp[0] = x;

   for (size_t i = 0; i < sz - 1; ++i) 
   {
      psp[i+1] = (a * psp[i] + c) % m;
   }
    char* ps = reinterpret_cast<char*>(psp); 
    return ps;
}; 

void* encrypt (void* chnk) {
   chunk* chunk_ = reinterpret_cast<chunk*>(chnk);
   size_t end = chunk_->end;

   for (size_t i = chunk_->begin; i < end; ++i ) {
      chunk_->out[i] = chunk_->in[i] ^ chunk_->psp[i];
   }

   int status = pthread_barrier_wait(chunk_->barr);
   if (status != PTHREAD_BARRIER_SERIAL_THREAD && status != 0){
      printf("Error wait barrier\n");
      exit(-1);
   }
};



int main(int argc, char* argv[]) {

    int arg = 0;
    params prms;

    if (argc != 13) {
        printf("Invalid parameters\n");
	exit(-1);
    }; // проверка на количество аргемнтов

    while ((arg = getopt(argc, argv, "i:o:x:a:c:m:")) != -1){
        switch (arg) {
             case 'i': {
                prms.input = optarg;
                break;
            } 
             case 'o': {
                prms.output = optarg;
                break;
             }
             case 'x': {
                prms.x = atoi(optarg);
                break;
             }
                
             case 'a': {
                prms.a = atoi(optarg);
                break;
             }
               
             case 'c': {
                prms.c = atoi(optarg);
                break;
             }

             case 'm':{
                prms.m = atoi(optarg);
                break;
             }
             default:
                printf("Invalid parameters\n");
                break;
        }      
    }

   printf(" Parameters:\n Input file: %s\n Output file: %s\n x: %u\n a: %u\n c: %u\n m: %u\n", prms.input, prms.output, prms.x, prms.a, prms.c, prms.m);
   
   int fd_in = open(prms.input, O_RDONLY);
	if (fd_in == -1) 
   {
      printf("Input file can`t be opened\n");
      exit(-1);  
   }
   
   prms.file_size = lseek(fd_in, 0, SEEK_END);
   if (prms.file_size == -1)
   {
      printf("Unable to get file`s size\n"); 
      exit(-1);
   }
   if (prms.file_size == 0)
   {
      printf("File is empty\n");
      exit(-1);
   }
   if(prms.file_size > MAX_FILE_SIZE)
   {
      printf("File`s size is too big\n");
      exit(-1);
   }

 
   lseek(fd_in, 0, SEEK_SET);
   char* input = new char [prms.file_size];

   if(read(fd_in, input, prms.file_size) <= 0){
      printf("Can`t read file\n");
      exit(-1);
   }

   close(fd_in);

   int cpu_count = get_nprocs ();
   //printf("num of cpu:%d\nfile_len:%d\n",cpu_count,prms.file_size);
   pthread_barrier_t barrier;

   pthread_t lkg, crypt[cpu_count];
   int* psp = nullptr; 

   if (pthread_create(&lkg, NULL, get_lkg_seq, &prms) !=0 ){
      printf("Can`t create lkg thread\n");
      exit(-1);
   }

   if (pthread_join(lkg ,(void**)&psp) != 0){
      printf("Can`t join lkg thread\n");
      exit(-1);
   }

   char* output = new char [prms.file_size];

   if(pthread_barrier_init(&barrier, NULL, cpu_count + 1) !=0 ){
      printf("Can`t init barrier\n");
      exit(-1);
   }

   chunk chunks[cpu_count];

   int iter = 0;
   if(prms.file_size % cpu_count > 0){
       iter = prms.file_size/cpu_count + 1;
   }
   else {
       iter = prms.file_size/cpu_count;
   }; 
   
   for (size_t i = 0; i < cpu_count; ++i) {

      chunks[i].in = input;
      chunks[i].out = output;
      chunks[i].psp = psp;
      chunks[i].barr = &barrier;
      chunks[i].begin = iter * i;
      if(i < cpu_count - 1){
         chunks[i].end = iter * (i+1);
      }
      else {
   	 chunks[i].end = prms.file_size;
      }
      //printf("begin = %d, end = %d\n",chunks[i].begin,chunks[i].end);
      if (pthread_create(&crypt[i], NULL, encrypt, &chunks[i]) !=0 ) {
         printf("Can`t create encrypt thread\n");
         exit(-1);
      }
   }

   int status = pthread_barrier_wait(&barrier);

   if (status != PTHREAD_BARRIER_SERIAL_THREAD && status != 0){
      printf("Error wait barrier\n");
      exit(-1);
   }

   int fd_out = open(prms.output, O_WRONLY);
	if (fd_out == -1) 
   {
      printf("Output file can`t be opened\n");
      exit(-1);  
   }

   if (write(fd_out, output, prms.file_size) != prms.file_size){
      printf(" Can`t write to file\n");
   }

   close(fd_in);
   pthread_barrier_destroy(&barrier);

   delete[] input;
   delete[] output;
   delete[] psp;

   return 0;
}

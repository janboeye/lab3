//**************************************************************************
// Multi-threaded Matrix Multiply benchmark
//--------------------------------------------------------------------------
// TA     : Christopher Celio
// Student: 
//
//
// This benchmark multiplies two 2-D arrays together and writes the results to
// a third vector. The input data (and reference data) should be generated
// using the matmul_gendata.pl perl script and dumped to a file named
// dataset.h. 


// print out arrays, etc.
//#define DEBUG

//--------------------------------------------------------------------------
// Includes 

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


//--------------------------------------------------------------------------
// Input/Reference Data

typedef float data_t;
#include "dataset.h"
 
  
//--------------------------------------------------------------------------
// Basic Utilities and Multi-thread Support

__thread unsigned long coreid;
unsigned long ncores;

#include "util.h"
   
#define stringify_1(s) #s
#define stringify(s) stringify_1(s)
#define stats(code) do { \
    unsigned long _c = -rdcycle(), _i = -rdinstret(); \
    code; \
    _c += rdcycle(), _i += rdinstret(); \
    if (coreid == 0) \
      printf("%s: %ld cycles, %ld.%ld cycles/iter, %ld.%ld CPI\n", \
             stringify(code), _c, _c/DIM_SIZE/DIM_SIZE/DIM_SIZE, 10*_c/DIM_SIZE/DIM_SIZE/DIM_SIZE%10, _c/_i, 10*_c/_i%10); \
  } while(0)
 

//--------------------------------------------------------------------------
// Helper functions
    
void printArray( char name[], int n, data_t arr[] )
{
   int i;
   if (coreid != 0)
      return;
  
   printf( " %10s :", name );
   for ( i = 0; i < n; i++ )
      printf( " %3ld ", (long) arr[i] );
   printf( "\n" );
}
      
void __attribute__((noinline)) verify(size_t n, const data_t* test, const data_t* correct)
{
   if (coreid != 0)
      return;

   size_t i;
   for (i = 0; i < n; i++)
   {
      if (test[i] != correct[i])
      {
         printf("FAILED test[%d]= %3ld, correct[%d]= %3ld\n", 
            i, (long)test[i], i, (long)correct[i]);
         exit(-1);
      }
   }
   
   return;
}
 
//--------------------------------------------------------------------------
// matmul function
 
// single-thread, naive version
void __attribute__((noinline)) matmul_naive(const int lda,  const data_t A[], const data_t B[], data_t C[] )
{
  int i, j, k;

   if (coreid > 0)
      return;
  
   for ( i = 0; i < lda; i++ )
      for ( j = 0; j < lda; j++ )  
      {
         for ( k = 0; k < lda; k++ ) 
         {
            C[i + j*lda] += A[j*lda + k] * B[k*lda + i];
         }
      }

}
 


void __attribute__((noinline)) matmul(const int lda,  const data_t A[], const data_t B[], data_t C[] )
{
   
   // ***************************** //
   // **** ADD YOUR CODE HERE ***** //
   // ***************************** //
   //
   // feel free to make a separate function for MI and MSI versions.
  int i, j, k, ii, jj, bsize, start;
  bsize = 16;
  start = bsize*coreid;
  for ( jj = start; jj < lda; jj += bsize*ncores) {
    int first = 1;
    for ( ii = start; ii !=start || first; ii=(bsize+ii) % lda) {
      first = 0;
      for ( j = jj; j < lda && j < jj + bsize; j+=4) {
	for ( i = ii; i < lda && i < ii + bsize; i+=2) {
	  data_t c1 = C[i + j*lda];
	  data_t c2 = C[i + j*lda + 1];
	  data_t c3 = C[i + (j+1)*lda];
	  data_t c4 = C[i + (j+1)*lda + 1];
	  data_t c5 = C[i + (j+2)*lda];
	  data_t c6 = C[i + (j+2)*lda + 1];
	  data_t c7 = C[i + (j+3)*lda];
	  data_t c8 = C[i + (j+3)*lda + 1];
	  for ( k = 0; k < lda; k+=8){
	    for (int x = 0; x < 8; x++) {
	    data_t a = A[j*lda + k+x];
	    data_t a1 = A[(j+1)*lda +k+x];
	    data_t a2 = A[(j+2)*lda +k+x];
	    data_t a3 = A[(j+3)*lda +k+x];
	    data_t b1 = B[(k+x)*lda + i];
	    data_t b2 = B[(k+x)*lda + i + 1];
	    c1 += a * b1;
	    c2 += a * b2;
	    c3 += a1* b1;
	    c4 += a1* b2;
	    c5 += a2* b1;
	    c6 += a2* b2;
	    c7 += a3* b1;
	    c8 += a3* b2;
	    }
	  }
	  C[i + j*lda] = c1;
	  C[i + j*lda + 1] = c2;
	  C[i + (j+1)*lda] = c3;
	  C[i + (j+1)*lda + 1] = c4;
	  C[i + (j+2)*lda] = c5;
	  C[i + (j+2)*lda + 1] = c6;
	  C[i + (j+3)*lda] = c7;
	  C[i + (j+3)*lda + 1] = c8;
	}
      }
   }
  }
}
//--------------------------------------------------------------------------
// Main
//
// all threads start executing thread_entry(). Use their "coreid" to
// differentiate between threads (each thread is running on a separate core).
  
void thread_entry(int cid, int nc)
{
   coreid = cid;
   ncores = nc;

   // static allocates data in the binary, which is visible to both threads
   static data_t results_data[ARRAY_SIZE];


//   // Execute the provided, naive matmul
//   barrier();
//   stats(matmul_naive(DIM_SIZE, input1_data, input2_data, results_data); barrier());
// 
//   
//   // verify
//   verify(ARRAY_SIZE, results_data, verify_data);
//   
//   // clear results from the first trial
//   size_t i;
//   if (coreid == 0) 
//      for (i=0; i < ARRAY_SIZE; i++)
//         results_data[i] = 0;
//   barrier();

   
   // Execute your faster matmul
   barrier();
   stats(matmul(DIM_SIZE, input1_data, input2_data, results_data); barrier());
 
#ifdef DEBUG
   printArray("results:", ARRAY_SIZE, results_data);
   printArray("verify :", ARRAY_SIZE, verify_data);
#endif
   
   // verify
   verify(ARRAY_SIZE, results_data, verify_data);
   barrier();

   exit(0);
}


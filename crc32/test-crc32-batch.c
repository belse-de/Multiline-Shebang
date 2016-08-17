//#!/usr/bin/tcc -run -Wall -Werror -g -I/usr/lib/gcc/x86_64-pc-linux-gnu/6.1.1/include/
// tcc Currently missing items are: complex and imaginary numbers and variable length arrays.

#ifdef __TINYC__
#define __GNUC_VA_LIST
typedef char* __gnuc_va_list;
typedef	char* __va_list;	/* pretend */
typedef char* __isoc_va_list;
#define __STDC_NO_COMPLEX__
#define _COMPLEX_H
#define _TGMATH_H
#endif /* __TINYC__ */

#define __FUNC__    __func__
#define TERM_RED    "\e[31;1m"
#define TERM_GREEN  "\e[32;1m"
#define TERM_YELLOW "\e[33;1m"
#define TERM_BLUE   "\e[34;1m"
#define TERM_NONE   "\e[0m"
#define clean_errno()     (errno == 0 ? "None" : strerror(errno))
#define log_err(M, ...)   fprintf(stderr,"[" TERM_RED    "ERROR" TERM_NONE "] (%s:%d:%s: errno: %s) " M "\n",__FILE__,__LINE__,__FUNC__,clean_errno() , ##__VA_ARGS__)
#define log_warn(M, ...)  fprintf(stderr,"[" TERM_YELLOW "WARN " TERM_NONE "] (%s:%d:%s: errno: %s) " M "\n",__FILE__,__LINE__,__FUNC__,clean_errno() , ##__VA_ARGS__)
#define log_info(M, ...)  fprintf(stderr,"[" TERM_GREEN  "INFO " TERM_NONE "] (%s:%d:%s) " M "\n",__FILE__,__LINE__,__FUNC__ , ##__VA_ARGS__)
#define log_debug(M, ...) fprintf(stderr,"[" TERM_BLUE   "DEBUG" TERM_NONE "] (%s:%d:%s) " M "\n",__FILE__,__LINE__,__FUNC__ , ##__VA_ARGS__)
#define check(A, M, ...)  do{if(!(A)) { log_err(M , ##__VA_ARGS__); errno=0; goto error; }}while(0)
#define sentinel(M, ...)  do{ log_err(M , ##__VA_ARGS__); errno=0; goto error;}while(0)
#define check_mem(A)      check((A), "Out of memory.")

#define CRC32_POLYNOME_NORMAL               0x04C11DB7
#define CRC32_POLYNOME_REVERSED             0xEDB88320
#define CRC32_POLYNOME_REVERSED_RECIPROCAL  0x82608EDB
#define CRC32_POLYNOME                      CRC32_POLYNOME_NORMAL
#define BIT(val,pos)                        (((val)&(1<<(pos)))>>(pos))

#include <stdlib.h> // exit EXIT_*
#include <stdio.h>  // *printf
#include <stdint.h>
#include <string.h> // strerror
#include <errno.h>  // errno
#include <assert.h>
#include <stdbool.h> // bool, true, false

typedef uint32_t (*func_crc32)(uint32_t crc32, uint8_t* buffer, size_t length);

const uint8_t bitcount    = 8;


// ---------------------------- reverse --------------------------------

// Reverses (reflects) bits in a 32-bit word.
uint32_t reverse(uint32_t x) {
   x = ((x & 0x55555555) <<  1) | ((x >>  1) & 0x55555555);
   x = ((x & 0x33333333) <<  2) | ((x >>  2) & 0x33333333);
   x = ((x & 0x0F0F0F0F) <<  4) | ((x >>  4) & 0x0F0F0F0F);
   x = (x << 24) | ((x & 0xFF00) << 8) | 
       ((x >> 8) & 0xFF00) | (x >> 24);
   return x;
}

// ----------------------------- crc32a --------------------------------

/* This is the basic CRC algorithm with no optimizations. It follows the
logic circuit as closely as possible. */

uint32_t crc32a(uint32_t _, uint8_t *buf, size_t length) {
   uint32_t byte, crc;

   crc = 0xFFFFFFFF;
   for (size_t i=0; i<length; i++) {
      byte = buf[i];            // Get next byte.
      byte = reverse(byte);         // 32-bit reversal.
      for (uint8_t j = 0; j <bitcount; j++) {    // Do eight times.
         if ((int)(crc ^ byte) < 0)
              crc = (crc << 1) ^ 0x04C11DB7;
         else crc = crc << 1;
         byte = byte << 1;          // Ready next msg bit.
      }
   }
   return reverse(~crc);
}

// ----------------------------- crc32b --------------------------------

/* This is the basic CRC-32 calculation with some optimization but no
table lookup. The the byte reversal is avoided by shifting the crc reg
right instead of left and by using a reversed 32-bit word to represent
the polynomial.
   When compiled to Cyclops with GCC, this function executes in 8 + 72n
instructions, where n is the number of bytes in the input message. It
should be doable in 4 + 61n instructions.
   If the inner loop is strung out (approx. 5*8 = 40 instructions),
it would take about 6 + 46n instructions. */

uint32_t crc32b(uint32_t _, uint8_t *buf, size_t length) {
   uint32_t byte, crc, mask;

   crc = 0xFFFFFFFF;
   for (size_t i=0; i<length; i++) {
      byte = buf[i];            // Get next byte.
      crc = crc ^ byte;
      for (uint8_t j = 0; j <bitcount; j++) {    // Do eight times.
         mask = -(crc & 1);
         crc = (crc >> 1) ^ (0xEDB88320 & mask);
      }
   }
   return ~crc;
}


// ----------------------------- crc32c --------------------------------

/* This is derived from crc32b but does table lookup. First the table
itself is calculated, if it has not yet been set up.
Not counting the table setup (which would probably be a separate
function), when compiled to Cyclops with GCC, this function executes in
7 + 13n instructions, where n is the number of bytes in the input
message. It should be doable in 4 + 9n instructions. In any case, two
of the 13 or 9 instrucions are load byte.
   This is Figure 14-7 in the text. */

uint32_t crc32c(uint32_t _, uint8_t *buf, size_t length) {
   uint32_t byte, crc, mask;
   static uint32_t table[256];

   /* Set up the table, if necessary. */

   if (table[1] == 0) {
      for (byte = 0; byte <= 255; byte++) {
         crc = byte;
         for (uint8_t j = 0; j <bitcount; j++) {    // Do eight times.
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
         }
         table[byte] = crc;
      }
   }

   /* Through with table setup, now calculate the CRC. */

   crc = 0xFFFFFFFF;
   for (size_t i=0; i<length; i++) {
      byte = buf[i];
      crc = (crc >> 8) ^ table[(crc ^ byte) & 0xFF];
   }
   return ~crc;
}


/* ---- main ---- */
int main(void) {
  size_t runs = 10000;
  size_t buffer_len = 2;
  uint8_t buffer[buffer_len];

  func_crc32 func_array[] = 
  {
    crc32a,   // slowest
    crc32b,
    crc32c,
    NULL
  };
  uint32_t func32_res[8] = { 0,0,0,0, 0,0,0,0 };

  for(size_t run=0; run<runs; run++){
    if(run%10==0) log_info("Run: %4lu %%%3lu", run, run*100/runs);
    for( int buf_step=0; buf_step<buffer_len; buf_step++){
      
      for(int cell_pos=0; cell_pos<=buf_step;){
        if(buffer[cell_pos] == 255){
          buffer[cell_pos] = 0;
          cell_pos += 1;
        } else {
          buffer[cell_pos] += 1;
          cell_pos = 0;
        }
        
        for(int i=0; func_array[i]; i++)
        {
          func32_res[i] = func_array[i](0,buffer,buffer_len);
        }
        
        bool error_found = false;
        for(int i=1; func_array[i]; i++)
        {
          if(func32_res[i-1]!=func32_res[i]) 
          {
            error_found = true;
            break;
          }
        }
        
        if( error_found )
        {
          log_err("Calced crc does not match");
          for( int print_pos=0; print_pos<buffer_len; print_pos++)
          {
             printf("%02x ", buffer[print_pos]);
          }printf("\n");
            
          for(int k=0; func_array[k]; k++){
            printf("[%d] crc32%c: 0x%08x\n", k, 'a'+k, func32_res[k]);
          }
        }
      }
    }
  }
    
  exit(EXIT_SUCCESS);
}

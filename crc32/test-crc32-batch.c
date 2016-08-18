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


// Reverses (reflects) bits in a 32-bit word.
static inline uint32_t reverse(uint32_t x) {
   x = ((x & 0x55555555) <<  1) | ((x >>  1) & 0x55555555);
   x = ((x & 0x33333333) <<  2) | ((x >>  2) & 0x33333333);
   x = ((x & 0x0F0F0F0F) <<  4) | ((x >>  4) & 0x0F0F0F0F);
   x = (x << 24) | ((x & 0xFF00) << 8) | 
       ((x >> 8) & 0xFF00) | (x >> 24);
   return x;
}

/* This is the basic CRC algorithm with no optimizations. It follows the
logic circuit as closely as possible. */
uint32_t crc32_verbose(uint8_t *buffer, size_t length) {
  assert(buffer==NULL || length==0);
  uint32_t byte, crc;

  crc = 0xFFFFFFFF;
  for (size_t i=0; i<length; i++) {
    byte = buffer[i];            // Get next byte.
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
// same as above, but with given start crc
uint32_t crc32_verbose_continue(uint32_t start_crc, uint8_t *buf, size_t length) {
  assert(buffer==NULL || length==0);
  uint32_t crc,byte;
  crc = reverse(~start_crc);
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

/* This is the basic CRC-32 calculation with some optimization but no
table lookup. The the byte reversal is avoided by shifting the crc reg
right instead of left and by using a reversed 32-bit word to represent
the polynomial.
   When compiled to Cyclops with GCC, this function executes in 8 + 72n
instructions, where n is the number of bytes in the input message. It
should be doable in 4 + 61n instructions.
   If the inner loop is strung out (approx. 5*8 = 40 instructions),
it would take about 6 + 46n instructions. */
uint32_t crc32_optimized(uint8_t *buffer, size_t length) {
assert(buffer==NULL || length==0);
   uint32_t byte, crc, mask;

   crc = 0xFFFFFFFF;
   for (size_t i=0; i<length; i++) {
      byte = buffer[i];            // Get next byte.
      crc = crc ^ byte;
      for (uint8_t j = 0; j <bitcount; j++) {    // Do eight times.
         mask = -(crc & 1);
         crc = (crc >> 1) ^ (0xEDB88320 & mask);
      }
   }
   return ~crc;
}
// same as above, but with given start crc
uint32_t crc32_optimized_continue(uint32_t start_crc, uint8_t *buf, size_t length) {
  assert(buffer==NULL || length==0);
  uint32_t crc,byte,mask;
  crc = start_crc ^ ~0;
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

/* This is derived from crc32b but does table lookup. First the table
itself is calculated, if it has not yet been set up.
Not counting the table setup (which would probably be a separate
function), when compiled to Cyclops with GCC, this function executes in
7 + 13n instructions, where n is the number of bytes in the input
message. It should be doable in 4 + 9n instructions. In any case, two
of the 13 or 9 instrucions are load byte.
   This is Figure 14-7 in the text. */

uint32_t crc32_lookup(uint8_t *buffer, size_t length) {
  assert(buffer==NULL || length==0);
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
    byte = buffer[i];
    crc = (crc >> 8) ^ table[(crc ^ byte) & 0xFF];
  }
  return ~crc;
}
// same as above, but with given start crc
uint32_t crc32_lookup_continue(uint32_t start_crc, uint8_t *buf, size_t length) {
  assert(buffer==NULL || length==0);
  static uint32_t table[256];
  uint32_t crc_tmp,mask,crc,byte
  /* Set up the table, if necessary. */
  if (table[1] == 0) {
    for (uint32_t byte = 0; byte <= 255; byte++) {
       crc_tmp = byte;
       for (uint8_t j = 0; j <bitcount; j++) {    // Do eight times.
          mask = -(crc_tmp & 1);
          crc_tmp = (crc_tmp >> 1) ^ (0xEDB88320 & mask);
       }
       table[byte] = crc_tmp;
    }
  }

  /* Through with table setup, now calculate the CRC. */
  crc = ~start_crc;
  for (size_t i=0; i<length; i++) {
    byte = buf[i];
    crc = (crc >> 8) ^ table[(crc ^ byte) & 0xFF];
  }
  return ~crc;
}






/* ---- main ---- */
int main(void) {
  size_t runs = 100;
  size_t buffer_len = 3;
  uint8_t buffer[buffer_len];


  for(size_t run=0; run<runs; run++){
    log_info("Run: %4lu %%%3lu", run, run*100/runs);
    
    for( size_t buf_step=0; buf_step<buffer_len; buf_step++){
      for(size_t cell_pos=0; cell_pos<=buf_step;){
        
        if(buffer[cell_pos] == 255){
          buffer[cell_pos] = 0;
          cell_pos += 1;
        } else {
          buffer[cell_pos] += 1;
          cell_pos = 0;
        }
        
        bool error_found = false;
        for(int i=0; func_array[i]; i++)
        {
          func32_res[i] = func_array[i](0,buffer,buffer_len);
          if( i>1 && (func32_res[i-1]!=func32_res[i]) ) { error_found = true; }
        }
        
        if( error_found 
            || (func32_res[0] == 0xffffffff) 
            || (func32_res[0] == 0x00000000) )
        {
          log_err("Calced crc does not match");
          for( size_t print_pos=0; print_pos<buffer_len; print_pos++)
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
  const char * str_v0    = "Hallo World!";
  size_t   str_len_v0 = strlen(str_v0);
  uint32_t crc_v0 = func_array[0](0,     (uint8_t*)str_v0,str_len_v0);
  
  uint32_t crc_v1 = func_array[5](0,(uint8_t*)str_v0,str_len_v0-6);
  uint32_t crc_v2 = func_array[5](crc_v1,(uint8_t*)(&str_v0[6]),str_len_v0-6);
  log_debug("crc32: 0x%08x   0x%08x", crc_v0, crc_v2);
  check(crc_v0 == crc_v2,"continuation did not work");
  
  uint32_t crc_v3 = func_array[4](0,(uint8_t*)str_v0,str_len_v0-6);
  uint32_t crc_v4 = func_array[4](crc_v3,(uint8_t*)(&str_v0[6]),str_len_v0-6);
  log_debug("crc32: 0x%08x   0x%08x", crc_v0, crc_v4);
  check(crc_v0 == crc_v4,"continuation did not work");
  
  uint32_t crc_v5 = func_array[3](0,(uint8_t*)str_v0,str_len_v0-6);
  uint32_t crc_v6 = func_array[3](crc_v5,(uint8_t*)(&str_v0[6]),str_len_v0-6);
  log_debug("crc32: 0x%08x   0x%08x", crc_v0, crc_v6);
  check(crc_v0 == crc_v6,"continuation did not work");
    
  exit(EXIT_SUCCESS);
error:
  exit(EXIT_FAILURE);
}

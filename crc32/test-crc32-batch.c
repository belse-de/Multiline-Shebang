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
#define check(A, M, ...)  do{if(!(A)) { log_err("" M , ##__VA_ARGS__); errno=0; goto error; }}while(0)
#define sentinel(M, ...)  do{ log_err(M , ##__VA_ARGS__); errno=0; goto error;}while(0)
#define check_mem(A)      check((A), "Out of memory.")

#define CRC32_POLYNOME_NORMAL               0x04C11DB7
#define CRC32_POLYNOME_REVERSED             0xEDB88320
#define CRC32_POLYNOME_REVERSED_RECIPROCAL  0x82608EDB
#define CRC32_POLYNOME                      CRC32_POLYNOME_NORMAL
#define BIT(val,pos)                        (((val)&(1<<(pos)))>>(pos))
#define LENGTH(array)                       sizeof((array))/sizeof((array)[0])

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
  assert(buffer!=NULL && length!=0);
  uint32_t byte, crc;

  crc = 0xFFFFFFFF;
  for (size_t i=0; i<length; i++) {
    byte = buffer[i];            // Get next byte.
    byte = reverse(byte);         // 32-bit reversal.
    for (uint8_t j = 0; j<8; j++) {    // Do eight times.
       if ((int)(crc ^ byte) < 0)
            crc = (crc << 1) ^ 0x04C11DB7;
       else crc = crc << 1;
       byte = byte << 1;          // Ready next msg bit.
    }
  }
  return reverse(~crc);
}
// same as above, but with given start crc
uint32_t crc32_verbose_continue(uint32_t start_crc, uint8_t *buffer, size_t length) {
  assert(buffer!=NULL && length!=0);
  uint32_t crc,byte;
  crc = reverse(~start_crc);
  for (size_t i=0; i<length; i++) {
    byte = buffer[i];            // Get next byte.
    byte = reverse(byte);         // 32-bit reversal.
    for (uint8_t j = 0; j <8; j++) {    // Do eight times.
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
assert(buffer!=NULL && length!=0);
   uint32_t byte, crc, mask;

   crc = 0xFFFFFFFF;
   for (size_t i=0; i<length; i++) {
      byte = buffer[i];            // Get next byte.
      crc = crc ^ byte;
      for (uint8_t j = 0; j <8; j++) {    // Do eight times.
         mask = -(crc & 1);
         crc = (crc >> 1) ^ (0xEDB88320 & mask);
      }
   }
   return ~crc;
}
// same as above, but with given start crc
uint32_t crc32_optimized_continue(uint32_t start_crc, uint8_t *buffer, size_t length) {
  assert(buffer!=NULL && length!=0);
  uint32_t crc,byte,mask;
  crc = start_crc ^ ~0;
  for (size_t i=0; i<length; i++) {
    byte = buffer[i];            // Get next byte.
    crc = crc ^ byte;
    for (uint8_t j = 0; j <8; j++) {    // Do eight times.
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
  assert(buffer!=NULL && length!=0);
  uint32_t byte, crc, mask;
  static uint32_t table[256];

  /* Set up the table, if necessary. */

  if (table[1] == 0) {
    for (byte = 0; byte <= 255; byte++) {
       crc = byte;
       for (uint8_t j = 0; j <8; j++) {    // Do eight times.
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
uint32_t crc32_lookup_continue(uint32_t start_crc, uint8_t *buffer, size_t length) {
  assert(buffer!=NULL && length!=0);
  static uint32_t table[256];
  uint32_t crc_tmp,mask,crc,byte;
  /* Set up the table, if necessary. */
  if (table[1] == 0) {
    for (uint32_t byte = 0; byte <= 255; byte++) {
       crc_tmp = byte;
       for (uint8_t j = 0; j <8; j++) {    // Do eight times.
          mask = -(crc_tmp & 1);
          crc_tmp = (crc_tmp >> 1) ^ (0xEDB88320 & mask);
       }
       table[byte] = crc_tmp;
    }
  }

  /* Through with table setup, now calculate the CRC. */
  crc = ~start_crc;
  for (size_t i=0; i<length; i++) {
    byte = buffer[i];
    crc = (crc >> 8) ^ table[(crc ^ byte) & 0xFF];
  }
  return ~crc;
}






/* ---- main ---- */
int main(void) {
  size_t runs = 1;
  size_t buffer_len = 4;
  uint8_t buffer[buffer_len];

    #if 0
          
        if(buffer[cell_pos]%128==0){
          log_info("rsp: %d %3d %3d", run, buf_step, cell_pos);
        }
    #endif
    
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


        //uint32_t crc_v  = crc32_verbose(buffer, buf_step+1);
        //uint32_t crc_vc = crc32_verbose_continue(0, buffer, buf_step+1);
        //uint32_t crc_o  = crc32_optimized(buffer, buf_step+1);
        //uint32_t crc_oc = crc32_optimized_continue(0, buffer, buf_step+1);
        uint32_t crc_l  = crc32_lookup           (buffer, buf_step+1);
        uint32_t crc_lc = crc32_lookup_continue(0,buffer, buf_step+1);
        
        uint32_t crc = crc_l;
        //check((crc == crc_v ), "miss match crc_v  exp:%08x got:%08x", crc, crc_v);
        //check((crc == crc_vc), "miss match crc_vc exp:%08x got:%08x", crc, crc_vc);
        //check((crc == crc_o ), "miss match crc_o  exp:%08x got:%08x", crc, crc_o);
        //check((crc == crc_oc), "miss match crc_oc exp:%08x got:%08x", crc, crc_oc);
        check((crc == crc_l ), "miss match crc_l  exp:%08x got:%08x", crc, crc_l);
        check((crc == crc_lc), "miss match crc_lc exp:%08x got:%08x", crc, crc_lc);
        
        if( (crc == 0xffffffff) || (crc == 0x00000000) )
        {
          log_info("nice crc: %08x", crc);
          for( size_t print_pos=0; print_pos<buffer_len; print_pos++)
          {
             fprintf(stderr, "%02x ", buffer[print_pos]);
          }fprintf(stderr,"\n");

        }
      }
    }
  }
  
  const char * str_v0    = "Hallo World!";
  size_t   str_len_v0 = strlen(str_v0);
  uint32_t crc_v0 = crc32_verbose_continue(0,     (uint8_t*)str_v0,str_len_v0);
  
  uint32_t crc_v1 = crc32_verbose_continue(0,(uint8_t*)str_v0,str_len_v0-6);
  uint32_t crc_v2 = crc32_verbose_continue(crc_v1,(uint8_t*)(&str_v0[6]),str_len_v0-6);
  check(crc_v0 == crc_v2,"continuation did not work");
  
  uint32_t crc_v3 = crc32_optimized_continue(0,(uint8_t*)str_v0,str_len_v0-6);
  uint32_t crc_v4 = crc32_optimized_continue(crc_v3,(uint8_t*)(&str_v0[6]),str_len_v0-6);
  check(crc_v0 == crc_v4,"continuation did not work");
  
  uint32_t crc_v5 = crc32_lookup_continue(0,(uint8_t*)str_v0,str_len_v0-6);
  uint32_t crc_v6 = crc32_lookup_continue(crc_v5,(uint8_t*)(&str_v0[6]),str_len_v0-6);
  check(crc_v0 == crc_v6,"continuation did not work");
  
  uint8_t nice1[] = {0xff,0xff,0xff,0xff}; 
  uint8_t nice2[] = {0x9d,0x0a,0xd9,0x6d};
  
  uint32_t nice1_crc = crc32_verbose(0,nice1,LENGTH(nice1));
    
  exit(EXIT_SUCCESS);
error:
  exit(EXIT_FAILURE);
}

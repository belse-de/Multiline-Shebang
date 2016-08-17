#!/usr/bin/tcc -run -Wall -Werror -g -I/usr/lib/gcc/x86_64-pc-linux-gnu/6.1.1/include/
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

#define THROW_IF(x, err) do{if((x)){THROW((err));}}while(0)
#define THROW(err)         longjmp(ex_buf__, err) 
#define TRY do{ jmp_buf ex_buf__; switch( setjmp(ex_buf__) ){ case 0: while(1){
#define CATCH_UNKNOWN break; default:
#define CATCH(x) break; case x:
#define ENDTRY FINALLY ENTRY
#define FINALLY break; }
#define ENTRY } }while(0)

#include <stdlib.h> // exit EXIT_*
#include <stdio.h>  // *printf
#include <stdint.h>
#include <string.h> // strerror
#include <errno.h>  // errno
#include <assert.h>

#define CRC32_POLYNOME 0x04C11DB7
#define BIT(val,pos)   (((val) & (1u << (pos) ) )>>(pos))
/*
uint32_t crc(bit array bitString[1..len], int len) {
   remainderPolynomial  := 0
   // A popular variant complements remainderPolynomial here
    for i from 1 to len {
       remainderPolynomial  := remainderPolynomial xor (bitstring[i] * xn-1)
       if (coefficient of xn-1 of remainderPolynomial) = 1 {
           remainderPolynomial  := (remainderPolynomial * x) xor generatorPolynomial
       } else {
           remainderPolynomial  := (remainderPolynomial * x)
       }
   }
   // A popular variant complements remainderPolynomial here
    return remainderPolynomial
}
*/

uint32_t crc32_wiki_en(uint8_t val)
{
  uint8_t bitcount    = 8;
  uint8_t n = 32;
  uint32_t crc32       = 0;             /* Schieberegister */
  for (uint8_t i = 0; i < bitcount; i++)
  {
    crc32 = crc32 ^ ( BIT(val,i) << (n-1) ); 
      if(BIT(crc32,(n-1)) )
          crc32 = (crc32 << 1) ^ CRC32_POLYNOME;
      else
          crc32 = (crc32 << 1);
  }
  fprintf(stderr, "%s: crc32: %10u 0x%08x\n", __func__, crc32, crc32);
  return crc32;
}

uint32_t crc32_uconnet(uint8_t val)
{
  uint8_t bitcount    = 8;
  uint8_t n = 32;
  uint32_t crc32       = 0;             /* Schieberegister */
  for( uint8_t i=bitcount; i; i--) // 8 bits per byte
  {
    if ((( crc32 & (1<<(n-1))) ? 1 : 0) != (val & 1)) // test LSB
      crc32 = (crc32<<1)^CRC32_POLYNOME;
    else
      crc32<<=1;
    val>>=1;
  }
  fprintf(stderr, "%s: crc32: %10u 0x%08x\n", __func__, crc32, crc32);
  return crc32;
}


uint32_t crc32_wiki_de(uint8_t val)
{
  uint8_t bitcount    = 8;
  uint8_t n = 32;
  uint32_t crc32       = 0;             /* Schieberegister */
  for (uint8_t i = 0; i < bitcount; i++)
  {
      if ( ((crc32 >> (n-1)) & 1) != BIT(val,i))
          crc32 = (crc32 << 1) ^ CRC32_POLYNOME;
      else
          crc32 = (crc32 << 1);
  }
  fprintf(stderr, "%s: crc32: %10u 0x%08x\n", __func__, crc32, crc32);
  return crc32;
}

uint32_t crc32_bels_v0(uint8_t val)
{
  uint8_t bitcount    = 8;
  uint8_t n = 32;
  uint32_t crc32       = 0;             /* Schieberegister */
  for (uint8_t i = 0; i < bitcount; i++)
  {
  
      if ( BIT(crc32,(n-1)) != BIT(val,i))
          crc32 = (crc32 << 1) ^ CRC32_POLYNOME;
      else
          crc32 = (crc32 << 1);
  
  }
  fprintf(stderr, "%s: crc32: %10u 0x%08x\n", __func__, crc32, crc32);
  return crc32;
}

uint32_t crc32_rosetta(uint32_t crc, uint8_t* buffer, size_t len)
{
  static uint32_t table[256];
  static uint8_t  table_exists = 0;
  uint8_t bitcount    = 8;
  const uint8_t *p;
  
  /* This check is not thread safe; there is no mutex. */
  if( !table_exists )
  {
    /* Calculate CRC table. */
    for(size_t i=0; i<(sizeof(table)/sizeof(table[0])); i++)
    {
      /* remainder from polynominal division */
      uint32_t rem = (uint32_t)i;
      for(size_t j=0; j<bitcount; j++)
      {
        if(rem & 1)
        {
          rem >>= 1;
          rem ^= CRC32_POLYNOME; //0x00edb883;
        }
        else
        {
          rem >>= 1;
        }
      }
      table[i] = rem;
    }
    table_exists = 1;
  }
  
  crc = ~crc;
  const uint8_t const *q = buffer + len;
  for(p=buffer; p<q; p++)
  {
    /* Cast to unsigned octet. */
    uint8_t octet = *p;
    crc = (crc >> 8) ^ table[(crc & 0xff) ^ octet];
  }
  fprintf(stderr, "%s: crc32: %10u 0x%08x\n", __func__, ~crc, ~crc);
  return ~crc;
}


int main(void)
{
  for(uint8_t i=0; i<32; i++)
  {
    uint32_t val = 1u<<i;
    assert(BIT( val,i)==0b1);
  }


  uint8_t val = '1';

  uint32_t crc32_wiki_en_res = crc32_wiki_en(val);
  uint32_t crc32_uconnet_res = crc32_uconnet(val);
  uint32_t crc32_wiki_de_res = crc32_wiki_de(val);
  uint32_t crc32_rosetta_res = crc32_rosetta(0, &val, 1);
  uint32_t crc32_bels_v0_res = crc32_bels_v0( val);
  
  exit(EXIT_SUCCESS);
error:
  exit(EXIT_FAILURE);
}

/*
TODO:
  1. create hash
  2. hash to hex
  3. look for .path.name.hash.out
  4. if .path.name.hash.out mathes -> exec
  5. delete old out
  6. compile new out
  7. exec new out
*/

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
#include <string.h> // strerror
#include <errno.h>  // errno

const char const sb_start[] = "#!";

void shebang_oneline_strip(char* file_name, FILE* in, FILE* out)
{
  char buffer[256];  
  for(size_t row = 0; fgets(buffer, sizeof(buffer), in); row++ )
  {
    if( row == 0 
        && strlen(buffer) >= sizeof(sb_start) 
        && !memcmp(sb_start, buffer, sizeof(sb_start)-1 ) )
    { 
      fprintf(out, "#line %lu \"%s\"\n", row+2, file_name);
    } 
    else { fprintf(out, "%s", buffer); }
  }
}

int main(int argC, char** argV)
{
  for(int argN=1; argN<argC; argN++)
  {
    log_debug("going to strip %s...", argV[argN]);
    FILE* fp = fopen(argV[argN],"rb");
    check( fp, "could not open file: %s!", argV[argN] );
    log_debug("stripping ...");
    shebang_oneline_strip(argV[argN], fp, stdout);
  }
  
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

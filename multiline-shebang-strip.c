#!/usr/bin/tcc -run -Wall -Werror -g -I/usr/lib/gcc/x86_64-pc-linux-gnu/6.1.1/include/

// tcc Currently missing items are: complex and imaginary numbers and variable length arrays.

#ifdef __TINYC__
#define __GNUC_VA_LIST
typedef char* __gnuc_va_list;
typedef	char*	 __va_list;	/* pretend */
typedef char*  __isoc_va_list;

#define __STDC_NO_COMPLEX__
#define _COMPLEX_H
#define _TGMATH_H
#endif

#include <stdlib.h> // exit EXIT_*
#include <stdio.h>  // *printf
#include <string.h> // strerror
#include <errno.h>  // errno

int main(int argC, char** argV)
{
  for(int i=0; i<argC; ++i) fprintf(stderr,"arg[%d]: %s\n", i, argV[i]);
  
  FILE* fp = stdin;
  if(argC>1)
  {
    for(int argN=1; argN<argC; ++argN)
    {
      fp = fopen(argV[argN],"rb");
      if( !fp )
      {
        fprintf(stderr, "could not open file(%s). %s\n", argV[argN], (errno == 0 ? "None" : strerror(errno)) );
        continue;
      }
 
      
      size_t row = 0, column = 0;
      
      int state = 0;
      int c = EOF;
      while((c = fgetc(fp)) != EOF)
      {
        swtich(state)
        {
          case 0:
            if     ( row==0 && column==0 && c == '#' ) { state = 1; }
            break;
          case 1:    
            if     ( row==0 && column==1 && c == '!' ) { state = 2; }
            else if( row==0 && column==1 && c != '!' ) { state = 0; putchar('#'); }
            break;
          case 2:
            if( column==0 && c == '!' ) { state = 3; }
            break;
          case 3:
            if     ( column==1 && c == '#' ) { state = 4; }
            else if( column==1 && c != '#' ) { state = 2; }
          case 4:
            if     ( c != '\n' ) { state = 2; }
            else if( c == '\n' )
            { 
              state = 0;
              printf("#line %lu \"%s\"", row+2, argV[argN]);
            }
            break;
          default:
            break;
        }
        
        if( state == 0 ) { putchar(c); }  
        
        column+=1;
        if(c=='\n') 
        {
          row+=1;
          column=0;
        }
      }
    }
  }
  
  
  exit(EXIT_SUCCESS);
error:
  exit(EXIT_FAILURE);
}

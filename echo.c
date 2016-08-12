//#!/usr/local/bin/script_c

#include <stdlib.h>
#include <stdio.h>
 
int main(int argc, char** argv, char** envp){
  int current_arg = 0;
  for( current_arg=1; current_arg<argc; current_arg++ )
  {
    printf("%s ", argv[current_arg]);
  }
  printf("\n");

  exit(EXIT_SUCCESS);
}

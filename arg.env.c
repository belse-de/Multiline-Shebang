//#!/usr/local/bin/script_c

#include <stdlib.h>
#include <stdio.h>


 
int main(int argc, char** argv, char** envp){
  int current_arg = 0;
  for( current_arg=0; current_arg<argc; current_arg++ )
  {
    printf("arg[%d]: %s\n", current_arg, argv[current_arg]);
  }
  
  for( ; *envp != NULL; envp++)
  {
    printf("env: %s\n", *envp);
  }

  exit(EXIT_SUCCESS);
}

#!/bin/bash
BIN="$0.out"
if [ "$0" -nt "$BIN" ]; then
  sed -n -e '9,$p' < "$0" | /usr/bin/gcc -x c -o "$BIN" -
fi
"$BIN" "$0" "$@"
STATUS=$?
exit $STATUS


#include <stdio.h>

int main(int argc, char **argv, char** envp)
{
  printf("%s(%d):%s\n\n", __FILE__, __LINE__, __PRETTY_FUNCTION__);
  
  int i;
  for (i = 0; i < argc; i++)
    printf("argv[%d] -> %s\n", i, argv[i]);
  printf("\n");
  
  for (i = 0; envp[i] != NULL; i++)
    printf("envp[%d] -> %s\n", i, envp[i]);
  printf("\n");
  
  return 0;
}

/*
 * cite source: https://rosettacode.org/wiki/Multiline_shebang
 * ### Multiline shebang ###
 *
 *  Simple shebangs can help with scripting, 
 *  e.g., #!/usr/bin/env python at the top of a Python script will allow it 
 *  to be run in a terminal as "./script.py".
 *
 *  Occasionally, a more complex shebang line is needed. 
 *  For example, some languages do not include the program name in ARGV; 
 *  a multiline shebang can reorder the arguments 
 *  so that the program name is included in ARGV.
 *
 * The syntax for a multiline shebang is complicated. 
 * The shebang lines must be simultaneously commented away 
 * from the main language and revealed to some shell (perhaps Bash) 
 * so that they can be executed. In other words, Polyglots.
 *
 * Warning: Using a multiline shebang of the form #!/bin/sh ... exec ... !# 
 * will set the code's mimetype to text/x-shellscript, 
 * which creates problems such as Emacs treating the file as a shell script, 
 * no matter which language and file extension it really uses. 
 *
 * see also:
 *  https://rosettacode.org/wiki/Native_shebang#Using_gcc_to_script_C
 */

#!/bin/bash
set -e; umask 077
BIN="$0.out"
CFLAGS='-std=c11 -Wall -Wextra -Werror -O0 -g3'
LINKER='-fsanitize=address'

SHEBANG_EOF=$(( $(grep -n "^!#\$" "$0" | grep -o "^[0-9]*") + 1 ))
CODE_LINE=$(echo -e "#line $SHEBANG_EOF \"$0\"\n")
CODE_RAW=$(sed -n -e ''"$SHEBANG_EOF"',$p' "$0")
CODE=$(echo "$CODE_LINE" && echo "$CODE_RAW")

if [ "$0" -nt "$BIN" ]; then
  echo "$CODE" | /usr/bin/gcc $CFLAGS -x c -o "$BIN" - $LINKER
fi

set +e
"$BIN" "$0" "$@"
STATUS=$?
exit $STATUS
!#

#include <stdio.h>

int main(int argc, char **argv, char** envp)
{
  printf("%s(%d):%s\n\n", __FILE__, __LINE__, __func__);
  
  for (int i = 0; i < argc; i++)
    printf("argv[%d] -> %s\n", i, argv[i]);
  printf("\n");
  
  for (int i = 0; envp[i] != NULL; i++)
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

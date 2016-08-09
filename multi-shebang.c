#!/bin/bash
set -e;       # terminate on error
umask 077;    # make ever out file read/write/execute(rwx) only for current user
BIN="$0.out"  # name of the output file
CFLAGS='-std=c11 -Wall -Wextra -Werror -O0 -g3'; # compiler / c flags
LINKER='-fsanitize=address' # linker flags

# calclutate position(line) of closing shebang('!#')
SHEBANG_EOF=$(( $(grep -n "^!#\$" "$0" | grep -o "^[0-9]*") + 1 ))
# generate preprocessor directive to fix file name and line number
CODE_LINE=$(echo -e "#line $SHEBANG_EOF \"$0\"\n")
# code without the multi line shebang
CODE_RAW=$(sed -n -e ''"$SHEBANG_EOF"',$p' "$0")
# concat of PP directiv and c code
CODE=$(echo "$CODE_LINE" && echo "$CODE_RAW")

if [ "$0" -nt "$BIN" ]; then
  echo -e "\033[33m Source is newer \033[0m -> \033[31m recompiling.. \033[0m" 1>&2
  echo "$CODE" | /usr/bin/gcc $CFLAGS -x c -o "$BIN" - $LINKER
fi

set +e;          # reenable continue  on error
"$BIN" "$0" "$@" # execute binary
exit $?          # return exit code of binary
!#
// c code starts in this line
#include <stdio.h>

int main(int argc, char **argv, char** envp)
{
  printf("%s(%d):%s\n\n", __FILE__, __LINE__, __func__);
  
  for (int i = 0; i < argc; i++)
    printf("argv[%d] -> %s\n", i, argv[i]);
  printf("\n");
  
  for (int i = 0; envp[i] != NULL; i++)
    printf("envp[%2d] -> %s\n", i, envp[i]);
  printf("\n");
  
  return 0;
}

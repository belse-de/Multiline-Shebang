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
  echo -e "\033[33m Source is newer \033[0m -> \033[31m recompiling.. \033[0m" 1>&2
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

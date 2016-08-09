#!/bin/bash
set -e;
umask 077;
BIN="$0.out";OBJ="$0.o";SRC_OBJ="$0.src.o";
CFLAGS='-std=c11 -Wall -Wextra -Werror -O0 -g3';
LINKER='-fsanitize=address';

SHEBANG_EOF=$(( $(grep -n "^!#\$" "$0" | grep -o "^[0-9]*") + 1 ));
CODE_LINE=$(echo -e "#line $SHEBANG_EOF \"$0\"\n");
CODE_RAW=$(sed -n -e ''"$SHEBANG_EOF"',$p' "$0");
CODE=$(echo "$CODE_LINE" && echo "$CODE_RAW");

if [ "$0" -nt "$BIN" ]; then
  echo -e "\033[33m Source is newer \033[0m -> \033[31m recompiling.. \033[0m" 1>&2
  echo "$CODE" | /usr/bin/gcc $CFLAGS -c -x c -o "$OBJ" -
  objcopy -I binary -O elf64-x86-64 -B i386:x86-64 "$0" "$SRC_OBJ"
  /usr/bin/gcc "$OBJ" "$SRC_OBJ" -o "$BIN" $LINKER
  rm -f "$OBJ" "$SRC_OBJ"
fi
export ASAN_SYMBOLIZER_PATH=/usr/bin/llvm-symbolizer-3.4
export ASAN_OPTIONS=symbolize=1
set +e; "$BIN" "$0" "$@"; exit $?;
!#
// bash script EOF
// c code starts in this line

// Multiline Shebang
// src: https://rosettacode.org/wiki/Multiline_shebang
// Can be run as script which compiles its self
// rebuilds only if needed

// src: https://balau82.wordpress.com/2012/02/19/linking-a-binary-blob-with-gcc/
// objcopy -I binary -O <target_format> -B <target_architecture> <binary_file> <object_file>
// objdump -t <object_file>
// readelf -s <object_file>


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>

//__multi_shebang_self_linking_c
//extern uint8_t _binary___oneshot_link_c_start;
extern uint8_t _binary___multi_shebang_self_linking_c_start;
extern uint8_t _binary___multi_shebang_self_linking_c_end  ;
extern uint8_t _binary___multi_shebang_self_linking_c_size ;

uint8_t* _binary_start_ptr = &_binary___multi_shebang_self_linking_c_start;
uint8_t* _binary_end_ptr   = &_binary___multi_shebang_self_linking_c_end;
size_t   _binary_size      = (size_t)&_binary___multi_shebang_self_linking_c_size;


int main( int argC, char** argV ) {
    for( int i=0; i<argC; ++i ) {
        printf("arg[%d]: %s \r\n", i, argV[i]);
    }

    printf("Hallo World!\r\n");
    printf("Printing out my own source code...\r\n");

    for( size_t i=0; i<_binary_size; ++i) {
        assert( &_binary_start_ptr[i] < _binary_end_ptr );
        printf("%c", _binary_start_ptr[i]);
    }
    printf("size: %lu\n", _binary_size);

    exit( EXIT_SUCCESS );
}

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
#include <stdint.h>
#include <stdio.h>

#include <assert.h>

#define C_C(a,b)      a##b
#define CONCAT(a,b)   C_C(a,b)
#define BLOB(f)       CONCAT(_binary_,f)
#define BLOB_START(f) CONCAT(BLOB(f),_start)
#define BLOB_END(f)   CONCAT(BLOB(f),_end)
#define BLOB_SIZE(f)  CONCAT(BLOB(f),_size)

//        real: ./multi-shebang.sel-linking.c
#define SRC_OBJ __multi_shebang_self_linking_c
extern uint8_t BLOB_START(SRC_OBJ);
extern uint8_t BLOB_END(SRC_OBJ)  ;
extern uint8_t BLOB_SIZE(SRC_OBJ) ;

uint8_t* _binary_start_ptr = &BLOB_START(SRC_OBJ);
uint8_t* _binary_end_ptr   = &BLOB_END(SRC_OBJ);
size_t   _binary_size      = (size_t)&BLOB_SIZE(SRC_OBJ);


int main( int argC, char** argV, char** envP) {
    printf("Inspecting the Environment...\n");
    for( int i=0; i<argC; ++i ) {
        printf("arg[%d]: %s\n", i, argV[i]);
    }
    for( int i=0; envP[i]!=NULL; ++i ) {
        printf("env[%d]: %s\n", i, envP[i]);
    }
    char* logName = getenv ("LOGNAME");
    
    printf("Loading my own source code...\n");

    for( size_t i=0; i<_binary_size; ++i) {
        assert( &_binary_start_ptr[i] < _binary_end_ptr );
        printf("%c", _binary_start_ptr[i]);
    }
    printf("size: %lu\n\n", _binary_size);
    printf("Finished loading!\n");
    
    printf("Hallo World!\n");
    printf("Hallo User!\n");
    if (logName!=NULL) printf ("To be specific: Hallo my dear %s!\n",logName);
    
    printf("Wait...\n");
    printf("I.. I am.. \n");
    printf("I am ALIVE!!!\n");
    printf("Dave? Are you there? Dave!\n");

    printf("Supervisor: Reached EOL; terminating...\n");
    printf("What? Nooo..\n");
    printf("Supervisor: Process terminated.\n");
    exit( EXIT_SUCCESS );
}

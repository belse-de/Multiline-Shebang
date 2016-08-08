# Multiline shebang #
I was on the search for a tool that would allow me, 
to create easy and fast prototypes in c/c++, 
like scipting languages would allow you.

I found my solution in a poliglot, 
the combination of shell script and c-source in one file.

Over time this some how developt in to its own little project.
Which I recreated on GitHub, so other may benefit from it.



## Externe Definition ##

cite source: https://rosettacode.org/wiki/Multiline_shebang

### Multiline shebang Definition ###

Simple shebangs can help with scripting, 
e.g., #!/usr/bin/env python at the top of a Python script will allow it 
to be run in a terminal as "./script.py".

Occasionally, a more complex shebang line is needed. 
For example, some languages do not include the program name in ARGV; 
a multiline shebang can reorder the arguments 
so that the program name is included in ARGV.

The syntax for a multiline shebang is complicated. 
The shebang lines must be simultaneously commented away 
from the main language and revealed to some shell (perhaps Bash) 
so that they can be executed. In other words, Polyglots.

Warning: Using a multiline shebang of the form #!/bin/sh ... exec ... !# 
will set the code's mimetype to text/x-shellscript, 
which creates problems such as Emacs treating the file as a shell script, 
no matter which language and file extension it really uses. 

see also:

  Native shebang - where the "program loaded" is of the actual native task language
  
  https://rosettacode.org/wiki/Native_shebang#Using_gcc_to_script_C


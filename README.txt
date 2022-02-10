README
=================

COMPILING
------------

Use the following command to compile program:

gcc --std=gnu99 -o smallsh smallsh.c

RUNNING
------------
Run the following command to run the compiled executable:
./smallsh

The built in commands are:
ls
cd
status
exit

All other commands are passed through execvp

KNOWN BUGS
-------------
$$ variable expansion not added yet
Text does not appear when background child process is finished
"xxx: missing operand" appears with xxx being the previous command when the user enters ^Z (SIGTSTP)


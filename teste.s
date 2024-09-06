push dword 29
call input
LOAD 29
DIV 28
STORE 30
MUL 28
STORE 31
LOAD 29
SUB 31
STORE 31
push dword 31
call output
COPY 30 29
LOAD 29
JMPP 4
STOP
push dword 2
push dword 0
call s_input

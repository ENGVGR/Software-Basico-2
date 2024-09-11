.data
overflow_msg db 'Erro: Overflow na multiplicacao!', 0xA
overflow_len equ $ - overflow_msg
LABEL1 dd 2

.bss
LABEL2 resb 4
LABEL3 resb 4
LABEL4 resb 4

.text
call input
MOV EAX, LABEL2
LABEL5: MOV EBX, LABEL1
IDIV EBX 
MOV LABEL3, EAX
MOV EBX, LABEL1
IMUL EBX
 JO overflow_handler
MOV LABEL4, EAX
MOV EAX, LABEL2
MOV EBX, LABEL4
SUB EAX, EBX
MOV LABEL4, EAX
call output
COPY 30 29
MOV EAX, LABEL2
JMPP 4
STOP
overflow_handler:
    MOV EAX, 4
    MOV EBX, 1
    MOV ECX, overflow_msg
    MOV EDX, overflow_len
    INT 0x80
    MOV EAX, 1
    MOV EBX, 1
    INT 0x80
end:
    MOV EAX, 1
    MOV EBX, 0
    INT 0x80


section .data
overflow_msg db 'Erro: Overflow na multiplicacao!', 0xA
overflow_len equ $ - overflow_msg
buffer dd 0,0,0,0,0,0,0,0,0,0,0
newline db 0xA, 0
LABEL1 dd 2

section .bss
LABEL2 resb 4
LABEL3 resb 4
LABEL4 resb 4

section .text
global _start

output:
enter 0, 0                ; Salva o frame da pilha
pusha
;mov eax, [ebp + 8]
;mov eax, [eax]
mov ecx, 10                 ; Divisor para extração de dígitos (base 10)
lea esi, [buffer + 11]  ; Aponta para o final do buffer
mov byte [esi], 0           ; Coloca o terminador nulo no final da string
dec esi                     ; Move o ponteiro para a posição anterior no buffer

convert_loop:
xor edx, edx          ; Limpa edx antes da divisão (necessário para div)
div ecx               ; Divide EAX por 10, quociente em EAX, resto em EDX
add dl, 0x30         ; Converte o dígito (resto) para ASCII
mov [esi], dl         ; Armazena o dígito convertido no buffer
dec esi               ; Move o ponteiro para o próximo caractere
cmp eax, 0            ; Verifica se o quociente é 0
jne convert_loop      ; Se EAX não for 0, continua a conversão
inc esi               ; Corrige o ponteiro para o início da string
mov eax, [buffer]
mov dword [buffer], eax
mov eax, 4
mov ebx, 1
mov ecx, buffer
mov edx, 21
int 0x80
mov eax, 4
mov ebx, 1
mov ecx, newline
mov edx, 1
int 0x80
mov ecx, 11
mov eax, 0

clear:
mov dword[buffer+eax*4],0
inc eax
loop clear
popa
leave
ret                   ; Retorna com a string pronta no buffer 

input:
%define ATUAL dword[ebp - 4]
%define RES dword[ebp - 8]
enter 4,0
pusha
mov RES, 0
mov eax, 3
mov ebx, 0
mov ecx, buffer      ;salva o input como uma string em buffer
mov edx, 32
int 0x80
mov edx, 0

loop:
mov al, [buffer + edx]         ; Carrega o caractere do buffer em al (parte baixa de eax)
cmp al, 0x0A             ; Verifica se o caractere é Enter (0x0A)
je saida                 ; Se for Enter, sai do loop
; Converter o caractere lido de ASCII para valor numérico
sub al, 0x30              ; Subtrai '0' (0x30) para converter para decimal
movzx eax, al            ; Expande al para eax sem sinal (agora temos o valor numérico)
; Atualiza o valor total em N
mov ebx, RES             ; Carrega o valor atual de N
mov ATUAL,ebx
mov ecx ,9

multi:
add ebx,ATUAL
loop multi
add ebx, eax             ; Soma o novo dígito
 mov RES, ebx             ; Salva o novo valor de N
  inc edx
   jmp loop                 ; Repete o loop

saida:
mov ecx, 11
mov eax, 0
clear:
mov dword[buffer+eax*4],0
inc eax
loop clear
popa
mov eax, RES
mov dword[ebp + 8], eax
leave
ret                   ; Retorna com a string pronta no buffer

_start:
PUSH LABEL2
call input
pop eax
MOV EAX, [LABEL2]
LABEL5: MOV EBX, [LABEL1]
XOR EDX, EDX
IDIV EBX 
MOV DWORD [LABEL3], EAX
MOV EBX, [LABEL1]
IMUL EBX
CMP IDX, 0
JNE overflow_handler
MOV DWORD [LABEL4], EAX
MOV EAX, [LABEL2]
MOV EBX, [LABEL4]
SUB EAX, EBX
MOV DWORD [LABEL4], EAX
PUSH LABEL4
call output
pop eax
MOV EDX, [LABEL3]
MOV DWORD [LABEL2], EDX
MOV EAX, [LABEL2]
JA LABEL5
JMP END

overflow_handler:
    MOV EAX, 4
    MOV EBX, 1
    MOV ECX, overflow_msg
    MOV EDX, overflow_len
    INT 0x80
    MOV EAX, 1
    MOV EBX, 1
    INT 0x80
END:
    MOV EAX, 1
    MOV EBX, 0
    INT 0x80


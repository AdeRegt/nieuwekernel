bits 32
org 0x2000

helloworld:
mov esi,message
mov edi,0xb8000
.loop:
mov al,byte [esi]
cmp al,0x00
je .end
mov [edi],al
inc edi
;mov al,0x00
;mov [edi],al
inc edi
inc esi
jmp .loop
.end:
ret
message db "HELLO WORLD!!",0x00

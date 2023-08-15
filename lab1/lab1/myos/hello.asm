mov ah, 0x0e		; set tele-type mode (output to screen)

mov al, 'V'		; one ascii character hex code in register AL
int 0x10		; send content of register to screen via an interrupt
mov al, 'i'
int 0x10
mov al, 'r'
int 0x10
mov al, 'e'
int 0x10
mov al, 'n'
int 0x10
mov al, 'd'
int 0x10
mov al, 'r'
int 0x10
mov al, 'a'
int 0x10

; below lines borrowed from boot_sector2.asm [see comments there]

loop:
	jmp loop

times 510 -( $ - $$ ) db 0

dw 0xaa55

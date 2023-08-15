; https://wiki.osdev.org/Babystep2
; code given at above link produces Hello World...

mov ax, 0x07c0

mov si, msg
cld

loop:lodsb		; load string from si
	or al, al	; zero => end of str ; see https://stackoverflow.com/questions/22123863/or-al-al-in-x86-assembly
	jz done
	mov ah, 0x0E
	int 0x10
	jmp loop
 
done:
   jmp done

msg db 'Virendra'

times 510-($-$$) db 0		; fill up remaining with 0

db 0x55
db 0xAA

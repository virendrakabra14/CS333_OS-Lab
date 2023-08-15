jmp $  ; jump to the beginning of the line. This create an infinite loop

times 510-($-$$) db 0  ; fill out '0' in all the bytes from the current position to the 510th bytes

dw 0xAA55 ; Write a word (2 bytes) data 0xAA55 at current position. This write 55 at 511th byte and AA at 512th byte. Why 55 in 0xAA55 gets written earlier than AA in the file ? It is a little bit complicated to explain.. but just to say the conclusion, it is because Intel chipset is using Little Endian format.
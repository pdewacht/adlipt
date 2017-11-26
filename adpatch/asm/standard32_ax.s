; arguments in al, ah
; modifies eax, edx
; 45 bytes

        bits 32

        mov dx, PORT
        out dx, al
        inc edx
        inc edx
        mov al, 13
        out dx, al
        mov al, 9
        out dx, al
        mov al, 13
        out dx, al
        in al, dx
        in al, dx
        in al, dx
        in al, dx
        in al, dx
        in al, dx
        dec edx
        dec edx

        mov al, ah
        out dx, al
        inc edx
        inc edx
        mov al, 12
        out dx, al
        mov al, 8
        out dx, al
        mov al, 12
        out dx, al

        mov ah, 35
L1:     in al, dx
        dec ah
        jnz L1

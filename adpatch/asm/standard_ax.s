; arguments in al, ah
; modifies ax, dx
; 44 bytes

        mov dx, PORT
        out dx, al
        inc dx
        inc dx
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
        dec dx
        dec dx

        mov al, ah
        out dx, al
        inc dx
        inc dx
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

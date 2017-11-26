; arguments in [bp+ARG1], [bp+ARG2]
; modifies ax, dx
; 48 bytes

        mov dx, PORT
        mov al, [bp+ARG1]
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

        mov al, [bp+ARG2]
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

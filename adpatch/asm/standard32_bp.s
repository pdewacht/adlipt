; arguments in [ebp+ARG1], [ebp+ARG2]
; modifies eax, edx
; 49 bytes

        bits 32

        mov dx, PORT
        mov al, [ebp+ARG1]
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

        mov al, [ebp+ARG2]
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

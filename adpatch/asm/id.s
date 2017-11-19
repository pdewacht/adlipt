; arguments in [bp+6], [bp+8]
; disables interrupts for most of the routine

        pushf
        cli

        mov dx, PORT
        mov al, [bp+6]
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

        mov al, [bp+8]
        out dx, al
        inc dx
        inc dx
        mov al, 12
        out dx, al
        mov al, 8
        out dx, al
        mov al, 12
        out dx, al

        popf

        push cx
        mov cx, 35
L1:     in al, dx
        loop L1
        pop cx

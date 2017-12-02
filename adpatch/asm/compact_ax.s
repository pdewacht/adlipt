; receives input on stack
; modifies ax, cx, dx
; 29 bytes

        mov cl, 13
L1:
        mov dx, PORT
        out dx, al
        inc dx
        inc dx
        mov al, cl
        out dx, al
        sub al, 4
        out dx, al
        add al, 4
        out dx, al
        mov ch, 0x22
L2:     in al, dx
        dec ch
        jnz L2
        mov al, ah
        dec cx
        jpe L1

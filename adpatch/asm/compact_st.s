; receives input on stack
; modifies ax, cx, dx, sp
; does not modify the stack data (softdisk relies on this!)
; 28 bytes

        mov cl, 13
L1:
        pop ax
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
        mov ah, 0x22
L2:     in al, dx
        dec ah
        jnz L2
        dec cx
        jpe L1

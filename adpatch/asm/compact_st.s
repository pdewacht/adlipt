; receives input on stack
; modifies ax, cx, dx, sp
; does not modify the stack data (softdisk relies on this!)
; 27 bytes

        %if __BITS__ != 16
        %error This file is intended for 16-bit code
        %endif

        stc
L1:
        pop ax
        mov dx, PORT
        out dx, al
        mov al, 12
        adc al, 0               ; carry=1 only on first iteration
        inc dx
        inc dx
        out dx, al
        sub al, 4
        out dx, al
        add al, 4               ; odd parity only on first iteration
        out dx, al
        mov cx, 35
L2:     in al, dx
        loop L2
        jpo L1

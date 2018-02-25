; input given by macros ARG1, ARG2
; ARG2 can't be al, cx, dx
; modifies ax, ecx, dx

; 33 bytes

        %if __BITS__ != 32
        %error This file is intended for 32-bit code
        %endif

        stc
L1:
        pop eax
        mov dx, PORT
        out dx, al
        mov al, 13
        mov ecx, 6
        jc L9           ; carry=1 iff first iteration
        dec eax
        mov cl, 35
L9:     inc edx
        inc edx
        out dx, al
        sub al, 4
        out dx, al
        add al, 4       ; clears carry. odd parity iff first iteration
        out dx, al
L2:     in al, dx
        loop L2
        jpo L1

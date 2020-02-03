; input given by macros ARG1, ARG2
; ARG2 can't be ax, dx
; modifies ax, dx

        bits 32
        ;;  UNFINISHED

        %if __BITS__ != 32
        %error This file is intended for 32-bit code
        %endif

        %ifnidni ARG1, ax
        mov ax, ARG1
        %endif

        or ah, ah
        mov ah, 13
        jz L1
        sub ah, 8

L1:
        mov dx, PORT
        out dx, al
L9:     inc edx
        inc edx
        mov al, ah
        dec ah
        out dx, al
        sub al, 4
        out dx, al
        add al, 4       ; odd parity iff first iteration
        out dx, al
        in al, dx
	in al, dx
        in al, dx
        in al, dx
        in al, dx
        in al, dx
        mov al, ARG2
        jpo L1

; input given by macros ARG1, ARG2
; ARG2 can't be al, cx, dx
; modifies ax, cx, dx

; 27 bytes with ARG1=al and ARG2=ah
; 31 bytes with ARG1=[bp+4] and ARG2=[bp+6]
; (1 byte shorter than compact.s, but uses the undocumented salc opcode)

        %if __BITS__ != 16
        %error This file is intended for 16-bit code
        %endif

        clc
        %ifnidni ARG1, al
        mov al, ARG1
        %endif
L1:
        mov dx, PORT
        out dx, al
        salc            ; carry=0 iff first iteration
        add al, 13
        inc dx
        inc dx
        out dx, al
        sub al, 4
        out dx, al
        sub al, -4      ; sets carry. odd parity iff first iteration
        out dx, al
        mov cx, 35
L2:     in al, dx
        loop L2
        mov al, ARG2
        jpo L1

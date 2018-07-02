; arguments given by macros ARG1, ARG2
; ARG2 can't be al, cx, dx
; modifies ax, cx, dx

; 31 bytes with ARG1=al and ARG2=ah
; 35 bytes with ARG1=[bp+4] and ARG2=[bp+6]

        %if __BITS__ != 16
        %error This file is intended for 16-bit code
        %endif

        stc             ; sets carry flag
        %ifnidni ARG1, al
        mov al, ARG1
        %endif
L1:
        mov dx, PORT
        db 0x00, 0x00   ; padding
        out dx, al
        mov al, 13
        mov cx, 6
        db 0x00, 0x00   ; padding
        jc L9           ; carry=1 iff first iteration
        dec ax
        mov cl, 35
L9:     inc dx
        inc dx
        out dx, al
        sub al, 4
        out dx, al
        add al, 4
        out dx, al
L2:     in al, dx
        loop L2
        db 0x8a, 0x04, 0x24    ; mov al,[esp+10h+var_10]
        jpo L1          ; go back to L1 if parity odd

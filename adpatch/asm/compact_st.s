; receives input on stack
; modifies ax, cx, dx, sp
; does not modify the stack data (softdisk relies on this!)
; 30 bytes

	mov dx, PORT
	mov cl, 13
L1:
	pop ax
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
	dec dx
	dec dx
	dec cx
	jpe L1

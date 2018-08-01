	.386

        public _amis_header
        public _amis_id
        public _amis_handler

        public _emm386_table
        public _qemm_handler

        extern _config : near
        extern emulate_opl2_address_io_ : proc
        extern emulate_opl2_data_io_ : proc
        extern emulate_opl3_high_address_io_ : proc
        extern emulate_opl3_data_io_ : proc


cmp_ah  macro
        db 0x80, 0xFC
        endm


        RESIDENT segment word use16 public 'CODE'


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; AMIS API IMPLEMENTATION


_amis_header:
        db 'SERDACO '           ;8 bytes: manufacturer
        db 'ADLIPT  '           ;8 bytes: product
        db 0                    ;no description
;;; Configuration pointer immediately follows AMIS header
        dw _config


;;; IBM Interrupt Sharing Protocol header
iisp_header macro chain
        jmp short $+0x12
chain:  dd 0
        dw 0x424B               ;signature
        db 0                    ;flags
        jmp short _retf         ;hardware reset routine
        db 7 dup (0)            ;unused/zero
        endm


_amis_handler:
        iisp_header amis_next_handler
        cmp_ah
_amis_id: db 0xFF
        je @@amis_match
        jmp dword ptr cs:amis_next_handler
@@amis_match:
        test al, al
        je @@amis_install_check
        cmp al, 4
        je @@amis_hook_table
        xor al, al
        iret
@@amis_install_check:
        mov al, 0xFF
        mov cx, (VERSION_MAJOR * 256 + VERSION_MINOR)
        mov dx, cs
        mov di, _amis_header
        iret
@@amis_hook_table:
        mov dx, cs
        mov bx, amis_hook_table
        iret

amis_hook_table:
        db 0x2D
        dw _amis_handler


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; EMM386 GLUE CODE


        even
_impl_table:
        dw emulate_opl2_address_io_
        dw emulate_opl2_data_io_
        dw emulate_opl3_high_address_io_
        dw emulate_opl3_data_io_

NUM_ENTRIES EQU 4

        even
_emm386_table:
        dw 0x0388, emm386_handler
        dw 0x0389, emm386_handler
        dw 0x038A, emm386_handler
        dw 0x038B, emm386_handler

emm386_handler:
        push bx
        mov bx, dx
        and bx, 3
        add bx, bx
        push 0xD8               ; EMM386 selector for user code segment
        push [bp+8]             ; user IP register
        call word ptr [_impl_table + bx]
        pop bx
        clc
_retf:  retf


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; QEMM GLUE CODE

_qemm_handler:
        iisp_header qemm_next_handler
        cmp dx, 0x200
        jl @@qemm_skip
        cmp dx, 0x38B
        jg @@qemm_skip
        ;; possible match, check the table
        push ds
        push bx
        push si
        mov bx, cs
        mov ds, bx
        mov bx, _emm386_table
        xor si, si
@@qemm_loop:
        cmp word ptr [bx], dx
        je @@qemm_found
        add bx, 4
        inc si
        inc si
        cmp si, 2*NUM_ENTRIES
        jne @@qemm_loop
@@qemm_not_found:
        pop si
        pop bx
        pop ds
@@qemm_skip:
        jmp dword ptr cs:qemm_next_handler
@@qemm_found:
        and cx, 4
        push dword ptr [esp + 0x0E]
        call word ptr ds:[_impl_table + si]
        pop si
        pop bx
	pop ds
        retf


        RESIDENT ends
        end

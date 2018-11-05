	.386

        public _amis_header
        public _amis_id
        public _amis_handler

        public _emm386_table
        public _qemm_handler

        extern _config : near
        extern _port_trap_ip : near
        extern get_port_handler_ : proc


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

emm386_handler:
        push bx
        mov bx, [bp+8]
        mov word ptr [_port_trap_ip+2], 0xD8    ; EMM386 selector for user code segment
        mov word ptr [_port_trap_ip], bx        ; user IP register
        call get_port_handler_
        call bx
        pop bx
        clc
_retf:  retf

        even
_emm386_table:
        dw 0, emm386_handler
        dw 0, emm386_handler
        dw 0, emm386_handler
        dw 0, emm386_handler
        dw 0, emm386_handler
        dw 0, emm386_handler
        dw 0, emm386_handler
        dw 0, emm386_handler
        dw 0, emm386_handler
        dw 0, emm386_handler
        dw 0, emm386_handler
        dw 0, emm386_handler
        dw 0, emm386_handler
        dw 0, emm386_handler
        dw 0, emm386_handler
        dw 0, emm386_handler


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; QEMM GLUE CODE

_qemm_handler:
        iisp_header qemm_next_handler
        cmp dx, 0x200
        jl @@qemm_skip

        push ds
        push ebx
        mov bx, cs
        mov ds, bx

        mov bx, sp
        mov ebx, dword ptr ss:[bx + 0x0E]
        mov dword ptr ds:[_port_trap_ip], ebx

        call get_port_handler_
        test bx, bx
        je @@qemm_not_for_us
        call bx

        pop ebx
        pop ds
        retf

@@qemm_not_for_us:
        pop ebx
        pop ds
@@qemm_skip:
        jmp dword ptr cs:qemm_next_handler


        RESIDENT ends
        end

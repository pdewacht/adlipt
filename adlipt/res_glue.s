        public _config

        public _amis_header
        public _amis_id
        public _amis_chain
        public amis_hook_

        public _emm386_glue
        public qemm_glue_
        public _qemm_chain

        extern emulate_adlib_io_ : proc


cmp_ah  macro
        db 0x80, 0xFC
        endm

jmp_far macro
        db 0xEA
        endm


        RESIDENT segment word use16 public 'CODE'


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; AMIS API IMPLEMENTATION


_amis_header:
        db 'SERDACO '           ;8 bytes: manufacturer
        db 'ADLIPT  '           ;8 bytes: product
        db 0                    ;no description

;;; Configuration immediately follows AMIS header
_config:
enable:   db 1
lpt_port: dw 0
bios_id:  dw -1


;;; IBM Interrupt Sharing Protocol header
iisp_header macro chain
        jmp short $+0x12
chain:  dd 0
        dw 0x424B               ;signature
        db 0                    ;flags
        jmp short _retf         ;hardware reset routine
        db 7 dup (0)            ;unused/zero
        endm


amis_hook_:
        iisp_header _amis_chain
        cmp_ah
_amis_id: db 0xFF
        je @@amis_match
        jmp dword ptr cs:_amis_chain
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
        dw amis_hook_


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; EMM386 GLUE CODE


        even
_emm386_glue:
        dw 0x0388, emm386_handler
        dw 0x0389, emm386_handler

emm386_handler:
        cmp byte ptr cs:[enable], 0
        je @@emm_not_enabled
        push cx
        push dx
        call emulate_adlib_io_
        pop dx
        pop cx
        clc
        retf
@@emm_not_enabled:
        stc
_retf:  retf


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; QEMM GLUE CODE


qemm_glue_:
        cmp dx, 0x0388
        jl @@qemm_ignore
        cmp dx, 0x0389
        jg @@qemm_ignore
        cmp byte ptr cs:[enable], 0
        jz @@qemm_ignore
        and cx, 4
        push ds
        push cs
        pop ds
        call emulate_adlib_io_
        pop ds
        retf
@@qemm_ignore:
        jmp_far
_qemm_chain: dw 0, 0


        RESIDENT ends
        end

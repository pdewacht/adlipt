/*
 * Miscellaneous JEMM-related definitions.
 * JEMM doesn't come with a usable or complete header file.
 */

/*
Get_VMM_Version = 0x10000
Get_Cur_VM_Handle = 0x10001
Allocate_V86_Call_Back = 0x10002
Crash_Cur_VM = 0x10003
Hook_V86_Int_Chain = 0x10004
Get_V86_Int_Vector = 0x10005
Set_V86_Int_Vector = 0x10006
Get_PM_Int_Vector = 0x10007
Set_PM_Int_Vector = 0x10008
Simulate_Int = 0x10009
Simulate_Iret = 0x1000A
Simulate_Far_Call = 0x1000B
Simulate_Far_Jmp = 0x1000C
Simulate_Far_Ret = 0x1000D
Simulate_Far_Ret_N = 0x1000E
Build_Int_Stack_Frame = 0x1000F
Simulate_Push = 0x10010
Simulate_Pop = 0x10011
_PageFree = 0x10012
_PhysIntoV86 = 0x10013
_LinMapIntoV86 = 0x10014
Hook_V86_Fault = 0x10015
Hook_PM_Fault = 0x10016
Begin_Nest_Exec = 0x10017
Exec_Int = 0x10018
Resume_Exec = 0x10019
End_Nest_Exec = 0x1001A
Save_Client_State = 0x1001B
Restore_Client_State = 0x1001C
Simulate_IO = 0x1001D
Install_Mult_IO_Handlers = 0x1001E
Install_IO_Handler = 0x1001F
VMM_Add_DDB = 0x10020
VMM_Remove_DDB = 0x10021
Remove_IO_Handler = 0x10022
Remove_Mult_IO_Handlers = 0x10023
Unhook_V86_Int_Chain = 0x10024
Unhook_V86_Fault = 0x10025
Unhook_PM_Fault = 0x10026
_PageReserve = 0x10027
_PageCommit = 0x10028
_PageDecommit = 0x10029
_PageCommitPhys = 0x1002A
*/

#define DLL_PROCESS_ATTACH 1
#define DLL_PROCESS_DETACH 0

#define UNDEFINED_DEVICE_ID 0x0000
#define UNDEFINED_INIT_ORDER 0x80000000

_Packed struct vxd_desc_block {
  unsigned Next;
  unsigned short Version;
  unsigned short Req_Device_Number;
  char Dev_Major_Version;
  char Dev_Minor_Version;
  unsigned short Flags;
  char Name[8];
  unsigned Init_Order;
  unsigned Control_Proc;
  unsigned V86_API_Proc;
  unsigned PM_API_Proc;
  unsigned V86_API_CSIP;
  unsigned PM_API_CSIP;
  unsigned Reference_Data;
  unsigned Service_Table_Ptr;
  unsigned Service_Table_Size;
  unsigned Win32_Service_Table;
  unsigned Prev;
  unsigned Size;
  unsigned Reserved1;
  unsigned Reserved2;
  unsigned Reserved3;
};

_Packed struct jlcomm {
  unsigned short ldr_cs;
  unsigned short flags;
  char *cmd_line;
  union {
    unsigned request;
    unsigned drivers;
  };
};

_Packed struct Client_Reg_Struc {
  unsigned Client_EDI;       /* Client's EDI */
  unsigned Client_ESI;       /* Client's ESI */
  unsigned Client_EBP;       /* Client's EBP */
  unsigned Client_res0;      /* ESP at pushall */
  unsigned Client_EBX;       /* Client's EBX */
  unsigned Client_EDX;       /* Client's EDX */
  unsigned Client_ECX;       /* Client's ECX */
  unsigned Client_EAX;       /* Client's EAX */
  unsigned Client_IntNo;
  unsigned Client_Error;     /* Dword error code */
  unsigned Client_EIP;       /* EIP */
  unsigned short Client_CS;       /* CS */
  unsigned short Client_res1;     /*   (padding) */
  unsigned Client_EFlags;    /* EFLAGS */
  unsigned Client_ESP;       /* ESP */
  unsigned short Client_SS;       /* SS */
  unsigned short Client_res2;     /*   (padding) */
  unsigned short Client_ES;       /* ES */
  unsigned short Client_res3;     /*   (padding) */
  unsigned short Client_DS;       /* DS */
  unsigned short Client_res4;     /*   (padding) */
  unsigned short Client_FS;       /* FS */
  unsigned short Client_res5;     /*   (padding) */
  unsigned short Client_GS;       /* GS */
  unsigned short Client_res6;     /*   (padding) */
};

_Packed struct cb_s {
  unsigned CB_VM_Status;     /* VM status flags */
  unsigned CB_High_Linear;   /* Address of VM mapped high */
  struct Client_Reg_Struc *CB_Client_Pointer;
  unsigned CB_VMID;
  unsigned CB_Signature;
};

__declspec(naked) static struct cb_s *Get_Cur_VM_Handle() {
  __asm {
    int 0x20
    dd 0x10001
    ret
  }
}
#pragma aux Get_Cur_VM_Handle value [ebx] modify exact [eax ecx edx]

__declspec(naked) static void Begin_Nest_Exec(struct Client_Reg_Struc *pcl) {
  __asm {
    push ebp
    mov ebp, edi
    int 0x20
    dd 0x10017
    pop ebp
    ret
  }
}
#pragma aux Begin_Nest_Exec parm [edi] modify exact [eax ecx edx]

__declspec(naked) static void End_Nest_Exec(struct Client_Reg_Struc *pcl) {
  __asm {
    push ebp
    mov ebp, edi
    int 0x20
    dd 0x1001A
    pop ebp
    ret
  }
}
#pragma aux End_Nest_Exec parm [edi] modify exact [eax ecx edx]

__declspec(naked) static void Exec_Int(struct Client_Reg_Struc *pcl, int intno) {
  __asm {
    push ebp
    mov ebp, edi
    int 0x20
    dd 0x10018
    pop ebp
    ret
  }
}
#pragma aux Exec_Int parm [edi] [eax] modify exact [eax ecx edx]

__declspec(naked) static int Install_IO_Handler(int port, void (*handler)()) {
  __asm {
    int 0x20
    dd 0x1001F
    sbb eax, eax
    ret
  }
}
#pragma aux Install_IO_Handler parm [edx] [esi] modify exact [eax ecx edx]

__declspec(naked) static void Remove_IO_Handler(int port) {
  __asm {
    int 0x20
    dd 0x10022
    ret
  }
}
#pragma aux Remove_IO_Handler parm [edx] modify exact [eax ecx edx]

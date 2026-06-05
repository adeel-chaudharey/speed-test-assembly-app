; ============================================================
;  typing_analyzer.asm  –  Typing Speed Test Core Logic
;  Assembler : NASM (x86-64)
;  OS        : Windows 11 (Win64 calling convention via GCC)
;
;  Usage:
;    typing_analyzer.exe <input_file> <output_file>
; ============================================================

default rel
bits 64

; ── Windows API imports ─────────────────────────────────────
extern CreateFileA
extern ReadFile
extern WriteFile
extern CloseHandle
extern ExitProcess

; Win32 constants
GENERIC_READ         equ 0x80000000
GENERIC_WRITE        equ 0x40000000
OPEN_EXISTING        equ 3
CREATE_ALWAYS        equ 2
FILE_SHARE_READ       equ 1
FILE_ATTRIBUTE_NORMAL equ 0x80
INVALID_HANDLE        equ -1

section .bss
    buf_original    resb 4096
    buf_typed       resb 4096
    file_buf        resb 8192
    out_buf         resb 512
    bytes_read      resd 1
    out_len         resq 1

    ; Results (64-bit)
    correct_chars   resq 1
    incorrect_chars resq 1
    total_chars     resq 1
    word_count      resq 1

    ; Handles
    h_in            resq 1
    h_out           resq 1

    ; Argument pointers
    arg_in_ptr      resq 1
    arg_out_ptr     resq 1

section .text
global main

; ── Entry point ─────────────────────────────────────────────
main:
    push rbp
    mov  rbp, rsp
    sub  rsp, 64                ; Shadow space (32) + alignment

    ; Check if argc >= 3
    cmp  rcx, 3
    jl   .bad_usage

    ; Extract argv[1] and argv[2]
    mov  rax, [rdx + 8]         ; argv[1]
    mov  [arg_in_ptr], rax
    mov  rax, [rdx + 16]        ; argv[2]
    mov  [arg_out_ptr], rax

    ; ── Open input file ──────────────────────────────────────
    mov  rcx, [arg_in_ptr]
    mov  rdx, GENERIC_READ
    mov  r8,  FILE_SHARE_READ
    xor  r9d, r9d
    mov  dword [rsp+32], OPEN_EXISTING
    mov  dword [rsp+40], FILE_ATTRIBUTE_NORMAL
    mov  qword [rsp+48], 0
    call CreateFileA
    cmp  rax, INVALID_HANDLE
    je   .err_open_in
    mov  [h_in], rax

    ; ── Read input file ──────────────────────────────────────
    mov  rcx, [h_in]
    lea  rdx, [file_buf]
    mov  r8d, 8000
    lea  r9,  [bytes_read]
    mov  qword [rsp+32], 0
    call ReadFile
    test eax, eax
    jz   .err_read

    mov  eax, dword [bytes_read]
    lea  r10, [file_buf]
    mov  byte [r10 + rax], 0

    mov  rcx, [h_in]
    call CloseHandle

    ; ── Split file_buf into two lines ────────────────────────
    lea  rsi, [file_buf]
    lea  rdi, [buf_original]
    call copy_line              

    lea  rdi, [buf_typed]
    call copy_line              

    ; ── Core analysis ────────────────────────────────────────
    call analyze_text

    ; ── Build output string ──────────────────────────────────
    call build_output

    ; ── Open / create output file ────────────────────────────
    mov  rcx, [arg_out_ptr]
    mov  rdx, GENERIC_WRITE
    xor  r8d, r8d
    xor  r9d, r9d
    mov  dword [rsp+32], CREATE_ALWAYS
    mov  dword [rsp+40], FILE_ATTRIBUTE_NORMAL
    mov  qword [rsp+48], 0
    call CreateFileA
    cmp  rax, INVALID_HANDLE
    je   .err_open_out
    mov  [h_out], rax

    ; ── Write output ─────────────────────────────────────────
    mov  rcx, [h_out]
    lea  rdx, [out_buf]
    mov  r8,  [out_len]
    lea  r9,  [bytes_read]
    mov  qword [rsp+32], 0
    call WriteFile

    mov  rcx, [h_out]
    call CloseHandle

    xor  ecx, ecx
    call ExitProcess

.bad_usage:
    mov  ecx, 1
    call ExitProcess
.err_open_in:
    mov  ecx, 2
    call ExitProcess
.err_open_out:
    mov  ecx, 3
    call ExitProcess
.err_read:
    mov  ecx, 4
    call ExitProcess

; ============================================================
;  copy_line
; ============================================================
copy_line:
    push rbp
    mov  rbp, rsp
.loop:
    movzx eax, byte [rsi]
    cmp  al, 0
    je   .done
    cmp  al, 13
    je   .skip_cr
    cmp  al, 10
    je   .newline
    mov  [rdi], al
    inc  rsi
    inc  rdi
    jmp  .loop
.skip_cr:
    inc  rsi
    jmp  .loop
.newline:
    inc  rsi
.done:
    mov  byte [rdi], 0
    pop  rbp
    ret

; ============================================================
;  analyze_text
; ============================================================
analyze_text:
    push rbp
    mov  rbp, rsp
    push rbx                    ; 1
    push r12                    ; 2
    push r13                    ; 3
    push r14                    ; 4
    push r15                    ; 5
    sub  rsp, 8                 ; Align stack perfectly

    mov  qword [correct_chars],   0
    mov  qword [incorrect_chars], 0
    mov  qword [total_chars],     0
    mov  qword [word_count],      0

    lea  r12, [buf_original]
    lea  r13, [buf_typed]
    xor  r14, r14               

.loop:
    movzx ebx, byte [r13]
    test bl, bl
    jz   .done_loop

    inc  qword [total_chars]

    cmp  bl, ' '
    je   .is_space
    test r14, r14
    jnz  .not_new_word
    inc  qword [word_count]
    mov  r14, 1
    jmp  .not_new_word
.is_space:
    mov  r14, 0
.not_new_word:

    movzx ecx, byte [r12]
    cmp  bl, cl
    jne  .wrong

    inc  qword [correct_chars]
    inc  r12
    inc  r13
    jmp  .loop

.wrong:
    inc  qword [incorrect_chars]
    movzx ecx, byte [r12]
    test cl, cl
    jz   .skip_r12_adv
    inc  r12
.skip_r12_adv:
    inc  r13
    jmp  .loop

.done_loop:
    add  rsp, 8                 ; Clean up alignment space
    pop  r15                    ; 5
    pop  r14                    ; 4
    pop  r13                    ; 3
    pop  r12                    ; 2
    pop  rbx                    ; 1 (FIXED: Pop rbx restored!)
    pop  rbp
    ret

; ============================================================
;  build_output
; ============================================================
build_output:
    push rbp
    mov  rbp, rsp
    push rbx                    ; 1
    push r12                    ; 2

    lea  r12, [out_buf]

    mov  rax, [correct_chars]
    call append_decimal
    call append_crlf

    mov  rax, [incorrect_chars]
    call append_decimal
    call append_crlf

    mov  rax, [total_chars]
    call append_decimal
    call append_crlf

    mov  rax, [word_count]
    call append_decimal
    call append_crlf

    lea  rax, [out_buf]
    sub  r12, rax
    mov  [out_len], r12

    pop  r12                    ; 2
    pop  rbx                    ; 1 (FIXED: Pop rbx restored!)
    pop  rbp
    ret

append_crlf:
    mov  byte [r12],   13
    mov  byte [r12+1], 10
    add  r12, 2
    ret

append_decimal:
    push rbp
    mov  rbp, rsp
    push rbx
    push rcx
    push rdx
    push rdi
    sub  rsp, 32                

    lea  rdi, [rsp]
    xor  rbx, rbx

    test rax, rax
    jnz  .cvt_loop
    mov  byte [rdi], '0'
    inc  rbx
    jmp  .emit

.cvt_loop:
    test rax, rax
    jz   .emit
    xor  rdx, rdx
    mov  rcx, 10
    div  rcx
    add  dl, '0'
    mov  [rdi + rbx], dl
    inc  rbx
    jmp  .cvt_loop

.emit:
    lea  rcx, [rdi + rbx]
.emit_loop:
    dec  rcx
    cmp  rcx, rdi
    jl   .emit_done
    movzx eax, byte [rcx]
    mov  [r12], al
    inc  r12
    jmp  .emit_loop
.emit_done:

    add  rsp, 32
    pop  rdi
    pop  rdx
    pop  rcx
    pop  rbx
    pop  rbp
    ret
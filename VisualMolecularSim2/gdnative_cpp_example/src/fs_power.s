bits 64

section .text

global _fs_power

;// https://stackoverflow.com/questions/16782746/what-is-faster-than-stdpow

;// Raise 'x' to the power 'n' (INT-only) in ASM by the great Agner Fog!

;// extern double fs_power(double x, long n);
_fs_power:
    MOV  EAX, EDI     ;// Move 'n' to eax
;// abs(n) is calculated by inverting all bits and adding 1 if n < 0:
    CDQ               ;// Get sign bit into all bits of edx
    XOR  EAX, EDX     ;// Invert bits if negative
    SUB  EAX, EDX     ;// Add 1 if negative. Now eax = abs(n)
    JZ   RETZERO      ;// End if n = 0

    mov rdi, 1
    MOVQ XMM1, rdi     ;; FLD1              ;// ST(0) = 1.0 (FPU push1)

    ;; FLD  QWORD [ESP+4] ;// Load 'x' : ST(0) = 'x', ST(1) = 1.0 (FPU push2)
    JMP  L2           ;// Jump into loop
L1: ;// Top of loop
    mulsd xmm0, xmm0 ;; FMUL ST0, ST0 ;// Square x
L2: ;// Loop entered here
    SHR  EAX, 1       ;// Get each bit of n into carry flag
    JNC  L1           ;// No carry. Skip multiplication, goto next
    mulsd xmm1, xmm0 ;; FMUL ST1, ST0 ;// Multiply by x squared i times for bit # i
    JNZ  L1           ;// End of loop. Stop when nn = 0
    ;; FSTP ST0        ;// Discard ST(0) (FPU Pop1)
    TEST EDX, EDX     ;// Test if 'n' was negative
    JNS  RETPOS       ;// Finish if 'n' was positive
    ;; FLD1              ;// ST(0) = 1.0, ST(1) = x^abs(n)
    ;; FDIVR             ;// Reciprocal
    ;; ^^reciprocal:
    MOVQ XMM1, rdi
    divsd XMM0, XMM1
RETPOS:    ;// Finish, success!
    ;; RET  4 ;//(FPU Pop2 occurs by compiler on assignment
    ret

RETZERO:
    mov rdi, 0
    MOVQ XMM0, rdi

    ;; FLDZ   ;// Ret 0.0, fail, if n was 0
    ;; RET  4
    ret
;//}}

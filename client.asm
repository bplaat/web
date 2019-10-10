; nasm -f elf64 client.asm -o client.o && ld client.o -o client && rm client.o && ./client
; objdump -S -M intel client

; https://filippo.io/linux-syscall-table/
; https://linux.die.net/man/7/ip

%include 'syscalls.inc'

section .data
    socket dq 0
    sockaddr dw AF_INET
        dw 0x5000 ; 80
        dd 0x8e11d9ac ; 172.217.17.142
        dq 0

    request db 'GET / HTTP/1.0', 13, 10, 13, 10
    request_length equ $ - request

    buffer_length equ 1024
    buffer times buffer_length db 0

section .text
global _start

_start:
    sys_socket AF_INET, SOCK_STREAM, IPPROTO_IP
    mov [socket], rax

    sys_connect [socket], sockaddr, 16

    sys_write [socket], request, request_length

read_write_loop:
    sys_read [socket], buffer, buffer_length
    cmp rax, 0
    je .end

    sys_write stdout, buffer, rax
    jmp read_write_loop
.end:

    sys_close [socket]

    sys_exit EXIT_SUCCESS

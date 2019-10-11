; nasm -f elf64 client.asm -o client.o && ld -s client.o -o client && rm client.o && ./client
; objdump -S -M intel client

%include 'syscalls.inc'

section .data
    client_socket dq 0
    client_address dw AF_INET
        dw 0x5000 ; 80
        dd 0x8e11d9ac ; 172.217.17.142
        dq 0
    client_address_length equ $ - client_address

    request db 'GET / HTTP/1.0', 13, 10, 13, 10
    request_length equ $ - request

    buffer_length equ 1024
    buffer times buffer_length db 0

section .text
global _start

_start:
    sys_socket AF_INET, SOCK_STREAM, IPPROTO_TCP
    mov [client_socket], rax

    sys_connect [client_socket], client_address, client_address_length

    sys_write [client_socket], request, request_length

read_buffer_write_stout_loop:
    sys_read [client_socket], buffer, buffer_length
    cmp rax, 0
    je .done

    sys_write stdout, buffer, rax
    jmp read_buffer_write_stout_loop
.done:

    sys_close [client_socket]

    sys_exit EXIT_SUCCESS

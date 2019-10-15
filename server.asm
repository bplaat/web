; nasm -f elf64 server.asm -o server.o && ld -s server.o -o server && rm server.o && strace -f ./server
; objdump -S -M intel server

%include 'syscalls.inc'

section .data
    server_socket dq 0
    server_address dw AF_INET
        dw 0x901f ; 8080
        dd INADDR_ANY ; 0.0.0.0
        times 8 db 0
    server_address_length equ $ - server_address

    client_socket dq 0
    client_address times 16 db 0
    client_address_length dq $ - client_address

    response_header db 'HTTP/1.0 200 OK', 13, 10, 13, 10
    response_header_length equ $ - response_header

    filename db 'file.html', 0
    file dq 0

    buffer_length equ 64
    buffer times buffer_length db 0

    sigaction:
        sa_handler  dq SIG_IGN
        sa_flags    dq 0x04000000
        sa_restorer dq NULL
        sa_mask     dq 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

    sendfile_length equ 1024

section .text
global _start

_start:
    sys_rt_sigaction SIGCHLD, sigaction, NULL, 8

    sys_socket AF_INET, SOCK_STREAM, IPPROTO_TCP
    cmp rax, 0
    jl exit_error
    mov [server_socket], rax

    sys_bind [server_socket], server_address, server_address_length
    cmp rax, 0
    jl exit_error

    sys_listen [server_socket], 1000

client_loop:
    sys_accept [server_socket], client_address, client_address_length
    mov [client_socket], rax
    cmp rax, 0
    jl stop_server

    sys_fork
    cmp rax, 0
    jl stop_server

    cmp rax, 0
    je handle_client

    sys_close [client_socket]
    jmp client_loop

stop_server:
    sys_close [server_socket]
exit_error:
    sys_exit EXIT_FAILURE

handle_client:

read_request:
    sys_read [client_socket], buffer, buffer_length
    cmp rax, buffer_length
    je read_request

    sys_write [client_socket], response_header, response_header_length

    sys_open filename, O_RDONLY, 0
    mov [file], rax
sendfile:
    sys_sendfile [client_socket], [file], NULL, sendfile_length
    cmp rax, sendfile_length
    je sendfile
    sys_close [file]

    sys_close [client_socket]
    sys_exit EXIT_SUCCESS

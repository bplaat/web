; https://filippo.io/linux-syscall-table/

NULL equ 0

stdin equ 0
stdout equ 1
stderr equ 2

%macro sys_read 3
    mov rdx, %3
    mov rsi, %2
    mov rdi, %1
    mov rax, 0
    syscall
%endmacro

%macro sys_write 3
    mov rdx, %3
    mov rsi, %2
    mov rdi, %1
    mov rax, 1
    syscall
%endmacro

O_RDONLY equ 0
O_WRONLY equ 1
O_RDWR equ 2

%macro sys_open 3
    mov rdx, %3
    mov rsi, %2
    mov rdi, %1
    mov rax, 2
    syscall
%endmacro

%macro sys_close 1
    mov rdi, %1
    mov rax, 3
    syscall
%endmacro

SIGCHLD equ 17
SIG_IGN equ 1

%macro sys_rt_sigaction 4
    mov r10, %4
    mov rdx, %3
    mov rsi, %2
    mov rdi, %1
    mov rax, 13
    syscall
%endmacro

%macro sys_sendfile 4
    mov r10, %4
    mov rdx, %3
    mov rsi, %2
    mov rdi, %1
    mov rax, 40
    syscall
%endmacro

AF_INET equ 2
SOCK_STREAM equ 1
IPPROTO_TCP equ 6

%macro sys_socket 3
    mov rdx, %3
    mov rsi, %2
    mov rdi, %1
    mov rax, 41
    syscall
%endmacro

%macro sys_connect 3
    mov rdx, %3
    mov rsi, %2
    mov rdi, %1
    mov rax, 42
    syscall
%endmacro

%macro sys_accept 3
    mov rdx, %3
    mov rsi, %2
    mov rdi, %1
    mov rax, 43
    syscall
%endmacro

INADDR_ANY equ 0

%macro sys_bind 3
    mov rdx, %3
    mov rsi, %2
    mov rdi, %1
    mov rax, 49
    syscall
%endmacro

%macro sys_listen 2
    mov rsi, %2
    mov rdi, %1
    mov rax, 50
    syscall
%endmacro

%macro sys_fork 0
    mov rax, 57
    syscall
%endmacro

EXIT_SUCCESS equ 0
EXIT_FAILURE equ 1

%macro sys_exit 1
    mov rdi, %1
    mov rax, 60
    syscall
%endmacro

; https://filippo.io/linux-syscall-table/

_read equ 0

stdin equ 0
stdout equ 1
stderr equ 2

%macro sys_read 3
    mov rdx, %3
    mov rsi, %2
    mov rdi, %1
    mov rax, _read
    syscall
%endmacro

_write equ 1

%macro sys_write 3
    mov rdx, %3
    mov rsi, %2
    mov rdi, %1
    mov rax, _write
    syscall
%endmacro

_open equ 2

O_RDONLY equ 0
O_WRONLY equ 1
O_RDWR equ 2

%macro sys_open 3
    mov rdx, %3
    mov rsi, %2
    mov rdi, %1
    mov rax, _open
    syscall
%endmacro

_close equ 3

%macro sys_close 1
    mov rdi, %1
    mov rax, _close
    syscall
%endmacro

_socket equ 41

AF_INET equ 2
SOCK_STREAM equ 1
IPPROTO_IP equ 0

%macro sys_socket 3
    mov rdx, %3
    mov rsi, %2
    mov rdi, %1
    mov rax, _socket
    syscall
%endmacro

_connect equ 42

%macro sys_connect 3
    mov rdx, %3
    mov rsi, %2
    mov rdi, %1
    mov rax, _connect
    syscall
%endmacro

_accept equ 43

%macro sys_accept 3
    mov rdx, %3
    mov rsi, %2
    mov rdi, %1
    mov rax, _accept
    syscall
%endmacro

_bind equ 49

INADDR_ANY equ 0

%macro sys_bind 3
    mov rdx, %3
    mov rsi, %2
    mov rdi, %1
    mov rax, _bind
    syscall
%endmacro

_listen equ 50

%macro sys_listen 2
    mov rsi, %2
    mov rdi, %1
    mov rax, _listen
    syscall
%endmacro

_exit equ 60

EXIT_SUCCESS equ 0
EXIT_FAILURE equ 1

%macro sys_exit 1
    mov rdi, %1
    mov rax, _exit
    syscall
%endmacro

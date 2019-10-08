cmake_minimum_required(VERSION 3.9)
project(ziz19_project2)

set(CMAKE_CXX_STANDARD 11)

add_executable(ziz19_project2
        info_sheet.txt
        README.md
        sys.c
        syscall_table.S
        unistd.h prodcons.c)

#pragma once

#define FORCEINLINE __attribute__((always_inline))

#define NULL (0)

#define NAKED __attribute__((naked))

/* GDT 和 IDT 中描述符的个数 */
enum 
{
    GDT_SIZE = 128,
    IDT_SIZE = 256
};

/* 权限 */
enum
{
    PRIVILEGE_KRNL = 0,
    PRIVILEGE_TASK = 1,
    PRIVILEGE_USER = 3
};
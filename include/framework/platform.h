#pragma once

#include <string.h>

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#else

#include <stdint.h>

typedef void* HINSTANCE;
#define WINAPI
typedef char* LPSTR;

#define ZeroMemory(ptr, size) memset((ptr), 0, (size))

#define VK_ESCAPE 0x1B
#define VK_RETURN 0x0D
#define VK_LEFT   0x25
#define VK_UP     0x26
#define VK_RIGHT  0x27
#define VK_DOWN   0x28
#define VK_F1     0x70

void platformShowCursor(int show);
#define ShowCursor(show) platformShowCursor((show) ? 1 : 0)

#endif

#ifndef PSEUDOCONSOLE_H
#define PSEUDOCONSOLE_H

#ifndef PSEUDOCONSOLE_INHERIT_CURSOR
#define PSEUDOCONSOLE_INHERIT_CURSOR 1
#endif

typedef struct _HPCON_INTERNAL
{
    HANDLE hWritePipe;
    HANDLE hConDrvReference;
    HANDLE hConHostProcess;
} HPCON_INTERNAL;

HRESULT
WINAPI
CreatePseudoConsole_mod(
    COORD size,
    HANDLE hInput,
    HANDLE hOutput,
    DWORD dwFlags,
    HPCON* phPC
);

#define RESIZE_CONHOST_SIGNAL_BUFFER 8

typedef struct _RESIZE_PSEUDO_CONSOLE_BUFFER
{
    SHORT Flags;
    SHORT SizeX;
    SHORT SizeY;
} RESIZE_PSEUDO_CONSOLE_BUFFER;

HRESULT
WINAPI
ResizePseudoConsole_mod(HPCON hPC, COORD size);

void
WINAPI
ClosePseudoConsole_mod(HPCON hPC);

#endif /* PSEUDOCONSOLE_H */

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
} HPCON_INTERNAL, *PHPCON_INTERNAL;

BOOL
WINAPI
CreatePseudoConsoleAsUser_mod(HANDLE TokenHandle,
                              COORD ConsoleSize,
                              HANDLE hInput,
                              HANDLE hOutput,
                              DWORD dwFlags,
                              PHPCON_INTERNAL hpCon);

BOOL
WINAPI
CreatePseudoConsole_mod(COORD ConsoleSize,
                        HANDLE hInput,
                        HANDLE hOutput,
                        DWORD dwFlags,
                        PHPCON_INTERNAL hpCon);

#define RESIZE_CONHOST_SIGNAL_BUFFER 8

typedef struct _RESIZE_PSEUDO_CONSOLE_BUFFER
{
    USHORT Flags;
    USHORT SizeX;
    USHORT SizeY;
} RESIZE_PSEUDO_CONSOLE_BUFFER, *PRESIZE_PSEUDO_CONSOLE_BUFFER;

BOOL
WINAPI
ResizePseudoConsole_mod(PHPCON_INTERNAL hPCon,
                        COORD ConsoleSize);

void
WINAPI
ClosePseudoConsole_mod(PHPCON_INTERNAL hpCon);

#endif /* PSEUDOCONSOLE_H */

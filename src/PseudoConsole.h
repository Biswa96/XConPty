#ifndef PSEUDOCONSOLE_H
#define PSEUDOCONSOLE_H

typedef struct _X_HPCON {
    HANDLE hWritePipe;
    HANDLE hConDrvReference;
    HANDLE hConHostProcess;
} X_HPCON, *PX_HPCON;

HRESULT
WINAPI
X_CreatePseudoConsoleAsUser(HANDLE TokenHandle,
                            COORD ConsoleSize,
                            HANDLE hInput,
                            HANDLE hOutput,
                            DWORD dwFlags,
                            PX_HPCON hpCon);

HRESULT
WINAPI
X_CreatePseudoConsole(COORD ConsoleSize,
                      HANDLE hInput,
                      HANDLE hOutput,
                      DWORD dwFlags,
                      PX_HPCON hpCon);

#define RESIZE_CONHOST_SIGNAL_BUFFER 8

typedef struct _RESIZE_PSEUDO_CONSOLE_BUFFER {
    USHORT Flags;
    USHORT SizeX;
    USHORT SizeY;
} RESIZE_PSEUDO_CONSOLE_BUFFER, *PRESIZE_PSEUDO_CONSOLE_BUFFER;

HRESULT
WINAPI
X_ResizePseudoConsole(PX_HPCON hPC,
                      COORD ConsoleSize);

VOID
WINAPI
X_ClosePseudoConsole(PX_HPCON hpCon);

#endif // PSEUDOCONSOLE_H

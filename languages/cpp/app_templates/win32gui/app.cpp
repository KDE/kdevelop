#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <windows.h>

int STDCALL
WinMain (HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow)
{
        MessageBox (NULL, "Hello, Windows!", "Hello", MB_OK);
        return 0;
}


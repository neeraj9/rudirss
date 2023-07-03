// rudirss.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "rudirss.h"
#include "RudiRSSMainWindow.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    RudiRSSMainWindow mainWindow;
    mainWindow.Initialize(hInstance);

    return static_cast<int>(mainWindow.MessageLoop());
}
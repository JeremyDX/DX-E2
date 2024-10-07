#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include "Engine.h"
#include "ScreenManagerSystem.h"

#pragma comment(lib, "D3D11.lib")

LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam);

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    const wchar_t CLASS_NAME[] = L"WindowClass";

    WNDCLASS wc = { };
    wc.lpszClassName = CLASS_NAME;
    wc.hInstance = hInstance;
    wc.lpfnWndProc = WindowProc;

    RegisterClass(&wc);

    ScreenManagerSystem::UpdateScreenParameters(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));

    HWND hWnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"Learn to Program Windows",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, ScreenManagerSystem::GetScreenWidth(), ScreenManagerSystem::GetScreenHeight(),

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

    ShowWindow(hWnd, nShowCmd);

    const int RETURNED_ERROR_CODE = Engine::StartGameLoop(hWnd);

    if (RETURNED_ERROR_CODE != 0)
    {
        // Format the message string including the integer value
        wchar_t Message[32];
        wsprintf(Message, L"Error Received : %d", RETURNED_ERROR_CODE);

        // Display a message box with the formatted message
        MessageBox(NULL, Message, L"Crash Error Detected!", MB_OK | MB_ICONINFORMATION);
    };

    return RETURNED_ERROR_CODE;
}

LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    switch (Msg)
    {
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }

        case WM_CLOSE:
        {
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProc(hWnd, Msg, wParam, lParam);
}
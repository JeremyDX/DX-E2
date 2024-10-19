#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>

#include "Engine.h"
#include "ScreenManagerSystem.h"
#include "XGameInput.h"
/*
* Required Lib Files
* Project Properties -> Linker -> Input -> Additional Dependencies 
*  
dxgi.lib
d3dcompiler.lib
d3d11.lib
xinput.lib
dxguid.lib
*/

LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam);

void RegisterRawInputDevices(HWND hwnd)
{
    RAWINPUTDEVICE rid[2];

    // Keyboard
    rid[0].usUsagePage = 0x01; // Generic desktop controls
    rid[0].usUsage = 0x06;     // Keyboard
    rid[0].dwFlags = RIDEV_INPUTSINK;        // Default flags
    rid[0].hwndTarget = hwnd;  // Target window

    // Mouse
    rid[1].usUsagePage = 0x01; // Generic desktop controls
    rid[1].usUsage = 0x02;     // Mouse
    rid[1].dwFlags = RIDEV_INPUTSINK;        // Default flags
    rid[1].hwndTarget = hwnd;  // Target window

    if (RegisterRawInputDevices(rid, 2, sizeof(rid[0])) == FALSE) {
        // Registration failed. Call GetLastError for the cause of the error.
        MessageBox(NULL, L"Failed to register raw input devices.", L"Error", MB_OK);
    }
}

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

    RegisterRawInputDevices(hWnd);

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
        case WM_INPUT:
        {
            UINT dwSize = sizeof(RAWINPUT);
            static BYTE lpb[sizeof(RAWINPUT)];

            if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)))
            {
                RAWINPUT* raw = (RAWINPUT*)lpb;
                XGameInput::StoreRawInputStateChanges(raw);
            }

            break;
        }

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
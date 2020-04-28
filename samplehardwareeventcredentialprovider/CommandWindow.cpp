//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//

#include "CommandWindow.h"
#include <strsafe.h>

// Custom messages for managing the behavior of the window thread.
#define WM_EXIT_THREAD              WM_USER + 1
#define WM_TOGGLE_CONNECTED_STATUS  WM_USER + 2

const WCHAR c_szClassName[] = L"EventWindow";
const WCHAR c_szConnected[] = L"Connected";
const WCHAR c_szDisconnected[] = L"Disconnected";

CCommandWindow::CCommandWindow() : _hWnd(NULL), _hInst(NULL), _pProvider(NULL)
{
}

CCommandWindow::~CCommandWindow()
{
    // If we have an active window, we want to post it an exit message.
    if (_hWnd != NULL)
    {
        PostMessage(_hWnd, WM_EXIT_THREAD, 0, 0);
        _hWnd = NULL;
    }

    // We'll also make sure to release any reference we have to the provider.
    if (_pProvider != NULL)
    {
        _pProvider->Release();
        _pProvider = NULL;
    }
}

// Performs the work required to spin off our message so we can listen for events.
HRESULT CCommandWindow::Initialize(__in CSampleProvider *pProvider)
{
    HRESULT hr = S_OK;

    // Be sure to add a release any existing provider we might have, then add a reference
    // to the provider we're working with now.
    if (_pProvider != NULL)
    {
        _pProvider->Release();
    }
    _pProvider = pProvider;
    _pProvider->AddRef();
    
    // Create and launch the window thread.
    HANDLE hThread = CreateThread(NULL, 0, _ThreadProc, this, 0, NULL);
    if (hThread == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}

// Wraps our internal connected status so callers can easily access it.
BOOL CCommandWindow::GetConnectedStatus()
{
    return !_rdpState.is_rdp_active();
}

//
//  FUNCTION: _MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
HRESULT CCommandWindow::_MyRegisterClass()
{
    WNDCLASSEX wcex = { sizeof(wcex) };
    wcex.style            = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc      = _WndProc;
    wcex.hInstance        = _hInst;
    wcex.hIcon            = NULL;
    wcex.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground    = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName    = c_szClassName;

    return RegisterClassEx(&wcex) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}

//
//   FUNCTION: _InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HRESULT CCommandWindow::_InitInstance()
{
    HRESULT hr = S_OK;

    // Create our window to receive events.
    // 
    // This dialog is for demonstration purposes only.  It is not recommended to create 
    // dialogs that are visible even before a credential enumerated by this credential 
    // provider is selected.  Additionally, any dialogs that are created by a credential
    // provider should not have a NULL hwndParent, but should be parented to the HWND
    // returned by ICredentialProviderCredentialEvents::OnCreatingWindow.
    _hWnd = CreateWindowEx(
        WS_EX_TOPMOST, 
        c_szClassName, 
        c_szDisconnected, 
        WS_DLGFRAME,
        200, 200, 200, 80, 
        NULL,
        NULL, _hInst, NULL);
    if (_hWnd == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    if (SUCCEEDED(hr))
    {
        _rdpState.set_callback(rdp_state_changed, this);
        _rdpState.start();        

        if (SUCCEEDED(hr))
        {
            // Show and update the window.
            if (!ShowWindow(_hWnd, SW_NORMAL))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }

            if (SUCCEEDED(hr))
            {
                if (!UpdateWindow(_hWnd))
                {
                   hr = HRESULT_FROM_WIN32(GetLastError());
                }
            }
        }
    }

    return hr;
}

void CCommandWindow::rdp_state_changed(CCommandWindow *win)
{
    auto& rdp = win->_rdpState;
    if (rdp.is_rdp_active()) {
        char text[256];
        snprintf(text, sizeof(text), "RDP active with %s", rdp.get_rdp_text());
        SetWindowTextA(win->_hWnd, text);
    }
    else
        SetWindowTextA(win->_hWnd, "RDP is not active");

    win->_pProvider->OnConnectStatusChanged();
}

// Called from the separate thread to process the next message in the message queue. If
// there are no messages, it'll wait for one.
BOOL CCommandWindow::_ProcessNextMessage()
{
    // Grab, translate, and process the message.
    MSG msg;
    GetMessage(&(msg), _hWnd, 0, 0);
    TranslateMessage(&(msg));
    DispatchMessage(&(msg));

    // This section performs some "post-processing" of the message. It's easier to do these
    // things here because we have the handles to the window, its button, and the provider
    // handy.
    if (msg.message == WM_EXIT_THREAD)
        return FALSE;
    return TRUE;
}

// Manages window messages on the window thread.
LRESULT CALLBACK CCommandWindow::_WndProc(__in HWND hWnd, __in UINT message, __in WPARAM wParam, __in LPARAM lParam)
{
    switch (message)
    {
    // To play it safe, we hide the window when "closed" and post a message telling the 
    // thread to exit.
    case WM_CLOSE:
        ShowWindow(hWnd, SW_HIDE);
        PostMessage(hWnd, WM_EXIT_THREAD, 0, 0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Our thread procedure. We actually do a lot of work here that could be put back on the 
// main thread, such as setting up the window, etc.
DWORD WINAPI CCommandWindow::_ThreadProc(__in LPVOID lpParameter)
{
    CCommandWindow *pCommandWindow = static_cast<CCommandWindow *>(lpParameter);
    if (pCommandWindow == NULL)
    {
        // TODO: What's the best way to raise this error?
        return 0;
    }

    HRESULT hr = S_OK;

    // Create the window.
    pCommandWindow->_hInst = GetModuleHandle(NULL);
    if (pCommandWindow->_hInst != NULL)
    {            
        hr = pCommandWindow->_MyRegisterClass();
        if (SUCCEEDED(hr))
        {
            hr = pCommandWindow->_InitInstance();
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    // ProcessNextMessage will pump our message pump and return false if it comes across
    // a message telling us to exit the thread.
    if (SUCCEEDED(hr))
    {        
        while (pCommandWindow->_ProcessNextMessage()) 
        {
        }
    }
    else
    {
        if (pCommandWindow->_hWnd != NULL)
        {
            pCommandWindow->_hWnd = NULL;
        }
    }

    return 0;
}


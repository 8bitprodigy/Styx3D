/*!
	@file main.cpp
	@author Anthony Rufrano (paradoxnj)
	@brief The program's entry point
*/
#include <windows.h>
#include "GameMgr.h"
#include "ScriptMgr.h"
#include "GameLog.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WNDCLASS						wc;
	HWND							hWnd;
	MSG								msg;
	bool							running = true;

	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = "Jet Game Shell";
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	RegisterClass(&wc);

	hWnd = CreateWindow(wc.lpszClassName, wc.lpszClassName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);
	if (!hWnd)
	{
		MessageBox(NULL, "Could not create main window!!", "Jet Game Shell Error...", 48);
		return 0;
	}

	ShowWindow(hWnd, nShowCmd);
	UpdateWindow(hWnd);
	SetFocus(hWnd);

	InitializeGameLog("JetShell.log");
	CScriptMgr::GetPtr()->Initialize();

	if (!CGameMgr::GetPtr()->Initialize(hWnd))
	{
		MessageBox(hWnd, "Could not hstart game manager!!", "Jet Game Shell Error...", 48);
		CGameMgr::GetPtr()->Release();
		DestroyWindow(hWnd);
		return 0;
	}

	while (running)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				running = false;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if (!CGameMgr::GetPtr()->Frame())
				running = false;
		}
	}

	CGameMgr::GetPtr()->Release();
	CScriptMgr::GetPtr()->Release();

	DestroyWindow(hWnd);
	UnregisterClass(wc.lpszClassName, hInstance);
	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
	case WM_CLOSE:
		{
			PostQuitMessage(0);
			return FALSE;
		}
	case WM_KEYDOWN:
		{
			switch (wParam)
			{
			case VK_ESCAPE:
				{
					PostQuitMessage(0);
					break;
				}
			}

			return FALSE;
		}
	}

	return DefWindowProc(hWnd, iMsg, wParam, lParam);
}

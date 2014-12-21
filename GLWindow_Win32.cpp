/** 
 * Copyright (c) 2013 - Jorma Rebane 
 */
#include "GLWindow.h"
#include <stdio.h>
#include <malloc.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h> // Win32 API message system

namespace GL3
{

	/**
	 * GLWindow
	 * (C) Jorma Rebane, 2013
	 */


	#define WS_RESIZABLE WS_OVERLAPPEDWINDOW
	#define WS_NONRESIZABLE WS_CAPTION|WS_MINIMIZEBOX|WS_POPUPWINDOW


	static GLWindow* WindowsList = nullptr;
	static GLWindow* GetWindow(int handle)
	{
		GLWindow* ptr = WindowsList;
		while(ptr && (ptr->Handle != handle)) ptr = ptr->Next;	
		return ptr; // nullptr OR our Window*
	}
	static void AddWindow(GLWindow* window)
	{
		window->Next = WindowsList;
		WindowsList = window;
	}
	static void RemoveWindow(GLWindow* window)
	{
		if(!window) return;
		GLWindow* prev = 0;
		GLWindow* ptr = WindowsList;
		while(ptr)
		{
			if(ptr == window)
			{
				if(!prev)
					WindowsList = window->Next;
				else
					prev->Next = window->Next;
				return;
			}
			prev = ptr;
			ptr = ptr->Next; // assume second var is 'Window* Next'
		}
	}




	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam,  LPARAM lParam);



	GLWindow::GLWindow()
	{
		memset(this, 0, sizeof(GLWindow)); // initialize instance variables to 0
		AddWindow(this);
	}

	GLWindow::~GLWindow()
	{
		this->Destroy();
	}


	bool GLWindow::SetResizable(bool pIsResizable)
	{
		if(this->IsFullscreen() == false) // only if not in fullscreen
		{
			Resizable = pIsResizable;
			DWORD dwStyle = pIsResizable ? WS_RESIZABLE : WS_NONRESIZABLE;
			SetWindowLong((HWND)Handle, GWL_STYLE, dwStyle|WS_VISIBLE);
			return true;
		}
		return false;
	}

	void GLWindow::OnResize(int newx, int newy, int oldx, int oldy)
	{
		return; // we don't really need to do anything here
	}

	bool GLWindow::SetTitle(const char* title)
	{
		SetWindowTextA((HWND)Handle, title);
		return true;
	}

	void GLWindow::SetWindowPos(int x, int y)
	{
		::SetWindowPos((HWND)Handle, 0, x, y, 0, 0, SWP_NOSIZE|SWP_NOSENDCHANGING|SWP_NOREDRAW);
	}

	void GLWindow::SetExclusive(bool value)
	{
		if(value == false && Exclusive)
			ReleaseCapture(); // immediatelly back out of exclusive mode when requested

		// exclusive mode is just a suggestion, so we can't really do anything when true
		Exclusive = value;
	}


	bool GLWindow::Create(const char* title, int width, int height, bool fullscreen, bool vsync, bool resizable)
	{
		static bool isRegistered = false; // incase we destroy & recreate the window
		//static HCURSOR cursor = 0;

		if(Handle)
			return false; // cannot create twice

		if(!isRegistered) // only register if not already registered
		{
			WNDCLASS wc = {0};
			wc.style		= CS_HREDRAW|CS_VREDRAW;	// redraw on move
			wc.lpfnWndProc	= &WndProc;					// WndProc handles messages
			wc.hInstance	= GetModuleHandle(NULL);	// set the proc. instance
			wc.hIcon		= LoadIcon(wc.hInstance, MAKEINTRESOURCE(101));	// Load default icon
			wc.hCursor		= LoadCursor(NULL, IDC_ARROW);	// Load the arrow pointer (important)
			wc.lpszClassName= "GL3Window";				// Set the class name
			//cursor = wc.hCursor;
			RegisterClassA(&wc);
			isRegistered = true;
		}

		DWORD dwExStyle = WS_EX_APPWINDOW; // show on taskbar
		DWORD dwStyle = fullscreen ? (WS_POPUP) : (resizable ? WS_RESIZABLE : WS_NONRESIZABLE);
		Resizable = resizable;

		RECT r = { 0, 0, width, height };
		AdjustWindowRectEx(&r, dwStyle, FALSE, dwExStyle); // calculate host rect based on desired client rect
		//Cursor = cursor;

		// Try to create the window
		if(!(Handle = (int)CreateWindowExA(dwExStyle, "GL3Window", title, dwStyle, 
			0, 0, r.right-r.left, r.bottom-r.top,	0, 0, GetModuleHandle(NULL), 0)))
		{
			MessageBox(0, "Failed to create Window", "ERROR", MB_ICONERROR);
			Destroy();
			return false; // Failed to create window
		}

		Device = new GLDevice();
		if(!Device->Create(this, width, height, fullscreen, vsync))
			return false; // failed :(

		Input::Add(this);

		ShowWindow((HWND)Handle, SW_SHOW);	// WinAPI Show the window
		SetForegroundWindow((HWND)Handle);	// slightly higher priority

		return true; // SUCCESS!
	}



	void GLWindow::Destroy() // properly destroy the window
	{
		if(Handle) // destroy the window
		{
			Input::Remove(this);
			delete Device;
			Device = nullptr;

			CloseWindow((HWND)Handle); // first lets send WM_CLOSE
			DestroyWindow((HWND)Handle); // destroy the WinAPI Window handle
			RemoveWindow(this);  // now we can delete all of this
			Handle = NULL; // reset the handle
		}
	}



	bool GLWindow::StartRendering()
	{
		MSG msg;
		LARGE_INTEGER freq, start, stop;
		double timeSlice;

		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&start);

		while(true)
		{
			// process all pending Windows events
			if(PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if(msg.message == WM_QUIT)
					return false; // by WM_QUIT

				{ // limit the scope of 'id' for optimization
					int id = msg.message;
					if(WM_KEYDOWN <= id && id <= WM_SYSKEYDOWN) // WM_CHAR and WM_DEADCHAR should never get here!
					{
						Input::TriggerKey(msg.wParam, MapVirtualKeyW(msg.wParam, MAPVK_VK_TO_CHAR), !(id & 0x001));
						goto NextFrame;
					}
				}
				DispatchMessageA(&msg);
			}
	NextFrame:
			// measure timeSlice (time since last frame):
			QueryPerformanceCounter(&stop);		// mark stop
			timeSlice = (stop.QuadPart - start.QuadPart) / (double)freq.QuadPart;
			QueryPerformanceCounter(&start);	// mark start for next frame

			if(FrameStart(timeSlice))	// call the FrameStart function
				return true; // TRUE
		}
	}



	// (private) The Window's Callback procedure
	LRESULT CALLBACK WndProc(HWND hWnd, // handle for this window
			UINT uMsg,					// message for this window
			WPARAM wParam,				// additional message information
			LPARAM lParam)				// additional message information
	{
		GLWindow* dxw = GetWindow((int)hWnd);
		switch(uMsg)
		{
			case WM_ACTIVATE:
				if(LOWORD(wParam))	// check focus state
				{
					dxw->Active = true;		// running
					//SetCursor(dxw->Cursor);	// reset the cursor
				}
				else
				{
					dxw->Active = false;	// paused
					//ShowCursor(TRUE);		// restore cursor
					Input::TriggerFocusLost(); // notify input about focus loss
				}
				break;
			case WM_SYSCOMMAND:
				switch(wParam) 
				{
					case SC_SCREENSAVE:		// screensaver trying to start?
					case SC_MONITORPOWER:	// monitor trying to enter powersave?
						return 0;	// prevent from happening!!
				}
				break;

			case WM_CLOSE: // window close message?
				PostQuitMessage(0); // send a quit message
				return 0;

			case WM_MOVE:
				dxw->WinX = LOWORD(lParam), dxw->WinY = HIWORD(lParam);
				return 0;
			case WM_SETFOCUS:
				if(dxw->Exclusive) // go into exclusive mode
					SetCapture((HWND)dxw->Handle);
				return 0;
			case WM_KILLFOCUS:
				if(dxw->Exclusive) // revert from exclusive mode
					ReleaseCapture();
				return 0;

			case WM_SIZE: // window size change msg
				switch(wParam)
				{
					case SIZE_RESTORED: 
					case SIZE_MAXIMIZED:
						dxw->Minimized = false;	return 0;	// ignore WM_SIZE
					case SIZE_MINIMIZED:
						dxw->Minimized = true;	return 0;	// ignore WM_SIZE (dont change display to w:0 h:0)
				}
				int oldw, oldh;
				oldw = dxw->WinW, oldh = dxw->WinH;

				// loword=width,hiword=height
				dxw->WinW = LOWORD(lParam), dxw->WinH = HIWORD(lParam); // always update real W and H

				// if Graphics is busy resizing, it means the graphics device is resizing
				// the window, in that case, we shouldn't invoke ResizeViewport
				if(dxw->Device->IsBusyResizing())
					return 0; // ignore WM_SIZE
				if(!dxw->Handle)
					return 0;

				// try to resize the device viewport
				// if viewport resize actually happened, call OnResize event
				if(dxw->Device->ResizeViewport(dxw->WinW, dxw->WinH))
					dxw->OnResize(dxw->WinW, dxw->WinH, oldw, oldh);
				return 0;


			//// ---- MOUSE ---- ////
			case WM_MOUSEWHEEL:
				Input::TriggerMouseMove(-1, -1, GET_WHEEL_DELTA_WPARAM(wParam));
				break;
			case WM_MOUSEMOVE:
				Input::TriggerMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 0);
				break;

			case WM_LBUTTONDBLCLK:	Input::TriggerMouseButton(MK_LBUTTON, false, true); break;
			case WM_MBUTTONDBLCLK:	Input::TriggerMouseButton(MK_MBUTTON, false, true); break;
			case WM_RBUTTONDBLCLK:	Input::TriggerMouseButton(MK_RBUTTON, false, true); break;
			case WM_XBUTTONDBLCLK:	Input::TriggerMouseButton(GET_XBUTTON_WPARAM(wParam), false, true); break;
		
			case WM_LBUTTONDOWN:	Input::TriggerMouseButton(MK_LBUTTON, true, false); break;
			case WM_MBUTTONDOWN:	Input::TriggerMouseButton(MK_MBUTTON, true, false); break;
			case WM_RBUTTONDOWN:	Input::TriggerMouseButton(MK_RBUTTON, true, false); break;
			case WM_XBUTTONDOWN:	Input::TriggerMouseButton(GET_XBUTTON_WPARAM(wParam), true, false); break;
			
			case WM_LBUTTONUP:	Input::TriggerMouseButton(MK_LBUTTON, false, false); break;
			case WM_MBUTTONUP:	Input::TriggerMouseButton(MK_MBUTTON, false, false); break;
			case WM_RBUTTONUP:	Input::TriggerMouseButton(MK_RBUTTON, false, false); break;
			case WM_XBUTTONUP:	Input::TriggerMouseButton(GET_XBUTTON_WPARAM(wParam), false, false); break;


			//// ---- KEYBOARD ---- ////
			case WM_KEYDOWN:
			case WM_KEYUP:
			case WM_CHAR:
			case WM_DEADCHAR:
			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
				return 0; // handled
		}
		// pass all unhandled messages to DefWindowProc
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}




} // namespace GL3
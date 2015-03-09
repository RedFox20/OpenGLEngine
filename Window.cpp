#include "Window.h"
//#include <stdio.h>
//#include <malloc.h>
//#include <stdarg.h>
#include <windowsx.h> // GET_X_LPARAM
#include <new> // placement new


	#define WS_RESIZABLE WS_OVERLAPPEDWINDOW
	#define WS_NONRESIZABLE WS_CAPTION|WS_MINIMIZEBOX|WS_POPUPWINDOW

	Window* WindowsList = nullptr;
	static Window* GetWindow(HWND handle)
	{
		Window* ptr = WindowsList;
		while (ptr && (ptr->Handle != handle)) ptr = ptr->Next;	
		return ptr; // nullptr OR our Window*
	}
	static void AddWindow(Window* window)
	{
		window->Next = WindowsList;
		WindowsList = window;
	}
	static void RemoveWindow(Window* window)
	{
		if (!window) return;
		Window* prev = 0;
		Window* ptr = WindowsList;
		while (ptr)
		{
			if (ptr == window)
			{
				if (!prev)
					WindowsList = window->Next;
				else
					prev->Next = window->Next;
				return;
			}
			prev = ptr;
			ptr  = ptr->Next; // assume second var is 'Window* Next'
		}
	}








	Window::Window()
	{
		memset(this, 0, sizeof(Window)); // initialize instance variables to 0
		AddWindow(this);
	}

	Window::~Window()
	{
		this->Destroy();
	}


	bool Window::SetResizable(bool pIsResizable)
	{
		if (this->IsFullscreen() == false) // only if not in fullscreen
		{
			Resizable = pIsResizable;
			DWORD dwStyle = pIsResizable ? WS_RESIZABLE : WS_NONRESIZABLE;
			SetWindowLong(Handle, GWL_STYLE, dwStyle|WS_VISIBLE);
			return true;
		}
		return false;
	}

	void Window::WindowResize(int newx, int newy, int oldx, int oldy)
	{
		return; // we don't really need to do anything here
	}

	bool Window::SetTitle(const char* title)
	{
		SetWindowTextA(Handle, title);
		return true;
	}

	void Window::SetWindowPos(int x, int y)
	{
		::SetWindowPos(Handle, 0, x, y, 0, 0, SWP_NOSIZE|SWP_NOSENDCHANGING|SWP_NOREDRAW);
	}

	void Window::SetExclusive(bool value)
	{
		if(value == false && Exclusive)
			ReleaseCapture(); // immediatelly back out of exclusive mode when requested

		// exclusive mode is just a suggestion, so we can't really do anything when true
		Exclusive = value;
	}


	bool Window::Create(const char* title, int width, int height, bool fullscreen, bool resizable)
	{
		static bool isRegistered = false; // incase we destroy & recreate the window
		static HCURSOR cursor = 0;

		if (Handle)
			return false; // cannot create twice

		if (!isRegistered) // only register if not already registered
		{
			WNDCLASS wc = { 0 };
			wc.style		= CS_HREDRAW|CS_VREDRAW; // redraw on move
			wc.lpfnWndProc	= &Window::WndProc;	// WndProc handles messages
			wc.hInstance	= GetModuleHandle(NULL);// set the proc. instance
			wc.hIcon		= LoadIcon(wc.hInstance, MAKEINTRESOURCE(101));	// Load default icon
			wc.hCursor		= LoadCursor(NULL, IDC_ARROW);	// Load the arrow pointer (important)
			wc.lpszClassName= "LegacyDX3D11";			// Set the class name
			cursor = wc.hCursor;
			RegisterClassA(&wc);
			isRegistered = true;
		}

		DWORD dwExStyle = WS_EX_APPWINDOW; // show on taskbar
		DWORD dwStyle   = fullscreen ? (WS_POPUP) : (resizable ? WS_RESIZABLE : WS_NONRESIZABLE);
		Resizable = resizable;

		RECT r = { 0, 0, width, height };
		AdjustWindowRectEx(&r, dwStyle, FALSE, dwExStyle); // calculate host rect based on desired client rect
		Cursor = cursor;

		// Try to create the window
		if (!(Handle = CreateWindowExA(dwExStyle, "LegacyDX3D11", title, dwStyle, 
			0, 0, r.right-r.left, r.bottom-r.top,	0, 0, GetModuleHandle(NULL), 0)))
		{
			MessageBoxA(0, "Failed to create Window", "ERROR", MB_ICONERROR);
			Destroy();
			return false; // Failed to create window
		}

		GfxDevice = (Graphics*)_aligned_malloc(sizeof(Graphics), 16);
		new (GfxDevice) Graphics();
		if (!GfxDevice->Create(Handle, width, height, fullscreen))
			return false; // failed :(

		Input::Add(this);

		ShowWindow(Handle, SW_SHOW);	// WinAPI Show the window
		SetForegroundWindow(Handle);	// slightly higher priority

		return true; // SUCCESS!
	}



	void Window::Destroy() // properly destroy the window
	{
		if (Handle) // destroy the window
		{
			Input::Remove(this);
			GfxDevice->Destroy();
			_aligned_free(GfxDevice);

			CloseWindow(Handle); // first lets send WM_CLOSE
			DestroyWindow(Handle); // destroy the WinAPI Window handle
			RemoveWindow(this);  // now we can delete all of this
			Handle = NULL; // reset the handle
		}
	}



	bool Window::StartMainLoop()
	{
		MSG msg;
		LARGE_INTEGER start, stop;
		double timeSlice;
		QueryPerformanceFrequency(&start);
		double freq = (double)start.QuadPart;
		QueryPerformanceCounter(&start);

		while (true)
		{
			// process all pending Windows events
			if (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
					return false; // by WM_QUIT

				{ // limit the scope of 'id' for optimization
					register int id = msg.message;
					if (WM_KEYDOWN <= id && id <= WM_SYSKEYUP) // WM_CHAR and WM_DEADCHAR should never get here!
					{
						// process key input events
						register int key = msg.wParam;
						Input::TriggerKey(key, MapVirtualKeyA(key, MAPVK_VK_TO_CHAR), !(id & 1));
						goto NextFrame;
					}
				}
				DispatchMessageA(&msg);
			}
	NextFrame:
			// measure timeSlice (time since last frame):
			QueryPerformanceCounter(&stop);		// mark stop
			timeSlice = double(stop.QuadPart - start.QuadPart) / freq;
			QueryPerformanceCounter(&start);	// mark start for next frame

			if (FrameStart(timeSlice))	// call the FrameStart function
				return true; // TRUE
		}
	}



	// (private) The Window's Callback procedure
	LRESULT CALLBACK Window::WndProc(HWND hWnd, // handle for this window
			UINT uMsg,					// message for this window
			WPARAM wParam,				// additional message information
			LPARAM lParam)				// additional message information
	{
		Window* dxw = GetWindow(hWnd);
		switch (uMsg)
		{
			case WM_ACTIVATE:
				if (LOWORD(wParam))	// check focus state
				{
					dxw->Active = true;		// running
					SetCursor(dxw->Cursor);	// reset the cursor
				}
				else
				{
					dxw->Active = false;	// paused
					ShowCursor(TRUE);		// restore cursor
					Input::TriggerFocusLost(); // notify input
				}
				break;
			case WM_SYSCOMMAND:
				switch (wParam) 
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
				if (!dxw)
					break; // no window handle
				dxw->WinX = LOWORD(lParam), dxw->WinY = HIWORD(lParam);
				return 0;
			case WM_SETFOCUS:
				if (dxw->Exclusive) // go into exclusive mode
					SetCapture(dxw->Handle);
				return 0;
			case WM_KILLFOCUS:
				if (dxw->Exclusive) // revert from exclusive mode
					ReleaseCapture();
				return 0;

			case WM_SIZE: // window size change msg
				if (!dxw)
					break; // no window handle
				switch (wParam)
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
				if (dxw->GfxDevice->IsBusyResizing())
					return 0; // ignore WM_SIZE
				if (dxw->Handle)
					if (dxw->GfxDevice->ResizeViewport(dxw->WinW, dxw->WinH))
						dxw->WindowResize(dxw->WinW, dxw->WinH, oldw, oldh);
				return 0;


			//// ---- MOUSE ---- ////
			case WM_MOUSEWHEEL:
				Input::TriggerMouseMove(mouseX(), mouseY(), GET_WHEEL_DELTA_WPARAM(wParam));
				break;
			case WM_MOUSEMOVE:
				Input::TriggerMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 0);
				break;

			case WM_LBUTTONDBLCLK:	Input::TriggerMouseButton(VK_LBUTTON, false, true); break;
			case WM_MBUTTONDBLCLK:	Input::TriggerMouseButton(VK_MBUTTON, false, true); break;
			case WM_RBUTTONDBLCLK:	Input::TriggerMouseButton(VK_RBUTTON, false, true); break;
			case WM_XBUTTONDBLCLK:	Input::TriggerMouseButton(VK_XBUTTON1 + HIWORD(wParam)-1, false, true); break;
		
			case WM_LBUTTONDOWN:	Input::TriggerMouseButton(VK_LBUTTON, true, false); break;
			case WM_MBUTTONDOWN:	Input::TriggerMouseButton(VK_MBUTTON, true, false); break;
			case WM_RBUTTONDOWN:	Input::TriggerMouseButton(VK_RBUTTON, true, false); break;
			case WM_XBUTTONDOWN:	Input::TriggerMouseButton(VK_XBUTTON1 + HIWORD(wParam)-1, true, false); break;
			
			case WM_LBUTTONUP:	Input::TriggerMouseButton(VK_LBUTTON, false, false); break;
			case WM_MBUTTONUP:	Input::TriggerMouseButton(VK_MBUTTON, false, false); break;
			case WM_RBUTTONUP:	Input::TriggerMouseButton(VK_RBUTTON, false, false); break;
			case WM_XBUTTONUP:	Input::TriggerMouseButton(VK_XBUTTON1 + HIWORD(wParam)-1, false, false); break;


			//// ---- KEYBOARD ---- ////
			case WM_KEYDOWN:
			case WM_KEYUP:
			case WM_CHAR:
			case WM_DEADCHAR:
			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
				#if _DEBUG
				printf("Window::WndProc KEYBOARD BUG\n");
				#endif
				return 0; // handled
		}
		// pass all unhandled messages to DefWindowProc
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}



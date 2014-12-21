/** 
 * Copyright (c) 2013 - Jorma Rebane 
 */


#include "GLDevice.h"
#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include "GL\glm\glm.hpp"
#include "GL\glm\gtx\transform.hpp" // perspective, lookAt

namespace GL3
{




	typedef struct DisplayInternals_
	{
		DisplayInternals_() 
		{
			memset(this, 0, sizeof(*this));
		}
		GLDisplayAdapter adapter;
		GLDisplayMode mode;
		DEVMODE devmode;

	} * DisplayInternals;


	// private forward declaration for our Adapter init
	static DisplayInternals InitBestAdapter();





	/**
	 * Initializes the core of the graphics device, but does not bind it
	 * to an existing window display and size.
	 */
	GLDevice::GLDevice()
	{
		memset(this, 0, sizeof(*this));
		ViewMatrix = 0;
		ViewPosition = 0;
	}
	GLDevice::~GLDevice()
	{
		Destroy();
	}




	/**
	 * Creates the OpenGL Device and sets the display mode and context.
	 * @param window Window
	 * @param width Width of the viewport (resolution).
	 * @param height Height of the viewport (resolution).
	 * @param fullscreen Should the window be created in fullscreen mode?
	 * @param vsync[false] Should there be Vertical Synchronization (framerate cap to display Hz).
	 */
	bool GLDevice::Create(GLWindow* window, int width, int height, bool fullscreen, bool vsync)
	{
		if(this->Window) Destroy(); // don't allow double create - just destroy previous state.

		DisplayInternals di;
		if(!Display)
		{
			Display = InitBestAdapter(); // get the best adapter on this PC
			di = DisplayInternals(Display);
			
			printf("Video Device:\t%s\n", di->adapter.AdapterName);
		}
		if(!Context)
		{
			Context = (int)CreateDC(NULL, di->adapter.AdapterName, NULL, &di->devmode);
			printf("Created GL3Device (on best adapter)\n");
		}

		// Set some basic params for this class
		Window = window;
		VSyncEnabled = vsync;
		if(!SetDisplayMode(width, height, fullscreen ? DISPLAY_MODE_FULLSCREEN : DISPLAY_MODE_WINDOWED))
			return false;

		Background = Vector4(0.05f, 0.05f, 0.05f, 1.0f); // dark gray background by default
		return true; // OK

	}
	

	/**
	 * Destroyes DirectX Device and Display mode context.
	 */
	void GLDevice::Destroy()
	{
		if(Fullscreen)
		{
			SetFullscreen(false);
			ShowCursor(TRUE); // enable cursor (incase someone did something with it)
			Fullscreen = false;
		}
		if(Context)
		{
			DeleteDC((HDC)Context);
			Context = 0;
		}
		if(Display)
		{
			delete DisplayInternals(Display);
		}
	}





	/**
	 * Clears the screen for rendering.
	 */
	void GLDevice::Clear()
	{
		++FrameNumber;

	}


	/**
	 * Swaps backbuffer to frontbuffer, so the new frame can be displayed.
	 */
	void GLDevice::Present()
	{

		SwapBuffers((HDC)Context);
	}





	/** @return Graphics resolution Width in pixels. */
	int GLDevice::Width() const { return DisplayInternals(Display)->mode.width; }
	/** @return Graphics resolution Height in pixels. */
	int GLDevice::Height() const{ return DisplayInternals(Display)->mode.height; }
	/** @return Graphics resolution Width:x Height:y in pixels. */
	Vector2 GLDevice::Size() const { return Vector2((float)Width(), (float)Height()); }
	/** @return Target viewport Width in pixels. */
	int GLDevice::ViewportWidth() const { return 0; }
	/** @return Target viewport Height in pixels. */
	int GLDevice::ViewportHeight() const { return 0; }
	/** @return Target viewport Width:x Height:y in pixels. */
	Vector2 GLDevice::ViewportSize() const { return Vector2(); }
	/** @return Desktop resolution Width:x Height:y in pixels. */
	Vector2 GLDevice::DesktopSize() { return Vector2((float)GetSystemMetrics(SM_CXSCREEN), (float)GetSystemMetrics(SM_CYSCREEN)); }
	/** @return TRUE if the window is fullscreen. */
	bool GLDevice::IsFullscreen() const { return Fullscreen; }
	/** @return TRUE if VSync is enabled. */
	bool GLDevice::IsVSyncEnabled() const { return VSyncEnabled; }
	/** @return TRUE if the Graphics device is busy changing its viewport or graphics resolution */
	bool GLDevice::IsBusyResizing() const { return BusyResizing; }


	/** Sets the color of the background */
	void GLDevice::BackgroundColor(const Vector4& background) { Background = background; }
	/** Gets the current background color */
	Vector4 GLDevice::BackgroundColor() const { return Background; }
	/** Enables or disables VSync */
	void GLDevice::SetVSync(bool enabled) { VSyncEnabled = enabled; }


	
	/** 
	 * Resizes the viewport.
	 * Viewport is the size of the 'area' where the Graphics output will be blitted.
	 * Viewport size can be different from the actual Graphics resolution, but the displayed image will be stretched.
	 * @return TRUE if viewport was resized, FALSE if it's already the requested size
	 */
	bool GLDevice::ResizeViewport(int width, int height)
	{
		if(ViewportW == width && ViewportH == height)
			return false; // nothing to do here
		ViewportW = width, ViewportH = height;
		PerspectiveMatrix = glm::perspective(45.0f, (float)width / (float)height, 0.1f, 4000.0f);
		OrthographicMatrix = glm::ortho(0.0f, (float)width, 0.0f, (float)height);
		return true;
	}








	///** Changes the current display mode to 'Fullscreen':true or 'Windowed':false. */
	//void GLDevice::SetFullscreen(bool isFullscreen)
	//{
	//	BusyResizing = true;
	//	if(Fullscreen = isFullscreen)
	//		SwapChain->ResizeTarget(&Mode); // change res when going to fullscreen


	//	// -- ResizeWindowToCenter
	//	RECT r = { 0, 0, Mode.Width, Mode.Height };
	//	AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);
	//	int w = r.right - r.left;
	//	int h = r.bottom - r.top;
	//	int x = GetSystemMetrics(SM_CXSCREEN)/2 - w/2;
	//	int y = GetSystemMetrics(SM_CYSCREEN)/2 - h/2 - 11;
	//	SetWindowPos((HWND)Window->Handle, 0, x, y, w, h, SWP_NOREDRAW|SWP_NOSENDCHANGING);

	//	// toggle input capture after screen mode change
	//	if(isFullscreen)
	//		SetCapture((HWND)Window->Handle);
	//	else
	//		ReleaseCapture();
	//	BusyResizing = false;
	//}

	void GLDevice::SetFullscreen(bool isFullscreen)
	{
		RECT r;
		if(Fullscreen != isFullscreen)
		{
			HWND hwnd = (HWND)Window->Handle;
			if(Fullscreen = isFullscreen)
			{
				SetWindowLongPtr(hwnd, GWL_STYLE, // set style to clean window
					WS_SYSMENU|WS_POPUP|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_VISIBLE);
				GetWindowRect(hwnd, &r); // get the window position
				Window->WinX = r.left;
				Window->WinY = r.top;
				SetWindowPos(hwnd, NULL, 0, 0, Width(), Height(), 0); // set fullscreen size

				DEVMODE dm = { 0 };			// device mode
				dm.dmSize = sizeof(dm);
				dm.dmPelsWidth = Width();	// screen width
				dm.dmPelsHeight = Height(); // screen height
				dm.dmBitsPerPel = 32;		// selected bits per pixel
				dm.dmFields = DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

				// CDS_FULLSCREEN gets rid of start bar
				if(ChangeDisplaySettings(&dm, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
				{
					Fullscreen = false; // use windowed mode
					MessageBox(NULL, "The requested Fullscreen mode is not supported", 
						"OpenGL", MB_OK|MB_ICONEXCLAMATION);
				}
			}
			else
			{
				r.left = 0; r.top = 0; // set desired size attributes WidthxHeight
				r.right = Width(); r.bottom = Height();

				// pick the correct style. Overlapped is Resizable
				DWORD dwStyle = Window->IsResizable() ? WS_OVERLAPPEDWINDOW : WS_CAPTION|WS_POPUPWINDOW;
				SetWindowLongPtr(hwnd, GWL_STYLE, dwStyle|WS_VISIBLE); // set style
				AdjustWindowRect(&r, dwStyle, FALSE); // adjust desired size to window style

				ChangeDisplaySettings(NULL, 0);		// Switch back to the desktop
				SetWindowPos(hwnd, (HWND)0, 
					Window->WinX, Window->WinY,	// set window pos
					r.right-r.left, r.bottom-r.top, 0);				// use adjusted size

			}
			SetFocus(hwnd); // refocus
		}
	}

	/** 
	 * Changes the display mode. Window & viewport sizes are altered to strictly support the specified resolution. 
	 * Fullscreen status is changed if needed. 
	 */
	bool GLDevice::SetDisplayMode(int width, int height, DISPLAY_MODE dpMode)
	{
		GLDisplayMode& Mode = DisplayInternals(Display)->mode;
		if(Mode.width == width && Mode.height == height)
			return true; // nothing to do here - success
		GLDisplayMode mode = {
			width, height, 0
		};
		GetClosestMatchingMode(&mode);	// get a suitable display mode
		if(mode.width != width || mode.height != height) // not what we wanted?
		{
			printf("Error! Invalid Graphics mode: %4dx%d\n", width, height);
			return false; // failed
		}
		BusyResizing = true;
		{
			BOOL fullscreen = dpMode == DISPLAY_MODE_AUTO ? Fullscreen : 
				(dpMode == DISPLAY_MODE_FULLSCREEN ? true : false);
			
			Mode = mode; // store the new mode

			printf("Graphics Mode:  %4dx%d\n", Mode.width, Mode.height);

			SetFullscreen(Fullscreen);
			ResizeViewport(Mode.width, Mode.height); // readjust the viewport
		}
		BusyResizing = false;
		return true;
	}







	/** (private) Returns IDXGIAdapter with the most dedicated video ram */
	static DisplayInternals InitBestAdapter()
	{
		DisplayInternals di = new DisplayInternals_();
		DISPLAY_DEVICE ddev;
		EnumDisplayDevices(NULL, 0, &ddev, 0);
		strcpy_s(di->adapter.AdapterName, ddev.DeviceName);
		strcpy_s(di->adapter.AdapterDescr, ddev.DeviceString);
	
		return di;
	}




}
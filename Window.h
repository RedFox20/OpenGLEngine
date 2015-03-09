#pragma once
#include "Graphics.h"
#include "Input.h"


	/**
	 * Base Window class
	 */
	class Window : public IKeyMouseListener
	{
	public:
		HWND Handle;			// our window handle
		Window* Next;			// just a way to keep track of our windows
	protected:
		int WinX, WinY;			// window X & Y pos
		int WinW, WinH;			// window width & height

		HCURSOR Cursor;			// current cursor

		bool Resizable;			// is the window resizable?
		bool Minimized;			// is the window Minimized?
		bool Active;			// is the window focused?
		bool Exclusive;			// should the window be in exclusive mode?

		bool Fullscreen;		// is the window fullscreen?
	
	public:

		Graphics* GfxDevice;

		/**
		 * Default-Initializes the Window
		 */
		Window();
		~Window();


		/**
		 * User implemented method, triggered when frame rendering should start
		 * @param timeSlice Time fraction elapsed since last FrameStart
		 * @return TRUE to stop rendering, FALSE to continue
		 */
		virtual BOOL FrameStart(double timeSlice) = 0;


		/**
		 * User implemented method, triggered when the window is resized
		 */
		virtual void WindowResize(int width, int height, int owidth, int oheight);

		/**
		 * @return Current x position of the window
		 */
		inline int X() const { return WinX; }
		/**
		 * @return Current y position of the window
		 */
		inline int Y() const { return WinY; }
		/**
		 * @return Current width of the window
		 */
		inline int W() const { return WinW; }
		/**
		 * @return Current height of the window
		 */
		inline int H() const { return WinH; }



		/**
		 * Starts the main loop and begins triggering FrameStart method with timeSlice info
		 * @return TRUE if exit from FrameStart, FALSE if WM_QUIT was requested
		 */
		bool StartMainLoop();


		/**
		 * Is the Window currently fullscreen?
		 * @return TRUE if the window is currently in Full Screen mode.
		 */
		inline bool IsFullscreen() const { return Fullscreen; }


		/**
		 * Sets the window resizable or non-resizable.
		 * The change will not be apparent in full screen
		 * @param resizable Will the window be resizable?
		 * @return TRUE if successful (windowed mode)
		 */
		bool SetResizable(bool resizable);


	
		/**
		 * Sets the title of the window
		 * @return TRUE if the window has been created
		 */
		bool SetTitle(const char* title);


		/**
		 * Creates the DirectX Window
		 * @param title Title for the new window
		 * @param width Width of the Window
		 * @param height Height of the Window
		 * @param fullscreen Create the window in fullscreen?
		 * @param resizable[false] Make the window border resizable?
		 * @return FALSE on failure
		 */
		bool Create(const char* title, int width, int height, bool fullscreen, bool resizable = false);

		/**
		 * Destroys the DirectX Window
		 */
		void Destroy();


		/**
		 * Checks the Window's Active flag.
		 * @return TRUE if the window is Active, FALSE if it has lost focus
		 */
		inline bool IsActive() const { return Active; }
		/**
		 * Checks the Window's Minimized flag.
		 * @return TRUE if the window is Minimized, FALSE if the window is in view
		 */
		inline bool IsMinimized() const { return Minimized; }
		/**
		 * Checks if the window has been created
		 * @return TRUE if the window has been create, FALSE if not
		 */
		inline bool IsCreated() const { return Handle ? true : false; }
		/** 
		 * Is this Window resizable?
		 * @return TRUE if the window is resizable
		 */
		inline bool IsResizable() const { return Resizable; }

		/**
		 * @return Is the Window in exclusive mode?
		 */
		inline bool IsExclusive() const { return Exclusive; }
		/**
		 * Should the window be in exclusive mode (in windowed state)?
		 * Exclusive mode captures input, which is useful for playing games in windowed mode.
		 */
		void SetExclusive(bool value);

		/**
		 * Sets the window position
		 */
		void SetWindowPos(int x, int y);

	private:

		static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam,  LPARAM lParam);

	};


/** 
 * Copyright (c) 2013 - Jorma Rebane 
 */
#pragma once
#ifndef GL_WINDOW_H
#define GL_WINDOW_H
#include "Input.h"
#include "GLDevice.h"

namespace GL3
{
	/**
	 * GLWindow
	 * (C) Jorma Rebane, 2013
	 */
	class GLWindow : public IKeyMouseListener
	{
	public:
		int Handle;				// our window handle
		GLWindow* Next;			// just a way to keep track of our windows
		
		short WinX, WinY;		// window X & Y pos
		short WinW, WinH;		// window width & height

		bool Resizable;			// is the window resizable?
		bool Minimized;			// is the window Minimized?
		bool Active;			// is the window focused?
		bool Exclusive;			// should the window be in exclusive mode? @note Check IsExclusive() for details.
	
		GLDevice* Device;


		/**
		 * Default-Initializes the Window
		 */
		GLWindow();
		~GLWindow();


		/**
		 * User implemented method, triggered when frame rendering should start
		 * @param timeSlice Time fraction elapsed since last FrameStart
		 * @return TRUE to stop rendering, FALSE to continue
		 */
		virtual bool FrameStart(double timeSlice) = 0;


		/**
		 * User implemented method, triggered when the window is resized
		 * @param width New width of the window
		 * @param height New height of the window
		 * @param owidth Old width of the window
		 * @param oheight Old height of the window
		 */
		virtual void OnResize(int width, int height, int owidth, int oheight);

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
		 * Starts the main rendering loop and begins triggering FrameStart method
		 * @return TRUE if exit from FrameStart, FALSE if WM_QUIT was requested
		 */
		bool StartRendering();

		/**
		 * Is the Window currently fullscreen?
		 * @return TRUE if the window is currently in Full Screen mode.
		 */
		inline bool IsFullscreen() const { return Device->IsFullscreen(); }


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
		 * @param width Width of the Window Client Area (the viewport width)
		 * @param height Height of the Window Client Area (the viewport height)
		 * @param fullscreen Create the window in fullscreen?
		 * @return FALSE on failure
		 */
		bool Create(const char* title, int width, int height, bool fullscreen, bool vsync = false, bool resizable = false);

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
		 * @note Exclusive mode locks input to this window only, which is useful for playing games in windowed mode.
		 * @return Is the Window in exclusive mode?
		 */
		inline bool IsExclusive() const { return Exclusive; }
		/**
		 * Should the window be in exclusive mode (in windowed state)?
		 * @note Exclusive mode locks input to this window only, which is useful for playing games in windowed mode.
		 */
		void SetExclusive(bool value);

		/**
		 * Sets the window position
		 */
		void SetWindowPos(int x, int y);

	};

} // namespace GL3

#endif // GL_WINDOW_H
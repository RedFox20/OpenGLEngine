/** 
 * Copyright (c) 2013 - Jorma Rebane 
 */
#pragma once
#ifndef GL_DEVICE_H
#define GL_DEVICE_H

#include "Basetypes.h"
#include "GLWindow.h"

#include "GL\glew.h"
namespace GL3
{

	enum DISPLAY_MODE
	{
		DISPLAY_MODE_AUTO,		// auto-decides display mode
		DISPLAY_MODE_FULLSCREEN,// always switches to fullscreen
		DISPLAY_MODE_WINDOWED,	// always switches to windowed
	};

	enum FILL_MODE
	{
		
	};

	struct GLDisplayAdapter
	{
		char AdapterName[32];
		char AdapterDescr[32];
		ushort ModeCount;
		uint AdapterMemory;
	};

	struct GLDisplayMode
	{
		ushort width; // width of the display
		ushort height; // 
		ushort rate;
	};

	/**
	 * GraphicsDevice class contains all the required methods and interfaces to
	 * control the graphics card, its display mode(s) and display graphics
	 * on the screen.
	 */
	class GLDevice
	{
	public:
		/**
		 * Initializes the core of the graphics device, but does not bind it
		 * to an existing window display and size.
		 */
		GLDevice();
		~GLDevice();

		/**
		 * Creates the DirectX Device and sets the display mode and context.
		 * @param window Handle to the window to render into.
		 * @param width Width of the viewport (resolution).
		 * @param height Height of the viewport (resolution).
		 * @param fullscreen Should the window be created in fullscreen mode?
		 * @param vsync[false] Should there be Vertical Synchronization (framerate cap to display Hz).
		 */
		bool Create(GLWindow* window, int width, int height, bool fullscreen, bool vsync = false);
		
		/**
		 * Destroyes DirectX Device and Display mode context.
		 */
		void Destroy();

		/**
		 * Clears the screen for rendering.
		 */
		void Clear();

		/**
		 * Swaps backbuffer to frontbuffer, so the new frame can be displayed.
		 */
		void Present();

		/** @return Graphics resolution Width in pixels. */
		int Width() const;
		/** @return Graphics resolution Height in pixels. */
		int Height() const;
		/** @return Graphics resolution Width:x Height:y in pixels. */
		Vector2 Size() const;
		/** @return Target viewport Width in pixels. */
		int ViewportWidth() const;
		/** @return Target viewport Height in pixels. */
		int ViewportHeight() const;
		/** @return Target viewport Width:x Height:y in pixels. */
		Vector2 ViewportSize() const;
		/** @return Desktop resolution Width:x Height:y in pixels. */
		static Vector2 DesktopSize();
		/** @return TRUE if the window is fullscreen. */
		bool IsFullscreen() const;
		/** @return TRUE if VSync is enabled. */
		bool IsVSyncEnabled() const;
		/** @return TRUE if the Graphics device is busy changing its viewport or graphics resolution */
		bool IsBusyResizing() const;
		

		/** Sets the color of the background */
		void BackgroundColor(const Vector4& background);
		/** Gets the current background color */
		Vector4 BackgroundColor() const;
		/** Enables or disables VSync */
		void SetVSync(bool enabled);



		/** 
		 * Resizes the viewport.
		 * Viewport is the size of the 'area' where the Graphics output will be blitted.
		 * Viewport size can be different from the actual Graphics resolution, but the displayed image will be stretched.
		 * @return TRUE if viewport was resized, FALSE if it's already the requested size
		 */
		bool ResizeViewport(int width, int height);

		/** Changes the current display mode to 'Fullscreen':true or 'Windowed':false. */
		void SetFullscreen(bool isFullscreen);

	public:
		/** 
		 * Changes the display mode. Window & viewport sizes are altered to strictly support the specified resolution. 
		 * Fullscreen status is changed if needed. 
		 */
		bool SetDisplayMode(int width, int height, DISPLAY_MODE dpMode = DISPLAY_MODE_AUTO);


		///** Sets the rasterizer FILL_MODE and CULL_MODE */
		//void SetRasterizerMode(D3D10_FILL_MODE fill, D3D10_CULL_MODE cull);
		///** @return Current FILL_MODE */
		//D3D10_FILL_MODE FillMode() const;
		///** @return Current CULL_MODE */
		//D3D10_CULL_MODE CullMode() const;



	private:
		// DepthStencilState allows us to control what type of depth test Direct3D will do for each pixel
		bool _SetDepthStencil(int width, int height);

	public:
		/**
		 * Polls the driver for the max MSAA count. Valid 
		 * MSAA Count values are: 1 2 4 8 16 32, where 1 means NOMSAA
		 * @return Maximum supported MSAA value
		 */
		int MaxMSAACount() const;
		/**
		 * Sets the multisample (MSAA) value. Default (no sampling) is 1.
		 * Valid values are { 1 2 4 8 16 32 } <= MaxMSAACount
		 */
		bool SetMSAACount(int count);
		/** @return The current MSAA Count. */
		int MSAACount() const;



	public:
		/**
		 * @return The number of available display modes.
		 */
		static int GetModeCount();
		/**
		 * Copies all supported modes to [outModes].
		 * Use GetModeCount() to determine the size of [outModes].
		 */
		static void GetModes(GLDisplayMode* outModes);
		/**
		 * Enumerates the closest matching mode provided.
		 * @param inoutMode Mode description acts as input parameter and out result.
		 */
		static void GetClosestMatchingMode(GLDisplayMode* inoutMode);


		/**
		 * R
		 */
		static int GetAdapterCount();


		Matrix4 PerspectiveMatrix;
		Matrix4 OrthographicMatrix;
		Vector4 Background;		// Current color of the background
		Matrix4* ViewMatrix;			// Current view matrix
		Vector3* ViewPosition;			// Current view position
		uint FrameNumber;				// number of the current frame

	private:

		
		// Misc. stuff
		GLWindow* Window;
		int Context;				// device context handle
		void* Display;				// internal structure for DisplayMode and Adapter
		int ViewportW;
		int ViewportH;
		bool VSyncEnabled;
		bool Fullscreen;
		bool BusyResizing; // is the graphics device busy resizing its viewport or graphics resolution?
		bool __reserved__;

	};

} // namespace Dx

#endif // GL_DEVICE_H
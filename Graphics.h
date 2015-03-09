#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include "Vector234.h"

enum DISPLAY_MODE
{
	DISPLAY_MODE_AUTO,		// auto-decides display mode (retain current)
	DISPLAY_MODE_FULLSCREEN,// always switches to fullscreen
	DISPLAY_MODE_WINDOWED,	// always switches to windowed
};



class Graphics
{
public:

		/**
		 * Initializes the core of the graphics device, but does not bind it
		 * to an existing window display and size.
		 */
		Graphics();
		~Graphics();

		/**
		 * Creates the Graphics Device and sets the display mode and context.
		 * @param window Handle to the window to render into.
		 * @param width Width of the viewport (resolution).
		 * @param height Height of the viewport (resolution).
		 * @param fullscreen Should the window be created in fullscreen mode?
		 */
		bool Create(HWND window, int width, int height, bool fullscreen);
		
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
		/** @return TRUE if the Graphics device is busy changing its viewport or graphics resolution */
		bool IsBusyResizing() const;
		

		/** Sets the color of the background */
		void BackgroundColor(const Vector4& background);
		/** Gets the current background color */
		Vector4 BackgroundColor() const;

		/** @return Graphics Device's factory description string */
		static const char* DeviceDescription();
		/** @return Total MB-s of graphics device memory */
		static int DeviceMemory();



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



		/**
		 * Returns the number of available display modes.
		 * @note DxGraphics must be created.
		 */
		static int GetModeCount();
		/**
		 * Copies all supported modes to [outModes].
		 * Use GetModeCount() to determine the size of [outModes].
		 */
		static void GetModes(void* outModes);
		/**
		 * Enumerates the closest matching mode provided.
		 * @param inoutMode Mode description acts as input parameter and out result.
		 */
		static void GetClosestMatchingMode(void* inoutMode);
};


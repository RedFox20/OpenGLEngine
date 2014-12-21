/**
 * Copyright (c) 2013 - Jorma Rebane
 */
#pragma once
#ifndef INPUT_H
#define INPUT_H

	enum MouseButton {
		MBLEFT,
		MBMIDDLE,
		MBRIGHT,
		MBXBUTTON,
	};


	enum InputKey {
		KEY_F1 = 0x70, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,
		KEY_PAGEUP = 0x21, KEY_PAGEDOWN, KEY_END, KEY_HOME,
		KEY_LEFT = 0x25, KEY_UP, KEY_RIGHT, KEY_DOWN, 
		KEY_SNAPSHOT = 0x2C, KEY_INSERT, KEY_DELETE,
		
		KEY_SHIFT = 0x10, KEY_CTRL, KEY_ALT,
		KEY_BACKSPACE = 8,
		KEY_ENTER = 10,
		KEY_ESCAPE = 27,

	};




	typedef void (*KeyChangeFunc)(int key, wchar_t keyChar, bool down, bool repeat);
	typedef void (*MouseMoveFunc)(int relX, int relY, int relZ);
	typedef void (*MouseButtonFunc)(int button, bool down, bool doubleClick);




	/**
	 * A virtual KeyListener interface.
	 * Derive your class from IKeyListener if you want to register it as an input listener object.
	 * @note A KeyListener interface only receives Key events
	 * @example class MyClass : public IKeyListener;
	 * @example Input::Add( &myClass );
	 */
	struct IKeyListener
	{
		/**
		 * Triggered on a key change event
		 * @param key Virtual keycode of the key
		 * @param keyChar Character code of the pressed key
		 * @param down TRUE if the key is now down (pressed), FALSE if the key is up
		 * @param repeat TRUE if this is a key-repeat event
		 */
		virtual void OnKeyChange(int key, wchar_t keyChar, bool down, bool repeat)
		{
			return; // full impl. not required
		}
	};




	/**
	 * A virtual MouseListener interface.
	 * Derive your class from IMouseListener if you want to register it as an input listener object.
	 * @note A MouseListener interface only receives Mouse events
	 * @example class MyClass : public IMouseListener;
	 * @example Input::Add( &myClass );
	 */
	struct IMouseListener
	{
		/**
		 * Triggered when the mouse moves
		 * @param relX Relative x pixels moved from previous OnMouseMove
		 * @param relY Relative y pixels moved from previous OnMouseMove
		 * @param relZ Relative z scroll value since previous OnMouseMove
		 */
		virtual void OnMouseMove(int relX, int relY, int relZ) 
		{
			return; // full impl. not required
		}
		/**
		 * Triggered when a mouse button is pressed
		 * @param button Mouse button that was just pressed
		 * @param down TRUE if the button is down (pressed), FALSE if the button is up
		 * @param doubleClick TRUE if this button event is considered a double-click
		 */
		virtual void OnMouseButton(int button, bool down, bool doubleClick)
		{
			return; // full impl. not required
		}
	};




	/**
	 * A virtual KeyMouseListener interface.
	 * Derive your class from IKeyMouseListener if you want to register it as an input listener object.
	 * @note A KeyMouseListener interface receives both Key and Mouse events
	 * @example class MyClass : public IKeyMouseListener;
	 * @example Input::Add( &myClass );
	 */
	struct IKeyMouseListener : public IKeyListener, IMouseListener
	{
	};




	/* @return Current mouse X pixel position on the screen */
	int mouseX();
	/* @return Current mouse Y pixel position on the screen */
	int mouseY();
	/* @return Current mouse Z scroll accumulation */
	int mouseZ();
	/* @return Relative mouse X pixel position change */
	int relX();
	/* @return Relative mouse Y pixel position change */
	int relY();
	/* @return Relative mouse scroll change [-1..+1] */
	int relZ();
	/* @return Is the specified Key down? */
	bool isKeyDown(unsigned char key);
	/* @return Has the specified Key changed? */
	bool isKeyChanged(unsigned char key);
	/* @return Is the specified Virtual Mouse Key down? */
	bool isMouseDown(MouseButton button);



/**
 * The Input (manager) class is used to Add/Remove input event listeners
 * This class also facilitates a control mechanism for triggering input.
 * @example Register a new listener object: Add(&keyListenerObject);
 * @example Register a new listener func: AddKeyChange(&onKeyChangeFunc);
 * @example Trigger a new input event: TriggerKey(10, L'\n', true);
 */
struct Input
{
	static void BindGLUT();

	/**
	 * Adds a new key event listener
	 */
	static void Add(IKeyListener* listener);

	/**
	 * Adds a new mouse event listener
	 */
	static void Add(IMouseListener* listener);

	/**
	 * Adds a new key and mouse event listener
	 */
	static void Add(IKeyMouseListener* listener);
	
	/**
	 * Removes an existing key event listener
	 */
	static void Remove(IKeyListener* listener);

	/**
	 * Removes an existing mouse event listener
	 */
	static void Remove(IMouseListener* listener);
	
	/**
	 * Removes an existing key and mouse event listener
	 */
	static void Remove(IKeyMouseListener* listener);



	/**
	 * Adds a Key event callback func
	 */
	static void AddKeyChange(KeyChangeFunc key);

	/**
	 * Adds a mouse move event callback func
	 */
	static void AddMouseMove(MouseMoveFunc mouse);

	/**
	 * Adds a mouse button event callback func
	 */
	static void AddMouseButton(MouseButtonFunc mouse);

	/**
	 * Removes an existing key event callback func
	 */
	static void Remove(KeyChangeFunc key);

	/**
	 * Removes an existing mouse move event callback func
	 */
	static void Remove(MouseMoveFunc mouse);

	/**
	 * Removes an existing mouse button event callback func
	 */
	static void Remove(MouseButtonFunc mouse);



	/**
	 * Triggers a new key event
	 * @param key Virtual keycode of the key to trigger
	 * @param keyChar Character code of the key
	 * @param down TRUE if the key is in down state
	 */
	static void TriggerKey(int key, wchar_t keyChar, bool down);

	/**
	 * Triggers a new mouse movement event
	 * @param newX New X position of the mouse
	 * @param newY New Y position of the mouse
	 * @param relZ Relative scroll change
	 */
	static void TriggerMouseMove(int newX, int newY, int relZ);

	/**
	 * Triggers a new mouse button event
	 * @param button Virtual keycode for mouse to trigger
	 * @param down TRUE if the mouse button is in down state
	 * @param doubleClick TRUE if this event should be considered a 'double-click' event instead
	 */
	static void TriggerMouseButton(int button, bool down, bool doubleClick);
	
	/**
	 * Notifies the Input manager that input capture was temporarily lost.
	 * This in turn releases any keys or buttons held, triggering their 'up' evenets.
	 */
	static void TriggerFocusLost();

	/**
	 * Resets the 'global' scroll state. This will set mouseZ() back to 0.
	 */
	static void ResetScroll();
};

#endif // INPUT_H
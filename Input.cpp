/**
 * Copyright (c) 2013 - Jorma Rebane
 */
#include "Input.h"
#include <vector>
using namespace std;
#include "GL\glut.h"

	//// ---- input management - very simple, favors speed over memory ---- ////

	static bool keysdown[256] = { 0 }; // all keys up
	static bool keyschanged[256] = { 0 }; // no changes
	static bool buttons[4] = { 0 }; // lmb:0 mmb:1 rmb:2
	static int mousex = 0, mousey = 0, mousez;
	static int relx = 0, rely = 0, relz;



	//// ---- GLUT Specific Bindings ---- ////

	void update_modifiers()
	{
		int m = glutGetModifiers();
		keysdown[KEY_SHIFT] = m & GLUT_ACTIVE_SHIFT ? true : false;
		keysdown[KEY_CTRL]  = m & GLUT_ACTIVE_CTRL  ? true : false;
		keysdown[KEY_ALT]   = m & GLUT_ACTIVE_CTRL  ? true : false;
	}

	void keyboard_down(unsigned char ch, int, int) 
	{ 
		Input::TriggerKey(ch, ch, true);
	}

	void keyboard_up(unsigned char ch, int, int) 
	{ 
		Input::TriggerKey(ch, ch, false);
	}

	unsigned char get_special_key(int ch)
	{
		if (ch <= GLUT_KEY_F12)
			return KEY_F1 + (ch - 0x01);
		return KEY_LEFT + (ch - 0x64);
	}

	void special_up(int ch, int, int)
	{
		update_modifiers();
		keyboard_up(get_special_key(ch), 0, 0);
	}

	void special_down(int ch, int, int)
	{
		update_modifiers();
		keyboard_down(get_special_key(ch), 0, 0);
	}

	void mouse_button(int button, int state, int x, int y) 
	{ 
		if (button <= 2) // LMB, MMB, RMB
		{
			Input::TriggerMouseButton(button, state == GLUT_DOWN, false);
		}
		else if (button <= 4) // scrlup, scrldn
		{
			// each wheel event reports like a button click, GLUT_DOWN then GLUT_UP
			if(state == GLUT_UP) return; // disregard redundant event
			Input::TriggerMouseMove(x, y, button == 3 ? +1 : -1);
		}
		// ignore others
	}

	void mouse_move(int x, int y) 
	{
		Input::TriggerMouseMove(x, y, 0);
	}

	void mouse_focus(int state)
	{
		if(state == GLUT_LEFT)
			Input::TriggerFocusLost();
	}



	//// ---- GLOBAL GETTERS ---- ////

	int mouseX() { return mousex; }
	int mouseY() { return mousey; }
	int mouseZ() { return mousez; }
	int relX() { return relx; }
	int relY() { return rely; }
	int relZ() { return relz; }
	bool isKeyDown(unsigned char key)    { return keysdown[key];    }
	bool isKeyChanged(unsigned char key) { return keyschanged[key]; }
	bool isMouseDown(MouseButton button) { return buttons[button];  }




	//// ---- Listener Implementations ---- /////

	void Input::BindGLUT()
	{
		glutKeyboardFunc(keyboard_down);
		glutKeyboardUpFunc(keyboard_up);
		glutSpecialFunc(special_down);
		glutSpecialUpFunc(special_up);

		glutMouseFunc(mouse_button);
		glutPassiveMotionFunc(mouse_move);
		glutEntryFunc(mouse_focus);
	}


	static vector<IKeyListener*>   keyListeners;
	static vector<IMouseListener*> mouseListeners;
	static vector<KeyChangeFunc>   keyChangeListeners;
	static vector<MouseMoveFunc>   mouseMoveListeners;
	static vector<MouseButtonFunc> mouseButtonListeners;

	template<class T> inline void remove_first(const T& item, std::vector<T>& vec)
	{
		auto end = vec.end();
		for (auto it = vec.begin(); it != end; ++it)
			if (*it == item) {
				vec.erase(it);
				break;
			}
	}


	void Input::Add(IKeyListener* listener)
	{
		if (listener) keyListeners.push_back(listener);
	}
	void Input::Add(IMouseListener* listener)
	{
		if (listener) mouseListeners.push_back(listener);
	}
	void Input::Add(IKeyMouseListener* listener)
	{
		if (listener) keyListeners.push_back(listener), mouseListeners.push_back(listener);
	}
	void Input::Remove(IKeyListener* listener)
	{
		remove_first(listener, keyListeners);
	}
	void Input::Remove(IMouseListener* listener)
	{
		remove_first(listener, mouseListeners);
	}
	void Input::Remove(IKeyMouseListener* listener)
	{
		Remove((IKeyListener*)listener), Remove((IMouseListener*)listener);
	}


	void Input::AddKeyChange(KeyChangeFunc key)
	{
		if (key) keyChangeListeners.push_back(key);
	}
	void Input::AddMouseMove(MouseMoveFunc mouse)
	{
		if (mouse) mouseMoveListeners.push_back(mouse);
	}
	void Input::AddMouseButton(MouseButtonFunc mouse)
	{
		if (mouse) mouseButtonListeners.push_back(mouse);
	}
	void Input::Remove(KeyChangeFunc key)
	{
		remove_first(key, keyChangeListeners);
	}
	void Input::Remove(MouseMoveFunc mouse)
	{
		remove_first(mouse, mouseMoveListeners);
	}
	void Input::Remove(MouseButtonFunc mouse)
	{
		remove_first(mouse, mouseButtonListeners);
	}



	//// ---- Triggger Implementations ---- ////


	void Input::TriggerKey(int key, wchar_t keyChar, bool down)
	{
		bool repeat;
		if (keysdown[key] != down) // changed?
		{
			keyschanged[key] = true; // set changed
			keysdown[key] = down; // set key
			repeat = false; // no repeat
		}
		else // not changed
		{
			keyschanged[key] = false; // key not changed
			repeat = true; // repeating key
		}
		for (auto f : keyListeners) f->OnKeyChange(key, keyChar, down, repeat);
		for (auto f : keyChangeListeners) f(key, keyChar, down, repeat);
	}
	
	
	void Input::TriggerMouseMove(int newX, int newY, int relZ)
	{
		if (newX == -1)
			relx = 0;
		else
			relx = newX - mousex, 
			mousex = newX;
		
		if (newY == -1)
			rely = 0;
		else
			rely = newY - mousey, 
			mousey = newY;
		
		if (relZ)
			relz = (relZ > 0 ? 1 : -1), 
			mousez += relz;
		
		if (!relx && !rely && !relz) // no changes?
			return; // don't trigger if no changes

		for (auto f : mouseListeners) f->OnMouseMove(relx, rely, relz);
		for (auto f : mouseMoveListeners) f(relx, rely, relz);
	}
	
	
	void Input::TriggerMouseButton(int button, bool down, bool doubleClick)
	{
		buttons[button] = down;
		for (auto f : mouseListeners) f->OnMouseButton(button, down, doubleClick);
		for (auto f : mouseButtonListeners) f(button, down, doubleClick);
	}
	
	
	void Input::TriggerFocusLost()
	{
		// trigger mouseup for all buttons that are down
		for (int i = 0; i < sizeof(buttons); i++)
			if(buttons[i]) Input::TriggerMouseButton(i, false, false);
		// trigger keyup for all keys that are down
		for (int i = 0; i < sizeof(keysdown); i++)
			if(keysdown[i]) Input::TriggerKey(i, (wchar_t)i, false);
	}


	void Input::ResetScroll()
	{
		mousez = 0;
		relz = 0;
	}
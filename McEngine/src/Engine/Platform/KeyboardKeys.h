//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		vk defs
//
// $NoKeywords: $vk $os
//===============================================================================//

#ifndef KEYBOARDKEYS_H
#define KEYBOARDKEYS_H

#if defined MCENGINE_FEATURE_SDL

#include "SDL.h"

// alphabet
#define KEY_A				SDL_SCANCODE_A
#define KEY_B				SDL_SCANCODE_B
#define KEY_C				SDL_SCANCODE_C
#define KEY_D				SDL_SCANCODE_D
#define KEY_E				SDL_SCANCODE_E
#define KEY_F				SDL_SCANCODE_F
#define KEY_G				SDL_SCANCODE_G
#define KEY_H				SDL_SCANCODE_H
#define KEY_I				SDL_SCANCODE_I
#define KEY_J				SDL_SCANCODE_J
#define KEY_K				SDL_SCANCODE_K
#define KEY_L				SDL_SCANCODE_L
#define KEY_M				SDL_SCANCODE_M
#define KEY_N				SDL_SCANCODE_N
#define KEY_O				SDL_SCANCODE_O
#define KEY_P				SDL_SCANCODE_P
#define KEY_Q				SDL_SCANCODE_Q
#define KEY_R				SDL_SCANCODE_R
#define KEY_S				SDL_SCANCODE_S
#define KEY_T				SDL_SCANCODE_T
#define KEY_U				SDL_SCANCODE_U
#define KEY_V				SDL_SCANCODE_V
#define KEY_W				SDL_SCANCODE_W
#define KEY_X				SDL_SCANCODE_X
#define KEY_Y				SDL_SCANCODE_Y
#define KEY_Z				SDL_SCANCODE_Z

// numbers
#define KEY_0				SDL_SCANCODE_0
#define KEY_1				SDL_SCANCODE_1
#define KEY_2				SDL_SCANCODE_2
#define KEY_3				SDL_SCANCODE_3
#define KEY_4				SDL_SCANCODE_4
#define KEY_5				SDL_SCANCODE_5
#define KEY_6				SDL_SCANCODE_6
#define KEY_7				SDL_SCANCODE_7
#define KEY_8				SDL_SCANCODE_8
#define KEY_9				SDL_SCANCODE_9

// numpad
#define KEY_NUMPAD0			SDL_SCANCODE_KP_0
#define KEY_NUMPAD1			SDL_SCANCODE_KP_1
#define KEY_NUMPAD2			SDL_SCANCODE_KP_2
#define KEY_NUMPAD3			SDL_SCANCODE_KP_3
#define KEY_NUMPAD4			SDL_SCANCODE_KP_4
#define KEY_NUMPAD5			SDL_SCANCODE_KP_5
#define KEY_NUMPAD6			SDL_SCANCODE_KP_6
#define KEY_NUMPAD7			SDL_SCANCODE_KP_7
#define KEY_NUMPAD8			SDL_SCANCODE_KP_8
#define KEY_NUMPAD9			SDL_SCANCODE_KP_9
#define KEY_MULTIPLY		SDL_SCANCODE_KP_MULTIPLY
#define KEY_ADD				SDL_SCANCODE_KP_PLUS
#define KEY_SEPARATOR		SDL_SCANCODE_KP_EQUALS
#define KEY_SUBTRACT		SDL_SCANCODE_KP_MINUS
#define KEY_DECIMAL			SDL_SCANCODE_KP_DECIMAL
#define KEY_DIVIDE			SDL_SCANCODE_KP_DIVIDE

// function keys
#define KEY_F1				SDL_SCANCODE_F1
#define KEY_F2				SDL_SCANCODE_F2
#define KEY_F3				SDL_SCANCODE_F3
#define KEY_F4				SDL_SCANCODE_F4
#define KEY_F5				SDL_SCANCODE_F5
#define KEY_F6				SDL_SCANCODE_F6
#define KEY_F7				SDL_SCANCODE_F7
#define KEY_F8				SDL_SCANCODE_F8
#define KEY_F9				SDL_SCANCODE_F9
#define KEY_F10				SDL_SCANCODE_F10
#define KEY_F11				SDL_SCANCODE_F11
#define KEY_F12				SDL_SCANCODE_F12

// arrow keys
#define KEY_LEFT			SDL_SCANCODE_LEFT
#define KEY_RIGHT			SDL_SCANCODE_RIGHT
#define KEY_UP				SDL_SCANCODE_UP
#define KEY_DOWN			SDL_SCANCODE_DOWN

// special keys
#define KEY_TAB				SDL_SCANCODE_TAB
#define KEY_RETURN			SDL_SCANCODE_RETURN
#define KEY_ENTER			SDL_SCANCODE_RETURN
#define KEY_SHIFT			SDL_SCANCODE_LSHIFT
#define KEY_CONTROL			SDL_SCANCODE_LCTRL
#define KEY_ALT				SDL_SCANCODE_LALT
#define KEY_ESCAPE			SDL_SCANCODE_ESCAPE
#define KEY_SPACE			SDL_SCANCODE_SPACE
#define KEY_BACKSPACE		SDL_SCANCODE_BACKSPACE
#define KEY_END				SDL_SCANCODE_END
#define KEY_INSERT			SDL_SCANCODE_INSERT
#define KEY_DELETE			SDL_SCANCODE_DELETE
#define KEY_HELP			SDL_SCANCODE_HELP
#define KEY_HOME			SDL_SCANCODE_HOME
#define KEY_SUPER			SDL_SCANCODE_LGUI

#elif defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

#include <windows.h>

// ASCII letters

// alphabet
#define KEY_A				0x41
#define KEY_B				0x42
#define KEY_C				0x43
#define KEY_D				0x44
#define KEY_E				0x45
#define KEY_F				0x46
#define KEY_G				0x47
#define KEY_H				0x48
#define KEY_I				0x49
#define KEY_J				0x4A
#define KEY_K				0x4B
#define KEY_L				0x4C
#define KEY_M				0x4D
#define KEY_N				0x4E
#define KEY_O				0x4F
#define KEY_P				0x50
#define KEY_Q				0x51
#define KEY_R				0x52
#define KEY_S				0x53
#define KEY_T				0x54
#define KEY_U				0x55
#define KEY_V				0x56
#define KEY_W				0x57
#define KEY_X				0x58
#define KEY_Y				0x59
#define KEY_Z				0x5A

// numbers
#define KEY_0				0x30
#define KEY_1				0x31
#define KEY_2				0x32
#define KEY_3				0x33
#define KEY_4				0x34
#define KEY_5				0x35
#define KEY_6				0x36
#define KEY_7				0x37
#define KEY_8				0x38
#define KEY_9				0x39

// numpad
#define KEY_NUMPAD0			VK_NUMPAD0
#define KEY_NUMPAD1			VK_NUMPAD1
#define KEY_NUMPAD2			VK_NUMPAD2
#define KEY_NUMPAD3			VK_NUMPAD3
#define KEY_NUMPAD4			VK_NUMPAD4
#define KEY_NUMPAD5			VK_NUMPAD5
#define KEY_NUMPAD6			VK_NUMPAD6
#define KEY_NUMPAD7			VK_NUMPAD7
#define KEY_NUMPAD8			VK_NUMPAD8
#define KEY_NUMPAD9			VK_NUMPAD9
#define KEY_MULTIPLY		VK_MULTIPLY
#define KEY_ADD				VK_ADD
#define KEY_SEPARATOR		VK_SEPARATOR
#define KEY_SUBTRACT		VK_SUBTRACT
#define KEY_DECIMAL			VK_DECIMAL
#define KEY_DIVIDE			VK_DIVIDE

// function keys
#define KEY_F1				VK_F1
#define KEY_F2				VK_F2
#define KEY_F3				VK_F3
#define KEY_F4				VK_F4
#define KEY_F5				VK_F5
#define KEY_F6				VK_F6
#define KEY_F7				VK_F7
#define KEY_F8				VK_F8
#define KEY_F9				VK_F9
#define KEY_F10				VK_F10
#define KEY_F11				VK_F11
#define KEY_F12				VK_F12

// arrow keys
#define KEY_LEFT			VK_LEFT
#define KEY_UP				VK_UP
#define KEY_RIGHT			VK_RIGHT
#define KEY_DOWN			VK_DOWN

// special keys
#define KEY_TAB				VK_TAB
#define KEY_RETURN			VK_RETURN
#define KEY_ENTER			VK_RETURN
#define KEY_SHIFT			VK_SHIFT
#define KEY_CONTROL			VK_CONTROL
#define KEY_ALT				VK_MENU
#define KEY_ESCAPE			VK_ESCAPE
#define KEY_SPACE			VK_SPACE
#define KEY_BACKSPACE		VK_BACK
#define KEY_END				VK_END
#define KEY_INSERT			VK_INSERT
#define KEY_DELETE			VK_DELETE
#define KEY_HELP			VK_HELP
#define KEY_HOME			VK_HOME
#define KEY_SUPER			VK_LWIN

#elif defined __linux__

#define XK_MISCELLANY
#define XK_LATIN1
#include <X11/keysymdef.h>

// alphabet
#define KEY_A				XK_A
#define KEY_B				XK_B
#define KEY_C				XK_C
#define KEY_D				XK_D
#define KEY_E				XK_E
#define KEY_F				XK_F
#define KEY_G				XK_G
#define KEY_H				XK_H
#define KEY_I				XK_I
#define KEY_J				XK_J
#define KEY_K				XK_K
#define KEY_L				XK_L
#define KEY_M				XK_M
#define KEY_N				XK_N
#define KEY_O				XK_O
#define KEY_P				XK_P
#define KEY_Q				XK_Q
#define KEY_R				XK_R
#define KEY_S				XK_S
#define KEY_T				XK_T
#define KEY_U				XK_U
#define KEY_V				XK_V
#define KEY_W				XK_W
#define KEY_X				XK_X
#define KEY_Y				XK_Y
#define KEY_Z				XK_Z

// numbers
#define KEY_0				XK_equal		// XK_0
#define KEY_1				XK_exclam		// XK_1
#define KEY_2				XK_quotedbl		// XK_2
#define KEY_3				XK_section		// XK_3
#define KEY_4				XK_dollar		// XK_4
#define KEY_5				XK_percent		// XK_5
#define KEY_6				XK_ampersand	// XK_6
#define KEY_7				XK_slash		// XK_7
#define KEY_8				XK_quoteright 	// XK_8
#define KEY_9				XK_parenleft	// XK_9

// numpad
#define KEY_NUMPAD0			XK_KP_0
#define KEY_NUMPAD1			XK_KP_1
#define KEY_NUMPAD2			XK_KP_2
#define KEY_NUMPAD3			XK_KP_3
#define KEY_NUMPAD4			XK_KP_4
#define KEY_NUMPAD5			XK_KP_5
#define KEY_NUMPAD6			XK_KP_6
#define KEY_NUMPAD7			XK_KP_7
#define KEY_NUMPAD8			XK_KP_8
#define KEY_NUMPAD9			XK_KP_9
#define KEY_MULTIPLY		XK_KP_Multiply
#define KEY_ADD				XK_KP_Add
#define KEY_SEPARATOR		XK_KP_Separator
#define KEY_SUBTRACT		XK_KP_Subtract
#define KEY_DECIMAL			XK_KP_Decimal
#define KEY_DIVIDE			XK_KP_Divide

// function keys
#define KEY_F1				XK_F1
#define KEY_F2				XK_F2
#define KEY_F3				XK_F3
#define KEY_F4				XK_F4
#define KEY_F5				XK_F5
#define KEY_F6				XK_F6
#define KEY_F7				XK_F7
#define KEY_F8				XK_F8
#define KEY_F9				XK_F9
#define KEY_F10				XK_F10
#define KEY_F11				XK_F11
#define KEY_F12				XK_F12

// arrow keys
#define KEY_LEFT			XK_Left
#define KEY_RIGHT			XK_Right
#define KEY_UP				XK_Up
#define KEY_DOWN			XK_Down

// special keys
#define KEY_TAB				XK_Tab
#define KEY_RETURN			XK_Return
#define KEY_ENTER			XK_Return
#define KEY_SHIFT			XK_Shift_L
#define KEY_CONTROL			XK_Control_L
#define KEY_ALT				XK_Caps_Lock // HACKHACK: using caps lock for now
#define KEY_ESCAPE			XK_Escape
#define KEY_SPACE			XK_space
#define KEY_BACKSPACE		XK_BackSpace
#define KEY_END				XK_End
#define KEY_INSERT			XK_Insert
#define KEY_DELETE			XK_Delete
#define KEY_HELP			XK_Help
#define KEY_HOME			XK_Home
#define KEY_SUPER			XK_Super_L

#elif defined __APPLE__

#ifndef MCENGINE_FEATURE_SDL

#include <Carbon/Carbon.h>

// alphabet
#define KEY_A				kVK_ANSI_A
#define KEY_B				kVK_ANSI_B
#define KEY_C				kVK_ANSI_C
#define KEY_D				kVK_ANSI_D
#define KEY_E				kVK_ANSI_E
#define KEY_F				kVK_ANSI_F
#define KEY_G				kVK_ANSI_G
#define KEY_H				kVK_ANSI_H
#define KEY_I				kVK_ANSI_I
#define KEY_J				kVK_ANSI_J
#define KEY_K				kVK_ANSI_K
#define KEY_L				kVK_ANSI_L
#define KEY_M				kVK_ANSI_M
#define KEY_N				kVK_ANSI_N
#define KEY_O				kVK_ANSI_O
#define KEY_P				kVK_ANSI_P
#define KEY_Q				kVK_ANSI_Q
#define KEY_R				kVK_ANSI_R
#define KEY_S				kVK_ANSI_S
#define KEY_T				kVK_ANSI_T
#define KEY_U				kVK_ANSI_U
#define KEY_V				kVK_ANSI_V
#define KEY_W				kVK_ANSI_W
#define KEY_X				kVK_ANSI_X
#define KEY_Y				kVK_ANSI_Y
#define KEY_Z				kVK_ANSI_Z

// numbers
#define KEY_0				kVK_ANSI_0
#define KEY_1				kVK_ANSI_1
#define KEY_2				kVK_ANSI_2
#define KEY_3				kVK_ANSI_3
#define KEY_4				kVK_ANSI_4
#define KEY_5				kVK_ANSI_5
#define KEY_6				kVK_ANSI_6
#define KEY_7				kVK_ANSI_7
#define KEY_8				kVK_ANSI_8
#define KEY_9				kVK_ANSI_9

// numpad
#define KEY_NUMPAD0			kVK_ANSI_Keypad0
#define KEY_NUMPAD1			kVK_ANSI_Keypad1
#define KEY_NUMPAD2			kVK_ANSI_Keypad2
#define KEY_NUMPAD3			kVK_ANSI_Keypad3
#define KEY_NUMPAD4			kVK_ANSI_Keypad4
#define KEY_NUMPAD5			kVK_ANSI_Keypad5
#define KEY_NUMPAD6			kVK_ANSI_Keypad6
#define KEY_NUMPAD7			kVK_ANSI_Keypad7
#define KEY_NUMPAD8			kVK_ANSI_Keypad8
#define KEY_NUMPAD9			kVK_ANSI_Keypad9
#define KEY_MULTIPLY		kVK_ANSI_KeypadMultiply
#define KEY_ADD				kVK_ANSI_KeypadPlus
#define KEY_SEPARATOR		kVK_ANSI_KeypadEquals // TODO: ???
#define KEY_SUBTRACT		kVK_ANSI_KeypadMinus
#define KEY_DECIMAL			kVK_ANSI_KeypadDecimal
#define KEY_DIVIDE			kVK_ANSI_KeypadDivide

// function keys
#define KEY_F1				kVK_F1
#define KEY_F2				kVK_F2
#define KEY_F3				kVK_F3
#define KEY_F4				kVK_F4
#define KEY_F5				kVK_F5
#define KEY_F6				kVK_F6
#define KEY_F7				kVK_F7
#define KEY_F8				kVK_F8
#define KEY_F9				kVK_F9
#define KEY_F10				kVK_F10
#define KEY_F11				kVK_F11
#define KEY_F12				kVK_F12

// arrow keys
#define KEY_LEFT			kVK_LeftArrow
#define KEY_RIGHT			kVK_RightArrow
#define KEY_UP				kVK_UpArrow
#define KEY_DOWN			kVK_DownArrow

// special keys
#define KEY_TAB				kVK_Tab
#define KEY_RETURN			kVK_Return
#define KEY_ENTER			kVK_Return
#define KEY_SHIFT			kVK_Shift
#define KEY_CONTROL			kVK_Control
#define KEY_ALT				kVK_Option
#define KEY_ESCAPE			kVK_Escape
#define KEY_SPACE			kVK_Space
#define KEY_BACKSPACE		kVK_Delete
#define KEY_END				kVK_End
#define KEY_INSERT			kVK_Help
#define KEY_DELETE			kVK_ForwardDelete
#define KEY_HELP			kVK_Help
#define KEY_HOME			kVK_Home
#define KEY_SUPER			kVK_Command

#endif

#endif

#endif

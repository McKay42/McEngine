//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		main entry point
//
// $NoKeywords: $main
//===============================================================================//

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

//#define MCENGINE_WINDOWS_REALTIMESTYLUS_SUPPORT
//#define MCENGINE_WINDOWS_TOUCH_SUPPORT

#ifdef MCENGINE_WINDOWS_TOUCH_SUPPORT
#define WINVER 0x0A00 // Windows 10, to enable the ifdefs in winuser.h for touch
#endif



#include "cbase.h"

// because "Please include winsock2.h before windows.h"
#ifdef MCENGINE_FEATURE_NETWORKING
#include <winsock2.h>
#endif

#include <windows.h>
#include <dwmapi.h>



#ifdef MCENGINE_FEATURE_SDL

#include "WinSDLEnvironment.h"

extern int mainSDL(int argc, char *argv[], SDLEnvironment *customSDLEnvironment);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// disable IME text input
	if (strstr(lpCmdLine, "-noime") != NULL)
	{
		typedef BOOL (WINAPI *pfnImmDisableIME)(DWORD);

		HMODULE hImm32 = LoadLibrary("imm32.dll");
		if (hImm32 != NULL)
		{
			pfnImmDisableIME pImmDisableIME = (pfnImmDisableIME)GetProcAddress(hImm32, "ImmDisableIME");
			if (pImmDisableIME == NULL)
				FreeLibrary(hImm32);
			else
			{
				pImmDisableIME(-1);
				FreeLibrary(hImm32);
			}
		}
	}

	// if supported (>= Windows Vista), enable DPI awareness so that GetSystemMetrics returns correct values
	// without this, on e.g. 150% scaling, the screen pixels of a 1080p monitor would be reported by GetSystemMetrics(SM_CXSCREEN/SM_CYSCREEN) as only 720p!
	if (strstr(lpCmdLine, "-nodpi") == NULL)
	{
		typedef WINBOOL (WINAPI *PSPDA)(void);
		PSPDA g_SetProcessDPIAware = (PSPDA)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "SetProcessDPIAware");
		if (g_SetProcessDPIAware != NULL)
			g_SetProcessDPIAware();
	}

	// build "fake" argc + argv
	const int argc = 2;
	char *argv[argc];
	char arg1 = '\0';
	argv[0] = &arg1;
	argv[1] = lpCmdLine;

	return mainSDL(argc, argv, new WinSDLEnvironment());
}

#else



#ifdef MCENGINE_WINDOWS_TOUCH_SUPPORT
#include <winuser.h>
typedef WINBOOL (WINAPI *PGPI)(UINT32 pointerId, POINTER_INFO *pointerInfo);
PGPI g_GetPointerInfo = (PGPI)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "GetPointerInfo");
#ifndef TWF_WANTPALM
#define TWF_WANTPALM 0x00000002
#endif
#endif

#ifdef MCENGINE_WINDOWS_REALTIMESTYLUS_SUPPORT
#include "WinRealTimeStylus.h"
#endif

#define MI_WP_SIGNATURE 0xFF515700
#define SIGNATURE_MASK 0xFFFFFF00
#define IsPenEvent(dw) (((dw) & SIGNATURE_MASK) == MI_WP_SIGNATURE)

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>

#include "Engine.h"
#include "Profiler.h"
#include "ConVar.h"
#include "Timer.h"
#include "Mouse.h"

#include "DirectX11Interface.h"
#include "WinGLLegacyInterface.h"
#include "WinEnvironment.h"

#define WINDOW_TITLE L"McEngine"

// #define WINDOW_FRAMELESS
// #define WINDOW_MAXIMIZED // start maximized
// #define WINDOW_GHOST // click-through overlay mode (experimental); NOTE: must uncomment "pfd.cAlphaBits = 8;" in WinGLLegacyInterface.cpp!

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define WINDOW_WIDTH_MIN 100
#define WINDOW_HEIGHT_MIN 100



#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL 0x020E
#endif

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0
#endif

#define WM_NCUAHDRAWCAPTION 0x00AE
#define WM_NCUAHDRAWFRAME 0x00AF



extern bool g_bCursorVisible; // set by WinEnvironment, for client area cursor invis
extern bool g_bARBMultisampleSupported;



Engine *g_engine = NULL;

bool g_bRunning = true;
bool g_bUpdate = true;
bool g_bDraw = true;
bool g_bDrawing = false;

bool g_bMinimized = false; // for fps_max_background
bool g_bHasFocus = false; // for fps_max_background
bool g_bIsCursorVisible = true; // local variable
bool g_bSupportsPerMonitorDpiAwareness = false; // local variable

std::vector<unsigned int> g_vTouches;

extern "C" { // force switch to the high performance gpu in multi-gpu systems (mostly laptops)
__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001; // http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
__declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001; // https://community.amd.com/thread/169965
}

ConVar fps_max("fps_max", 60.0f, "framerate limiter, foreground");
ConVar fps_max_yield("fps_max_yield", true, "always release rest of timeslice once per frame (call scheduler via sleep(0))");
ConVar fps_max_background("fps_max_background", 30.0f, "framerate limiter, background");
ConVar fps_max_background_interleaved("fps_max_background_interleaved", 1, "experimental, update normally but only draw every n-th frame");
ConVar fps_unlimited("fps_unlimited", false);
ConVar fps_unlimited_yield("fps_unlimited_yield", true, "always release rest of timeslice once per frame (call scheduler via sleep(0)), even if unlimited fps are enabled");

ConVar win_mouse_raw_input_buffer("win_mouse_raw_input_buffer", false, "use GetRawInputBuffer() to reduce wndproc event queue overflow stalls on insane mouse usb polling rates above 1000 Hz");

extern ConVar *win_realtimestylus;



//****************//
//	Message loop  //
//****************//

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
#ifdef WINDOW_GHOST

	// window click-through
	//case WM_NCHITTEST:
	//	return HTNOWHERE;
	//	break;

#endif

	case WM_NCCREATE:
		if (g_bSupportsPerMonitorDpiAwareness)
		{
			typedef WINBOOL (WINAPI *EPNCDS)(HWND);
			EPNCDS g_EnableNonClientDpiScaling = (EPNCDS)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "EnableNonClientDpiScaling");
			if (g_EnableNonClientDpiScaling != NULL)
				g_EnableNonClientDpiScaling(hwnd);
		}
		return DefWindowProcW(hwnd, msg, wParam, lParam);

#if defined(WINDOW_FRAMELESS) && !defined(WINDOW_GHOST)

		// ignore
		/*
		case WM_ERASEBKGND:
			return 1;
		*/

		// window border paint
		/*
		case WM_NCPAINT:
			{
				// draw beautifully blurred windows 7 background + shadows
				return DefWindowProcW(hwnd, msg, wParam, lParam);

				// draw white rectangle over everything except the shadows
				//HDC hdc;
				//hdc = GetDCEx(hwnd, (HRGN)wParam, DCX_WINDOW|DCX_INTERSECTRGN);
				//PAINTSTRUCT ps;
				//hdc = BeginPaint(hwnd, &ps);
				//RECT wr;
				//GetClientRect(hwnd, &wr);
				//HBRUSH br;
				//br = GetSysColorBrush(COLOR_WINDOW);
				//FillRect(hdc, &wr, br);
				//ReleaseDC(hwnd, hdc);
			}
			/// return 0;
		*/

		case WM_NCCALCSIZE:
			{
				if (wParam == TRUE)
				{
					LPNCCALCSIZE_PARAMS pncc = (LPNCCALCSIZE_PARAMS)lParam;

					//debugLog("new rectang: top = %i, right = %i, bottom = %i, left = %i\n", pncc->rgrc[0].top, pncc->rgrc[0].right, pncc->rgrc[0].bottom, pncc->rgrc[0].left);
					//debugLog("old rectang: top = %i, right = %i, bottom = %i, left = %i\n", pncc->rgrc[1].top, pncc->rgrc[1].right, pncc->rgrc[1].bottom, pncc->rgrc[1].left);
					//debugLog("client rect: top = %i, right = %i, bottom = %i, left = %i\n", pncc->rgrc[2].top, pncc->rgrc[2].right, pncc->rgrc[2].bottom, pncc->rgrc[2].left);

					if (IsZoomed(hwnd))
					{
						// HACKHACK: use center instead of MonitorFromWindow() in order to workaround windows display scaling bullshit bug
						POINT centerOfWindow;
						{
							centerOfWindow.x = pncc->rgrc[0].left + (pncc->rgrc[0].right - pncc->rgrc[0].left)/2;
							centerOfWindow.y = pncc->rgrc[0].top + (pncc->rgrc[0].bottom - pncc->rgrc[0].top)/2;
						}
						HMONITOR monitor = MonitorFromPoint(centerOfWindow, MONITOR_DEFAULTTONEAREST);

						MONITORINFO info;
						info.cbSize = sizeof(MONITORINFO);
						GetMonitorInfo(monitor, &info);

						//McRect mr(info.rcMonitor.left, info.rcMonitor.top, std::abs(info.rcMonitor.left - info.rcMonitor.right), std::abs(info.rcMonitor.top - info.rcMonitor.bottom));
						//printf("monitor.x = %i, y = %i, width = %i, height = %i\n", (int)mr.getX(), (int)mr.getY(), (int)mr.getWidth(), (int)mr.getHeight());

						// old (broken for multi-monitor setups)
						//pncc->rgrc[0].right += pncc->rgrc[0].left;
						//pncc->rgrc[0].bottom += pncc->rgrc[0].top;
						//pncc->rgrc[0].top = 0;
						//pncc->rgrc[0].left = 0;

						// new (still feels incorrect and fragile, but works for what I've tested it on)
						pncc->rgrc[0].right += pncc->rgrc[0].left - info.rcMonitor.left;
						pncc->rgrc[0].bottom += pncc->rgrc[0].top - info.rcMonitor.top;
						pncc->rgrc[0].top = info.rcMonitor.top;
						pncc->rgrc[0].left = info.rcMonitor.left;
					}

					//printf("after:  right = %i, bottom = %i, top = %i, left = %i\n", (int)pncc->rgrc[0].right, (int)pncc->rgrc[0].bottom, (int)pncc->rgrc[0].top, (int)pncc->rgrc[0].left);
				}
			}
			// "When wParam is TRUE, simply returning 0 without processing the NCCALCSIZE_PARAMS rectangles will cause the client area to resize to the size of the window,
			// including the window frame. This will remove the window frame and caption items from your window, leaving only the client area displayed."
			// "Starting with Windows Vista, removing the standard frame by simply returning 0 when the wParam is TRUE does not affect frames that are extended into the client area using the DwmExtendFrameIntoClientArea function.
			// Only the standard frame will be removed."
			return 0;

		// window border
		case WM_NCHITTEST:
			{
				if (g_engine != NULL)
				{
					long val = DefWindowProcW(hwnd, msg, wParam, lParam);
					if (val != HTCLIENT)
						return val;

					const float dpiScale = g_engine->getEnvironment()->getDPIScale();

					// fake window moving
					Vector2 mousePos = g_engine->getEnvironment()->getMousePos();
					McRect dragging = McRect(40*dpiScale, 4*dpiScale, engine->getScreenWidth() - 80*dpiScale, 20*dpiScale); // HACKHACK: hardcoded 40 for stuff
					if (dragging.contains(mousePos))
						val = HTCAPTION;

					if (IsZoomed(hwnd))
						return val;

					// fake window resizing
					McRect resizeN = McRect(0, 0, engine->getScreenWidth(), 4*dpiScale);
					McRect resizeW = McRect(0, 0, 4*dpiScale, engine->getScreenHeight());
					McRect resizeO = McRect(engine->getScreenWidth() - 5*dpiScale, 0, engine->getScreenWidth(), engine->getScreenHeight());
					McRect resizeS = McRect(0, engine->getScreenHeight() - 5*dpiScale, engine->getScreenWidth(), engine->getScreenHeight());
					if (resizeN.contains(mousePos))
						val = HTTOP;
					if (resizeW.contains(mousePos))
						val = HTLEFT;
					if (resizeO.contains(mousePos))
						val = HTRIGHT;
					if (resizeS.contains(mousePos))
						val = HTBOTTOM;

					if (resizeN.contains(mousePos) && resizeW.contains(mousePos))
						val = HTTOPLEFT;
					if (resizeN.contains(mousePos) && resizeO.contains(mousePos))
						val = HTTOPRIGHT;
					if (resizeO.contains(mousePos) && resizeS.contains(mousePos))
						val = HTBOTTOMRIGHT;
					if (resizeS.contains(mousePos) && resizeW.contains(mousePos))
						val = HTBOTTOMLEFT;

					return val;
				}
				else
					return DefWindowProcW(hwnd, msg, wParam, lParam);
			}
			break;
#endif

		// graceful shutdown request
		case WM_DESTROY:
			if (g_engine != NULL)
				g_engine->shutdown(); // this will in turn trigger a WM_CLOSE
			return 0;

		// alt-f4, window X button press, right click > close, "exit" ConCommand and WM_DESTROY will all send WM_CLOSE
		case WM_CLOSE:
			if (g_bRunning)
			{
				g_bRunning = false;
				if (g_engine != NULL)
					g_engine->onShutdown();
			}
			return 0;

		// paint nothing on repaint
		case WM_PAINT:
			{
				// variant 1 (apparently not the correct way of doing this?):
				/*
				ValidateRect(hwnd, NULL);
				*/

				// variant 2 (seems to be what DefWindowProc is doing):
				PAINTSTRUCT ps;
				BeginPaint(hwnd, &ps);
				EndPaint(hwnd, &ps);

				// debug:
				/*
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hwnd, &ps);

				RECT wr;
				GetClientRect(hwnd, &wr);
				HBRUSH br;
				br = (HBRUSH)GetStockObject(BLACK_BRUSH);
				FillRect(hdc, &wr, br);

				///br = (HBRUSH)GetStockObject(GRAY_BRUSH);
				///wr.right = 100;
				///wr.bottom = 100;
				///FillRect(hdc, &wr, br);

				EndPaint(hwnd,&ps);
				*/
			}
			return 0;

		// focus and active
		case WM_ACTIVATE:
			if (g_bRunning)
			{
				if (!HIWORD(wParam)) // if we are not minimized
				{
					//if (g_engine != NULL)
					//	printf("WndProc() : WM_ACTIVATE, not minimized\n");

					g_bUpdate = true;
					g_bDraw = true;
				}
				else
				{
					//if (g_engine != NULL)
					//	printf("WndProc() : WM_ACTIVATE, minimized\n");

					/// g_bUpdate = false;
					g_bDraw = false;
					g_bHasFocus = false;
				}
			}
			break;

		// focus
		case WM_SETFOCUS:
			g_bHasFocus = true;
			if (g_bRunning && g_engine != NULL)
			{
				if (!g_engine->hasFocus())
					g_engine->onFocusGained();
			}
			break;

		case WM_KILLFOCUS:
			g_bHasFocus = false;
			if (g_bRunning && g_engine != NULL)
			{
				if (g_engine->hasFocus())
					g_engine->onFocusLost();
			}
			break;

		// OnKeyDown
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			if (g_engine != NULL)
			{
				g_engine->onKeyboardKeyDown(wParam);
				return 0;
			}
			break;

		// OnKeyUp
		case WM_SYSKEYUP:
		case WM_KEYUP:
			if (g_engine != NULL)
			{
				g_engine->onKeyboardKeyUp(wParam);
				return 0;
			}
			break;

		// OnCharDown
		case WM_CHAR:
			if (g_engine != NULL)
			{
				g_engine->onKeyboardChar(wParam);
				return 0;
			}
			break;

		// left mouse button, inject as keyboard key as well
		case WM_LBUTTONDOWN:
			if (g_engine != NULL && (!win_realtimestylus->getBool() || !IsPenEvent(GetMessageExtraInfo()))) // if realtimestylus support is enabled, all clicks are handled by it and not here
			{
				g_engine->onMouseLeftChange(true);
				g_engine->onKeyboardKeyDown(VK_LBUTTON);
			}
			SetCapture(hwnd);
			break;
		case WM_LBUTTONUP:
			if (g_engine != NULL)
			{
				g_engine->onMouseLeftChange(false);
				g_engine->onKeyboardKeyUp(VK_LBUTTON);
			}
			ReleaseCapture();
			break;

		// middle mouse button, inject as keyboard key as well
		case WM_MBUTTONDOWN:
			if (g_engine != NULL)
			{
				g_engine->onMouseMiddleChange(true);
				g_engine->onKeyboardKeyDown(VK_MBUTTON);
			}
			SetCapture(hwnd);
			break;
		case WM_MBUTTONUP:
			if (g_engine != NULL)
			{
				g_engine->onMouseMiddleChange(false);
				g_engine->onKeyboardKeyUp(VK_MBUTTON);
			}
			ReleaseCapture();
			break;

		// right mouse button, inject as keyboard key as well
		case WM_RBUTTONDOWN:
			if (g_engine != NULL && (!win_realtimestylus->getBool() || !IsPenEvent(GetMessageExtraInfo()))) // if realtimestylus support is enabled, all pen clicks are handled by it and not here
			{
				g_engine->onMouseRightChange(true);
				g_engine->onKeyboardKeyDown(VK_RBUTTON);
			}
			SetCapture(hwnd);
			break;
		case WM_RBUTTONUP:
			if (g_engine != NULL)
			{
				g_engine->onMouseRightChange(false);
				g_engine->onKeyboardKeyUp(VK_RBUTTON);
			}
			ReleaseCapture();
			break;

		// mouse sidebuttons (4 and 5), inject them as keyboard keys as well
		case WM_XBUTTONDOWN:
			if (g_engine != NULL)
			{
				const DWORD fwButton = GET_XBUTTON_WPARAM(wParam);
				if (fwButton == XBUTTON1)
				{
					g_engine->onMouseButton4Change(true);
					g_engine->onKeyboardKeyDown(VK_XBUTTON1);
				}
				else if (fwButton == XBUTTON2)
				{
					g_engine->onMouseButton5Change(true);
					g_engine->onKeyboardKeyDown(VK_XBUTTON2);
				}
			}
			return TRUE;
		case WM_XBUTTONUP:
			if (g_engine != NULL)
			{
				const DWORD fwButton = GET_XBUTTON_WPARAM(wParam);
				if (fwButton == XBUTTON1)
				{
					g_engine->onMouseButton4Change(false);
					g_engine->onKeyboardKeyUp(VK_XBUTTON1);
				}
				else if (fwButton == XBUTTON2)
				{
					g_engine->onMouseButton5Change(false);
					g_engine->onKeyboardKeyUp(VK_XBUTTON2);
				}
			}
			return TRUE;

		// cursor visibility handling (for client area)
		case WM_SETCURSOR:
			if (!g_bCursorVisible)
			{
				if (LOWORD(lParam) == HTCLIENT) // hide if in client area
				{
					if (g_bIsCursorVisible)
					{
						g_bIsCursorVisible = false;

						int check = ShowCursor(false);
						for (int i=0; i<=check; i++)
						{
							ShowCursor(false);
						}
					}
				}
				else if (!g_bIsCursorVisible) // show if not in client area (e.g. window border)
				{
					g_bIsCursorVisible = true;

					int check = ShowCursor(true);
					if (check < 0)
					{
						for (int i=check; i<=0; i++)
						{
							ShowCursor(true);
						}
					}
				}
			}

			// this logic below would mean that we have to handle the cursor when moving from resizing into the client area
			// seems very annoying; unfinished
			/*
			if (LOWORD(lParam) == HTCLIENT) // if we are inside the client area, we handle the cursor
				return TRUE;
			else
				break; // if not, let DefWindowProc do its thing
			*/

			// avoid cursor flicker when using non-normal cursor set by engine
			if (g_engine != NULL && g_engine->getEnvironment()->getCursor() != CURSORTYPE::CURSOR_NORMAL)
				return TRUE;

			break;

#ifdef MCENGINE_WINDOWS_TOUCH_SUPPORT
		case WM_POINTERUP:
		case WM_POINTERDOWN:
		case WM_POINTERUPDATE:
		case WM_POINTERENTER:
		case WM_POINTERLEAVE:
		case WM_POINTERCAPTURECHANGED:
		case WM_POINTERWHEEL:
		case WM_POINTERHWHEEL:
		if (g_engine != NULL && g_GetPointerInfo != NULL)
		{
			POINTER_INFO pointerInfo;
			unsigned long id = LOWORD(wParam);
			if (g_GetPointerInfo(LOWORD(wParam), &pointerInfo))
			{
				if (pointerInfo.pointerFlags & POINTER_FLAG_PRIMARY)
				{
					// bit of a hack, but it should work fine
					// convert to fake raw tablet coordinates (0 to 65536)
					int rawAbsoluteX = ((float)pointerInfo.ptPixelLocation.x / (float)g_engine->getEnvironment()->getNativeScreenSize().x)*65536;
					int rawAbsoluteY = ((float)pointerInfo.ptPixelLocation.y / (float)g_engine->getEnvironment()->getNativeScreenSize().y)*65536;
					g_engine->onMouseRawMove(rawAbsoluteX, rawAbsoluteY, true, true);
				}

				if (pointerInfo.pointerFlags & POINTER_FLAG_DOWN)
				{
					bool contains = false;
					for (size_t i=0; i<g_vTouches.size(); i++)
					{
						if (g_vTouches[i] == id)
						{
							contains = true;
							break;
						}
					}

					if (!contains)
					{
						bool already = g_vTouches.size() > 0;
						g_vTouches.push_back(id);

						if (already)
							g_engine->onMouseRightChange(true);
						else
							g_engine->onMouseLeftChange(true);
					}
				}
				else if (pointerInfo.pointerFlags & POINTER_FLAG_UP)
				{
					for (size_t i=0; i<g_vTouches.size(); i++)
					{
						if (g_vTouches[i] == id)
						{
							g_vTouches.erase(g_vTouches.begin() + i);
							i--;
						}
					}

					bool still = g_vTouches.size() > 0;

					if (still)
						g_engine->onMouseRightChange(false);
					// WTF: this is already called by WM_LBUTTONUP, fuck touch
					//else
					//	g_engine->onMouseLeftChange(false);
				}
			}
		}
		break;
#endif

		// raw input handling (only for mouse movement atm)
		case WM_INPUT:
			{
				RAWINPUT raw;
				UINT dwSize = sizeof(raw);

				GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &raw, &dwSize, sizeof(RAWINPUTHEADER));

				if (raw.header.dwType == RIM_TYPEMOUSE)
				{
					if (g_engine != NULL)
					{
						g_engine->onMouseRawMove(raw.data.mouse.lLastX, raw.data.mouse.lLastY, (raw.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE), (raw.data.mouse.usFlags & MOUSE_VIRTUAL_DESKTOP));
					}
				}
			}
			break;

		// vertical mouse wheel
		case WM_MOUSEWHEEL:
			if (g_engine != NULL)
				g_engine->onMouseWheelVertical(GET_WHEEL_DELTA_WPARAM(wParam));
			break;

		// horizontal mouse wheel
		case WM_MOUSEHWHEEL:
			if (g_engine != NULL)
				g_engine->onMouseWheelHorizontal(GET_WHEEL_DELTA_WPARAM(wParam));
			break;

		// minimizing/maximizing
		case WM_SYSCOMMAND:
			switch (wParam)
			{
			case SC_MINIMIZE:
				g_bDrawing = false;
				g_bDraw = false;
				/// g_bUpdate = false;
				g_bMinimized = true;
				if (g_engine != NULL)
					g_engine->onMinimized();
				break;
			case SC_MAXIMIZE:
				g_bMinimized = false;
				if (g_engine != NULL)
				{
					RECT rect;
					GetClientRect(hwnd, &rect);
					g_engine->onResolutionChange(Vector2(rect.right, rect.bottom));
					g_engine->onMaximized();
				}
				break;
			case SC_RESTORE:
				g_bUpdate = true;
				g_bDraw = true;
				g_bMinimized = false;
				if (g_engine != NULL)
					g_engine->onRestored();
				break;

			// ignore ALT key opening the window context menu
			case SC_KEYMENU:
				if ((lParam>>16) <= 0) // still allow for mouse tho
					return 0;
				break;
			}
			break;

		// resolution change
		case WM_DISPLAYCHANGE:
		case WM_SIZE:
		///case WM_EXITSIZEMOVE: // this would fire resize events for every pixel change, destroying performance if e.g. RenderTargets are resized in onResolutionChange()
			if (g_engine != NULL && g_bUpdate)
			{
				RECT rect;
				GetClientRect(hwnd, &rect);
				g_engine->requestResolutionChange(Vector2(rect.right, rect.bottom));
			}
			break;

		// DPI scaling change (e.g. because user changed scaling in settings, or moved the window to a monitor with different scaling)
		case WM_DPICHANGED:
			if (g_engine != NULL)
			{
				WinEnvironment *winEnv = dynamic_cast<WinEnvironment*>(g_engine->getEnvironment());
				if (winEnv != NULL)
				{
					winEnv->setDPIOverride(HIWORD(wParam));
					g_engine->onDPIChange();

					RECT* const prcNewWindow = (RECT*)lParam;
					SetWindowPos(hwnd,
						NULL,
						prcNewWindow->left,
						prcNewWindow->top,
						prcNewWindow->right - prcNewWindow->left,
						prcNewWindow->bottom - prcNewWindow->top,
						SWP_NOZORDER | SWP_NOACTIVATE);
				}
			}
			break;

		// resize limit
		case WM_GETMINMAXINFO:
			{
				// NOTE: if rendering via DirectX then don't interfere here, since it handles all window management stuff for us
#ifdef MCENGINE_FEATURE_DIRECTX11

				if (g_engine != NULL && dynamic_cast<DirectX11Interface*>(g_engine->getGraphics()) != NULL)
					return DefWindowProcW(hwnd, msg, wParam, lParam);

#endif

				WINDOWPLACEMENT wPos;
				{
					wPos.length = sizeof(WINDOWPLACEMENT);
				}
				GetWindowPlacement(hwnd, &wPos);

				// min
				LPMINMAXINFO pMMI = (LPMINMAXINFO)lParam;

				//printf("before: %i %i %i %i\n", (int)pMMI->ptMinTrackSize.x, (int)pMMI->ptMinTrackSize.y, (int)pMMI->ptMaxTrackSize.x, (int)pMMI->ptMaxTrackSize.y);
				//printf("window pos: left = %i, top = %i, bottom = %i, right = %i\n", (int)wPos.rcNormalPosition.left, (int)wPos.rcNormalPosition.top, (int)wPos.rcNormalPosition.bottom, (int)wPos.rcNormalPosition.right);

				pMMI->ptMinTrackSize.x = WINDOW_WIDTH_MIN;
				pMMI->ptMinTrackSize.y = WINDOW_HEIGHT_MIN;

				// NOTE: this is only required for OpenGL and custom renderers
				// allow dynamic overscale (offscreen window borders/decorations)
				// this also clamps all user-initiated resolution changes to the resolution of the monitor the window is on

				//HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
				// HACKHACK: use center instead of MonitorFromWindow() in order to workaround windows display scaling bullshit bug
				POINT centerOfWindow;
				{
					centerOfWindow.x = wPos.rcNormalPosition.left + (wPos.rcNormalPosition.right - wPos.rcNormalPosition.left)/2;
					centerOfWindow.y = wPos.rcNormalPosition.top + (wPos.rcNormalPosition.bottom - wPos.rcNormalPosition.top)/2;
				}
				HMONITOR monitor = MonitorFromPoint(centerOfWindow, MONITOR_DEFAULTTONEAREST);
				{
					MONITORINFO info;
					info.cbSize = sizeof(MONITORINFO);

					if (GetMonitorInfo(monitor, &info) >= 0)
					{
						RECT clientRect;
						RECT windowRect;

						GetClientRect(hwnd, &clientRect);
						GetWindowRect(hwnd, &windowRect);

						const LONG decorationsSumWidth = (windowRect.right - windowRect.left) - clientRect.right;
						const LONG decorationsSumHeight = (windowRect.bottom - windowRect.top) - clientRect.bottom;

						//printf("window rect: left = %i, top = %i, bottom = %i, right = %i\n", (int)windowRect.left, (int)windowRect.top, (int)windowRect.bottom, (int)windowRect.right);
						//printf("client rect: left = %i, top = %i, bottom = %i, right = %i\n", (int)clientRect.left, (int)clientRect.top, (int)clientRect.bottom, (int)clientRect.right);
						//printf("monitor width = %i, height = %i\n", (int)std::abs(info.rcMonitor.left - info.rcMonitor.right), (int)std::abs(info.rcMonitor.top - info.rcMonitor.bottom));
						//printf("decorations: width = %i, height = %i\n", (int)decorationsSumWidth, (int)decorationsSumHeight);

						pMMI->ptMaxTrackSize.x = std::abs(info.rcMonitor.left - info.rcMonitor.right) + decorationsSumWidth;
						pMMI->ptMaxTrackSize.y = std::abs(info.rcMonitor.top - info.rcMonitor.bottom) + decorationsSumHeight;
					}
				}

				//printf("after: %i %i %i %i\n", (int)pMMI->ptMinTrackSize.x, (int)pMMI->ptMinTrackSize.y, (int)pMMI->ptMaxTrackSize.x, (int)pMMI->ptMaxTrackSize.y);
			}
			return 0;
	}

	return DefWindowProcW(hwnd, msg, wParam, lParam);
}



//*******************//
//	Window creation  //
//*******************//

HWND createWinWindow(HINSTANCE hInstance)
{
	HWND hwnd;

	// window style
	LONG_PTR style = WinEnvironment::getWindowStyleWindowed();
#ifdef WINDOW_FRAMELESS
	///style = WS_OVERLAPPEDWINDOW & (~WS_SYSMENU);
	style = WS_POPUP | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CAPTION;
#endif
	LONG_PTR exStyle = WS_EX_WINDOWEDGE;

#ifdef WINDOW_GHOST
	style = WS_POPUP;
#endif

#ifdef WINDOW_GHOST
	///exStyle = WS_EX_COMPOSITED | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST;
	exStyle = WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT;
#endif

	style &= ~WS_VISIBLE; // always start out invisible, we have a ShowWindow() call later anyway

	int xPos = (GetSystemMetrics(SM_CXSCREEN)/2) - (WINDOW_WIDTH/2);
	int yPos = (GetSystemMetrics(SM_CYSCREEN)/2) - (WINDOW_HEIGHT/2);
	int width = WINDOW_WIDTH;
	int height = WINDOW_HEIGHT;

	RECT clientArea;
	clientArea.left = xPos;
	clientArea.top = yPos;
	clientArea.right = xPos+width;
	clientArea.bottom = yPos+height;
	AdjustWindowRect(&clientArea, style, FALSE);

	xPos = clientArea.left;
	yPos = clientArea.top;
	width = clientArea.right - clientArea.left;
	height = clientArea.bottom - clientArea.top;

#ifdef WINDOW_MAXIMIZED
	RECT workArea;
	if (SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0))
	{
		xPos = yPos = 0;
		width = workArea.right;
		height = workArea.bottom;
	}
#endif

#ifdef WINDOW_GHOST
	xPos = yPos = 0;
	width = GetSystemMetrics(SM_CXSCREEN)+1;
	height = GetSystemMetrics(SM_CYSCREEN); // WTF: if these are both equal to exactly the current screen size, Windows will NOT draw anything behind the engine even though the window is transparent!
#endif

	// create the window
	hwnd = CreateWindowExW(
		exStyle,
		WINDOW_TITLE,
		WINDOW_TITLE,
		style,
		xPos, yPos, width, height,
		NULL, NULL, hInstance, NULL);

	if (hwnd == NULL)
	{
		MessageBox(NULL, "Couldn't CreateWindowEx()!", "Fatal Error", MB_ICONEXCLAMATION | MB_OK);
		return NULL;
	}

	return hwnd;
}



//********************//
//	Main entry point  //
//********************//

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// NOTE: add -mwindows to linker options to disable console window if compiling with Eclipse

	// IPC URL protocol handler (1)
/*
	const size_t sharedMemorySize = sizeof(char) * 4096;

#pragma pack(1)

	enum class IPC_STATE : uint8_t
	{
		IDLE,
		MESSAGE
	};

	enum class IPC_MESSAGE_TYPE : uint8_t
	{
		URL
	};

	struct IPCState
	{
		IPC_STATE state;
		IPC_MESSAGE_TYPE type;
	};

#pragma pack()

	HANDLE ipcMappedFile = NULL;
	LPVOID ipcSharedMemory = NULL;
	{
		const char *sharedMemoryName = "Local\\McEngine";

		// check for other running instance
		ipcMappedFile = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, sharedMemoryName);

		// if we are the first instance
		bool slave = true;
		if (ipcMappedFile == NULL)
		{
			slave = false;
			ipcMappedFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sharedMemorySize, sharedMemoryName);
		}

		if (ipcMappedFile != NULL)
		{
			ipcSharedMemory = MapViewOfFile(ipcMappedFile, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, sharedMemorySize);

			if (ipcSharedMemory != NULL)
			{
				if (slave)
				{
					if (strncmp(lpCmdLine, "mcengine://", 11) == 0)
					{
						if (((IPCState*)ipcSharedMemory)->state == IPC_STATE::IDLE)
						{
							const size_t bytesToCopy = std::min(strlen(lpCmdLine), sharedMemorySize - sizeof(IPCState) - 2) + 1;

							//printf("Copying %u bytes\n", bytesToCopy);

							CopyMemory(((char*)ipcSharedMemory) + sizeof(IPCState), lpCmdLine, bytesToCopy);

							IPCState ipcState;

							ipcState.state = IPC_STATE::MESSAGE;
							ipcState.type = IPC_MESSAGE_TYPE::URL;

							CopyMemory(ipcSharedMemory, &ipcState, sizeof(IPCState));
						}

						UnmapViewOfFile(ipcSharedMemory);
						CloseHandle(ipcMappedFile);

						return 0; // we are done here, shutdown slave
					}
				}
				else
				{
					IPCState ipcState;

					ipcState.state = IPC_STATE::IDLE;

					CopyMemory(ipcSharedMemory, &ipcState, sizeof(IPCState));
				}
			}
			else
				printf("IPC ERROR: Couldn't MapViewOfFile()!\n");
		}
		else
			printf("IPC ERROR: Couldn't OpenFileMapping() or CreateFileMapping()!\n");
	}
*/



/*
	// TEMP: register URL protocol handler in registry
	// TODO: this needs admin elevation, otherwise just silently fails, very annoying for usability
	{
		wchar_t fullPathToExe[MAX_PATH];
		GetModuleFileNameW(NULL, fullPathToExe, MAX_PATH);

		UString fullPathString = UString(fullPathToExe);

		UString defaultIconString = UString(fullPathString.wc_str()); // copy
		defaultIconString.insert(0, L'"');
		defaultIconString.append("\",1");

		UString shellOpenCommandString = UString(fullPathString.wc_str()); // copy
		shellOpenCommandString.insert(0, L'"');
		shellOpenCommandString.append("\" \"%1\"");

		HKEY hkey;
		if (RegCreateKeyW(HKEY_CLASSES_ROOT, L"mcengine", &hkey) == ERROR_SUCCESS)
		{
			RegSetValueW(hkey, NULL, REG_SZ, L"mcengine url handler", 0);
			RegSetValueExW(hkey, L"URL Protocol", 0, REG_SZ, NULL, 0);

			HKEY defaultIcon;
			RegCreateKeyW(hkey, L"DefaultIcon", &defaultIcon);
			RegSetValueW(defaultIcon, NULL, REG_SZ, defaultIconString.wc_str(), 0);

			HKEY command;
			RegCreateKeyW(hkey, L"shell\\open\\command", &command);
			RegSetValueW(command, NULL, REG_SZ, shellOpenCommandString.wc_str(), 0);
		}
		else
			printf("REG ERROR: Couldn't RegCreateKeyW()!\n");
	}
*/



	// disable IME text input
	if (strstr(lpCmdLine, "-noime") != NULL)
	{
		typedef BOOL (WINAPI *pfnImmDisableIME)(DWORD);

		HMODULE hImm32 = LoadLibrary("imm32.dll");
		if (hImm32 != NULL)
		{
			pfnImmDisableIME pImmDisableIME = (pfnImmDisableIME)GetProcAddress(hImm32, "ImmDisableIME");
			if (pImmDisableIME == NULL)
				FreeLibrary(hImm32);
			else
			{
				pImmDisableIME(-1);
				FreeLibrary(hImm32);
			}
		}
	}

	// enable fancy themed windows controls (v6+), requires McEngine.exe.manifest AND linking to comctl32, for fucks sake
	// only noticeable in MessageBox()-es and a few other dialogs
	{
		INITCOMMONCONTROLSEX icc;
		icc.dwSize = sizeof(icc);
		icc.dwICC = ICC_WIN95_CLASSES;
		InitCommonControlsEx(&icc);
	}

	// if supported (>= Windows Vista), enable DPI awareness so that GetSystemMetrics returns correct values
	// without this, on e.g. 150% scaling, the screen pixels of a 1080p monitor would be reported by GetSystemMetrics(SM_CXSCREEN/SM_CYSCREEN) as only 720p!
	// also on even newer systems (>= Windows 8.1) we can get WM_DPICHANGED notified
	if (strstr(lpCmdLine, "-nodpi") == NULL)
	{
		// Windows 8.1+
		// per-monitor dpi scaling
		{
			HMODULE shcore = GetModuleHandle(TEXT("shcore.dll"));
			if (shcore != NULL)
			{
				typedef HRESULT (WINAPI *PSPDAN)(int);
				PSPDAN g_SetProcessDpiAwareness = (PSPDAN)GetProcAddress(shcore, "SetProcessDpiAwareness");
				if (g_SetProcessDpiAwareness != NULL)
				{
					const HRESULT result = g_SetProcessDpiAwareness(2); // 2 == PROCESS_PER_MONITOR_DPI_AWARE
					g_bSupportsPerMonitorDpiAwareness = (result == S_OK || result == E_ACCESSDENIED);
				}
			}
		}

		if (!g_bSupportsPerMonitorDpiAwareness)
		{
			// Windows Vista+
			// system-wide dpi scaling
			{
				typedef WINBOOL (WINAPI *PSPDA)(void);
				PSPDA g_SetProcessDPIAware = (PSPDA)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "SetProcessDPIAware");
				if (g_SetProcessDPIAware != NULL)
					g_SetProcessDPIAware();
			}
		}
	}

	// prepare window class
	WNDCLASSEXW wc;

	wc.cbSize			= sizeof(WNDCLASSEX);
	wc.style			= 0;
	wc.lpfnWndProc		= WndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= hInstance;
	wc.hIcon			= NULL;
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= (HBRUSH)CreateSolidBrush(0x00000000);
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= WINDOW_TITLE;
	//wc.hIconSm			= LoadIcon(hInstance, MAKEINTRESOURCE(1)); // NOTE: load icon named "1" in /Main/WinIcon.rc file, must link to WinIcon.o which was created with windres.exe
	wc.hIconSm			= LoadIcon(NULL, IDI_APPLICATION);

	// register window class
	if (!RegisterClassExW(&wc))
	{
		MessageBox(NULL, "Couldn't RegisterClassEx()!", "Fatal Error", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}

	// create the window
	HWND hwnd = createWinWindow(hInstance);
	if (hwnd == NULL)
	{
		printf("FATAL ERROR: hwnd == NULL!!!\n");
		MessageBox(NULL, "Couldn't createWinWindow()!", "Fatal Error", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}

#ifdef MCENGINE_FEATURE_OPENGL

	// try to enable OpenGL MSAA
	// lots of windows api bullshit behaviour here
	/*
	if (!g_bARBMultisampleSupported && true)
	{
		FAKE_CONTEXT context = WinGLLegacyInterface::createAndMakeCurrentWGLContext(hwnd, getPixelFormatDescriptor());

		if (initWinGLMultisample(context.hdc, hInstance, hwnd, 4))
		{
			printf("OpenGL: MSAA is supported!\n");

			// we have AA support, delete EVERYTHING up until now
			wglMakeCurrent(context.hdc, NULL);
			wglDeleteContext(context.hglrc);
			ReleaseDC(hwnd, context.hdc);
			DestroyWindow(hwnd);
			hwnd = NULL;

			// create second window, which will be the real one
			hwnd = createWinWindow(hInstance);
			if (hwnd == NULL)
			{
				printf("FATAL ERROR: hwnd == NULL!!!\n");
				MessageBox(NULL, "Couldn't createWinWindow()!", "Fatal Error", MB_ICONEXCLAMATION | MB_OK);
				return -1;
			}
		}
		else
		{
			printf("OpenGL: MSAA is NOT supported.\n");

			// no AA support, destroy all temporaries and continue with the first window
			wglMakeCurrent(context.hdc, NULL);
			wglDeleteContext(context.hglrc);
			ReleaseDC(hwnd, context.hdc);
		}
		printf("\n");
	}
	*/

#endif

#ifdef WINDOW_FRAMELESS
	{
		MARGINS m;
		{
			  m.cxLeftWidth = 1;
			  m.cxRightWidth = 1;
			  m.cyTopHeight = 1;
			  m.cyBottomHeight = 1;
		}
		//HRESULT result = DwmExtendFrameIntoClientArea(hwnd, &m);
		DwmExtendFrameIntoClientArea(hwnd, &m);
		//printf("DwmExtendFrameIntoClientArea() = %x\n", (int)result);
		SetWindowPos(hwnd, NULL, 0,0,0,0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);

		//DWM_BLURBEHIND bb;
		//{
		//	bb.dwFlags = DWM_BB_ENABLE;
		//	bb.fEnable = 0;
		//	bb.hRgnBlur = NULL;
		//	bb.fTransitionOnMaximized = FALSE;
		//}
		//DwmEnableBlurBehindWindow(hwnd, &bb);
	}
#endif

#ifdef WINDOW_GHOST
	{
		DWM_BLURBEHIND bb = {0};
		HRGN hRgn = CreateRectRgn(0, 0, -1, -1);

		bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
		bb.hRgnBlur = hRgn;
		bb.fEnable = TRUE;

		printf("DwmEnableBlurBehindWindow() = %x\n", (int)DwmEnableBlurBehindWindow(hwnd, &bb));

		SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
	}
#endif

	// get the screen refresh rate, and set fps_max to that as default
	{
		DEVMODE lpDevMode;
		memset(&lpDevMode, 0, sizeof(DEVMODE));
		lpDevMode.dmSize = sizeof(DEVMODE);
		lpDevMode.dmDriverExtra = 0;

		if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &lpDevMode))
		{
			float displayFrequency = static_cast<float>(lpDevMode.dmDisplayFrequency);
			///printf("Display Refresh Rate is %.2f Hz, setting fps_max to %i.\n\n", displayFrequency, (int)displayFrequency);
			fps_max.setValue((int)displayFrequency);
			fps_max.setDefaultFloat((int)displayFrequency);
		}
	}

	// initialize raw input
	{
		RAWINPUTDEVICE Rid[1];
		Rid[0].usUsagePage = ((USHORT) 0x01);
		Rid[0].usUsage = ((USHORT) 0x02);
		Rid[0].dwFlags = /*RIDEV_INPUTSINK | RIDEV_DEVNOTIFY*/0;
		Rid[0].hwndTarget = hwnd;
		if (RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])) == FALSE)
		{
			printf("WARNING: Couldn't RegisterRawInputDevices(), GetLastError() = %i\n", (int)GetLastError());
			MessageBox(NULL, "Couldn't RegisterRawInputDevices()!", "Warning", MB_ICONEXCLAMATION | MB_OK);
		}
	}

#ifdef MCENGINE_WINDOWS_REALTIMESTYLUS_SUPPORT
	// initialize RealTimeStylus (COM)
	{
		HRESULT hr = CoInitialize(NULL);
		if (hr == S_OK || hr == S_FALSE) // if we initialized successfully, or if we are already initialized
		{
			if (!InitRealTimeStylus(hInstance, hwnd))
				printf("WARNING: Couldn't InitRealTimeStylus()! RealTimeStylus is not going to work.\n");
		}
		else
			printf("WARNING: Couldn't CoInitialize()! RealTimeStylus is not going to work.\n");
	}
#endif

#ifdef MCENGINE_WINDOWS_TOUCH_SUPPORT
	// initialize touch support/settings
	// http://robertinventor.com/bmwiki/How_to_disable_guestures_etc._for_multi_touch_playable_on_screen_keyboards
	{
		LPCTSTR tabletAtom = "MicrosoftTabletPenServiceProperty";
		unsigned short pressAndHoldAtomID = GlobalAddAtom(tabletAtom);
		if (pressAndHoldAtomID != 0)
		{
			DWORD dwHwndTabletProperty = TABLET_DISABLE_PRESSANDHOLD
									| TABLET_DISABLE_PENTAPFEEDBACK
									| TABLET_DISABLE_PENBARRELFEEDBACK
									| TABLET_DISABLE_FLICKS
									| TABLET_DISABLE_SMOOTHSCROLLING
									| TABLET_DISABLE_FLICKFALLBACKKEYS
									| TABLET_ENABLE_MULTITOUCHDATA;

			SetProp(hwnd, tabletAtom, (HANDLE)dwHwndTabletProperty);
		}

		typedef BOOL (WINAPI *pfnRegisterTouchWindow)(HWND, ULONG);
		pfnRegisterTouchWindow pRegisterTouchWindow = (pfnRegisterTouchWindow)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "RegisterTouchWindow");
		if (pRegisterTouchWindow != NULL)
			pRegisterTouchWindow(hwnd, TWF_WANTPALM);
	}
#endif

	// create timers
	Timer *frameTimer = new Timer();
	frameTimer->start();
	frameTimer->update();

	Timer *deltaTimer = new Timer();
	deltaTimer->start();
	deltaTimer->update();

	// NOTE: it seems that focus events get lost between CreateWindow() above and ShowWindow() here
	// can be simulated by sleeping and alt-tab for testing.
	// seems to be a Windows bug? if you switch to the window after all of this, no activate or focus events will be received!
	//Sleep(1000);

	// make the window visible
	ShowWindow(hwnd, nCmdShow);
#ifdef WINDOW_MAXIMIZED
	ShowWindow(hwnd, SW_MAXIMIZE);
#endif

	// initialize engine
	WinEnvironment *environment = new WinEnvironment(hwnd, hInstance);
	g_engine = new Engine(environment, lpCmdLine);
	g_engine->loadApp();

	g_bHasFocus = g_bHasFocus && (GetForegroundWindow() == hwnd);	// HACKHACK: workaround (1), see above
	bool wasLaunchedInBackgroundAndWaitingForFocus = !g_bHasFocus;	// HACKHACK: workaround (2), see above

	if (g_bHasFocus)
		g_engine->onFocusGained();

	frameTimer->update();
	deltaTimer->update();

	// main loop
	MSG msg;
	msg.message = WM_NULL;
	unsigned long tickCounter = 0;
	UINT currentRawInputBufferNumBytes = 0;
	unsigned char *currentRawInputBuffer = NULL;
	while (g_bRunning)
	{
		VPROF_MAIN();

		// handle windows message queue
		{
			VPROF_BUDGET("Windows", VPROF_BUDGETGROUP_WNDPROC);

			if (win_mouse_raw_input_buffer.getBool())
			{
				UINT minRawInputBufferNumBytes = 0;
				UINT hr = GetRawInputBuffer(NULL, &minRawInputBufferNumBytes, sizeof(RAWINPUTHEADER));
				if (hr != (UINT)-1 && minRawInputBufferNumBytes > 0)
				{
					// resize buffer up to 1 MB sanity limit (if we lagspike this could easily be hit, 8000 Hz polling rate will produce ~0.12 MB per second)
					const UINT numAlignmentBytes = 8;
					const UINT rawInputBufferNumBytes = clamp<UINT>(minRawInputBufferNumBytes * numAlignmentBytes * 1024, 1, 1024 * 1024);
					if (currentRawInputBuffer == NULL || currentRawInputBufferNumBytes < rawInputBufferNumBytes)
					{
						currentRawInputBufferNumBytes = rawInputBufferNumBytes;
						{
							if (currentRawInputBuffer != NULL)
								delete[] currentRawInputBuffer;
						}
						currentRawInputBuffer = new unsigned char[currentRawInputBufferNumBytes];
					}

					// grab and go through all buffered RAWINPUT events
					hr = GetRawInputBuffer((RAWINPUT*)currentRawInputBuffer, &currentRawInputBufferNumBytes, sizeof(RAWINPUTHEADER));
					if (hr != (UINT)-1)
					{
						RAWINPUT *currentRawInput = (RAWINPUT*)currentRawInputBuffer;
						for (; hr>0; hr--) // (hr = number of rawInputs)
						{
							if (currentRawInput->header.dwType == RIM_TYPEMOUSE)
							{
								const LONG lastX = ((RAWINPUT*)(&((BYTE*)currentRawInput)[numAlignmentBytes]))->data.mouse.lLastX;
								const LONG lastY = ((RAWINPUT*)(&((BYTE*)currentRawInput)[numAlignmentBytes]))->data.mouse.lLastY;
								const USHORT usFlags = ((RAWINPUT*)(&((BYTE*)currentRawInput)[numAlignmentBytes]))->data.mouse.usFlags;

								g_engine->onMouseRawMove(lastX, lastY, (usFlags & MOUSE_MOVE_ABSOLUTE), (usFlags & MOUSE_VIRTUAL_DESKTOP));
							}

							currentRawInput = NEXTRAWINPUTBLOCK(currentRawInput);
						}
					}
				}

				// handle all remaining non-WM_INPUT messages
				while (PeekMessageW(&msg, NULL, 0U, WM_INPUT - 1, PM_REMOVE) != 0 || PeekMessageW(&msg, NULL, WM_INPUT + 1, 0U, PM_REMOVE) != 0)
				{
					TranslateMessage(&msg);
					DispatchMessageW(&msg);
				}
			}
			else
			{
				while (PeekMessageW(&msg, NULL, 0U, 0U, PM_REMOVE) != 0)
				{
					TranslateMessage(&msg);
					DispatchMessageW(&msg);
				}
			}
		}

		// HACKHACK: focus bug workaround (3), see above
		{
			if (wasLaunchedInBackgroundAndWaitingForFocus)
			{
				const bool actuallyGotFocusEvenThoughThereIsNoFocusOrActivateEvent = (GetForegroundWindow() == hwnd);
				if (actuallyGotFocusEvenThoughThereIsNoFocusOrActivateEvent)
				{
					wasLaunchedInBackgroundAndWaitingForFocus = false;

					g_bHasFocus = true;
					if (!g_engine->hasFocus())
						g_engine->onFocusGained();
				}
			}
		}

		// IPC (2)
/*
		if (ipcSharedMemory != NULL && ((IPCState*)ipcSharedMemory)->state == IPC_STATE::MESSAGE)
		{
			g_engine->debugLog("IPC: Received message @ %f\n", g_engine->getTime());

			switch (((IPCState*)ipcSharedMemory)->type)
			{
			case IPC_MESSAGE_TYPE::URL:
				char *content = (((char*)ipcSharedMemory) + sizeof(IPCState));
				const size_t length = std::min(strlen(content), sharedMemorySize - sizeof(IPCState) - 2);
				const UString ustring = UString(content, length);

				g_engine->debugLog("IPC: Received URL %s\n", ustring.toUtf8());

				if (ustring.find("mcengine://") == 0)
					Console::processCommand(ustring.substr(11));

				break;
			}

			((IPCState*)ipcSharedMemory)->state = IPC_STATE::IDLE;
		}
*/

		// update
		{
			deltaTimer->update();
			g_engine->setFrameTime(deltaTimer->getDelta());

			if (g_bUpdate)
				g_engine->onUpdate();
		}

		const bool inBackground = (g_bMinimized || !g_bHasFocus);

		// draw
		{
			const unsigned long interleaved = fps_max_background_interleaved.getInt();
			if (g_bDraw && (!inBackground || interleaved < 2 || tickCounter % interleaved == 0))
			{
				g_bDrawing = true;
				{
					g_engine->onPaint();
				}
				g_bDrawing = false;
			}
		}

		tickCounter++;

		// delay the next frame
		{
			VPROF_BUDGET("FPSLimiter", VPROF_BUDGETGROUP_SLEEP);

			frameTimer->update();

			if ((!fps_unlimited.getBool() && fps_max.getInt() > 0) || inBackground)
			{
				double delayStart = frameTimer->getElapsedTime();
				double delayTime;
				if (inBackground)
					delayTime = (1.0 / (double)fps_max_background.getFloat()) - frameTimer->getDelta();
				else
					delayTime = (1.0 / (double)fps_max.getFloat()) - frameTimer->getDelta();

				const bool didSleep = delayTime > 0.0;
				while (delayTime > 0.0)
				{
					if (inBackground) // real waiting (very inaccurate, but very good for little background cpu utilization)
						Sleep((int)((1.0f / fps_max_background.getFloat())*1000.0f));
					else // more or less "busy" waiting, but giving away the rest of the timeslice at least
						Sleep(0); // yes, there is a zero in there

					// decrease the delayTime by the time we spent in this loop
					// if the loop is executed more than once, note how delayStart now gets the value of the previous iteration from getElapsedTime()
					// this works because the elapsed time is only updated in update(). now we can easily calculate the time the Sleep() took and subtract it from the delayTime
					delayStart = frameTimer->getElapsedTime();
					frameTimer->update();
					delayTime -= (frameTimer->getElapsedTime() - delayStart);
				}

				if (!didSleep && fps_max_yield.getBool())
					Sleep(0); // yes, there is a zero in there
			}
			else if (fps_unlimited_yield.getBool())
				Sleep(0); // yes, there is a zero in there
		}
	}

	// uninitialize RealTimeStylus (COM)
#ifdef MCENGINE_WINDOWS_REALTIMESTYLUS_SUPPORT
	UninitRealTimeStylus();
	CoUninitialize();
#endif

	// release the timers
	SAFE_DELETE(frameTimer);
	SAFE_DELETE(deltaTimer);

	const bool isRestartScheduled = environment->isRestartScheduled();

	// release engine
	SAFE_DELETE(g_engine);

	// finally, destroy the window
	DestroyWindow(hwnd);

	// IPC (3)
/*
	{
		if (ipcSharedMemory != NULL)
			UnmapViewOfFile(ipcSharedMemory);

		if (ipcMappedFile != NULL)
			CloseHandle(ipcMappedFile);
	}
*/

	// handle potential restart
	if (isRestartScheduled)
	{
		wchar_t full_path[MAX_PATH];
		GetModuleFileNameW(NULL, full_path, MAX_PATH);
		STARTUPINFOW startupInfo = {sizeof(STARTUPINFOW)};
		PROCESS_INFORMATION processInfo;
		CreateProcessW(full_path, NULL, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &startupInfo, &processInfo);
	}

	return 0;
}

#endif

#endif

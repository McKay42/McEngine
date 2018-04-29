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

extern int mainSDL(int argc, char *argv[]);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	return mainSDL(0, NULL);
}

#else



#ifdef MCENGINE_WINDOWS_TOUCH_SUPPORT
#include <winuser.h>
typedef WINBOOL (WINAPI *PGPI)(UINT32 pointerId, POINTER_INFO *pointerInfo);
PGPI g_GetPointerInfo = (PGPI)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "GetPointerInfo");
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
#include "ConVar.h"
#include "Timer.h"
#include "Mouse.h"

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


#define WM_MOUSEHWHEEL 0x020E

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

std::vector<unsigned int> g_vTouches;

extern "C" { // force switch to the high performance gpu in multi-gpu systems (mostly laptops)
__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001; // http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
__declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001; // https://community.amd.com/thread/169965
}

ConVar fps_max("fps_max", 60.0f);
ConVar fps_max_background("fps_max_background", 30.0f);
ConVar fps_unlimited("fps_unlimited", false);

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

#if defined(WINDOW_FRAMELESS) && !defined(WINDOW_GHOST)

    	// ignore
    	/*case WM_ERASEBKGND:
    		return 1;*/

    	// window border paint
    	case WM_NCPAINT:
    		{
    			// draw beautifully blurred windows 7 background + shadows
    			return DefWindowProc(hwnd, msg, wParam, lParam);

    			// draw white rectangle over everything except the shadows
    			/*
				HDC hdc;
				hdc = GetDCEx(hwnd, (HRGN)wParam, DCX_WINDOW|DCX_INTERSECTRGN);
				PAINTSTRUCT ps;
				hdc = BeginPaint(hwnd, &ps);
				RECT wr;
				GetClientRect(hwnd, &wr);
				HBRUSH br;
				br = GetSysColorBrush(COLOR_WINDOW);
				FillRect(hdc, &wr, br);
				ReleaseDC(hwnd, hdc);
				*/
    		}
    		/// return 0;

		case WM_NCCALCSIZE:
			{
				LPNCCALCSIZE_PARAMS pncc = (LPNCCALCSIZE_PARAMS)lParam;

				/*
				debugLog("new Rectangle: top = %i, right = %i, bottom = %i, left = %i\n",pncc->rgrc[0].top,pncc->rgrc[0].right,pncc->rgrc[0].bottom,pncc->rgrc[0].left);
				debugLog("old Rectangle: top = %i, right = %i, bottom = %i, left = %i\n",pncc->rgrc[1].top,pncc->rgrc[1].right,pncc->rgrc[1].bottom,pncc->rgrc[1].left);
				debugLog("client Rectan: top = %i, right = %i, bottom = %i, left = %i\n",pncc->rgrc[2].top,pncc->rgrc[2].right,pncc->rgrc[2].bottom,pncc->rgrc[2].left);
				*/

				if (IsZoomed(hwnd))
				{
					pncc->rgrc[0].right += pncc->rgrc[0].left;
					pncc->rgrc[0].bottom += pncc->rgrc[0].top;
					pncc->rgrc[0].top = 0;
					pncc->rgrc[0].left = 0;
				}

				// "When wParam is TRUE, simply returning 0 without processing the NCCALCSIZE_PARAMS rectangles will cause the client area to resize to the size of the window,
				// including the window frame. This will remove the window frame and caption items from your window, leaving only the client area displayed."
				if (wParam == TRUE)
					return TRUE;
			}
			return 0;

		// window border
		case WM_NCHITTEST:
			{
				if (g_engine != NULL)
				{
					long val = DefWindowProc(hwnd, msg, wParam, lParam);
					if (val != HTCLIENT)
						return val;

					// fake window moving
					Vector2 mousePos = g_engine->getEnvironment()->getMousePos();
					McRect dragging = McRect(40, 4, engine->getScreenWidth()-80, 20); // HACKHACK: hardcoded 40 for stuff
					if (dragging.contains(mousePos))
						val = HTCAPTION;

					if (IsZoomed(hwnd))
						return val;

					// fake window resizing
					McRect resizeN = McRect(0, 0, engine->getScreenWidth(), 4);
					McRect resizeW = McRect(0, 0, 4, engine->getScreenHeight());
					McRect resizeO = McRect(engine->getScreenWidth()-5, 0, engine->getScreenWidth(), engine->getScreenHeight());
					McRect resizeS = McRect(0, engine->getScreenHeight()-5, engine->getScreenWidth(), engine->getScreenHeight());
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
					return DefWindowProc(hwnd, msg, wParam, lParam);
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
				ValidateRect(hwnd, NULL);
				/*
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hwnd,&ps);

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
					if (g_engine != NULL)
						printf("WndProc() : WM_ACTIVATE, not minimized\n");

					g_bUpdate = true;
					g_bDraw = true;
				}
				else
				{
					if (g_engine != NULL)
						printf("WndProc() : WM_ACTIVATE, minimized\n");

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
				g_engine->onFocusGained();
			}
			break;

		case WM_KILLFOCUS:
			g_bHasFocus = false;
			if (g_bRunning && g_engine != NULL)
			{
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

		// left mouse button
		case WM_LBUTTONDOWN:
			if (g_engine != NULL && (!win_realtimestylus->getBool() || !IsPenEvent(GetMessageExtraInfo()))) // if realtimestylus support is enabled, all clicks are handled by it and not here
				g_engine->onMouseLeftChange(true);
			SetCapture(hwnd);
			break;
		case WM_LBUTTONUP:
			if (g_engine != NULL)
				g_engine->onMouseLeftChange(false);
			ReleaseCapture();
			break;

		// middle mouse button
		case WM_MBUTTONDOWN:
			if (g_engine != NULL)
				g_engine->onMouseMiddleChange(true);
			SetCapture(hwnd);
			break;
		case WM_MBUTTONUP:
			if (g_engine != NULL)
				g_engine->onMouseMiddleChange(false);
			ReleaseCapture();
			break;

		// right mouse button
		case WM_RBUTTONDOWN:
			if (g_engine != NULL && (!win_realtimestylus->getBool() || !IsPenEvent(GetMessageExtraInfo()))) // if realtimestylus support is enabled, all pen clicks are handled by it and not here
				g_engine->onMouseRightChange(true);
			SetCapture(hwnd);
			break;
		case WM_RBUTTONUP:
			if (g_engine != NULL)
				g_engine->onMouseRightChange(false);
			ReleaseCapture();
			break;

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
			break;

			// this logic below would mean that we have to handle the cursor when moving from resizing into the client area
			// seems very annoying; unfinished
			/*
			if (LOWORD(lParam) == HTCLIENT) // if we are inside the client area, we handle the cursor
				return TRUE;
			else
				break; // if not, let DefWindowProc do its thing
			*/

		// raw input experiments
		/*
		case WM_INPUT_DEVICE_CHANGE:
			if (g_engine != NULL)
			{
				RID_DEVICE_INFO deviceInfo[1];
				deviceInfo->cbSize = sizeof(RID_DEVICE_INFO);
				unsigned int devInfoSize = sizeof(RID_DEVICE_INFO);

				char deviceName[255];
				unsigned int numChars = 255;
				GetRawInputDeviceInfo((void*)lParam, RIDI_DEVICENAME, deviceName, &numChars);
				debugLog("WINDOWS: RawInputDevice %s\n", deviceName);

				UString message = "WINDOWS: WM_INPUT_DEVICE_CHANGE of ";
				if (wParam == GIDC_ARRIVAL)
				{
					// WARNING: using GetRawInputDeviceInfo() directly in the if-block does NOT work! the condition will never be true for some reason >:(
					int result = GetRawInputDeviceInfo((void*)lParam, RIDI_DEVICEINFO, deviceInfo, &devInfoSize);
					if (result > -1)
					{
						if (deviceInfo->dwType == RIM_TYPEMOUSE)
							message.append("RIM_TYPEMOUSE");
						else if (deviceInfo->dwType == RIM_TYPEKEYBOARD)
							message.append("RIM_TYPEKEYBOARD");
						else if (deviceInfo->dwType == RIM_TYPEHID)
							message.append("RIM_TYPEHID");

						message.append(" with GIDC_ARRIVAL\n");
						engine->debugLog(0xff00ff00, message.toUtf8());
					}
				}
				else if (wParam == GIDC_REMOVAL)
				{
					message.append("GIDC_REMOVAL\n");
					engine->debugLog(0xffff0000, message.toUtf8());
				}

				unsigned int numDevices = 0;
				GetRawInputDeviceList(NULL, &numDevices, sizeof(RAWINPUTDEVICELIST));
				debugLog("numDevices = %i\n", numDevices);

				RAWINPUTDEVICELIST devices[numDevices];
				if (GetRawInputDeviceList(devices, &numDevices, sizeof(RAWINPUTDEVICELIST)) != (UINT)-1)
				{
					for (int i=0; i<numDevices; i++)
					{
						if (devices[i].dwType == RIM_TYPEMOUSE)
							debugLog("got mouse @ %i\n", i);
					}
				}
			}
			return 0;
		*/

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
					for (int i=0; i<g_vTouches.size(); i++)
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
					for (int i=0; i<g_vTouches.size(); i++)
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
				UINT dwSize;

				GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
				LPBYTE lpb[dwSize]; // the msdn example uses heap allocation here, but stack should be faster

				if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize && g_engine != NULL)
					debugLog("WARNING: GetRawInputData() does not return the correct size!!!\n");

				RAWINPUT* raw = (RAWINPUT*)lpb;

				if (raw->header.dwType == RIM_TYPEMOUSE && g_engine != NULL)
				{
					g_engine->onMouseRawMove(raw->data.mouse.lLastX, raw->data.mouse.lLastY, (raw->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE), (raw->data.mouse.usFlags & MOUSE_VIRTUAL_DESKTOP));
					return 0;
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
					GetClientRect(hwnd,&rect);
					g_engine->onResolutionChange(Vector2(rect.right,rect.bottom));
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
				GetClientRect(hwnd,&rect);
				g_engine->requestResolutionChange(Vector2(rect.right,rect.bottom));
			}
			break;

		// resize limit
		case WM_GETMINMAXINFO:
			{
				LPMINMAXINFO pMMI = (LPMINMAXINFO)lParam;
				pMMI->ptMinTrackSize.x = WINDOW_WIDTH_MIN;
				pMMI->ptMinTrackSize.y = WINDOW_HEIGHT_MIN;
			}
			break;
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

typedef BOOL (WINAPI *pfnImmDisableIME)(DWORD);
void DisableIME()
{
    HMODULE hImm32 = LoadLibrary("imm32.dll");
    if (hImm32 == NULL)
    	return;

    pfnImmDisableIME pImmDisableIME = (pfnImmDisableIME)GetProcAddress(hImm32, "ImmDisableIME");
    if (pImmDisableIME == NULL)
    {
        FreeLibrary(hImm32);
        return;
    }

    pImmDisableIME(-1);
    FreeLibrary(hImm32);
}



//********************//
//	Main entry point  //
//********************//

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// NOTE: add -mwindows to linker options to disable console window if compiling with Eclipse

	// disable IME text input
	DisableIME();

	// enable fancy themed windows controls (v6+), requires McEngine.exe.manifest AND linking to comctl32, for fucks sake
	// only noticeable in MessageBox()-es and a few other dialogs
	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(icc);
	icc.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&icc);

	// if supported (>= Windows Vista), enable DPI awareness so that GetSystemMetrics returns correct values
	// without this, on e.g. 150% scaling, the screen pixels of a 1080p monitor would be reported by GetSystemMetrics(SM_CXSCREEN/SM_CYSCREEN) as only 720p!
	typedef WINBOOL (WINAPI *PSPDA)(void);
	PSPDA g_SetProcessDPIAware = (PSPDA)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "SetProcessDPIAware");
	if (g_SetProcessDPIAware != NULL)
		g_SetProcessDPIAware();

	// prepare window class
    WNDCLASSEXW wc;

    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = NULL;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)CreateSolidBrush(0x00000000);
    wc.lpszMenuName  = NULL;
	wc.lpszClassName = WINDOW_TITLE;
    //wc.hIconSm       = LoadIcon(hInstance, MAKEINTRESOURCE(1)); // load icon named "1" in /Main/WinIcon.rc file, must link to WinIcon.o which was created with windres
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

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

	MARGINS m = {1,1,1,1};
	printf("DwmExtendFrameIntoClientArea() = %x\n",(int)DwmExtendFrameIntoClientArea(hwnd, &m));
	SetWindowPos(hwnd, NULL, 0,0,0,0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);

	/*
	DWM_BLURBEHIND bb = {0};
	bb.dwFlags = DWM_BB_ENABLE;
	bb.fEnable = 0;
	bb.hRgnBlur = NULL;
	DwmEnableBlurBehindWindow(hwnd, &bb);
	*/

#endif

#ifdef WINDOW_GHOST

	DWM_BLURBEHIND bb = {0};
	HRGN hRgn = CreateRectRgn(0, 0, -1, -1);

	bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
	bb.hRgnBlur = hRgn;
	bb.fEnable = TRUE;

	printf("DwmEnableBlurBehindWindow() = %x\n", (int)DwmEnableBlurBehindWindow(hwnd, &bb));

	SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

#endif

	// get the screen refresh rate, and set fps_max to that as default
	DEVMODE lpDevMode;
	memset(&lpDevMode, 0, sizeof(DEVMODE));
	lpDevMode.dmSize = sizeof(DEVMODE);
	lpDevMode.dmDriverExtra = 0;

	if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &lpDevMode))
	{
		float displayFrequency = static_cast<float>(lpDevMode.dmDisplayFrequency);
		///printf("Display Refresh Rate is %.2f Hz, setting fps_max to %i.\n\n", displayFrequency, (int)displayFrequency);
		fps_max.setValue((int)displayFrequency);
	}

	// initialize raw input
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

	// initialize RealTimeStylus (COM)
#ifdef MCENGINE_WINDOWS_REALTIMESTYLUS_SUPPORT
	HRESULT hr = CoInitialize(NULL);
	if (hr == S_OK || hr == S_FALSE) // if we initialized successfully, or if we are already initialized
	{
		if (!InitRealTimeStylus(hInstance, hwnd))
			printf("WARNING: Couldn't InitRealTimeStylus()! RealTimeStylus is not going to work.\n");
	}
	else
		printf("WARNING: Couldn't CoInitialize()! RealTimeStylus is not going to work.\n");
#endif

    // create timers
    Timer *frameTimer = new Timer();
    frameTimer->start();
    frameTimer->update();

    Timer *deltaTimer = new Timer();
    deltaTimer->start();
    deltaTimer->update();

	// make the window visible
	ShowWindow(hwnd, nCmdShow);
#ifdef WINDOW_MAXIMIZED
	ShowWindow(hwnd, SW_MAXIMIZE);
#endif

    // initialize engine
	WinEnvironment *environment = new WinEnvironment(hwnd, hInstance);
    g_engine = new Engine(environment, lpCmdLine);
    g_engine->loadApp();

    if (g_bHasFocus)
    {
    	g_engine->onFocusGained();
    }

	// main loop
	while (g_bRunning)
	{
		frameTimer->update();

		// handle windows message queue
		MSG msg;
		msg.message = WM_NULL;
		while (PeekMessageW(&msg, NULL, 0U, 0U, PM_REMOVE) != 0)
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}

		// update
		if (g_bUpdate)
			g_engine->onUpdate();

		// draw
		if (g_bDraw)
		{
			g_bDrawing = true;
				g_engine->onPaint();
			g_bDrawing = false;
		}

		// delay the next frame
		frameTimer->update();
		deltaTimer->update();

		engine->setFrameTime(deltaTimer->getDelta());

		const bool inBackground = g_bMinimized || !g_bHasFocus;
		if (!fps_unlimited.getBool() || inBackground)
		{
			double delayStart = frameTimer->getElapsedTime();
			double delayTime;
			if (inBackground)
				delayTime = (1.0 / (double)fps_max_background.getFloat()) - frameTimer->getDelta();
			else
				delayTime = (1.0 / (double)fps_max.getFloat()) - frameTimer->getDelta();

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

	bool isRestartScheduled = environment->isRestartScheduled();

    // release engine
    SAFE_DELETE(g_engine);

	// finally, destroy the window
	DestroyWindow(hwnd);

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

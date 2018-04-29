//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		windows
//
// $NoKeywords: $winenv
//===============================================================================//

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

#include "WinEnvironment.h"
#include "Engine.h"
#include "ConVar.h"
#include "Mouse.h"

#include "NullGraphicsInterface.h"
#include "DirectX11Interface.h"
#include "WinGLLegacyInterface.h"
#include "WinGL3Interface.h"
#include "WinSWGraphicsInterface.h"
#include "VulkanGraphicsInterface.h"
#include "WinContextMenu.h"

#include <Lmcons.h>
#include <Shlobj.h>

#include <tchar.h>
#include <string>

#define WINDOW_RESIZABLE

bool g_bCursorVisible = true;

bool WinEnvironment::m_bResizable = true;
std::vector<McRect> WinEnvironment::m_vMonitors;

WinEnvironment::WinEnvironment(HWND hwnd, HINSTANCE hinstance) : Environment()
{
	m_hwnd = hwnd;
	m_hInstance = hinstance;

	m_mouseCursor = m_mouseCursorArrow = LoadCursor(NULL, IDC_ARROW);
	m_bFullScreen = false;
	m_vWindowSize = getWindowSize();
	m_bCursorClipped = false;

	m_bIsRestartScheduled = false;

	// init
	enumerateMonitors();

	if (m_vMonitors.size() < 1)
	{
		debugLog("WARNING: No monitors found! Adding default monitor ...\n");
		m_vMonitors.push_back(McRect(0, 0, m_vWindowSize.x, m_vWindowSize.y));
	}
}

void WinEnvironment::update()
{
	m_bIsCursorInsideWindow = McRect(0, 0, engine->getScreenWidth(), engine->getScreenHeight()).contains(getMousePos());
}

Graphics *WinEnvironment::createRenderer()
{
	//return new NullGraphicsInterface();
	//return new VulkanGraphicsInterface();
	//return new WinSWGraphicsInterface(m_hwnd);
	return new WinGLLegacyInterface(m_hwnd);
	//return new WinGL3Interface(m_hwnd);
	//return new DirectX11Interface(m_hwnd);
}

ContextMenu *WinEnvironment::createContextMenu()
{
	return new WinContextMenu();
}

Environment::OS WinEnvironment::getOS()
{
	return Environment::OS::OS_WINDOWS;
}

void WinEnvironment::shutdown()
{
	SendMessage(m_hwnd, WM_CLOSE, 0, 0);
}

void WinEnvironment::restart()
{
	m_bIsRestartScheduled = true;
	shutdown();
}

void WinEnvironment::sleep(unsigned int us)
{
	Sleep(us/1000);
}

UString WinEnvironment::getUsername()
{
	DWORD username_len = UNLEN+1;
	wchar_t username[username_len];

	if (GetUserNameW(username, &username_len))
		return UString(username);

	return UString("");
}

UString WinEnvironment::getUserDataPath()
{
	wchar_t path[PATH_MAX];

	if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path)))
		return UString(path);

	return UString("");
}

UString WinEnvironment::getExecutablePath()
{
	wchar_t path[MAX_PATH];

	if (GetModuleFileNameW(NULL, path, MAX_PATH))
		return UString(path);

	return UString("");
}

bool WinEnvironment::fileExists(UString filename)
{
	WIN32_FIND_DATAW FindFileData;
	HANDLE handle = FindFirstFileW(filename.wc_str(), &FindFileData);
	if (handle == INVALID_HANDLE_VALUE)
		return std::ifstream(filename.toUtf8()).good();
	else
	{
		FindClose(handle);
		return true;
	}
}

bool WinEnvironment::directoryExists(UString filename)
{
	DWORD dwAttrib = GetFileAttributesW(filename.wc_str());
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool WinEnvironment::createDirectory(UString directoryName)
{
	return CreateDirectoryW(directoryName.wc_str(), NULL);
}

bool WinEnvironment::renameFile(UString oldFileName, UString newFileName)
{
	return MoveFileW(oldFileName.wc_str(), newFileName.wc_str());
}

bool WinEnvironment::deleteFile(UString filePath)
{
	return DeleteFileW(filePath.wc_str());
}

UString WinEnvironment::getClipBoardText()
{
	UString result = "";
	HANDLE clip = NULL;
	if (OpenClipboard(NULL))
	{
		clip = GetClipboardData(CF_UNICODETEXT);
		wchar_t *pchData = (wchar_t*)GlobalLock(clip);

		if (pchData != NULL)
			result = UString(pchData);

		GlobalUnlock(clip);
		CloseClipboard();
	}
	return result;
}

void WinEnvironment::setClipBoardText(UString text)
{
	if (text.length() < 1) return;

	if (OpenClipboard(NULL) && EmptyClipboard())
	{
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, ((text.length() + 1) * sizeof(WCHAR)));

		if (hMem == NULL)
        {
			debugLog("ERROR: hglbCopy == NULL!\n");
            CloseClipboard();
            return;
        }

		memcpy(GlobalLock(hMem), text.wc_str(), (text.length() + 1) * sizeof(WCHAR));
        GlobalUnlock(hMem);

		SetClipboardData(CF_UNICODETEXT, hMem);

		CloseClipboard();
	}
}

void WinEnvironment::openURLInDefaultBrowser(UString url)
{
	ShellExecuteW(m_hwnd, L"open", url.wc_str(), NULL, NULL, SW_SHOW);
}

UString WinEnvironment::openFileWindow(const char *filetypefilters, UString title, UString initialpath)
{
	disableFullscreen();

	OPENFILENAME fn;
	ZeroMemory(&fn, sizeof(fn));

	char fileNameBuffer[255];

	// fill it
	fn.lStructSize = sizeof(fn);
	fn.hwndOwner = NULL;
	fn.lpstrFile = fileNameBuffer;
	fn.lpstrFile[0] = '\0';
	fn.nMaxFile = sizeof(fileNameBuffer);
	fn.lpstrFilter = filetypefilters;
	fn.nFilterIndex = 1;
	fn.lpstrFileTitle = NULL;
	fn.nMaxFileTitle = 0;
	fn.lpstrTitle = title.length() > 1 ? title.toUtf8() : NULL;
	fn.lpstrInitialDir = initialpath.length() > 1 ? initialpath.toUtf8() : NULL;
	fn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_ENABLESIZING;

	// open the dialog
	GetOpenFileName(&fn);

	return fn.lpstrFile;
}

UString WinEnvironment::openFolderWindow(UString title, UString initialpath)
{
	disableFullscreen();

	OPENFILENAME fn;
	ZeroMemory(&fn, sizeof(fn));

	char fileNameBuffer[255];

	fileNameBuffer[0] = 's';
	fileNameBuffer[1] = 'k';
	fileNameBuffer[2] = 'i';
	fileNameBuffer[3] = 'n';
	fileNameBuffer[4] = '.';
	fileNameBuffer[5] = 'i';
	fileNameBuffer[6] = 'n';
	fileNameBuffer[7] = 'i';
	fileNameBuffer[8] = '\0';

	// fill it
	fn.lStructSize = sizeof(fn);
	fn.hwndOwner = NULL;
	fn.lpstrFile = fileNameBuffer;
	///fn.lpstrFile[0] = '\0';
	fn.nMaxFile = sizeof(fileNameBuffer);
	fn.nFilterIndex = 1;
	fn.lpstrFileTitle = NULL;
	fn.lpstrTitle = title.length() > 1 ? title.toUtf8() : NULL;
	fn.lpstrInitialDir = initialpath.length() > 1 ? initialpath.toUtf8() : NULL;
	fn.Flags = OFN_PATHMUSTEXIST | OFN_ENABLESIZING;

	// open the dialog
	GetOpenFileName(&fn);

	return fn.lpstrFile;
}

std::vector<UString> WinEnvironment::getFilesInFolder(UString folder)
{
	folder.append("*.*");
	WIN32_FIND_DATAW data;
	std::wstring buffer;
	std::vector<UString> files;

	HANDLE handle = FindFirstFileW(folder.wc_str(), &data);

	while (true)
	{
		std::wstring filename(data.cFileName);

		if (filename != buffer)
		{
			buffer = filename;

			if (filename.length() > 0)
			{
				if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) //directory
				{
					///if (filename.length() > 0)
					///	folders.push_back(filename.c_str());
				}
				else // file
				{
					if (filename.length() > 0)
						files.push_back(filename.c_str());
				}
			}

			FindNextFileW(handle, &data);
		}
		else
			break;
	}

	FindClose(handle);
	return files;
}

std::vector<UString> WinEnvironment::getFoldersInFolder(UString folder)
{
	folder.append("*.*");
	WIN32_FIND_DATAW data;
	std::wstring buffer;
	std::vector<UString> folders;

	HANDLE handle = FindFirstFileW(folder.wc_str(), &data);

	while (true)
	{
		std::wstring filename(data.cFileName);

		if (filename != buffer)
		{
			buffer = filename;

			if (filename.length() > 0)
			{
				if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) //directory
				{
					if (filename.length() > 0)
						folders.push_back(filename.c_str());
				}
				else // file
				{
					///if (filename.length() > 0)
					///	files.push_back(filename.c_str());
				}
			}

			FindNextFileW(handle, &data);
		}
		else
			break;
	}

	FindClose(handle);
	return folders;
}

std::vector<UString> WinEnvironment::getLogicalDrives()
{
	std::vector<UString> drives;
	DWORD dwDrives = GetLogicalDrives();

	for (int i=0; i<26; i++)
	{
		// 26 letters in [A..Z] range
		if (dwDrives & 1)
		{
			UString driveName = UString::format("%c", 'A'+i);
			UString driveExecName = driveName;
			driveExecName.append(":/");

			UString driveNameForGetDriveFunction = driveName;
			driveNameForGetDriveFunction.append(":\\");

			DWORD attributes = GetFileAttributes(driveNameForGetDriveFunction.toUtf8());

			//debugLog("checking %s, type = %i, free clusters = %lu\n", driveNameForGetDriveFunction.toUtf8(), GetDriveType(driveNameForGetDriveFunction.toUtf8()), attributes);

			// check if the drive is valid, and if there is media in it (e.g. ignore empty dvd drives)
			if (GetDriveType(driveNameForGetDriveFunction.toUtf8()) > DRIVE_NO_ROOT_DIR && attributes != INVALID_FILE_ATTRIBUTES)
			{
				if (driveExecName.length() > 0)
					drives.push_back(driveExecName);
			}
		}

		dwDrives >>= 1; // next bit
	}

	return drives;
}

UString WinEnvironment::getFolderFromFilePath(UString filepath)
{
	char *aString = (char*)filepath.toUtf8();
	path_strip_filename(aString);
	return aString;
}

UString WinEnvironment::getFileExtensionFromFilePath(UString filepath, bool includeDot)
{
	int idx = filepath.findLast(".");
	if (idx != -1)
		return filepath.substr(idx+1);
	else
		return UString("");
}

void WinEnvironment::showMessageInfo(UString title, UString message)
{
	bool wasFullscreen = m_bFullScreen;
	handleShowMessageFullscreen();
	MessageBoxW(m_hwnd, message.wc_str(), title.wc_str(), MB_ICONINFORMATION | MB_OK);
	if (wasFullscreen)
	{
		maximize();
		enableFullscreen();
	}
}

void WinEnvironment::showMessageWarning(UString title, UString message)
{
	bool wasFullscreen = m_bFullScreen;
	handleShowMessageFullscreen();
	MessageBoxW(m_hwnd, message.wc_str(), title.wc_str(), MB_ICONWARNING | MB_OK);
	if (wasFullscreen)
	{
		maximize();
		enableFullscreen();
	}
}

void WinEnvironment::showMessageError(UString title, UString message)
{
	bool wasFullscreen = m_bFullScreen;
	handleShowMessageFullscreen();
	MessageBoxW(m_hwnd, message.wc_str(), title.wc_str(), MB_ICONERROR | MB_OK);
	if (wasFullscreen)
	{
		maximize();
		enableFullscreen();
	}
}

void WinEnvironment::showMessageErrorFatal(UString title, UString message)
{
	bool wasFullscreen = m_bFullScreen;
	handleShowMessageFullscreen();
	MessageBox(m_hwnd, message.toUtf8(), title.toUtf8(), MB_ICONSTOP | MB_OK);
	if (wasFullscreen)
	{
		maximize();
		enableFullscreen();
	}
}

void WinEnvironment::focus()
{
	SetForegroundWindow(m_hwnd);
}

void WinEnvironment::center()
{
	RECT clientRect;
	GetClientRect(m_hwnd, &clientRect);

	// get nearest monitor and center on that, build windowed pos + size
	const McRect desktopRect = getDesktopRect();
	int width = std::abs(clientRect.right - clientRect.left);
	int height = std::abs(clientRect.bottom - clientRect.top);
	int xPos = desktopRect.getX() + (desktopRect.getWidth()/2) - (int)(width/2);
	int yPos = desktopRect.getY() + (desktopRect.getHeight()/2) - (int)(height/2);

	// calculate window size for client size (to respect borders etc.)
	RECT clientArea;
	clientArea.left = xPos;
	clientArea.top = yPos;
	clientArea.right = xPos + width;
	clientArea.bottom = yPos + height;
	AdjustWindowRect(&clientArea, getWindowStyleWindowed(), FALSE);

	// set window pos as prev pos, apply it
	xPos = clientArea.left;
	yPos = clientArea.top;
	width = std::abs(clientArea.right - clientArea.left);
	height = std::abs(clientArea.bottom - clientArea.top);
	m_vLastWindowPos.x = xPos;
	m_vLastWindowPos.y = yPos;
	MoveWindow(m_hwnd, xPos, yPos, width, height, FALSE); // non-client width/height!
}

void WinEnvironment::minimize()
{
	ShowWindow(m_hwnd, SW_MINIMIZE);
}

void WinEnvironment::maximize()
{
	ShowWindow(m_hwnd, SW_MAXIMIZE);
}

void WinEnvironment::enableFullscreen()
{
	if (m_bFullScreen) return;

	// backup prev window pos + size
	RECT rect;
	GetWindowRect(m_hwnd, &rect);
	m_vLastWindowPos.x = rect.left;
	m_vLastWindowPos.y = rect.top;
	m_vLastWindowSize = m_vWindowSize;

	// get nearest monitor, build fullscreen resolution
	const McRect desktopRect = getDesktopRect();
	const int width = desktopRect.getWidth();
	const int height = desktopRect.getHeight() + (m_bFullscreenWindowedBorderless ? 1 : 0);

	// and apply everything (move + resize)
	SetWindowLongPtr(m_hwnd, GWL_STYLE, getWindowStyleFullscreen());
	MoveWindow(m_hwnd, (int)(desktopRect.getX()), (int)(desktopRect.getY()), width, height, FALSE);

	m_bFullScreen = true;
}

void WinEnvironment::disableFullscreen()
{
	if (!m_bFullScreen) return;

	// clamp window size to monitor
	const McRect desktopRect = getDesktopRect();
	m_vLastWindowSize.x = std::min(m_vLastWindowSize.x, desktopRect.getWidth());
	m_vLastWindowSize.y = std::min(m_vLastWindowSize.y, desktopRect.getHeight());

	SetWindowLongPtr(m_hwnd, GWL_STYLE, getWindowStyleWindowed());
	MoveWindow(m_hwnd, (int)m_vLastWindowPos.x, (int)m_vLastWindowPos.y, (int)m_vLastWindowSize.x, (int)m_vLastWindowSize.y, FALSE); // non-client width/height!

	m_bFullScreen = false;
}

void WinEnvironment::setWindowTitle(UString title)
{
	SetWindowText(m_hwnd, title.toUtf8());
}

void WinEnvironment::setWindowPos(int x, int y)
{
	SetWindowPos(m_hwnd, m_hwnd, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
}

void WinEnvironment::setWindowSize(int width, int height)
{
	// backup last window pos
	RECT rect;
	GetWindowRect(m_hwnd, &rect);
	m_vLastWindowPos.x = rect.left;
	m_vLastWindowPos.y = rect.top;

	// request window size based on client size
	rect.left = 0;
	rect.top = 0;
	rect.right = width;
	rect.bottom = height;
	AdjustWindowRect(&rect, getWindowStyleWindowed(), FALSE);

	// build new size, set it as the last size
	m_vWindowSize.x = std::abs(rect.right - rect.left);
	m_vWindowSize.y = std::abs(rect.bottom - rect.top);
	m_vLastWindowSize = m_vWindowSize;

	MoveWindow(m_hwnd, (int)m_vLastWindowPos.x, (int)m_vLastWindowPos.y, (int)m_vWindowSize.x, (int)m_vWindowSize.y, FALSE); // non-client width/height!
}

void WinEnvironment::setWindowResizable(bool resizable)
{
	m_bResizable = resizable;
	SetWindowLongPtr(m_hwnd, GWL_STYLE, getWindowStyleWindowed());
}

void WinEnvironment::setWindowGhostCorporeal(bool corporeal)
{
	LONG_PTR exStyle = 0;
	if (corporeal)
		exStyle = WS_EX_WINDOWEDGE;
	else
		exStyle = WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT;

	SetWindowLongPtr(m_hwnd, GWL_EXSTYLE, exStyle);
}

void WinEnvironment::setMonitor(int monitor)
{
	monitor = clamp<int>(monitor, 0, m_vMonitors.size()-1);
	if (monitor == getMonitor()) return;

	const McRect desktopRect = m_vMonitors[monitor];
	const bool wasFullscreen = m_bFullScreen;

	if (wasFullscreen)
		disableFullscreen();

	// build new window size, clamp to monitor size (otherwise the borders would be hidden offscreen)
	RECT windowRect;
	GetWindowRect(m_hwnd, &windowRect);
	const Vector2 windowSize = Vector2(std::abs((int)(windowRect.right - windowRect.left)), std::abs((int)(windowRect.bottom - windowRect.top)));
	const int width = std::min((int)windowSize.x, (int)desktopRect.getWidth());
	const int height = std::min((int)windowSize.y, (int)desktopRect.getHeight());

	// move and resize, force center
	MoveWindow(m_hwnd, desktopRect.getX(), desktopRect.getY(), width, height, FALSE); // non-client width/height!
	center();

	if (wasFullscreen)
		enableFullscreen();
}

Vector2 WinEnvironment::getWindowPos()
{
	POINT p;
	p.x = 0;
	p.y = 0;
	ClientToScreen(m_hwnd, &p); // this respects the window border, because the engine only works in client coordinates
	return Vector2(p.x, p.y);
}

Vector2 WinEnvironment::getWindowSize()
{
	RECT clientRect;
	GetClientRect(m_hwnd, &clientRect);
	return Vector2(clientRect.right, clientRect.bottom);
}

int WinEnvironment::getMonitor()
{
	const McRect desktopRect = getDesktopRect();

	for (int i=0; i<m_vMonitors.size(); i++)
	{
		if (((int)m_vMonitors[i].getX()) == ((int)desktopRect.getX()) && ((int)m_vMonitors[i].getY()) == ((int)desktopRect.getY()))
			return i;
	}

	debugLog("WARNING: Environment::getMonitor() found no matching monitor, returning default monitor ...\n");
	return 0;
}

std::vector<McRect> WinEnvironment::getMonitors()
{
	return m_vMonitors;
}

Vector2 WinEnvironment::getNativeScreenSize()
{
	const McRect desktopRect = getDesktopRect();
	return Vector2(desktopRect.getWidth(), desktopRect.getHeight());
}

McRect WinEnvironment::getVirtualScreenRect()
{
	return McRect(GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN), GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_CYVIRTUALSCREEN));
}

McRect WinEnvironment::getDesktopRect()
{
	HMONITOR monitor = MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTONEAREST);

	MONITORINFO info;
	info.cbSize = sizeof(MONITORINFO);

	GetMonitorInfo(monitor, &info);

	return McRect(info.rcMonitor.left, info.rcMonitor.top, std::abs(info.rcMonitor.left - info.rcMonitor.right), std::abs(info.rcMonitor.top - info.rcMonitor.bottom));
}

bool WinEnvironment::isCursorInWindow()
{
	return m_bIsCursorInsideWindow;
}

bool WinEnvironment::isCursorClipped()
{
	return m_bCursorClipped;
}

bool WinEnvironment::isCursorVisible()
{
	return g_bCursorVisible;
}

Vector2 WinEnvironment::getMousePos()
{
	POINT mpos;
	GetCursorPos(&mpos);
	ScreenToClient(m_hwnd, &mpos);
	return Vector2(mpos.x, mpos.y);
}

McRect WinEnvironment::getCursorClip()
{
	return m_cursorClip;
}

CURSORTYPE WinEnvironment::getCursor()
{
	return m_cursorType;
}

void WinEnvironment::setCursor(CURSORTYPE cur)
{
	m_cursorType = cur;

	switch (cur)
	{
	case CURSOR_NORMAL:
		m_mouseCursor = LoadCursor(NULL, IDC_ARROW);
		break;
	case CURSOR_WAIT:
		m_mouseCursor = LoadCursor(NULL, IDC_WAIT);
		break;
	case CURSOR_SIZE_H:
		m_mouseCursor = LoadCursor(NULL, IDC_SIZEWE);
		break;
	case CURSOR_SIZE_V:
		m_mouseCursor = LoadCursor(NULL, IDC_SIZENS);
		break;
	case CURSOR_SIZE_HV:
		m_mouseCursor = LoadCursor(NULL, IDC_SIZENESW);
		break;
	case CURSOR_SIZE_VH:
		m_mouseCursor = LoadCursor(NULL, IDC_SIZENWSE);
		break;
	case CURSOR_TEXT:
		m_mouseCursor = LoadCursor(NULL, IDC_IBEAM);
		break;
	default:
		m_mouseCursor = LoadCursor(NULL, IDC_ARROW);
		break;
	}

	SetCursor(m_mouseCursor);
}

void WinEnvironment::setCursorVisible(bool visible)
{
	g_bCursorVisible = visible; // notify main_Windows.cpp (for client area cursor invis)

	int check = ShowCursor(visible);
	if (!visible)
	{
		for (int i=0; i<=check; i++)
		{
			ShowCursor(visible);
		}
	}
	else if (check < 0)
	{
		for (int i=check; i<=0; i++)
		{
			ShowCursor(visible);
		}
	}
}

void WinEnvironment::setMousePos(int x, int y)
{
	POINT temp;
	temp.x = (LONG)x;
	temp.y = (LONG)y;
	ClientToScreen(m_hwnd, &temp);
	SetCursorPos((int)temp.x, (int)temp.y);
}

void WinEnvironment::setCursorClip(bool clip, McRect rect)
{
	m_bCursorClipped = clip;
	m_cursorClip = rect;

	if (clip)
	{
		RECT windowRect;

		if (rect.getWidth() == 0 && rect.getHeight() == 0)
		{
			RECT clientRect;
			GetClientRect(m_hwnd, &clientRect);

			POINT topLeft;
			topLeft.x = 0;
			topLeft.y = 0;
			ClientToScreen(m_hwnd, &topLeft);

			POINT bottomRight;
			bottomRight.x = clientRect.right;
			bottomRight.y = clientRect.bottom;
			ClientToScreen(m_hwnd, &bottomRight);

			windowRect.left = topLeft.x;
			windowRect.top = topLeft.y;
			windowRect.right = bottomRight.x;
			windowRect.bottom = bottomRight.y;

			m_cursorClip = McRect(0, 0, windowRect.right-windowRect.left, windowRect.bottom-windowRect.top);
		}

		// TODO: custom rect (only fullscreen works atm)

		ClipCursor(&windowRect);
	}
	else
		ClipCursor(NULL);
}

UString WinEnvironment::keyCodeToString(KEYCODE keyCode)
{
	UINT scanCode = MapVirtualKeyW(keyCode, MAPVK_VK_TO_VSC);

	WCHAR keyNameString[256];
	switch (keyCode)
	{
		case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN:
		case VK_PRIOR: case VK_NEXT:
		case VK_END: case VK_HOME:
		case VK_INSERT: case VK_DELETE:
		case VK_DIVIDE:
		case VK_NUMLOCK:
			scanCode |= 0x100;
			break;
	}

    if (!GetKeyNameTextW(scanCode << 16, keyNameString, 256))
    	return UString::format("%lu", keyCode); // fallback to raw number (better than having an empty string)

    return UString(keyNameString);
}



// helper functions

void WinEnvironment::path_strip_filename(TCHAR *Path)
{
	size_t Len = _tcslen(Path);

	if (Len==0)
		return;

	size_t Idx = Len-1;
	while (TRUE)
	{
		TCHAR Chr = Path[Idx];
		if (Chr==TEXT('\\')||Chr==TEXT('/'))
		{
			if (Idx==0||Path[Idx-1]==':')
				Idx++;

			break;
		}
		else if (Chr==TEXT(':'))
		{
			Idx++;
			break;
		}
		else
		{
			if (Idx==0)
				break;
			else
				Idx--;;
		};
	};

	Path[Idx] = TEXT('\0');
}

void WinEnvironment::handleShowMessageFullscreen()
{
	if (m_bFullScreen)
	{
		disableFullscreen();

		// HACKHACK: force minimize + focus if in fullscreen
		// minimizing allows us to view MessageBox()es at all, and focus brings it into the foreground (because minimize also unfocuses)
		minimize();
		focus();
	}
}

void WinEnvironment::enumerateMonitors()
{
	m_vMonitors.clear();
	EnumDisplayMonitors(NULL, NULL, WinEnvironment::monitorEnumProc, 0);
}

BOOL CALLBACK WinEnvironment::monitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	MONITORINFO monitorInfo;
	monitorInfo.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(hMonitor, &monitorInfo);

	const bool isPrimaryMonitor = (monitorInfo.dwFlags & MONITORINFOF_PRIMARY);
	const McRect monitorRect = McRect(lprcMonitor->left, lprcMonitor->top, std::abs(lprcMonitor->left - lprcMonitor->right), std::abs(lprcMonitor->top - lprcMonitor->bottom));
	if (isPrimaryMonitor)
		m_vMonitors.insert(m_vMonitors.begin(), monitorRect);
	else
		m_vMonitors.push_back(monitorRect);

	if (debug_env->getBool())
		debugLog("Monitor %i: (right = %ld, bottom = %ld, left = %ld, top = %ld), isPrimaryMonitor = %i\n", m_vMonitors.size(), lprcMonitor->right, lprcMonitor->bottom, lprcMonitor->left, lprcMonitor->top, (int)isPrimaryMonitor);

	return TRUE;
}

long WinEnvironment::getWindowStyleWindowed()
{
	long style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

	if (!m_bResizable)
		style = style & (~WS_SIZEBOX);

	return style;
}

long WinEnvironment::getWindowStyleFullscreen()
{
	return WS_SYSMENU | WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE;
}

#endif

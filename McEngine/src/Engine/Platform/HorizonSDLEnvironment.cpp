//================ Copyright (c) 2019, PG, All rights reserved. =================//
//
// Purpose:		nintendo switch SDL environment
//
// $NoKeywords: $nxsdlenv
//===============================================================================//

#ifdef __SWITCH__

#include "HorizonSDLEnvironment.h"

#ifdef MCENGINE_FEATURE_SDL

#include "Engine.h"
#include "ConVar.h"
#include "Mouse.h"
#include "SoundEngine.h"

#include "SDL.h"

#include <switch.h>

#include <dirent.h>
#include <sys/stat.h>

ConVar horizon_snd_chunk_size_docked("horizon_snd_chunk_size_docked", 512, FCVAR_NONE);
ConVar horizon_snd_chunk_size_undocked("horizon_snd_chunk_size_undocked", 256, FCVAR_NONE);

// HACKHACK: manual keyboard/mouse handling from sdl internals, until audio gets fixed, see https://github.com/devkitPro/SDL/commit/b91efb18a1a4752c03d56594b079aa804fe4e9ea
// audio was broken here: https://github.com/devkitPro/SDL/commit/51d12c191cdc7eb2ea7acca3daaf5e714b436128
static const HidKeyboardScancode switch_scancodes[MCENGINE_HORIZON_SDL_NUM_SCANCODES_SWITCH] = {
	KBD_A,
	KBD_B,
	KBD_C,
	KBD_D,
	KBD_E,
	KBD_F,
	KBD_G,
	KBD_H,
	KBD_I,
	KBD_J,
	KBD_K,
	KBD_L,
	KBD_M,
	KBD_N,
	KBD_O,
	KBD_P,
	KBD_Q,
	KBD_R,
	KBD_S,
	KBD_T,
	KBD_U,
	KBD_V,
	KBD_W,
	KBD_X,
	KBD_Y,
	KBD_Z,
	KBD_1,
	KBD_2,
	KBD_3,
	KBD_4,
	KBD_5,
	KBD_6,
	KBD_7,
	KBD_8,
	KBD_9,
	KBD_0,
	KBD_ENTER,
	KBD_ESC,
	KBD_BACKSPACE,
	KBD_TAB,
	KBD_SPACE,
	KBD_MINUS,
	KBD_EQUAL,
	KBD_LEFTBRACE,
	KBD_RIGHTBRACE,
	KBD_BACKSLASH,
	KBD_HASHTILDE,
	KBD_SEMICOLON,
	KBD_APOSTROPHE,
	KBD_GRAVE,
	KBD_COMMA,
	KBD_DOT,
	KBD_SLASH,
	KBD_CAPSLOCK,
	KBD_F1,
	KBD_F2,
	KBD_F3,
	KBD_F4,
	KBD_F5,
	KBD_F6,
	KBD_F7,
	KBD_F8,
	KBD_F9,
	KBD_F10,
	KBD_F11,
	KBD_F12,
	KBD_SYSRQ,
	KBD_SCROLLLOCK,
	KBD_PAUSE,
	KBD_INSERT,
	KBD_HOME,
	KBD_PAGEUP,
	KBD_DELETE,
	KBD_END,
	KBD_PAGEDOWN,
	KBD_RIGHT,
	KBD_LEFT,
	KBD_DOWN,
	KBD_UP,
	KBD_NUMLOCK,
	KBD_KPSLASH,
	KBD_KPASTERISK,
	KBD_KPMINUS,
	KBD_KPPLUS,
	KBD_KPENTER,
	KBD_KP1,
	KBD_KP2,
	KBD_KP3,
	KBD_KP4,
	KBD_KP5,
	KBD_KP6,
	KBD_KP7,
	KBD_KP8,
	KBD_KP9,
	KBD_KP0,
	KBD_KPDOT,
	KBD_102ND,
	KBD_COMPOSE,
	KBD_POWER,
	KBD_KPEQUAL,
	KBD_F13,
	KBD_F14,
	KBD_F15,
	KBD_F16,
	KBD_F17,
	KBD_F18,
	KBD_F19,
	KBD_F20,
	KBD_F21,
	KBD_F22,
	KBD_F23,
	KBD_F24,
	KBD_OPEN,
	KBD_HELP,
	KBD_PROPS,
	KBD_FRONT,
	KBD_STOP,
	KBD_AGAIN,
	KBD_UNDO,
	KBD_CUT,
	KBD_COPY,
	KBD_PASTE,
	KBD_FIND,
	KBD_MUTE,
	KBD_VOLUMEUP,
	KBD_VOLUMEDOWN,
	KBD_CAPSLOCK_ACTIVE,
	KBD_NUMLOCK_ACTIVE,
	KBD_SCROLLLOCK_ACTIVE,
	KBD_KPCOMMA,
	KBD_KPLEFTPAREN,
	KBD_KPRIGHTPAREN,
	KBD_LEFTCTRL,
	KBD_LEFTSHIFT,
	KBD_LEFTALT,
	KBD_LEFTMETA,
	KBD_RIGHTCTRL,
	KBD_RIGHTSHIFT,
	KBD_RIGHTALT,
	KBD_RIGHTMETA,
	KBD_MEDIA_PLAYPAUSE,
	KBD_MEDIA_STOPCD,
	KBD_MEDIA_PREVIOUSSONG,
	KBD_MEDIA_NEXTSONG,
	KBD_MEDIA_EJECTCD,
	KBD_MEDIA_VOLUMEUP,
	KBD_MEDIA_VOLUMEDOWN,
	KBD_MEDIA_MUTE,
	KBD_MEDIA_WWW,
	KBD_MEDIA_BACK,
	KBD_MEDIA_FORWARD,
	KBD_MEDIA_STOP,
	KBD_MEDIA_FIND,
	KBD_MEDIA_SCROLLUP,
	KBD_MEDIA_SCROLLDOWN,
	KBD_MEDIA_EDIT,
	KBD_MEDIA_SLEEP,
	KBD_MEDIA_COFFEE,
	KBD_MEDIA_REFRESH,
	KBD_MEDIA_CALC
};

ConVar *HorizonSDLEnvironment::m_mouse_sensitivity_ref = NULL;

uint8_t HorizonSDLEnvironment::locks = 0;
bool HorizonSDLEnvironment::keystate[] = {0};
uint64_t HorizonSDLEnvironment::prev_buttons = 0;

HorizonSDLEnvironment::HorizonSDLEnvironment() : SDLEnvironment(NULL)
{
	if (m_mouse_sensitivity_ref == NULL)
		m_mouse_sensitivity_ref = convar->getConVarByName("mouse_sensitivity");

	// the switch has its own internal deadzone handling already applied
	convar->getConVarByName("sdl_joystick0_deadzone")->setValue(0.0f);

	m_bDocked = false;

	m_fLastMouseDeltaTime = 0.0f;
}

HorizonSDLEnvironment::~HorizonSDLEnvironment()
{
}

void HorizonSDLEnvironment::update()
{
	// when switching between docked/undocked and sleeping/awake, restart audio and switch resolution
	// NOTE: for some reason, 256 byte audio buffer causes crackling only when docked, therefore we dynamically switch between 256 and 512
	// NOTE: if the console goes to sleep we get audio crackling later, therefore also restart audio engine when waking up
	const bool dockedChange = (isDocked() != m_bDocked);
	if (dockedChange)
	{
		if (dockedChange)
			m_bDocked = !m_bDocked;

		debugLog("HorizonSDLEnvironment: Switching to docked = %i (or waking up) ...\n", (int)m_bDocked);

		// restart sound engine
#ifdef MCENGINE_FEATURE_SDL_MIXER

		engine->getSound()->setMixChunkSize(m_bDocked ? horizon_snd_chunk_size_docked.getInt() : horizon_snd_chunk_size_undocked.getInt());

#endif
		convar->getConVarByName("snd_output_device")->setValue("Default");

		// switch resolution
		if (dockedChange)
		{
			const Vector2 resolution = (m_bDocked ? Vector2(1920, 1080) : Vector2(1280, 720));
			SDL_SetWindowSize(m_window, resolution.x, resolution.y);
		}
	}

	SDLEnvironment::update();
}

void HorizonSDLEnvironment::update_before_winproc()
{
	hidScanInput(); // for manually handling keyboard/mouse

	// HACKHACK: manually handle keyboard
	{
		for (int i=0; i<MCENGINE_HORIZON_SDL_NUM_SCANCODES_SWITCH; i++)
		{
			const HidKeyboardScancode keyCode = switch_scancodes[i];

			if (hidKeyboardHeld(keyCode) && !keystate[i])
			{
				switch (keyCode)
				{
					case SDL_SCANCODE_NUMLOCKCLEAR:
						if (!(locks & 0x1))
						{
							engine->onKeyboardKeyDown(keyCode);
							locks |= 0x1;
						}
						else
						{
							engine->onKeyboardKeyUp(keyCode);
							locks &= ~0x1;
						}
						break;

					case SDL_SCANCODE_CAPSLOCK:
						if (!(locks & 0x2))
						{
							engine->onKeyboardKeyDown(keyCode);
							locks |= 0x2;
						}
						else
						{
							engine->onKeyboardKeyUp(keyCode);
							locks &= ~0x2;
						}
						break;

					case SDL_SCANCODE_SCROLLLOCK:
						if (!(locks & 0x4))
						{
							engine->onKeyboardKeyDown(keyCode);
							locks |= 0x4;
						}
						else
						{
							engine->onKeyboardKeyUp(keyCode);
							locks &= ~0x4;
						}
						break;

					default:
						engine->onKeyboardKeyDown(keyCode);
				}

				keystate[i] = true;
			}
			else if (!hidKeyboardHeld(keyCode) && keystate[i])
			{
				switch (keyCode)
				{
					case SDL_SCANCODE_CAPSLOCK:
					case SDL_SCANCODE_NUMLOCKCLEAR:
					case SDL_SCANCODE_SCROLLLOCK:
						break;

					default:
						engine->onKeyboardKeyUp(keyCode);
				}

				keystate[i] = false;
			}
		}
	}

	// HACKHACK: manually handle mouse
	{
		uint64_t buttons = hidMouseButtonsHeld();
		uint64_t changed_buttons = buttons ^ prev_buttons;

		// buttons
		if (changed_buttons & MOUSE_LEFT)
		{
			if (prev_buttons & MOUSE_LEFT)
				engine->onMouseLeftChange(false);
			else
				engine->onMouseLeftChange(true);
		}

		if (changed_buttons & MOUSE_RIGHT)
		{
			if (prev_buttons & MOUSE_RIGHT)
				engine->onMouseRightChange(false);
			else
				engine->onMouseRightChange(true);
		}

		if (changed_buttons & MOUSE_MIDDLE)
		{
			if (prev_buttons & MOUSE_MIDDLE)
				engine->onMouseMiddleChange(false);
			else
				engine->onMouseMiddleChange(true);
		}

		prev_buttons = buttons;

		MousePosition newMousePos;
		hidMouseRead(&newMousePos);

		// raw delta
		// NOTE: the delta values are framerate dependent (poll-dependent), so basically unusable
		const int32_t dx = (int32_t)newMousePos.velocityX * 2;
		const int32_t dy = (int32_t)newMousePos.velocityY * 2;

		if (dx != 0 || dy != 0)
		{
			m_fLastMouseDeltaTime = engine->getTime();
			engine->onMouseRawMove(dx, dy);
		}

		// position
		// NOTE: the only use case for mouse control is docked mode. the raw coordinates from hidMouseRead() are restricted to 720p, so upscaling only gives 1.5x1.5 pixel accuracy at most
		if (engine->getTime() < m_fLastMouseDeltaTime + 0.5f)
		{
			const float rawRangeX = 1280.0f;
			const float rawRangeY = 720.0f;

			m_vMousePos.x = ((((float)newMousePos.x - rawRangeX/2.0f) * m_mouse_sensitivity_ref->getFloat()) + rawRangeX/2.0f) / rawRangeX;
			m_vMousePos.y = ((((float)newMousePos.y - rawRangeY/2.0f) * m_mouse_sensitivity_ref->getFloat()) + rawRangeY/2.0f) / rawRangeY;

			m_vMousePos *= engine->getScreenSize(); // scale to 1080p

			m_vMousePos.x = clamp<float>(m_vMousePos.x, 0.0f, engine->getScreenSize().x);
			m_vMousePos.y = clamp<float>(m_vMousePos.y, 0.0f, engine->getScreenSize().y);

			engine->getMouse()->onPosChange(m_vMousePos);
		}

		// scrolling
		const int scrollVelocityX = (int)newMousePos.scrollVelocityX;
		const int scrollVelocityY = (int)newMousePos.scrollVelocityY;
		if (scrollVelocityY != 0)
			engine->onMouseWheelHorizontal(scrollVelocityY);
		if (scrollVelocityX != 0)
			engine->onMouseWheelVertical(scrollVelocityX);
	}
}

Environment::OS HorizonSDLEnvironment::getOS()
{
	return Environment::OS::OS_HORIZON;
}

void HorizonSDLEnvironment::sleep(unsigned int us)
{
	svcSleepThread(us*1000);
}

UString HorizonSDLEnvironment::getUsername()
{
	UString uUsername = convar->getConVarByName("name")->getString();

	// this was directly taken from the libnx examples

    Result rc = 0;

    AccountUid userID;
    ///bool account_selected = 0;
    AccountProfile profile;
    AccountUserData userdata;
    AccountProfileBase profilebase;

    char username[0x21];

    memset(&userdata, 0, sizeof(userdata));
    memset(&profilebase, 0, sizeof(profilebase));

    rc = accountInitialize(AccountServiceType_Application);
    if (R_FAILED(rc))
        debugLog("accountInitialize() failed: 0x%x\n", rc);

    if (R_SUCCEEDED(rc))
    {
        rc = accountGetLastOpenedUser(&userID);

        if (R_FAILED(rc))
        	debugLog("accountGetActiveUser() failed: 0x%x\n", rc);
        ///else if(!account_selected)
        ///{
        ///	debugLog("No user is currently selected.\n");
        ///    rc = -1;
        ///}

        if (R_SUCCEEDED(rc))
        {
        	///debugLog("Current userID: 0x%lx 0x%lx\n", (u64)(userID>>64), (u64)userID);

            rc = accountGetProfile(&profile, userID);

            if (R_FAILED(rc))
            	debugLog("accountGetProfile() failed: 0x%x\n", rc);
        }

        if (R_SUCCEEDED(rc))
        {
            rc = accountProfileGet(&profile, &userdata, &profilebase); // userdata is otional, see libnx acc.h.

            if (R_FAILED(rc))
            	debugLog("accountProfileGet() failed: 0x%x\n", rc);

            if (R_SUCCEEDED(rc)) {
                memset(username,  0, sizeof(username));
                strncpy(username, profilebase.nickname, sizeof(username)-1); // even though profilebase.username usually has a NUL-terminator, don't assume it does for safety.
                debugLog("Username: %s\n", username);
                uUsername = UString(username);
            }
            accountProfileClose(&profile);
        }
        accountExit();
    }

    return uUsername;
}

std::vector<UString> HorizonSDLEnvironment::getFilesInFolder(UString folder)
{
	std::vector<UString> files;

    DIR *dir;
    struct dirent *ent;

    dir = opendir(folder.toUtf8());
    if (dir == NULL)
    	return files;

	while ((ent = readdir(dir)))
	{
		const char *name = ent->d_name;
		UString uName = UString(name);
		UString fullName = folder;
		fullName.append(uName);

		struct stat stDirInfo;
		int statret = stat(fullName.toUtf8(), &stDirInfo); // NOTE: lstat() always returns 0 in st_mode, seems broken, therefore using stat() for now
		if (statret < 0)
		{
			///perror (name);
			///debugLog("HorizonSDLEnvironment::getFilesInFolder() error, stat() returned %i!\n", lstatret);
			continue;
		}

		if (!S_ISDIR(stDirInfo.st_mode))
			files.push_back(uName);
	}
	closedir(dir);

	return files;
}

std::vector<UString> HorizonSDLEnvironment::getFoldersInFolder(UString folder)
{
	std::vector<UString> folders;

    DIR *dir;
    struct dirent *ent;

    dir = opendir(folder.toUtf8());
    if (dir == NULL)
    	return folders;

	while ((ent = readdir(dir)))
	{
		const char *name = ent->d_name;
		UString uName = UString(name);
		UString fullName = folder;
		fullName.append(uName);

		struct stat stDirInfo;
		int statret = stat(fullName.toUtf8(), &stDirInfo); // NOTE: lstat() always returns 0 in st_mode, seems broken, therefore using stat() for now
		if (statret < 0)
		{
			///perror (name);
			///debugLog("HorizonSDLEnvironment::getFoldersInFolder() error, stat() returned %i!\n", lstatret);
			continue;
		}

		if (S_ISDIR(stDirInfo.st_mode))
			folders.push_back(uName);
	}
	closedir(dir);

	return folders;
}

std::vector<UString> HorizonSDLEnvironment::getLogicalDrives()
{
	std::vector<UString> drives;

	drives.push_back("sdmc");
	drives.push_back("romfs");

	return drives;
}

UString HorizonSDLEnvironment::getFolderFromFilePath(UString filepath)
{
	// NOTE: #include <libgen.h> and dirname() is undefined, seems like it does not exist anywhere in devkitpro except the header file
	debugLog("WARNING: HorizonSDLEnvironment::getFolderFromFilePath() not available!\n");
	return filepath;
}

Vector2 HorizonSDLEnvironment::getMousePos()
{
	return m_vMousePos;
}

void HorizonSDLEnvironment::setMousePos(int x, int y)
{
	m_vMousePos.x = x;
	m_vMousePos.y = y;
}

void HorizonSDLEnvironment::showKeyboard()
{
	// TODO: this is broken. it only works if svcSetHeapSize(&addr, 0xf000000); // 128 MB, but that's not enough for the rest of the game. switching sizes also crashes later.
	// https://gbatemp.net/threads/software-keyboard-example-failing.528518/

	SwkbdConfig kbd;
	Result rc = swkbdCreate(&kbd, 0);
	printf("swkbdCreate(): 0x%x\n", rc);

	if (R_SUCCEEDED(rc))
	{
		swkbdConfigMakePresetDefault(&kbd);

		char str[256] = {0};
		rc = swkbdShow(&kbd, str, sizeof(str));
		printf("swkbdShow(): 0x%x\n", rc);

		swkbdClose(&kbd);

		if (R_SUCCEEDED(rc))
		{
			UString uStr = UString(str);
			for (int i=0; i<uStr.length(); i++)
			{
				engine->onKeyboardChar(uStr[i]);
			}
		}
	}
}

bool HorizonSDLEnvironment::isDocked()
{
	return (appletGetOperationMode() == AppletOperationMode_Docked);
}

int HorizonSDLEnvironment::getMemAvailableMB()
{
	uint64_t numBytes = 0;
	svcGetInfo(&numBytes, 6, CUR_PROCESS_HANDLE, 0);
	return (int)(numBytes/1024/1024);
}

int HorizonSDLEnvironment::getMemUsedMB()
{
	uint64_t numBytes = 0;
	svcGetInfo(&numBytes, 7, CUR_PROCESS_HANDLE, 0);
	return (int)(numBytes/1024/1024);
}

#endif

#endif

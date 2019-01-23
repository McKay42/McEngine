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

HorizonSDLEnvironment::HorizonSDLEnvironment() : SDLEnvironment(NULL)
{
	m_bDocked = false;

	/*
    hidGetSixAxisSensorHandles(&m_sensorHandles[0], 2, CONTROLLER_PLAYER_1, TYPE_JOYCON_PAIR);
    hidGetSixAxisSensorHandles(&m_sensorHandles[2], 1, CONTROLLER_PLAYER_1, TYPE_PROCONTROLLER);
    hidGetSixAxisSensorHandles(&m_sensorHandles[3], 1, CONTROLLER_HANDHELD, TYPE_HANDHELD);
    hidStartSixAxisSensor(m_sensorHandles[0]);
    hidStartSixAxisSensor(m_sensorHandles[1]);
    hidStartSixAxisSensor(m_sensorHandles[2]);
    hidStartSixAxisSensor(m_sensorHandles[3]);
    */
}

HorizonSDLEnvironment::~HorizonSDLEnvironment()
{
	/*
	hidStopSixAxisSensor(m_sensorHandles[0]);
	hidStopSixAxisSensor(m_sensorHandles[1]);
	hidStopSixAxisSensor(m_sensorHandles[2]);
	hidStopSixAxisSensor(m_sensorHandles[3]);
	*/
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

		engine->getSound()->setMixChunkSize(m_bDocked ? 512 : 256);

#endif
		convar->getConVarByName("snd_output_device")->setValue("Default");

		// switch resolution
		if (dockedChange)
		{
			const Vector2 resolution = (m_bDocked ? Vector2(1920, 1080) : Vector2(1280, 720));
			SDL_SetWindowSize(m_window, resolution.x, resolution.y);
		}
	}

	/*
	SixAxisSensorValues sixaxis;
	hidSixAxisSensorValuesRead(&sixaxis, CONTROLLER_P1_AUTO, 1);

    //printf("Accelerometer:    x=% .4f, y=% .4f, z=% .4f\n", sixaxis.accelerometer.x, sixaxis.accelerometer.y, sixaxis.accelerometer.z);
    //printf("Gyroscope:        x=% .4f, y=% .4f, z=% .4f\n", sixaxis.gyroscope.x, sixaxis.gyroscope.y, sixaxis.gyroscope.z);
    //printf("Orientation matrix:\n"
    //       "                  [ % .4f,   % .4f,   % .4f ]\n"
    //       "                  [ % .4f,   % .4f,   % .4f ]\n"
    //       "                  [ % .4f,   % .4f,   % .4f ]\n",
    //    sixaxis.orientation[0].x, sixaxis.orientation[0].y, sixaxis.orientation[0].z,
    //    sixaxis.orientation[1].x, sixaxis.orientation[1].y, sixaxis.orientation[1].z,
    //    sixaxis.orientation[2].x, sixaxis.orientation[2].y, sixaxis.orientation[2].z);

	// row column
	float yaw = std::atan2(sixaxis.orientation[1].x, sixaxis.orientation[0].x);
	float pitch = std::atan2(sixaxis.orientation[2].y, sixaxis.orientation[2].z);
	float roll = std::atan2(-sixaxis.orientation[2].x, std::sqrt(sixaxis.orientation[2].y*sixaxis.orientation[2].y + sixaxis.orientation[2].z*sixaxis.orientation[2].z));

    debugLog("ypr = [ % .4f,   % .4f,   % .4f ]\n", yaw, pitch, roll);

    yaw += 1.0f;
    yaw /= 2.0f;

    Vector2 mousePos = Vector2(engine->getScreenSize().x*yaw, engine->getScreenSize().y*pitch);

	setMousePos(mousePos.x, mousePos.y);
	engine->getMouse()->onPosChange(mousePos);
	*/

	SDLEnvironment::update();
}

Environment::OS HorizonSDLEnvironment::getOS()
{
	return Environment::OS::OS_HORIZON;
}

UString HorizonSDLEnvironment::getUsername()
{
	UString uUsername = convar->getConVarByName("name")->getString();

	// this was directly taken from the libnx examples

    Result rc = 0;

    u128 userID = 0;
    bool account_selected = 0;
    AccountProfile profile;
    AccountUserData userdata;
    AccountProfileBase profilebase;

    char username[0x21];

    memset(&userdata, 0, sizeof(userdata));
    memset(&profilebase, 0, sizeof(profilebase));

    rc = accountInitialize();
    if (R_FAILED(rc))
        debugLog("accountInitialize() failed: 0x%x\n", rc);

    if (R_SUCCEEDED(rc))
    {
        rc = accountGetActiveUser(&userID, &account_selected);

        if (R_FAILED(rc))
        	debugLog("accountGetActiveUser() failed: 0x%x\n", rc);
        else if(!account_selected)
        {
        	debugLog("No user is currently selected.\n");
            rc = -1;
        }

        if (R_SUCCEEDED(rc))
        {
        	debugLog("Current userID: 0x%lx 0x%lx\n", (u64)(userID>>64), (u64)userID);

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
                strncpy(username, profilebase.username, sizeof(username)-1); // even though profilebase.username usually has a NUL-terminator, don't assume it does for safety.
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

//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		handles sounds, bass library wrapper atm
//
// $NoKeywords: $snd
//===============================================================================//

#include "SoundEngine.h"

#ifdef MCENGINE_FEATURE_SOUND

#include <bass.h>

#endif

#include "Engine.h"
#include "ConVar.h"
#include "Sound.h"

ConVar snd_output_device("snd_output_device", "Default");
ConVar snd_restrict_play_frame("snd_restrict_play_frame", true, "only allow one new channel per frame for overlayable sounds (prevents lag and earrape)");

SoundEngine::SoundEngine()
{
	m_bReady = false;
	m_iLatency = -1;

#ifdef MCENGINE_FEATURE_SOUND

	// lib version check
	if (HIWORD(BASS_GetVersion()) != BASSVERSION)
	{
		engine->showMessageErrorFatal("Fatal Sound Error", "An incorrect version of the BASS library file was loaded!");
		engine->shutdown();
		return;
	}

	// apply default global settings
	BASS_SetConfig(BASS_CONFIG_BUFFER, 100);
	BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD, 10);
	BASS_SetConfig(BASS_CONFIG_UPDATETHREADS, 1);
	BASS_SetConfig(BASS_CONFIG_VISTA_TRUEPOS, 0); // if set to 1, increases latency from 30 ms to 40 ms on windows 7 (?)

	// add default output device
	m_iCurrentOutputDevice = -1;
	m_sCurrentOutputDevice = "nil";
	OUTPUT_DEVICE defaultOutputDevice;
	defaultOutputDevice.id = -1;
	defaultOutputDevice.name = "Default";
	snd_output_device.setValue(defaultOutputDevice.name);
	m_outputDevices.push_back(defaultOutputDevice);
	m_outputDeviceNames.push_back(defaultOutputDevice.name);

	// add all other output devices
	BASS_DEVICEINFO deviceInfo;
	int numDevices = 0;
	for (int d=0; BASS_GetDeviceInfo(d, &deviceInfo); d++)
	{
		if (deviceInfo.flags & BASS_DEVICE_ENABLED) // if the device can be used
		{
			numDevices++; // count it
			debugLog("SoundEngine: Device #%i = \"%s\"\n", numDevices, deviceInfo.name);

			if (d > 0) // the first device doesn't count ("No sound")
			{
				OUTPUT_DEVICE soundDevice;
				soundDevice.id = d;
				soundDevice.name = deviceInfo.name;

				bool nameCollision = false;
				do
				{
					nameCollision = false;
					for (int dd=0; dd<m_outputDevices.size(); dd++)
					{
						if (m_outputDevices[dd].name == soundDevice.name)
						{
							nameCollision = true;
							soundDevice.name.append(UString::format(" (%i)", dd));
							break;
						}
					}
				}
				while (nameCollision);

				m_outputDevices.push_back(soundDevice);
				m_outputDeviceNames.push_back(deviceInfo.name);
			}
		}
	}

	// load plugins
	/*
	m_FLACPluginHandle = BASS_PluginLoad("bassflac.dll", 0);
	if (m_FLACPluginHandle == NULL)
		engine->showInfo("Sound Error", "Couldn't load bassflac.dll plugin");
	*/

	initializeOutputDevice(defaultOutputDevice.id);

	// convar callbacks
	snd_output_device.setCallback( fastdelegate::MakeDelegate(this, &SoundEngine::setOutputDevice) );

#endif
}

bool SoundEngine::initializeOutputDevice(int id)
{
#ifdef MCENGINE_FEATURE_SOUND

	m_iCurrentOutputDevice = id;

	// cleanup potential previous device
	if (m_bReady)
		BASS_Free();

	// init
	if (BASS_Init(id, 44100, BASS_DEVICE_3D | BASS_DEVICE_LATENCY, 0, NULL))
		m_bReady = true;
	else
	{
		engine->showMessageError("Sound Error", "BASS_Init() failed!");
		m_bReady = false;
		return false;
	}

	for (int i=0; i<m_outputDevices.size(); i++)
	{
		if (m_outputDevices[i].id == id)
		{
			m_sCurrentOutputDevice = m_outputDevices[i].name;
			break;
		}
	}
	debugLog("SoundEngine: Output Device = \"%s\"\n", m_sCurrentOutputDevice.toUtf8());

	BASS_INFO info;
	BASS_GetInfo(&info);
	m_iLatency = info.minbuf;
	debugLog("SoundEngine: Minimum Latency = %i ms\n", info.latency);

	return true;

#else
	return false;
#endif
}

SoundEngine::~SoundEngine()
{
#ifdef MCENGINE_FEATURE_SOUND

	// and free it
	if (m_bReady)
		BASS_Free();

#endif
}

bool SoundEngine::play(Sound *snd)
{
	if (!m_bReady || snd == NULL || !snd->isReady()) return false;

#ifdef MCENGINE_FEATURE_SOUND

	if (!snd_restrict_play_frame.getBool() || engine->getTime() > snd->getLastPlayTime())
	{
		DWORD handle = snd->getHandle();
		if (BASS_ChannelIsActive(handle) != BASS_ACTIVE_PLAYING)
		{
			bool ret = BASS_ChannelPlay(handle, FALSE);
			if (!ret)
				debugLog("SoundEngine::play() couldn't BASS_ChannelPlay(), errorcode %i\n", BASS_ErrorGetCode());
			else
				snd->setLastPlayTime(engine->getTime());

			return ret;
		}
	}

#endif

	return false;
}

bool SoundEngine::play3d(Sound *snd, Vector3 pos)
{
	if (!m_bReady || snd == NULL || !snd->isReady() || !snd->is3d()) return false;

#ifdef MCENGINE_FEATURE_SOUND

	if (!snd_restrict_play_frame.getBool() || engine->getTime() > snd->getLastPlayTime())
	{
		DWORD handle = snd->getHandle();
		if (BASS_ChannelIsActive(handle) != BASS_ACTIVE_PLAYING)
		{
			BASS_3DVECTOR bassPos = BASS_3DVECTOR(pos.x, pos.y, pos.z);
			if (!BASS_ChannelSet3DPosition(handle, &bassPos, NULL, NULL))
				debugLog("SoundEngine::play3d() couldn't BASS_ChannelSet3DPosition(), errorcode %i\n", BASS_ErrorGetCode());
			else
				BASS_Apply3D(); // apply the changes

			bool ret = BASS_ChannelPlay(handle, FALSE);
			if (!ret)
				debugLog("SoundEngine::play3d() couldn't BASS_ChannelPlay(), errorcode %i\n", BASS_ErrorGetCode());
			else
				snd->setLastPlayTime(engine->getTime());

			return ret;
		}
	}

#endif

	return false;
}

void SoundEngine::pause(Sound *snd)
{
#ifdef MCENGINE_FEATURE_SOUND

	if (!m_bReady || snd == NULL || !snd->isReady()) return;

	BASS_ChannelPause(snd->getHandle());

#endif
}

void SoundEngine::stop(Sound *snd)
{
#ifdef MCENGINE_FEATURE_SOUND

	if (!m_bReady || snd == NULL || !snd->isReady()) return;

	BASS_ChannelStop(snd->getHandle());
	snd->setPosition(0.0); // HACKHACK: necessary
	snd->clear();
	snd->setLastPlayTime(0.0f);

#endif
}

void SoundEngine::setOutputDevice(UString outputDeviceName)
{
#ifdef MCENGINE_FEATURE_SOUND

	for (int i=0; i<m_outputDevices.size(); i++)
	{
		if (m_outputDevices[i].name == outputDeviceName)
		{
			if (m_outputDevices[i].id != m_iCurrentOutputDevice)
			{
				int previousOutputDevice = m_iCurrentOutputDevice;

				if (!initializeOutputDevice(m_outputDevices[i].id))
					initializeOutputDevice(previousOutputDevice); // if something went wrong, automatically switch back to the previous device

				return;
			}
			else
				debugLog("SoundEngine::setOutputDevice() \"%s\" already is the current device.\n", outputDeviceName.toUtf8());
		}
	}

	debugLog("SoundEngine::setOutputDevice() couldn't find output device \"%s\"!\n", outputDeviceName.toUtf8());

#endif
}

void SoundEngine::setVolume(float volume)
{
#ifdef MCENGINE_FEATURE_SOUND

	if (!m_bReady) return;

	volume = clamp<float>(volume, 0.0f, 1.0f);

	// 0 (silent) - 10000 (full).
	BASS_SetConfig(BASS_CONFIG_GVOL_SAMPLE, (DWORD)(volume*10000));
	BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, (DWORD)(volume*10000));
	BASS_SetConfig(BASS_CONFIG_GVOL_MUSIC, (DWORD)(volume*10000));

#endif
}

void SoundEngine::set3dPosition(Vector3 headPos, Vector3 viewDir, Vector3 viewUp)
{
#ifdef MCENGINE_FEATURE_SOUND

	if (!m_bReady) return;

	BASS_3DVECTOR bassHeadPos = BASS_3DVECTOR(headPos.x, headPos.y, headPos.z);
	BASS_3DVECTOR bassViewDir = BASS_3DVECTOR(viewDir.x, viewDir.y, viewDir.z);
	BASS_3DVECTOR bassViewUp = BASS_3DVECTOR(viewUp.x, viewUp.y, viewUp.z);
	if (!BASS_Set3DPosition(&bassHeadPos, NULL, &bassViewDir, &bassViewUp))
		debugLog("SoundEngine::set3dPosition() couldn't BASS_Set3DPosition(), errorcode %i\n", BASS_ErrorGetCode());
	else
		BASS_Apply3D(); // apply the changes

#endif
}

float SoundEngine::getAmplitude(Sound *snd)
{
#ifdef MCENGINE_FEATURE_SOUND

	if (!m_bReady || snd == NULL || !snd->isReady() || !snd->isPlaying()) return 0.0f;

	float fft[128];
	fft[0] = 0.0f;
	BASS_ChannelGetData(snd->getHandle(), &fft, BASS_DATA_FFT256);
	return fft[0];

#else
	return 0.0f;
#endif
}

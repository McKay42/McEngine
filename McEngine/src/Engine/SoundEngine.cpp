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
#include "Environment.h"
#include "WinEnvironment.h"

#include "Sound.h"

ConVar snd_output_device("snd_output_device", "Default");
ConVar snd_restrict_play_frame("snd_restrict_play_frame", true, "only allow one new channel per frame for overlayable sounds (prevents lag and earrape)");
ConVar snd_change_check_interval("snd_change_check_interval", 0.0f, "check for output device changes every this many seconds. 0 = disabled (default)");

SoundEngine::SoundEngine()
{
	m_bReady = false;
	m_iLatency = -1;

	m_fPrevOutputDeviceChangeCheckTime = 0.0f;

	m_outputDeviceChangeCallback = nullptr;

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
	m_sCurrentOutputDevice = "NULL";

	OUTPUT_DEVICE defaultOutputDevice;
	defaultOutputDevice.id = -1;
	defaultOutputDevice.name = "Default";
	defaultOutputDevice.enabled = true;
	defaultOutputDevice.isDefault = false; // custom -1 can never have default

	snd_output_device.setValue(defaultOutputDevice.name);
	m_outputDevices.push_back(defaultOutputDevice);

	// add all other output devices
	updateOutputDevices(false, true);

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

void SoundEngine::updateOutputDevices(bool handleOutputDeviceChanges, bool printInfo)
{
#ifdef MCENGINE_FEATURE_SOUND

	// NOTE: trust BASS to not reassign indices

	int currentOutputDeviceBASSIndex = BASS_GetDevice();

	bool outputDeviceChange = false;

	BASS_DEVICEINFO deviceInfo;
	int numDevices = 0;
	for (int d=0; (BASS_GetDeviceInfo(d, &deviceInfo) == true); d++)
	{
		const bool isEnabled = (deviceInfo.flags & BASS_DEVICE_ENABLED);
		const bool isDefault = (deviceInfo.flags & BASS_DEVICE_DEFAULT);

		if (printInfo)
		{
			debugLog("SoundEngine: Device %i = \"%s\", enabled = %i, default = %i\n", numDevices, deviceInfo.name, (int)isEnabled, (int)isDefault);
			numDevices++;
		}

		if (d > 0) // the first device doesn't count ("No sound") ~ Default in array
		{
			OUTPUT_DEVICE soundDevice;
			soundDevice.id = d;
			soundDevice.name = deviceInfo.name;
			soundDevice.enabled = isEnabled;
			soundDevice.isDefault = isDefault;

			// detect output device changes
			// TODO: this causes endless loops on audio devices with chinese/russian etc. characters in their name, may be caused by BASS reordering indices
			/*
			if (handleOutputDeviceChanges)
			{
				if (d == currentOutputDeviceBASSIndex && m_outputDevices.size() > d)
				{
					const bool hasEnabledChanged = (soundDevice.enabled != m_outputDevices[d].enabled);
					const bool hasDeviceChanged = (soundDevice.name != m_outputDevices[d].name);
					const bool hasDefaultChanged = (soundDevice.isDefault != m_outputDevices[d].isDefault);

					const bool hasChanged = (hasEnabledChanged || hasDeviceChanged || hasDefaultChanged);

					if (hasChanged)
					{
						debugLog("SoundEngine: Output device [\"%s\", enabled = %i, default = %i] changed to [\"%s\", enabled = %i, default = %i]\n", m_outputDevices[d].name.toUtf8(), (int)m_outputDevices[d].enabled, (int)m_outputDevices[d].isDefault, soundDevice.name.toUtf8(), (int)soundDevice.enabled, (int)soundDevice.isDefault);

						// update values
						m_outputDevices[d].name = soundDevice.name;
						m_outputDevices[d].enabled = soundDevice.enabled;
						m_outputDevices[d].isDefault = soundDevice.isDefault;

						// special case: if the user is using "Default" (index -1) and the actual system default changed, then we automatically re-initialize the device
						if (m_iCurrentOutputDevice == -1 && !outputDeviceChange)
						{
							initializeOutputDevice();

							// callback
							if (m_outputDeviceChangeCallback != nullptr)
								m_outputDeviceChangeCallback();

							// update state, redo loop
							outputDeviceChange = true; // only 1 change per tick
							currentOutputDeviceBASSIndex = BASS_GetDevice();
							d = 0;
							continue;
						}
					}
				}
			}
			*/

			if ((d+1) > m_outputDevices.size()) // only add new devices
				m_outputDevices.push_back(soundDevice);
		}

		// sanity
		if (d > 42)
		{
			debugLog("WARNING: SoundEngine::updateOutputDevices() found too many devices ...");
			break;
		}
	}

#endif
}

bool SoundEngine::initializeOutputDevice(int id)
{
#ifdef MCENGINE_FEATURE_SOUND

	debugLog("SoundEngine: initializeOutputDevice( %i ) ...\n", id);

	m_iCurrentOutputDevice = id;

	// cleanup potential previous device
	if (m_bReady)
		BASS_Free();

	// init
	const int freq = 44100;
	const unsigned int flags = /*BASS_DEVICE_3D | BASS_DEVICE_LATENCY | */0;
	bool ret = false;

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

		const WinEnvironment *winEnv = dynamic_cast<WinEnvironment*>(env);
		ret = BASS_Init(id, freq, flags, (winEnv != NULL ? winEnv->getHwnd() : (HWND)NULL), NULL);

#else

		ret = BASS_Init(id, freq, flags, 0, NULL);

#endif

	if (!ret)
	{
		m_bReady = false;
		engine->showMessageError("Sound Error", "BASS_Init() failed!");
		return false;
	}

	m_bReady = true;

	for (int i=0; i<m_outputDevices.size(); i++)
	{
		if (m_outputDevices[i].id == id)
		{
			m_sCurrentOutputDevice = m_outputDevices[i].name;
			break;
		}
	}
	debugLog("SoundEngine: Output Device = \"%s\"\n", m_sCurrentOutputDevice.toUtf8());

	/*
	BASS_INFO info;
	BASS_GetInfo(&info);
	m_iLatency = info.minbuf;
	debugLog("SoundEngine: Minimum Latency = %i ms\n", info.latency); // NOTE: needs BASS_DEVICE_LATENCY in init above, but that adds seconds to the startup time
	*/

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

void SoundEngine::update()
{
	if (snd_change_check_interval.getFloat() > 0.0f)
	{
		if (engine->getTime() > m_fPrevOutputDeviceChangeCheckTime)
		{
			m_fPrevOutputDeviceChangeCheckTime = engine->getTime() + snd_change_check_interval.getFloat();
			updateOutputDevices(true, false);
		}
	}
}

bool SoundEngine::play(Sound *snd, float pan)
{
	if (!m_bReady || snd == NULL || !snd->isReady()) return false;

#ifdef MCENGINE_FEATURE_SOUND

	if (!snd_restrict_play_frame.getBool() || engine->getTime() > snd->getLastPlayTime())
	{
		DWORD handle = snd->getHandle();
		if (BASS_ChannelIsActive(handle) != BASS_ACTIVE_PLAYING)
		{
			pan = clamp<float>(pan, -1.0f, 1.0f);
			BASS_ChannelSetAttribute(handle, BASS_ATTRIB_PAN, pan);

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

void SoundEngine::setOnOutputDeviceChange(std::function<void()> callback)
{
	m_outputDeviceChangeCallback = callback;
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
			}
			else
				debugLog("SoundEngine::setOutputDevice() \"%s\" already is the current device.\n", outputDeviceName.toUtf8());

			return;
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

std::vector<UString> SoundEngine::getOutputDevices()
{
	std::vector<UString> outputDevices;

	for (int i=0; i<m_outputDevices.size(); i++)
	{
		if (m_outputDevices[i].enabled)
			outputDevices.push_back(m_outputDevices[i].name);
	}

	return outputDevices;
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



//*********************//
//	Sound ConCommands  //
//*********************//

void _volume(UString oldValue, UString newValue)
{
	engine->getSound()->setVolume(newValue.toFloat());
}

ConVar _volume_("volume", 1.0f, _volume);

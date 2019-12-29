//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		handles sounds, bass library wrapper atm
//
// $NoKeywords: $snd
//===============================================================================//

#include "SoundEngine.h"

#include "Engine.h"
#include "ConVar.h"
#include "Environment.h"
#include "WinEnvironment.h"
#include "HorizonSDLEnvironment.h"

#include "Sound.h"

#ifdef MCENGINE_FEATURE_SOUND

#include <bass.h>

#endif

#ifdef MCENGINE_FEATURE_BASS_WASAPI

#include <basswasapi.h>
#include <bassmix.h>

SOUNDHANDLE g_wasapiOutputMixer = 0;

DWORD CALLBACK OutputWasapiProc(void *buffer, DWORD length, void *user)
{
	if (g_wasapiOutputMixer != 0)
	{
		const int c = BASS_ChannelGetData(g_wasapiOutputMixer, buffer, length);

		if (c < 0)
			return 0;

		return c;
	}

	return 0;
}

#endif

#if defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_SDL_MIXER)

#include "SDL.h"
#include "SDL_mixer.h"

#endif

ConVar snd_output_device("snd_output_device", "Default");

ConVar snd_chunk_size("snd_chunk_size", 256);

ConVar snd_restrict_play_frame("snd_restrict_play_frame", true, "only allow one new channel per frame for overlayable sounds (prevents lag and earrape)");
ConVar snd_change_check_interval("snd_change_check_interval", 0.0f, "check for output device changes every this many seconds. 0 = disabled (default)");

#ifdef MCENGINE_FEATURE_BASS_WASAPI

void _WIN_SND_WASAPI_BUFFER_SIZE_CHANGE(UString oldValue, UString newValue);
void _WIN_SND_WASAPI_PERIOD_SIZE_CHANGE(UString oldValue, UString newValue);
void _WIN_SND_WASAPI_EXCLUSIVE_CHANGE(UString oldValue, UString newValue);
ConVar win_snd_wasapi_buffer_size("win_snd_wasapi_buffer_size", 0.011f, "buffer size/length in seconds (e.g. 0.011 = 11 ms), directly responsible for audio delay and crackling", _WIN_SND_WASAPI_BUFFER_SIZE_CHANGE);
ConVar win_snd_wasapi_period_size("win_snd_wasapi_period_size", 0.0f, "interval between OutputWasapiProc calls in seconds (e.g. 0.016 = 16 ms) (0 = use default)", _WIN_SND_WASAPI_PERIOD_SIZE_CHANGE);
ConVar win_snd_wasapi_exclusive("win_snd_wasapi_exclusive", true, "whether to use exclusive device mode to further reduce latency", _WIN_SND_WASAPI_EXCLUSIVE_CHANGE);
void _WIN_SND_WASAPI_BUFFER_SIZE_CHANGE(UString oldValue, UString newValue)
{
	const int oldValueMS = std::round(oldValue.toFloat()*1000.0f);
	const int newValueMS = std::round(newValue.toFloat()*1000.0f);

	if (oldValueMS != newValueMS)
		engine->getSound()->setOutputDeviceForce(engine->getSound()->getOutputDevice()); // force restart
}
void _WIN_SND_WASAPI_PERIOD_SIZE_CHANGE(UString oldValue, UString newValue)
{
	const int oldValueMS = std::round(oldValue.toFloat()*1000.0f);
	const int newValueMS = std::round(newValue.toFloat()*1000.0f);

	if (oldValueMS != newValueMS)
		engine->getSound()->setOutputDeviceForce(engine->getSound()->getOutputDevice()); // force restart
}
void _WIN_SND_WASAPI_EXCLUSIVE_CHANGE(UString oldValue, UString newValue)
{
	const bool oldValueBool = oldValue.toInt();
	const bool newValueBool = newValue.toInt();

	if (oldValueBool != newValueBool)
		engine->getSound()->setOutputDeviceForce(engine->getSound()->getOutputDevice()); // force restart
}

#endif

SoundEngine::SoundEngine()
{
	m_bReady = false;
	m_iLatency = -1;

	m_fPrevOutputDeviceChangeCheckTime = 0.0f;

	m_outputDeviceChangeCallback = nullptr;

	m_fVolume = 1.0f;

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
	BASS_SetConfig(BASS_CONFIG_NET_BUFFER, 500);

#ifdef MCENGINE_FEATURE_BASS_WASAPI

	BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD, 0);
	BASS_SetConfig(BASS_CONFIG_UPDATETHREADS, 0);

#else

	BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD, 10);
	BASS_SetConfig(BASS_CONFIG_UPDATETHREADS, 1);

#endif

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

#elif defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_SDL_MIXER)

	m_iMixChunkSize = snd_chunk_size.getInt();
	m_fVolumeMixMusic = 1.0f;

	// NOTE: this seems unnecessary, and doesn't even work for SDL_INIT_OGG/SDL_INIT_MP3/etc., commented for now
	/*
	if (Mix_Init(0) == 0)
	{
		const char *error = SDL_GetError();
		debugLog("SoundEngine: Couldn't Mix_Init(): %s\n", error);
		engine->showMessageError("Sound Error", UString::format("Couldn't Mix_Init(): %s", error));
		return;
	}
	*/

	OUTPUT_DEVICE defaultOutputDevice;
	defaultOutputDevice.id = -1;
	defaultOutputDevice.name = "Default";
	defaultOutputDevice.enabled = true;
	defaultOutputDevice.isDefault = false; // custom -1 can never have default

	snd_output_device.setValue(defaultOutputDevice.name);
	m_outputDevices.push_back(defaultOutputDevice);

	initializeOutputDevice(defaultOutputDevice.id);

	// convar callbacks
	snd_output_device.setCallback( fastdelegate::MakeDelegate(this, &SoundEngine::setOutputDevice) );

#endif
}

void SoundEngine::updateOutputDevices(bool handleOutputDeviceChanges, bool printInfo)
{
#ifdef MCENGINE_FEATURE_SOUND

	int currentOutputDeviceBASSIndex = BASS_GetDevice();

	bool outputDeviceChange = false;

#ifdef MCENGINE_FEATURE_BASS_WASAPI

	BASS_WASAPI_DEVICEINFO deviceInfo;
	int numDevices = 0;
	for (int d=0; (BASS_WASAPI_GetDeviceInfo(d, &deviceInfo) == true); d++)
	{
		const bool isEnabled = (deviceInfo.flags & BASS_DEVICE_ENABLED);
		const bool isDefault = (deviceInfo.flags & BASS_DEVICE_DEFAULT);
		const bool isInput = (deviceInfo.flags & BASS_DEVICE_INPUT);

		if (isInput)
			continue;

#else

	BASS_DEVICEINFO deviceInfo;
	int numDevices = 0;
	for (int d=0; (BASS_GetDeviceInfo(d, &deviceInfo) == true); d++)
	{
		const bool isEnabled = (deviceInfo.flags & BASS_DEVICE_ENABLED);
		const bool isDefault = (deviceInfo.flags & BASS_DEVICE_DEFAULT);

#endif

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
	debugLog("SoundEngine: initializeOutputDevice( %i ) ...\n", id);

	m_iCurrentOutputDevice = id;

#ifdef MCENGINE_FEATURE_SOUND

	// cleanup potential previous device
	BASS_Free();

#ifdef MCENGINE_FEATURE_BASS_WASAPI

	BASS_WASAPI_Free();

#endif

	// init
	const int freq = 44100;
	const unsigned int flags = /*BASS_DEVICE_3D | BASS_DEVICE_LATENCY | */0;
	bool ret = false;

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

	int idForBassInit = id;

#ifdef MCENGINE_FEATURE_BASS_WASAPI

	idForBassInit = 0;

#endif

		const WinEnvironment *winEnv = dynamic_cast<WinEnvironment*>(env);
		ret = BASS_Init(idForBassInit, freq, flags, (winEnv != NULL ? winEnv->getHwnd() : (HWND)NULL), NULL);

#else

		ret = BASS_Init(id, freq, flags, 0, NULL);

#endif

	if (!ret)
	{
		m_bReady = false;
		engine->showMessageError("Sound Error", UString::format("BASS_Init() failed (%i)!", BASS_ErrorGetCode()));
		return false;
	}

#ifdef MCENGINE_FEATURE_BASS_WASAPI

	const float bufferSize = std::round(win_snd_wasapi_buffer_size.getFloat() * 1000.0f) / 1000.0f;		// in seconds
	const float updatePeriod = std::round(win_snd_wasapi_period_size.getFloat() * 1000.0f) / 1000.0f;	// in seconds

	debugLog("WASAPI Exclusive Mode = %i, bufferSize = %f, updatePeriod = %f\n", (int)win_snd_wasapi_exclusive.getBool(), bufferSize, updatePeriod);
	ret = BASS_WASAPI_Init(id, 0, 0, (win_snd_wasapi_exclusive.getBool() ? BASS_WASAPI_EXCLUSIVE : 0), bufferSize, updatePeriod, OutputWasapiProc, NULL);

	if (!ret)
	{
		m_bReady = false;

		const int errorCode = BASS_ErrorGetCode();

		if (errorCode == BASS_ERROR_WASAPI_BUFFER)
			debugLog("Sound Error: BASS_WASAPI_Init() failed with BASS_ERROR_WASAPI_BUFFER!");
		else
			engine->showMessageError("Sound Error", UString::format("BASS_WASAPI_Init() failed (%i)!", errorCode));

		return false;
	}

	if (!BASS_WASAPI_Start())
	{
		m_bReady = false;
		engine->showMessageError("Sound Error", UString::format("BASS_WASAPI_Start() failed (%i)!", BASS_ErrorGetCode()));
		return false;
	}

	BASS_WASAPI_INFO wasapiInfo;
	BASS_WASAPI_GetInfo(&wasapiInfo);

	g_wasapiOutputMixer = BASS_Mixer_StreamCreate(wasapiInfo.freq, wasapiInfo.chans, BASS_SAMPLE_FLOAT | BASS_STREAM_DECODE | BASS_MIXER_NONSTOP);

	if (g_wasapiOutputMixer == 0)
	{
		m_bReady = false;
		engine->showMessageError("Sound Error", UString::format("BASS_Mixer_StreamCreate() failed (%i)!", BASS_ErrorGetCode()));
		return false;
	}

#endif

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

#elif defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_SDL_MIXER)

	// cleanup potential previous device
	if (m_bReady)
		Mix_CloseAudio();

	const int freq = 44100;
	const int channels = 16;

	if (Mix_OpenAudio(freq, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, m_iMixChunkSize) < 0)
	{
		const char *error = SDL_GetError();
		debugLog("SoundEngine: Couldn't Mix_OpenAudio(): %s\n", error);
		engine->showMessageError("Sound Error", UString::format("Couldn't Mix_OpenAudio(): %s", error));
		return false;
	}

	const int numAllocatedChannels = Mix_AllocateChannels(channels);
	debugLog("SoundEngine: Allocated %i channels\n", numAllocatedChannels);

	// tag all channels to allow overriding in play()
	Mix_GroupChannels(0, numAllocatedChannels-1, 1);

	m_bReady = true;

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
	{
		BASS_Free();

#ifdef MCENGINE_FEATURE_BASS_WASAPI

		BASS_WASAPI_Free();

#endif

	}

#elif defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_SDL_MIXER)

	if (m_bReady)
		Mix_CloseAudio();

	Mix_Quit();

#endif
}

void SoundEngine::update()
{
	if (snd_change_check_interval.getFloat() > 0.0f)
	{
		if (engine->getTime() > m_fPrevOutputDeviceChangeCheckTime)
		{
			m_fPrevOutputDeviceChangeCheckTime = engine->getTime() + snd_change_check_interval.getFloat();
			///updateOutputDevices(true, false); // NOTE: commented for now, since it's not yet finished anyway
		}
	}
}

bool SoundEngine::play(Sound *snd, float pan)
{
	if (!m_bReady || snd == NULL || !snd->isReady()) return false;

	pan = clamp<float>(pan, -1.0f, 1.0f);

#ifdef MCENGINE_FEATURE_SOUND

#ifdef MCENGINE_FEATURE_BASS_WASAPI

	if (!snd_restrict_play_frame.getBool() || engine->getTime() > snd->getLastPlayTime())
	{
		SOUNDHANDLE handle = snd->getHandle();
		BASS_ChannelSetAttribute(handle, BASS_ATTRIB_PAN, pan);

		if (!snd->isStream())
			BASS_ChannelSetAttribute(handle, BASS_ATTRIB_NORAMP, 1.0f); // see https://github.com/ppy/osu-framework/pull/3146

		// HACKHACK: temp
		if (handle != 0)
		{
			if (BASS_Mixer_ChannelGetMixer(handle) == 0)
			{
				if (!BASS_Mixer_StreamAddChannel(g_wasapiOutputMixer, handle, (!snd->isStream() ? BASS_STREAM_AUTOFREE : 0) | BASS_MIXER_DOWNMIX | BASS_MIXER_NORAMPIN))
					debugLog("BASS_Mixer_StreamAddChannel() failed (%i)!", BASS_ErrorGetCode());
			}
		}

		if (BASS_ChannelIsActive(handle) != BASS_ACTIVE_PLAYING)
		{
			const bool ret = BASS_ChannelPlay(handle, TRUE);

			if (!ret)
				debugLog("SoundEngine::play() couldn't BASS_ChannelPlay(), errorcode %i\n", BASS_ErrorGetCode());

			return ret;
		}

		snd->setLastPlayTime(engine->getTime());

		return true;
	}

#else

	if (!snd_restrict_play_frame.getBool() || engine->getTime() > snd->getLastPlayTime())
	{
		SOUNDHANDLE handle = snd->getHandle();
		if (BASS_ChannelIsActive(handle) != BASS_ACTIVE_PLAYING)
		{
			BASS_ChannelSetAttribute(handle, BASS_ATTRIB_PAN, pan);

			if (!snd->isStream())
				BASS_ChannelSetAttribute(handle, BASS_ATTRIB_NORAMP, 1.0f); // see https://github.com/ppy/osu-framework/pull/3146

			const bool ret = BASS_ChannelPlay(handle, FALSE);
			if (!ret)
				debugLog("SoundEngine::play() couldn't BASS_ChannelPlay(), errorcode %i\n", BASS_ErrorGetCode());
			else
				snd->setLastPlayTime(engine->getTime());

			return ret;
		}
	}

#endif

#elif defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_SDL_MIXER)

	if (!snd_restrict_play_frame.getBool() || engine->getTime() > snd->getLastPlayTime())
	{
		if (snd->isStream() && Mix_PlayingMusic() && Mix_PausedMusic())
		{
			Mix_ResumeMusic();
			return true;
		}
		else
		{
			int channel = (snd->isStream() ? (Mix_PlayMusic((Mix_Music*)snd->getMixChunkOrMixMusic(), 1)) : Mix_PlayChannel(-1, (Mix_Chunk*)snd->getMixChunkOrMixMusic(), 0));

			// allow overriding (oldest channel gets reused)
			if (!snd->isStream() && channel < 0)
			{
				const int oldestChannel = Mix_GroupOldest(1);
				if (oldestChannel > -1)
				{
					Mix_HaltChannel(oldestChannel);
					channel = Mix_PlayChannel(-1, (Mix_Chunk*)snd->getMixChunkOrMixMusic(), 0);
				}
			}

			const bool ret = (channel > -1);

			if (!ret)
				debugLog(snd->isStream() ? "SoundEngine::play() couldn't Mix_PlayMusic(), error on %s!\n" : "SoundEngine::play() couldn't Mix_PlayChannel(), error on %s!\n", snd->getFilePath().toUtf8());
			else
			{
				snd->setHandle(channel);
				snd->setPan(pan);
				snd->setLastPlayTime(engine->getTime());
			}

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
		SOUNDHANDLE handle = snd->getHandle();
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

#elif defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_SDL_MIXER)

	return play(snd);

#endif

	return false;
}

void SoundEngine::pause(Sound *snd)
{
	if (!m_bReady || snd == NULL || !snd->isReady()) return;

#ifdef MCENGINE_FEATURE_SOUND

#ifdef MCENGINE_FEATURE_BASS_WASAPI

	SOUNDHANDLE handle = snd->getHandle();

	if (snd->isStream())
	{
		if (snd->isPlaying())
		{
			snd->setPrevPosition(snd->getPrevPosition());
			BASS_Mixer_ChannelRemove(handle);
		}
		else
		{
			play(snd);
			snd->setPositionMS(snd->getPrevPosition());
		}
	}
	else
	{
		if (!BASS_ChannelPause(handle))
		{
			debugLog("SoundEngine::pause() couldn't BASS_ChannelPause(), errorcode %i\n", BASS_ErrorGetCode());
		}
	}

#else

	BASS_ChannelPause(snd->getHandle());

#endif

#elif defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_SDL_MIXER)

	if (snd->isStream())
		Mix_PauseMusic();
	else
		Mix_Pause(snd->getHandle());

#endif
}

void SoundEngine::stop(Sound *snd)
{
	if (!m_bReady || snd == NULL || !snd->isReady()) return;

#ifdef MCENGINE_FEATURE_SOUND

	SOUNDHANDLE handle = snd->getHandle();

#ifdef MCENGINE_FEATURE_BASS_WASAPI

	if (BASS_Mixer_ChannelGetMixer(handle) != 0)
		BASS_Mixer_ChannelRemove(handle);

#endif

	BASS_ChannelStop(handle);
	{
		snd->setPosition(0.0); // HACKHACK: necessary
		snd->clear();
		snd->setLastPlayTime(0.0f);
	}

#elif defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_SDL_MIXER)

	if (snd->isStream())
		Mix_HaltMusic();
	else
		Mix_HaltChannel(snd->getHandle());

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

#elif defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_SDL_MIXER)

	initializeOutputDevice(-1);

#endif
}

void SoundEngine::setOutputDeviceForce(UString outputDeviceName)
{
#ifdef MCENGINE_FEATURE_SOUND

	for (int i=0; i<m_outputDevices.size(); i++)
	{
		if (m_outputDevices[i].name == outputDeviceName)
		{
			///if (m_outputDevices[i].id != m_iCurrentOutputDevice)
			{
				int previousOutputDevice = m_iCurrentOutputDevice;

				if (!initializeOutputDevice(m_outputDevices[i].id))
					initializeOutputDevice(previousOutputDevice); // if something went wrong, automatically switch back to the previous device
			}
			///else
			///	debugLog("SoundEngine::setOutputDevice() \"%s\" already is the current device.\n", outputDeviceName.toUtf8());

			return;
		}
	}

	debugLog("SoundEngine::setOutputDevice() couldn't find output device \"%s\"!\n", outputDeviceName.toUtf8());

#elif defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_SDL_MIXER)

	initializeOutputDevice(-1);

#endif
}

void SoundEngine::setVolume(float volume)
{
	if (!m_bReady) return;

	m_fVolume = clamp<float>(volume, 0.0f, 1.0f);

#ifdef MCENGINE_FEATURE_SOUND

	// 0 (silent) - 10000 (full).
	BASS_SetConfig(BASS_CONFIG_GVOL_SAMPLE, (DWORD)(m_fVolume*10000));
	BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, (DWORD)(m_fVolume*10000));
	BASS_SetConfig(BASS_CONFIG_GVOL_MUSIC, (DWORD)(m_fVolume*10000));

#ifdef MCENGINE_FEATURE_BASS_WASAPI

	BASS_WASAPI_SetVolume(BASS_WASAPI_CURVE_WINDOWS, m_fVolume);

#endif

#elif defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_SDL_MIXER)

	Mix_VolumeMusic((int)(m_fVolume*m_fVolumeMixMusic*MIX_MAX_VOLUME));
	Mix_Volume(-1, (int)(m_fVolume*MIX_MAX_VOLUME));

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
	if (!m_bReady || snd == NULL || !snd->isReady() || !snd->isPlaying()) return 0.0f;

#ifdef MCENGINE_FEATURE_SOUND

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

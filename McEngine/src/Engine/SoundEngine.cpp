//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		handles sounds, bass library wrapper atm
//
// $NoKeywords: $snd
//===============================================================================//

// TODO: async audio implementation still needs changes in Sound class playing-state handling
// TODO: async audio thread needs proper delay timing
// TODO: finish dynamic audio device updating, but can only do async due to potential lag, disabled by default

#include "SoundEngine.h"

#include "Engine.h"
#include "ConVar.h"
#include "Thread.h"
#include "Environment.h"
#include "WinEnvironment.h"
#include "HorizonSDLEnvironment.h"

#include "Sound.h"

#ifdef MCENGINE_FEATURE_MULTITHREADING

#include <mutex>
#include "WinMinGW.Mutex.h"

#endif



class SoundEngineThread
{
public:
	struct CHANNEL_PLAY_WORK
	{
		Sound *sound;
		Sound::SOUNDHANDLE handle;
	};

public:

#ifdef MCENGINE_FEATURE_MULTITHREADING

	// self
	McThread *thread;

	std::mutex workingMutex; // work vector lock

	std::atomic<bool> running;

#ifdef MCENGINE_FEATURE_SOUND

	std::vector<CHANNEL_PLAY_WORK> channelPlayWork;

#endif

#endif
};



#ifdef MCENGINE_FEATURE_SOUND

#include <bass.h>

#ifdef MCENGINE_FEATURE_MULTITHREADING

void *_soundEngineThread(void *data);

#endif

#endif



#ifdef MCENGINE_FEATURE_BASS_WASAPI

#include <basswasapi.h>
#include <bassmix.h>

Sound::SOUNDHANDLE g_wasapiOutputMixer = 0;

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



ConVar snd_output_device("snd_output_device", "Default", FCVAR_NONE);
ConVar snd_restart("snd_restart");
ConVar win_snd_fallback_dsound("win_snd_fallback_dsound", false, FCVAR_NONE, "use DirectSound instead of WASAPI");

ConVar snd_freq("snd_freq", 44100, FCVAR_NONE, "output sampling rate in Hz");
ConVar snd_updateperiod("snd_updateperiod", 10, FCVAR_NONE, "BASS_CONFIG_UPDATEPERIOD length in milliseconds");
ConVar snd_dev_period("snd_dev_period", 10, FCVAR_NONE, "BASS_CONFIG_DEV_PERIOD length in milliseconds, or if negative then in samples");
ConVar snd_dev_buffer("snd_dev_buffer", 30, FCVAR_NONE, "BASS_CONFIG_DEV_BUFFER length in milliseconds");
ConVar snd_chunk_size("snd_chunk_size", 256, FCVAR_NONE, "only used in horizon builds with sdl mixer audio");

ConVar snd_restrict_play_frame("snd_restrict_play_frame", true, FCVAR_NONE, "only allow one new channel per frame for overlayable sounds (prevents lag and earrape)");
ConVar snd_change_check_interval("snd_change_check_interval", 0.0f, FCVAR_NONE, "check for output device changes every this many seconds. 0 = disabled (default)");

///ConVar snd_async("snd_async", true, FCVAR_NONE, "play sounds on separate thread (e.g. removes BASS_ChannelPlay() ~0.3 ms blocking delay from main thread)");



#ifdef MCENGINE_FEATURE_BASS_WASAPI

void _WIN_SND_WASAPI_BUFFER_SIZE_CHANGE(UString oldValue, UString newValue);
void _WIN_SND_WASAPI_PERIOD_SIZE_CHANGE(UString oldValue, UString newValue);
void _WIN_SND_WASAPI_EXCLUSIVE_CHANGE(UString oldValue, UString newValue);

ConVar win_snd_wasapi_buffer_size("win_snd_wasapi_buffer_size", 0.011f, FCVAR_NONE, "buffer size/length in seconds (e.g. 0.011 = 11 ms), directly responsible for audio delay and crackling", _WIN_SND_WASAPI_BUFFER_SIZE_CHANGE);
ConVar win_snd_wasapi_period_size("win_snd_wasapi_period_size", 0.0f, FCVAR_NONE, "interval between OutputWasapiProc calls in seconds (e.g. 0.016 = 16 ms) (0 = use default)", _WIN_SND_WASAPI_PERIOD_SIZE_CHANGE);
ConVar win_snd_wasapi_exclusive("win_snd_wasapi_exclusive", true, FCVAR_NONE, "whether to use exclusive device mode to further reduce latency", _WIN_SND_WASAPI_EXCLUSIVE_CHANGE);
ConVar win_snd_wasapi_shared_volume_affects_device("win_snd_wasapi_shared_volume_affects_device", false, FCVAR_NONE, "if in shared mode, whether to affect device volume globally or use separate session volume (default)");

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

	m_fPrevOutputDeviceChangeCheckTime = 0.0f;

	m_outputDeviceChangeCallback = nullptr;

	m_fVolume = 1.0f;

#ifdef MCENGINE_FEATURE_SOUND

	// create audio thread
#ifdef MCENGINE_FEATURE_MULTITHREADING

	m_thread = NULL;

	///m_thread = new SoundEngineThread();
	///m_thread->running = true;
	///m_thread->thread = new Thread(_soundEngineThread, (void*)m_thread);

#endif

	// lib version check
	m_iBASSVersion = BASS_GetVersion();
	debugLog("SoundEngine: BASS version = 0x%08x\n", m_iBASSVersion);
	if (HIWORD(m_iBASSVersion) != BASSVERSION)
	{
		engine->showMessageErrorFatal("Fatal Sound Error", "An incorrect version of the BASS library file was loaded!");
		engine->shutdown();
		return;
	}

	// apply default global settings
	BASS_SetConfig(BASS_CONFIG_BUFFER, 100);
	BASS_SetConfig(BASS_CONFIG_NET_BUFFER, 500);
	//BASS_SetConfig(BASS_CONFIG_DEV_BUFFER, 10); // NOTE: only used by new osu atm, but not tested enough for offset problems
	BASS_SetConfig(BASS_CONFIG_MP3_OLDGAPS, 1); // NOTE: only used by osu atm (all beatmaps timed to non-iTunesSMPB + 529 sample deletion offsets on old dlls pre 2015)
	BASS_SetConfig(BASS_CONFIG_DEV_NONSTOP, 1); // NOTE: only used by osu atm (avoids lag/jitter in BASS_ChannelGetPosition() shortly after a BASS_ChannelPlay() after loading/silence)

#ifdef MCENGINE_FEATURE_BASS_WASAPI

	BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD, 0);
	BASS_SetConfig(BASS_CONFIG_UPDATETHREADS, 0);

#else

	BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD, 10); // NOTE: only used by osu atm (new osu uses 5 instead of 10, but not tested enough for offset problems)
	BASS_SetConfig(BASS_CONFIG_UPDATETHREADS, 1);

#endif

	BASS_SetConfig(BASS_CONFIG_VISTA_TRUEPOS, 0); // NOTE: if set to 1, increases sample playback latency +10 ms

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
	snd_freq.setCallback( fastdelegate::MakeDelegate(this, &SoundEngine::onFreqChanged) );
	snd_restart.setCallback( fastdelegate::MakeDelegate(this, &SoundEngine::restart) );
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
	snd_restart.setCallback( fastdelegate::MakeDelegate(this, &SoundEngine::restart) );
	snd_output_device.setCallback( fastdelegate::MakeDelegate(this, &SoundEngine::setOutputDevice) );

#endif
}

void SoundEngine::updateOutputDevices(bool handleOutputDeviceChanges, bool printInfo)
{
#ifdef MCENGINE_FEATURE_SOUND

	///int currentOutputDeviceBASSIndex = BASS_GetDevice();

	///bool outputDeviceChange = false;

	const bool allowNoSoundDevice = true;
	const int sanityLimit = 42;

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

		if (d > 0 || allowNoSoundDevice) // the first device doesn't count ("No sound") ~ Default in array
		{
			UString originalDeviceName = deviceInfo.name;

			OUTPUT_DEVICE soundDevice;
			soundDevice.id = d;
			soundDevice.name = originalDeviceName;
			soundDevice.enabled = isEnabled;
			soundDevice.isDefault = isDefault;

			// avoid duplicate names
			int duplicateNameCounter = 2;
			while (true)
			{
				bool foundDuplicateName = false;
				for (size_t i=0; i<m_outputDevices.size(); i++)
				{
					if (m_outputDevices[i].name == soundDevice.name)
					{
						foundDuplicateName = true;

						soundDevice.name = originalDeviceName;
						soundDevice.name.append(UString::format(" (%i)", duplicateNameCounter));

						duplicateNameCounter++;

						break;
					}
				}

				if (!foundDuplicateName)
					break;
			}

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

			if ((d+1 + (allowNoSoundDevice ? 1 : 0)) > m_outputDevices.size()) // only add new devices
				m_outputDevices.push_back(soundDevice);
		}

		// sanity
		if (d > sanityLimit)
		{
			debugLog("WARNING: SoundEngine::updateOutputDevices() found too many devices ...\n");
			break;
		}
	}

#endif
}

bool SoundEngine::initializeOutputDevice(int id)
{
	debugLog("SoundEngine: initializeOutputDevice( %i, fallback = %i ) ...\n", id, (int)win_snd_fallback_dsound.getBool());

	m_iCurrentOutputDevice = id;

#ifdef MCENGINE_FEATURE_SOUND

	// allow users to override some defaults (but which may cause beatmap desyncs)
	// we only want to set these if their values have been explicitly modified (to avoid sideeffects in the default case, and for my sanity)
	{
		if (snd_updateperiod.getFloat() != snd_updateperiod.getDefaultFloat())
			BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD, snd_updateperiod.getInt());

		if (snd_dev_buffer.getFloat() != snd_dev_buffer.getDefaultFloat())
			BASS_SetConfig(BASS_CONFIG_DEV_BUFFER, snd_dev_buffer.getInt());

		if (snd_dev_period.getFloat() != snd_dev_period.getDefaultFloat())
			BASS_SetConfig(BASS_CONFIG_DEV_PERIOD, snd_dev_period.getInt());
	}

	// cleanup potential previous device
	BASS_Free();

#ifdef MCENGINE_FEATURE_BASS_WASAPI

	BASS_WASAPI_Free();

#endif

	// dynamic runtime flags
	unsigned int runtimeFlags = 0;

#ifdef MCENGINE_FEATURE_BASS_WASAPI

	runtimeFlags = BASS_DEVICE_NOSPEAKER;

#else

	runtimeFlags = (win_snd_fallback_dsound.getBool() ? BASS_DEVICE_DSOUND : BASS_DEVICE_NOSPEAKER);

#endif

	// init
	const int freq = snd_freq.getInt();
	const unsigned int flags = /* BASS_DEVICE_3D | */ runtimeFlags;
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

#ifndef MCENGINE_FEATURE_BASS_WASAPI

		// try again with dsound fallback, once
		if (!win_snd_fallback_dsound.getBool())
		{
			debugLog("Sound Error: BASS_Init() failed (%i)!\n", BASS_ErrorGetCode());
			debugLog("Trying to fall back to DirectSound ...\n");

			win_snd_fallback_dsound.setValue(1.0f);

			const bool didFallbackSucceed = initializeOutputDevice(id);

			if (!didFallbackSucceed)
			{
				// we're fucked, reset and fail
				win_snd_fallback_dsound.setValue(0.0f);
			}

			return didFallbackSucceed;
		}
		else
#endif
		{
			engine->showMessageError("Sound Error", UString::format("BASS_Init() failed (%i)!", BASS_ErrorGetCode()));
			return false;
		}
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

	for (size_t i=0; i<m_outputDevices.size(); i++)
	{
		if (m_outputDevices[i].id == id)
		{
			m_sCurrentOutputDevice = m_outputDevices[i].name;
			break;
		}
	}
	debugLog("SoundEngine: Output Device = \"%s\"\n", m_sCurrentOutputDevice.toUtf8());

	return true;

#elif defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_SDL_MIXER)

	// cleanup potential previous device
	if (m_bReady)
		Mix_CloseAudio();

	const int freq = snd_freq.getInt();
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
	Mix_GroupChannels(0, numAllocatedChannels - 1, 1);

	m_bReady = true;

	return true;

#else

	return false;

#endif
}

SoundEngine::~SoundEngine()
{
#ifdef MCENGINE_FEATURE_SOUND

	// let the thread exit and wait for it to stop
#ifdef MCENGINE_FEATURE_MULTITHREADING

	if (m_thread != NULL)
	{
		m_thread->running = false;

		SAFE_DELETE(m_thread->thread);
		SAFE_DELETE(m_thread);
	}

#endif

	// and free it
	if (m_bReady)
	{
		BASS_Free();

#ifdef MCENGINE_FEATURE_BASS_WASAPI

		// NOTE: commented, because for some reason this now crashes in dllunload on Windows 10? this cleanup is irrelevant anyway since it only happens on shutdown
		//BASS_WASAPI_Free();

#endif

	}

#elif defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_SDL_MIXER)

	if (m_bReady)
		Mix_CloseAudio();

	Mix_Quit();

#endif
}

void SoundEngine::restart()
{
	setOutputDeviceForce(m_sCurrentOutputDevice);

	// callback (reload sound buffers etc.)
	if (m_outputDeviceChangeCallback != nullptr)
		m_outputDeviceChangeCallback();
}

void SoundEngine::update()
{
	/*
	if (snd_change_check_interval.getFloat() > 0.0f)
	{
		if (engine->getTime() > m_fPrevOutputDeviceChangeCheckTime)
		{
			m_fPrevOutputDeviceChangeCheckTime = engine->getTime() + snd_change_check_interval.getFloat();
			///updateOutputDevices(true, false); // NOTE: commented for now, since it's not yet finished anyway
		}
	}
	*/
}

bool SoundEngine::play(Sound *snd, float pan, float pitch)
{
	if (!m_bReady || snd == NULL || !snd->isReady()) return false;

	pan = clamp<float>(pan, -1.0f, 1.0f);
	pitch = clamp<float>(pitch, 0.0f, 2.0f);

	const bool allowPlayFrame = !snd->isOverlayable() || !snd_restrict_play_frame.getBool() || engine->getTime() > snd->getLastPlayTime();

	if (!allowPlayFrame)
		return false;

#ifdef MCENGINE_FEATURE_SOUND

#ifdef MCENGINE_FEATURE_BASS_WASAPI

		Sound::SOUNDHANDLE handle = snd->getHandle();
		BASS_ChannelSetAttribute(handle, BASS_ATTRIB_PAN, pan);

		if (snd->isStream() && snd->isLooped())
			BASS_ChannelFlags(handle, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP);

		if (!snd->isStream() && LOWORD(m_iBASSVersion) >= 0x0c00) // BASS_ATTRIB_NORAMP is available >= 2.4.12 - 10/3/2016
			BASS_ChannelSetAttribute(handle, BASS_ATTRIB_NORAMP, 1.0f); // see https://github.com/ppy/osu-framework/pull/3146

		// HACKHACK: force add to output mixer
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

#else

	Sound::SOUNDHANDLE handle = snd->getHandle();
	if (BASS_ChannelIsActive(handle) != BASS_ACTIVE_PLAYING)
	{
		BASS_ChannelSetAttribute(handle, BASS_ATTRIB_PAN, pan);

		if (pitch != 1.0f)
		{
			float freq = snd_freq.getFloat();
			BASS_ChannelGetAttribute(handle, BASS_ATTRIB_FREQ, &freq);

			const float semitonesShift = lerp<float>(-60.0f, 60.0f, pitch / 2.0f);
			BASS_ChannelSetAttribute(handle, BASS_ATTRIB_FREQ, std::pow(2.0f, (semitonesShift / 12.0f)) * freq);
		}

		if (snd->isStream() && snd->isLooped())
			BASS_ChannelFlags(handle, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP);

		if (!snd->isStream() && LOWORD(m_iBASSVersion) >= 0x0c00) // BASS_ATTRIB_NORAMP is available >= 2.4.12 - 10/3/2016
			BASS_ChannelSetAttribute(handle, BASS_ATTRIB_NORAMP, 1.0f); // see https://github.com/ppy/osu-framework/pull/3146

		bool ret = false;

#ifdef MCENGINE_FEATURE_MULTITHREADING

		/*
		if (snd_async.getBool() && m_thread != NULL)
		{
			ret = true;

			///snd->setIsWaiting(true);
			{
				SoundEngineThread::CHANNEL_PLAY_WORK channelPlayWork;

				channelPlayWork.sound = snd;
				channelPlayWork.handle = handle;

				// add work
				m_thread->workingMutex.lock();
				{
					m_thread->channelPlayWork.push_back(channelPlayWork);
				}
				m_thread->workingMutex.unlock();
			}
		}
		else
		*/
		{
			ret = BASS_ChannelPlay(handle, FALSE);
			if (!ret)
				debugLog("SoundEngine::play() couldn't BASS_ChannelPlay(), errorcode %i\n", BASS_ErrorGetCode());
		}

#else

		ret = BASS_ChannelPlay(handle, FALSE);
		if (!ret)
			debugLog("SoundEngine::play() couldn't BASS_ChannelPlay(), errorcode %i\n", BASS_ErrorGetCode());

#endif

		if (ret)
			snd->setLastPlayTime(engine->getTime());

		return ret;
	}

#endif

#elif defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_SDL_MIXER)

	if (snd->isStream() && Mix_PlayingMusic() && Mix_PausedMusic())
	{
		Mix_ResumeMusic();
		return true;
	}
	else
	{
		// special case: looped sounds are not supported for sdl/mixer, so do not let them kill other channels
		if (snd->isLooped())
			return false;

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

#endif

	return false;
}

bool SoundEngine::play3d(Sound *snd, Vector3 pos)
{
	if (!m_bReady || snd == NULL || !snd->isReady() || !snd->is3d()) return false;

#ifdef MCENGINE_FEATURE_SOUND

	if (!snd_restrict_play_frame.getBool() || engine->getTime() > snd->getLastPlayTime())
	{
		Sound::SOUNDHANDLE handle = snd->getHandle();
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

	Sound::SOUNDHANDLE handle = snd->getHandle();

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
	{
		snd->setLastPlayTime(0.0);
	}

#endif

#elif defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_SDL_MIXER)

	if (snd->isStream())
		Mix_PauseMusic();
	else if (!snd->isLooped()) // special case: looped sounds are not supported for sdl/mixer, so do not let them kill other channels
		Mix_Pause(snd->getHandle());

#endif
}

void SoundEngine::stop(Sound *snd)
{
	if (!m_bReady || snd == NULL || !snd->isReady()) return;

#ifdef MCENGINE_FEATURE_SOUND

	Sound::SOUNDHANDLE handle = snd->getHandle();

#ifdef MCENGINE_FEATURE_BASS_WASAPI

	if (BASS_Mixer_ChannelGetMixer(handle) != 0)
		BASS_Mixer_ChannelRemove(handle);

#endif

	BASS_ChannelStop(handle);
	{
		snd->setPosition(0.0);
		snd->setLastPlayTime(0.0);

		// allow next play()/getHandle() to reallocate (because BASS_ChannelStop() will free the channel)
		snd->m_HCHANNEL = 0;
		snd->m_HCHANNELBACKUP = 0;
	}

#elif defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_SDL_MIXER)

	if (snd->isStream())
		Mix_HaltMusic();
	else if (!snd->isLooped()) // special case: looped sounds are not supported here, so do not let them kill other channels
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

	for (size_t i=0; i<m_outputDevices.size(); i++)
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

	for (size_t i=0; i<m_outputDevices.size(); i++)
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

	BASS_WASAPI_SetVolume(BASS_WASAPI_CURVE_WINDOWS | (!win_snd_wasapi_exclusive.getBool() && !win_snd_wasapi_shared_volume_affects_device.getBool() ? BASS_WASAPI_VOL_SESSION : 0), m_fVolume);

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

void SoundEngine::onFreqChanged(UString oldValue, UString newValue)
{
	restart();
}

std::vector<UString> SoundEngine::getOutputDevices()
{
	std::vector<UString> outputDevices;

	for (size_t i=0; i<m_outputDevices.size(); i++)
	{
		if (m_outputDevices[i].enabled)
			outputDevices.push_back(m_outputDevices[i].name);
	}

	return outputDevices;
}



#ifdef MCENGINE_FEATURE_SOUND

#ifdef MCENGINE_FEATURE_MULTITHREADING

void *_soundEngineThread(void *data)
{
	SoundEngineThread *self = (SoundEngineThread*)data;

	std::vector<SoundEngineThread::CHANNEL_PLAY_WORK> channelPlayWork;

	while (self->running.load())
	{
		// quickly check if there is work to do (this can potentially cause engine lag!)
		self->workingMutex.lock();
		{
			for (size_t i=0; i<self->channelPlayWork.size(); i++)
			{
				channelPlayWork.push_back(self->channelPlayWork[i]);
			}
			self->channelPlayWork.clear();
		}
		self->workingMutex.unlock();

		// if we have work
		if (channelPlayWork.size() > 0)
		{
			for (size_t i=0; i<channelPlayWork.size(); i++)
			{
				if (!BASS_ChannelPlay(channelPlayWork[i].handle, FALSE))
					debugLog("SoundEngine::play() couldn't BASS_ChannelPlay(), errorcode %i\n", BASS_ErrorGetCode());

				///channelPlayWork[i].sound->setIsWaiting(false);
			}
			channelPlayWork.clear();
		}
		else
			env->sleep(1000); // 1000 Hz idle
	}

	return NULL;
}

#endif

#endif



//*********************//
//	Sound ConCommands  //
//*********************//

void _volume(UString oldValue, UString newValue)
{
	engine->getSound()->setVolume(newValue.toFloat());
}

ConVar _volume_("volume", 1.0f, FCVAR_NONE, _volume);

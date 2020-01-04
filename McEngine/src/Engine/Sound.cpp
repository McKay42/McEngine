//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		sound wrapper, either streamed or preloaded
//
// $NoKeywords: $snd $os
//===============================================================================//

#include "Sound.h"
#include "ConVar.h"
#include "File.h"

#ifdef MCENGINE_FEATURE_SOUND

#include <bass.h>
#include <bass_fx.h>

#ifdef MCENGINE_FEATURE_BASS_WASAPI

#include <bassmix.h>

#endif

#elif defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_SDL_MIXER)

#include "SDL.h"
#include "SDL_mixer_ext.h"

#endif

#include "Engine.h"
#include "SoundEngine.h"

#ifdef MCENGINE_FEATURE_SOUND

DWORD CALLBACK soundFXCallbackProc(HSTREAM handle, void *buffer, DWORD length, void *user);

#endif

ConVar debug_snd("debug_snd", false);

ConVar snd_speed_compensate_pitch("snd_speed_compensate_pitch", true, "automatically keep pitch constant if speed changes");

Sound::Sound(UString filepath, bool stream, bool threeD, bool loop, bool prescan) : Resource(filepath)
{
	m_fVolume = 1.0f;

	m_HSTREAM = 0;
	m_HSTREAMBACKUP = 0;
	m_HCHANNEL = 0;
	m_HCHANNELBACKUP = 0;

	m_bStream = stream;
	m_bIs3d = threeD;
	m_bIsLooped = loop;
	m_bPrescan = prescan;
	m_bIsOverlayable = false;

	m_fLastPlayTime = -1.0f;

	m_bisSpeedAndPitchHackEnabled = false;
	m_soundProcUserData = new SOUND_PROC_USERDATA();

	m_iPrevPosition = 0;
	m_mixChunkOrMixMusic = NULL;
	m_wasapiSampleBuffer = NULL;
	m_iWasapiSampleBufferSize = 0;
}

void Sound::init()
{
	if (m_sFilePath.length() < 2 || !m_bAsyncReady) return;

#ifdef MCENGINE_FEATURE_SOUND

	// error checking
	if (m_HSTREAM == 0 && m_iWasapiSampleBufferSize < 1)
	{
		UString msg = "Couldn't load sound \"";
		msg.append(m_sFilePath);
		msg.append(UString::format("\", stream = %i, errorcode = %i", (int)m_bStream, BASS_ErrorGetCode()));
		msg.append(", file = ");
		msg.append(m_sFilePath);
		msg.append("\n");
		debugLog(0xffdd3333, "%s", msg.toUtf8());
	}
	else
		m_bReady = true;

#elif defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_SDL_MIXER)

	m_bReady = m_bAsyncReady;

#endif
}

void Sound::initAsync()
{
	printf("Resource Manager: Loading %s\n", m_sFilePath.toUtf8());

#ifdef MCENGINE_FEATURE_SOUND

	// create the sound
	if (m_bStream)
	{
		DWORD extraStreamCreateFileFlags = 0;
		DWORD extraFXTempoCreateFlags = 0;

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

#ifdef MCENGINE_FEATURE_BASS_WASAPI

		extraStreamCreateFileFlags |= BASS_SAMPLE_FLOAT;
		extraFXTempoCreateFlags |= BASS_STREAM_DECODE;

#endif

		m_HSTREAM = BASS_StreamCreateFile(FALSE, m_sFilePath.wc_str(), 0, 0, BASS_STREAM_DECODE | BASS_UNICODE | (m_bPrescan ? BASS_STREAM_PRESCAN : 0) | extraStreamCreateFileFlags);

#elif defined __linux__

		m_HSTREAM = BASS_StreamCreateFile(FALSE, m_sFilePath.toUtf8(), 0, 0, BASS_STREAM_DECODE | (m_bPrescan ? BASS_STREAM_PRESCAN : 0) | extraStreamCreateFileFlags);

#else

		m_HSTREAM = BASS_StreamCreateFile(FALSE, m_sFilePath.toUtf8(), 0, 0, BASS_STREAM_DECODE | (m_bPrescan ? BASS_STREAM_PRESCAN : 0) | extraStreamCreateFileFlags);

#endif

		m_HSTREAM = BASS_FX_TempoCreate(m_HSTREAM, BASS_FX_FREESOURCE | extraFXTempoCreateFlags);

		BASS_ChannelSetAttribute(m_HSTREAM, BASS_ATTRIB_TEMPO_OPTION_USE_QUICKALGO, true);
		BASS_ChannelSetAttribute(m_HSTREAM, BASS_ATTRIB_TEMPO_OPTION_OVERLAP_MS, 4.0f);
		BASS_ChannelSetAttribute(m_HSTREAM, BASS_ATTRIB_TEMPO_OPTION_SEQUENCE_MS, 30.0f);

		m_HCHANNELBACKUP = m_HSTREAM;
	}
	else // not a stream
	{

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

#ifdef MCENGINE_FEATURE_BASS_WASAPI

		File file(m_sFilePath);
		if (file.canRead())
		{
			m_iWasapiSampleBufferSize = file.getFileSize();
			if (m_iWasapiSampleBufferSize > 0)
			{
				m_wasapiSampleBuffer = new char[file.getFileSize()];
				memcpy(m_wasapiSampleBuffer, file.readFile(), file.getFileSize());
			}
		}
		else
			debugLog("ERROR: Couldn't file.read on %s\n", m_sFilePath.toUtf8());

#else

		m_HSTREAM = BASS_SampleLoad(FALSE, m_sFilePath.wc_str(), 0, 0, 5, (m_bIsLooped ? BASS_SAMPLE_LOOP : 0 ) | (m_bIs3d ? BASS_SAMPLE_3D | BASS_SAMPLE_MONO : 0) | BASS_SAMPLE_OVER_POS | BASS_UNICODE);

#endif


#elif defined __linux__

		m_HSTREAM = BASS_SampleLoad(FALSE, m_sFilePath.toUtf8(), 0, 0, 5, (m_bIsLooped ? BASS_SAMPLE_LOOP : 0 ) | (m_bIs3d ? BASS_SAMPLE_3D | BASS_SAMPLE_MONO : 0) | BASS_SAMPLE_OVER_POS);

#else

		m_HSTREAM = BASS_SampleLoad(FALSE, m_sFilePath.toUtf8(), 0, 0, 5, (m_bIsLooped ? BASS_SAMPLE_LOOP : 0 ) | (m_bIs3d ? BASS_SAMPLE_3D | BASS_SAMPLE_MONO : 0) | BASS_SAMPLE_OVER_POS);

#endif

		m_HSTREAMBACKUP = m_HSTREAM; // needed for proper cleanup for FX HSAMPLES

		if (m_HSTREAM == 0)
			printf("Sound::initAsync() BASS_SampleLoad() error %i on %s !\n", BASS_ErrorGetCode(), m_sFilePath.toUtf8());
	}

	m_bAsyncReady = true;

#elif defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_SDL_MIXER)

	if (m_bStream)
		m_mixChunkOrMixMusic = Mix_LoadMUS(m_sFilePath.toUtf8());
	else
		m_mixChunkOrMixMusic = Mix_LoadWAV(m_sFilePath.toUtf8());

	if (m_mixChunkOrMixMusic == NULL)
		printf(m_bStream ? "Sound::initAsync() Mix_LoadMUS() error %s on %s!\n" : "Sound::initAsync() Mix_LoadWAV() error %s on %s!\n", SDL_GetError(), m_sFilePath.toUtf8());

	m_bAsyncReady = (m_mixChunkOrMixMusic != NULL);

#endif
}

SOUNDHANDLE Sound::getHandle()
{
#ifdef MCENGINE_FEATURE_SOUND

	// if the file is streamed from the disk, directly return HSTREAM
	// if not, create a channel of the stream if there is none, else return the previously created channel IFF this sound is not overlayable (because being overlayable implies multiple channels playing at once)

	if (m_bStream)
		return m_HSTREAM;
	else
	{

#ifdef MCENGINE_FEATURE_BASS_WASAPI

		// HACKHACK: wasapi stream objects can't be reused
		if (m_HCHANNEL == 0 || m_bIsOverlayable)
		{
			m_HCHANNEL = BASS_StreamCreateFile(TRUE, m_wasapiSampleBuffer, 0, m_iWasapiSampleBufferSize, BASS_SAMPLE_FLOAT | BASS_STREAM_DECODE | BASS_UNICODE | (m_bIsLooped ? BASS_SAMPLE_LOOP : 0));
			if (m_HCHANNEL == 0)
				debugLog("BASS_StreamCreateFile() error %i\n", BASS_ErrorGetCode());

			BASS_ChannelSetAttribute(m_HCHANNEL, BASS_ATTRIB_VOL, m_fVolume);
		}

		return m_HCHANNEL;

#endif

		if (m_HCHANNEL != 0 && !m_bIsOverlayable)
			return m_HCHANNEL;

		m_HCHANNEL = BASS_SampleGetChannel(m_HSTREAMBACKUP, FALSE);
		m_HCHANNELBACKUP = m_HCHANNEL;

		if (m_HCHANNEL == 0)
		{
			UString msg = "Couldn't BASS_SampleGetChannel \"";
			msg.append(m_sFilePath);
			msg.append(UString::format("\", stream = %i, errorcode = %i", (int)m_bStream, BASS_ErrorGetCode()));
			msg.append(", file = ");
			msg.append(m_sFilePath);
			msg.append("\n");
			debugLog(0xffdd3333, "%s", msg.toUtf8());
		}
		else
			BASS_ChannelSetAttribute(m_HCHANNEL, BASS_ATTRIB_VOL, m_fVolume);

		// special behaviour for FX HSAMPLES
		if (m_bisSpeedAndPitchHackEnabled && !m_bIsOverlayable)
		{
			BASS_CHANNELINFO ci;
			BASS_ChannelGetInfo(m_HCHANNEL, &ci);

			// create a fake stream from the HSAMPLE which was completely loaded from the disk, this fake stream will use the soundFXCallbackProc function to actually get the data from an FX stream
			m_soundProcUserData->originalSampleChannel = m_HCHANNEL;
			m_soundProcUserData->offset = 0;

			SOUNDHANDLE fakeStream = BASS_StreamCreate(ci.freq, ci.chans, ci.flags | BASS_STREAM_DECODE, soundFXCallbackProc, m_soundProcUserData);
			m_HSTREAM = BASS_FX_TempoCreate(fakeStream, 0); // overwrite the stream/sample pointer
			m_HCHANNEL = m_HSTREAM;

			BASS_ChannelSetAttribute(m_HCHANNEL, BASS_ATTRIB_VOL, m_fVolume);
		}

		return m_HCHANNEL;
	}

#elif defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_SDL_MIXER)

	return m_HCHANNEL;

#else
	return 0;
#endif
}

void Sound::destroy()
{
	if (!m_bReady) return;

	m_bReady = false;

#ifdef MCENGINE_FEATURE_SOUND

	if (m_bStream)
	{

#ifdef MCENGINE_FEATURE_BASS_WASAPI

		BASS_Mixer_ChannelRemove(m_HSTREAM);

#endif

		BASS_StreamFree(m_HSTREAM); // fx (but with BASS_FX_FREESOURCE)
	}
	else
	{

#ifdef MCENGINE_FEATURE_BASS_WASAPI

		if (m_wasapiSampleBuffer != NULL)
		{
			delete[] m_wasapiSampleBuffer;
			m_wasapiSampleBuffer = NULL;
		}

#endif

		if (m_HCHANNEL)
			BASS_ChannelStop(m_HCHANNEL);
		if (m_HSTREAMBACKUP)
			BASS_SampleFree(m_HSTREAMBACKUP);
	}

	m_HSTREAM = 0;
	m_HSTREAMBACKUP = 0;
	m_HCHANNEL = 0;

#elif defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_SDL_MIXER)

	if (m_bStream)
		Mix_FreeMusic((Mix_Music*)m_mixChunkOrMixMusic);
	else
		Mix_FreeChunk((Mix_Chunk*)m_mixChunkOrMixMusic);

	m_mixChunkOrMixMusic = NULL;

#endif

	SAFE_DELETE(m_soundProcUserData);
}

void Sound::setPosition(double percent)
{
	if (!m_bReady) return;

	percent = clamp<double>(percent, 0.0, 1.0);

#ifdef MCENGINE_FEATURE_SOUND

	QWORD length = BASS_ChannelGetLength(m_HCHANNELBACKUP, BASS_POS_BYTE);

	// HACKHACK:
	if (m_bisSpeedAndPitchHackEnabled && !m_bIsOverlayable)
	{
		setPositionMS(0, true);
		m_soundProcUserData->offset = (QWORD) ((double)(length)*percent);
	}
	else
	{
		if (!BASS_ChannelSetPosition(getHandle(), (QWORD) ((double)(length)*percent), BASS_POS_BYTE) && debug_snd.getBool())
			debugLog("Sound::setPosition( %f ) BASS_ChannelSetPosition() Error %i on %s !\n", percent, BASS_ErrorGetCode(), m_sFilePath.toUtf8());
	}

#elif defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_SDL_MIXER) && defined(SDL_MIXER_X)

	if (m_bStream)
	{
		const double length = Mix_GetMusicTotalTime((Mix_Music*)m_mixChunkOrMixMusic);
		if (length > 0.0)
		{
			Mix_RewindMusic();

			const double targetPositionS = length*percent;
			Mix_SetMusicPosition(targetPositionS);

			// NOTE: Mix_SetMusicPosition()/scrubbing is inaccurate depending on the underlying decoders, approach in 1 second increments
			const double targetPositionMS = targetPositionS * 1000.0;
			double positionMS = targetPositionMS;
			double actualPositionMS = getPositionMS();
			double deltaMS = actualPositionMS - targetPositionMS;
			int loopCounter = 0;
			while (std::abs(deltaMS) > 1.1*1000.0)
			{
				positionMS -= sign<double>(deltaMS) * 1000.0;

				Mix_SetMusicPosition(positionMS / 1000.0);

				actualPositionMS = getPositionMS();
				deltaMS = actualPositionMS - targetPositionMS;

				loopCounter++;
				if (loopCounter > 10000)
					break;
			}
		}
		else if (percent == 0.0)
			Mix_RewindMusic();
	}

#endif
}

void Sound::setPositionMS(unsigned long ms, bool internal)
{
	if (!m_bReady || ms > getLengthMS()) return;

#ifdef MCENGINE_FEATURE_SOUND

	SOUNDHANDLE handle = getHandle();

	// HACKHACK:
	if (m_bisSpeedAndPitchHackEnabled && !m_bIsOverlayable)
	{
		BASS_ChannelSetPosition(handle, 0, BASS_POS_BYTE);
		m_soundProcUserData->offset = BASS_ChannelSeconds2Bytes(handle, ms/1000.0);
	}
	else
	{
		if (!BASS_ChannelSetPosition(handle, BASS_ChannelSeconds2Bytes(handle, ms/1000.0), BASS_POS_BYTE) && !internal && debug_snd.getBool())
			debugLog("Sound::setPositionMS( %lu ) BASS_ChannelSetPosition() Error %i on %s !\n", ms, BASS_ErrorGetCode(), m_sFilePath.toUtf8());
	}

#elif defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_SDL_MIXER)

	if (m_bStream)
	{
		Mix_RewindMusic();
		Mix_SetMusicPosition((double)ms/1000.0);

		// NOTE: Mix_SetMusicPosition()/scrubbing is inaccurate depending on the underlying decoders, approach in 1 second increments
		const double targetPositionMS = (double)ms;
		double positionMS = targetPositionMS;
		double actualPositionMS = getPositionMS();
		double deltaMS = actualPositionMS - targetPositionMS;
		int loopCounter = 0;
		while (std::abs(deltaMS) > 1.1*1000.0)
		{
			positionMS -= sign<double>(deltaMS) * 1000.0;

			Mix_SetMusicPosition(positionMS / 1000.0);

			actualPositionMS = getPositionMS();
			deltaMS = actualPositionMS - targetPositionMS;

			loopCounter++;
			if (loopCounter > 10000)
				break;
		}
	}

#endif
}

void Sound::setVolume(float volume)
{
	if (!m_bReady || volume < 0.0f || volume > 1.0f) return;

	m_fVolume = clamp<float>(volume, 0.0f, 1.0f);

#ifdef MCENGINE_FEATURE_SOUND

	if (!m_bIsOverlayable)
		BASS_ChannelSetAttribute(getHandle(), BASS_ATTRIB_VOL, m_fVolume);

#elif defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_SDL_MIXER)

	if (m_bStream)
	{
		engine->getSound()->setVolumeMixMusic(m_fVolume);
		Mix_VolumeMusic((int)(m_fVolume*engine->getSound()->getVolume()*MIX_MAX_VOLUME));
	}
	else
		Mix_VolumeChunk((Mix_Chunk*)m_mixChunkOrMixMusic, (int)(m_fVolume*MIX_MAX_VOLUME));

#endif
}

void Sound::setSpeed(float speed)
{
	if (!m_bReady) return;

#ifdef MCENGINE_FEATURE_SOUND

	speed = clamp<float>(speed, 0.05f, 50.0f);

	/*
	if (!m_bStream)
	{
		debugLog("Sound::setSpeed() invalid call, this sound is not a stream!\n");
		return;
	}
	*/

	float originalFreq = 44100.0f;
	BASS_ChannelGetAttribute(m_HSTREAM, BASS_ATTRIB_FREQ, &originalFreq);

	BASS_ChannelSetAttribute(m_HSTREAM, (snd_speed_compensate_pitch.getBool() ? BASS_ATTRIB_TEMPO : BASS_ATTRIB_TEMPO_FREQ), (snd_speed_compensate_pitch.getBool() ? (speed-1.0f)*100.0f : speed*originalFreq));

#endif
}

void Sound::setPitch(float pitch)
{
	if (!m_bReady) return;

#ifdef MCENGINE_FEATURE_SOUND

	pitch = clamp<float>(pitch, 0.0f, 2.0f);

	/*
	if (!m_bStream)
	{
		debugLog("Sound::setPitch() invalid call, this sound is not a stream!\n");
		return;
	}
	*/

	BASS_ChannelSetAttribute(m_HSTREAM, BASS_ATTRIB_TEMPO_PITCH, (pitch-1.0f)*60.0f);

#endif
}

void Sound::setFrequency(float frequency)
{
	if (!m_bReady) return;

#ifdef MCENGINE_FEATURE_SOUND

	if (frequency > 99.0f)
		frequency = clamp<float>(frequency, 100.0f, 100000.0f);
	else
		frequency = 0.0f;

	BASS_ChannelSetAttribute(getHandle(), BASS_ATTRIB_FREQ, frequency);

#endif
}

void Sound::setPan(float pan)
{
	if (!m_bReady) return;

	pan = clamp<float>(pan, -1.0f, 1.0f);

#ifdef MCENGINE_FEATURE_SOUND

	BASS_ChannelSetAttribute(getHandle(), BASS_ATTRIB_PAN, pan);

#elif defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_SDL_MIXER)

	if (!m_bStream)
	{
		const float rangeHalfLimit = 96.0f; // trying to match BASS
		const int left = (int)lerp<float>(rangeHalfLimit/2.0f, 254.0f - rangeHalfLimit/2.0f, 1.0f - ((pan + 1.0f) / 2.0f));
		Mix_SetPanning(getHandle(), left, 254 - left);
	}

#endif
}

float Sound::getPosition()
{
	if (!m_bReady) return 0.0f;

#ifdef MCENGINE_FEATURE_SOUND

	QWORD length = BASS_ChannelGetLength(m_HCHANNELBACKUP, BASS_POS_BYTE);
	QWORD position = BASS_ChannelGetPosition(getHandle(), BASS_POS_BYTE);

	// HACKHACK:
	if (m_bisSpeedAndPitchHackEnabled && !m_bIsOverlayable)
	{
		return (float) ((double)(position + m_soundProcUserData->offset) / (double)(length));
	}
	else
		return (float) ((double)(position) / (double)(length));

#elif defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_SDL_MIXER) && defined(SDL_MIXER_X)

	if (m_bStream)
	{
		const double length = Mix_GetMusicTotalTime((Mix_Music*)m_mixChunkOrMixMusic);
		if (length > 0.0)
		{
			const double position = Mix_GetMusicPosition((Mix_Music*)m_mixChunkOrMixMusic);
			if (position > -0.0)
				return (float)(position / length);
		}
	}

	return 0.0f;

#else
	return 0.0f;
#endif
}

unsigned long Sound::getPositionMS()
{
	if (!m_bReady) return 0;

#ifdef MCENGINE_FEATURE_SOUND

	SOUNDHANDLE handle = getHandle();
	QWORD position = BASS_ChannelGetPosition(handle, BASS_POS_BYTE);

	double positionInSeconds = BASS_ChannelBytes2Seconds(handle, position);
	double positionInMilliSeconds = positionInSeconds * 1000.0;

	// HACKHACK:
	if (m_bisSpeedAndPitchHackEnabled && !m_bIsOverlayable)
	{
		double offsetInSeconds = BASS_ChannelBytes2Seconds(handle, m_soundProcUserData->offset);
		double offsetInMilliSeconds = offsetInSeconds * 1000.0;
		return static_cast<unsigned long>(positionInMilliSeconds + offsetInMilliSeconds);
	}
	else
		return static_cast<unsigned long>(positionInMilliSeconds);

#elif defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_SDL_MIXER) && defined(SDL_MIXER_X)

	if (m_bStream)
	{
		const double position = Mix_GetMusicPosition((Mix_Music*)m_mixChunkOrMixMusic);
		if (position > -0.0)
			return (unsigned long)(position*1000.0);
	}

	return 0;

#else
	return 0;
#endif
}

unsigned long Sound::getLengthMS()
{
	if (!m_bReady) return 0;

#ifdef MCENGINE_FEATURE_SOUND

	SOUNDHANDLE handle = m_bStream ? m_HSTREAM : m_HCHANNELBACKUP;
	QWORD length = BASS_ChannelGetLength(handle, BASS_POS_BYTE);
	double lengthInSeconds = BASS_ChannelBytes2Seconds(handle, length);
	double lengthInMilliSeconds = lengthInSeconds * 1000.0;

	return static_cast<unsigned long>(lengthInMilliSeconds);

#elif defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_SDL_MIXER) && defined(SDL_MIXER_X)

	if (m_bStream)
	{
		const double length = Mix_GetMusicTotalTime((Mix_Music*)m_mixChunkOrMixMusic);
		if (length > -0.0)
			return (unsigned long)(length*1000.0);
	}

	return 0;

#else
	return 0;
#endif
}

float Sound::getSpeed()
{
	if (!m_bReady) return 1.0f;

	/*
	if (!m_bStream)
	{
		debugLog("Sound::getSpeed() invalid call, this sound is not a stream!\n");
		return 0.0f;
	}
	*/

#ifdef MCENGINE_FEATURE_SOUND

	float speed = 0.0f;
	BASS_ChannelGetAttribute(getHandle(), BASS_ATTRIB_TEMPO, &speed);

	return (speed/100.0f)+1.0f;

#else
	return 1.0f;
#endif
}

float Sound::getPitch()
{
	if (!m_bReady) return 1.0f;

	/*
	if (!m_bStream)
	{
		debugLog("Sound::getPitch() invalid call, this sound is not a stream!\n");
		return 0.0f;
	}
	*/

#ifdef MCENGINE_FEATURE_SOUND

	float pitch = 0.0f;
	BASS_ChannelGetAttribute(getHandle(), BASS_ATTRIB_TEMPO_PITCH, &pitch);

	return (pitch/60.0f)+1.0f;

#else
	return 1.0f;
#endif
}

float Sound::getFrequency()
{
	if (!m_bReady) return 44100.0f;

#ifdef MCENGINE_FEATURE_SOUND

	float frequency = 44100.0f;
	BASS_ChannelGetAttribute(getHandle(), BASS_ATTRIB_FREQ, &frequency);

	return frequency;

#else
	return 44100.0f;
#endif
}

bool Sound::isPlaying()
{
	if (!m_bReady) return false;

#ifdef MCENGINE_FEATURE_SOUND

#ifdef MCENGINE_FEATURE_BASS_WASAPI

	DWORD handle = getHandle();

	return BASS_ChannelIsActive(handle) == BASS_ACTIVE_PLAYING && ((!m_bStream && m_bIsOverlayable) || BASS_Mixer_ChannelGetMixer(handle) != 0);

#else

	return BASS_ChannelIsActive(getHandle()) == BASS_ACTIVE_PLAYING;

#endif

#elif defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_SDL_MIXER)

	return (m_bStream ? Mix_PlayingMusic() && !Mix_PausedMusic() : Mix_Playing(m_HCHANNEL) && !Mix_Paused(m_HCHANNEL));

#endif
}

bool Sound::isFinished()
{
	if (!m_bReady) return false;

#ifdef MCENGINE_FEATURE_SOUND

	return BASS_ChannelIsActive(getHandle()) == BASS_ACTIVE_STOPPED;

#elif defined(MCENGINE_FEATURE_SDL) && defined(MCENGINE_FEATURE_SDL_MIXER)

	return (m_bStream ? !Mix_PlayingMusic() && !Mix_PausedMusic() : !Mix_Playing(m_HCHANNEL) && !Mix_Paused(m_HCHANNEL));

#endif
}

void Sound::refactor(UString newFilePath)
{
	// HACKHACK: this refactor function shouldn't exist
	m_sFilePath = newFilePath;
	reload();
}

void Sound::clear()
{
	//if (!(m_bisSpeedAndPitchHackEnabled && !m_bIsOverlayable)) // HACKHACK: this function also shouldn't exist
	{
		m_HCHANNEL = 0;
		m_HCHANNELBACKUP = 0;
	}
}



#ifdef MCENGINE_FEATURE_SOUND

DWORD CALLBACK soundFXCallbackProc(HSTREAM handle, void *buffer, DWORD length, void *user)
{
	Sound::SOUND_PROC_USERDATA *userData = (Sound::SOUND_PROC_USERDATA*)user;

	QWORD fakeStreamPosition = BASS_ChannelGetPosition(handle, BASS_POS_BYTE) + userData->offset;
	BASS_ChannelSetPosition(userData->originalSampleChannel, fakeStreamPosition, BASS_POS_BYTE);

	QWORD actualSampleLength = BASS_ChannelGetLength(userData->originalSampleChannel, BASS_POS_BYTE);

	if (fakeStreamPosition <= actualSampleLength)
		return BASS_ChannelGetData(userData->originalSampleChannel, buffer, length);
	else
		return BASS_STREAMPROC_END;
}

#endif

//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		sound wrapper, either streamed or preloaded
//
// $NoKeywords: $snd $os
//===============================================================================//

#include "Sound.h"
#include "ConVar.h"

#ifdef MCENGINE_FEATURE_SOUND

#include <bass.h>
#include <bass_fx.h>

#endif

#include "Engine.h"

#ifdef MCENGINE_FEATURE_SOUND

DWORD CALLBACK soundFXCallbackProc(HSTREAM handle, void *buffer, DWORD length, void *user);

#endif

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

	m_fLastPlayTime = 0.0;

	m_bisSpeedAndPitchHackEnabled = false;
	m_soundProcUserData = new SOUND_PROC_USERDATA();
}

void Sound::init()
{
#ifdef MCENGINE_FEATURE_SOUND

	if (m_sFilePath.length() < 2 || !m_bAsyncReady) return;

	// error checking
	if (m_HSTREAM == 0)
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

#endif
}

void Sound::initAsync()
{
#ifdef MCENGINE_FEATURE_SOUND

	printf("Resource Manager: Loading %s\n", m_sFilePath.toUtf8());

	// create the sound
	if (m_bStream)
	{

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

		m_HSTREAM = BASS_StreamCreateFile(FALSE, m_sFilePath.wc_str(), 0, 0, BASS_STREAM_DECODE | BASS_UNICODE | (m_bPrescan ? BASS_STREAM_PRESCAN : 0));

#elif defined __linux__

		m_HSTREAM = BASS_StreamCreateFile(FALSE, m_sFilePath.toUtf8(), 0, 0, BASS_STREAM_DECODE | (m_bPrescan ? BASS_STREAM_PRESCAN : 0));

#else

		m_HSTREAM = BASS_StreamCreateFile(FALSE, m_sFilePath.toUtf8(), 0, 0, BASS_STREAM_DECODE | (m_bPrescan ? BASS_STREAM_PRESCAN : 0));

#endif

		m_HSTREAM = BASS_FX_TempoCreate(m_HSTREAM, BASS_FX_FREESOURCE);

		BASS_ChannelSetAttribute(m_HSTREAM, BASS_ATTRIB_TEMPO_OPTION_USE_QUICKALGO, true);
		BASS_ChannelSetAttribute(m_HSTREAM, BASS_ATTRIB_TEMPO_OPTION_OVERLAP_MS, 4.0f);
		BASS_ChannelSetAttribute(m_HSTREAM, BASS_ATTRIB_TEMPO_OPTION_SEQUENCE_MS, 30.0f);

		m_HCHANNELBACKUP = m_HSTREAM;
	}
	else
	{

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

		m_HSTREAM = BASS_SampleLoad(FALSE, m_sFilePath.wc_str(), 0, 0, 24, (m_bIsLooped ? BASS_SAMPLE_LOOP : 0 ) | (m_bIs3d ? BASS_SAMPLE_3D | BASS_SAMPLE_MONO : 0) | BASS_SAMPLE_OVER_VOL | BASS_UNICODE);

#elif defined __linux__

		m_HSTREAM = BASS_SampleLoad(FALSE, m_sFilePath.toUtf8(), 0, 0, 24, (m_bIsLooped ? BASS_SAMPLE_LOOP : 0 ) | (m_bIs3d ? BASS_SAMPLE_3D | BASS_SAMPLE_MONO : 0) | BASS_SAMPLE_OVER_VOL);

#else

		m_HSTREAM = BASS_SampleLoad(FALSE, m_sFilePath.toUtf8(), 0, 0, 24, (m_bIsLooped ? BASS_SAMPLE_LOOP : 0 ) | (m_bIs3d ? BASS_SAMPLE_3D | BASS_SAMPLE_MONO : 0) | BASS_SAMPLE_OVER_VOL);

#endif

		m_HSTREAMBACKUP = m_HSTREAM; // needed for proper cleanup for FX HSAMPLES

		if (m_HSTREAM == 0)
			printf("Sound::initAsync() BASS_SampleLoad() error %i on %s !\n", BASS_ErrorGetCode(), m_sFilePath.toUtf8());
	}

	m_bAsyncReady = true;

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

#else
	return NULL;
#endif
}

void Sound::destroy()
{
#ifdef MCENGINE_FEATURE_SOUND

	if (!m_bReady) return;

	m_bReady = false;

	if (m_bStream)
	{
		BASS_StreamFree(m_HSTREAM); // fx (but with BASS_FX_FREESOURCE)
	}
	else
	{
		if (m_HCHANNEL)
			BASS_ChannelStop(m_HCHANNEL);
		if (m_HSTREAMBACKUP)
			BASS_SampleFree(m_HSTREAMBACKUP);
	}

	m_HSTREAM = 0;
	m_HSTREAMBACKUP = 0;
	m_HCHANNEL = 0;

	SAFE_DELETE(m_soundProcUserData);

#endif
}

void Sound::setPosition(double percent)
{
#ifdef MCENGINE_FEATURE_SOUND

	if (!m_bReady || percent < 0.0f || percent >= 1.0f) return;

	QWORD length = BASS_ChannelGetLength(m_HCHANNELBACKUP, BASS_POS_BYTE);

	// HACKHACK:
	if (m_bisSpeedAndPitchHackEnabled && !m_bIsOverlayable)
	{
		setPositionMS(0, true);
		m_soundProcUserData->offset = (QWORD) ((double)(length)*percent);
	}
	else
	{
		if (!BASS_ChannelSetPosition(getHandle(), (QWORD) ((double)(length)*percent), BASS_POS_BYTE))
			debugLog("Sound::setPosition( %f ) BASS_ChannelSetPosition() Error %i on %s !\n", percent, BASS_ErrorGetCode(), m_sFilePath.toUtf8());
	}

#endif
}

void Sound::setPositionMS(unsigned long ms, bool internal)
{
#ifdef MCENGINE_FEATURE_SOUND

	if (!m_bReady || ms > getLengthMS()) return;

	SOUNDHANDLE handle = getHandle();

	// HACKHACK:
	if (m_bisSpeedAndPitchHackEnabled && !m_bIsOverlayable)
	{
		BASS_ChannelSetPosition(handle, 0, BASS_POS_BYTE);
		m_soundProcUserData->offset = BASS_ChannelSeconds2Bytes(handle, ms/1000.0);
	}
	else
	{
		if (!BASS_ChannelSetPosition(handle, BASS_ChannelSeconds2Bytes(handle, ms/1000.0), BASS_POS_BYTE) && !internal)
			debugLog("Sound::setPositionMS( %lu ) BASS_ChannelSetPosition() Error %i on %s !\n", ms, BASS_ErrorGetCode(), m_sFilePath.toUtf8());
	}

#endif
}

void Sound::setVolume(float volume)
{
#ifdef MCENGINE_FEATURE_SOUND

	if (!m_bReady || volume < 0.0f || volume > 1.0f) return;
	m_fVolume = volume;

	if (!m_bIsOverlayable)
		BASS_ChannelSetAttribute(getHandle(), BASS_ATTRIB_VOL, m_fVolume);

#endif
}

void Sound::setSpeed(float speed)
{
#ifdef MCENGINE_FEATURE_SOUND

	if (!m_bReady) return;

	speed = clamp<float>(speed, 0.05f, 50.0f);

	/*
	if (!m_bStream)
	{
		debugLog("Sound::setSpeed() invalid call, this sound is not a stream!\n");
		return;
	}
	*/

	BASS_ChannelSetAttribute(m_HSTREAM, BASS_ATTRIB_TEMPO, (speed-1.0f)*100.0f);

#endif
}

void Sound::setPitch(float pitch)
{
#ifdef MCENGINE_FEATURE_SOUND

	if (!m_bReady) return;

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
#ifdef MCENGINE_FEATURE_SOUND

	if (!m_bReady) return;

	if (frequency > 99.0f)
		frequency = clamp<float>(frequency, 100.0f, 100000.0f);
	else
		frequency = 0.0f;

	BASS_ChannelSetAttribute(getHandle(), BASS_ATTRIB_FREQ, frequency);

#endif
}

void Sound::setPan(float pan)
{
#ifdef MCENGINE_FEATURE_SOUND

	if (!m_bReady) return;

	pan = clamp<float>(pan, -1.0f, 1.0f);

	BASS_ChannelSetAttribute(getHandle(), BASS_ATTRIB_PAN, pan);

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

#endif
}

float Sound::getSpeed()
{
	if (!m_bReady) return 0.0f;
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

#endif
}

float Sound::getPitch()
{
	if (!m_bReady) return 0.0f;
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

#endif
}

float Sound::getFrequency()
{
	if (!m_bReady) return 44100.0f;

#ifdef MCENGINE_FEATURE_SOUND

	float frequency = 44100.0f;
	BASS_ChannelGetAttribute(getHandle(), BASS_ATTRIB_FREQ, &frequency);

	return frequency;

#endif
}

bool Sound::isPlaying()
{
	if (!m_bReady) return false;

#ifdef MCENGINE_FEATURE_SOUND

	return BASS_ChannelIsActive(getHandle()) == BASS_ACTIVE_PLAYING;

#endif
}

bool Sound::isFinished()
{
	if (!m_bReady) return false;

#ifdef MCENGINE_FEATURE_SOUND

	return BASS_ChannelIsActive(getHandle()) == BASS_ACTIVE_STOPPED;

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

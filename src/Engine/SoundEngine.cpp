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
#include "Sound.h"

SoundEngine::SoundEngine()
{
	m_bReady = false;
	m_iLatency = -1;

#ifdef MCENGINE_FEATURE_SOUND

	int soundDevice = -1; 	// default sounddevice
	int sampleRate = 44100; // sample rate (Hz)

	if (HIWORD(BASS_GetVersion()) != BASSVERSION)
	{
		engine->showMessageErrorFatal("Fatal Sound Error", "An incorrect version of BASS.DLL was loaded!");
		engine->shutdown();
		return;
	}

	BASS_SetConfig(BASS_CONFIG_BUFFER, 100);
	BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD, 10);
	BASS_SetConfig(BASS_CONFIG_UPDATETHREADS, 1);
	BASS_SetConfig(BASS_CONFIG_VISTA_TRUEPOS, 0); // if set to 1, increases latency from 30 ms to 40 ms here

	// init bass library
	if (BASS_Init(soundDevice, sampleRate, BASS_DEVICE_3D | BASS_DEVICE_LATENCY, 0, NULL))
		m_bReady = true;
	else
	{
		engine->showMessageError("Sound Error", "BASS_Init() failed!");
		m_bReady = false;
		return;
	}

	BASS_INFO info;
	BASS_GetInfo(&info);

	m_iLatency = info.minbuf;
	debugLog("SoundEngine: Minimum Latency = %i ms\n", info.latency);

	// load plugins
	/*
	m_FLACPluginHandle = BASS_PluginLoad("bassflac.dll", 0);
	if (m_FLACPluginHandle == NULL)
		engine->showInfo("Sound Error", "Couldn't load bassflac.dll plugin");
	*/

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
	if (!m_bReady || snd == NULL) return false;

#ifdef MCENGINE_FEATURE_SOUND

	if (snd->isReady() && !snd->isPlaying())
	{
		bool ret = BASS_ChannelPlay(snd->getHandle(), FALSE);
		if (!ret)
			debugLog("SOUND: Couldn't BASS_ChannelPlay(), errorcode %i\n", BASS_ErrorGetCode());
		return ret;
	}
	else
		return false;

#endif
}

bool SoundEngine::play3d(Sound *snd, Vector3 pos)
{
	if (!m_bReady || snd == NULL) return false;

#ifdef MCENGINE_FEATURE_SOUND

	if (snd->isReady() && !snd->isPlaying() && snd->is3d())
	{
		DWORD handle = snd->getHandle();

		BASS_3DVECTOR bassPos = BASS_3DVECTOR(pos.x, pos.y, pos.z);
		if (!BASS_ChannelSet3DPosition(handle, &bassPos, NULL, NULL))
			debugLog("SOUND: Couldn't BASS_ChannelSet3DPosition(), errorcode %i\n", BASS_ErrorGetCode());
		else
			BASS_Apply3D(); // apply the changes

		bool ret = BASS_ChannelPlay(handle, FALSE);
		if (!ret)
			debugLog("SOUND: Couldn't BASS_ChannelPlay(), errorcode %i\n", BASS_ErrorGetCode());
		return ret;
	}
	else
		return false;

#endif
}

void SoundEngine::stop(Sound *snd)
{
#ifdef MCENGINE_FEATURE_SOUND

	if (!m_bReady || snd == NULL) return;

	if (snd->isReady())
	{
		BASS_ChannelStop(snd->getHandle());
		snd->setPosition(0.0);
		snd->clear();
	}

#endif
}

void SoundEngine::pause(Sound *snd)
{
#ifdef MCENGINE_FEATURE_SOUND

	if (!m_bReady || snd == NULL) return;

	if (snd->isReady())
		BASS_ChannelPause(snd->getHandle());

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
		debugLog("SOUND: Couldn't BASS_Set3DPosition(), errorcode %i\n", BASS_ErrorGetCode());
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

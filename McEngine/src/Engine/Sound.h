//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		sound wrapper, either streamed or preloaded
//
// $NoKeywords: $snd $os
//===============================================================================//

#ifndef SOUND_H
#define SOUND_H

#include "Resource.h"

typedef unsigned long SOUNDHANDLE;

class Sound : public Resource
{
public:
	Sound(UString filepath, bool stream, bool threeD, bool loop);
	virtual ~Sound() {destroy();}

	void setPosition(double percent);
	void setPositionMS(unsigned long ms);
	void setVolume(float volume);
	void setSpeed(float speed);
	void setPitch(float pitch);
	void setFrequency(float frequency);
	void setOverlayable(bool overlayable) {m_bIsOverlayable = overlayable;}

	SOUNDHANDLE getHandle();
	float getPosition();
	unsigned long getPositionMS();
	unsigned long getLengthMS();
	float getSpeed();
	float getPitch();

	bool isPlaying();
	bool isFinished();
	inline bool is3d() const {return m_bIs3d;}
	inline bool isLooped() const {return m_bIsLooped;}

	void refactor(UString newFilePath);

	// HACKHACK: this is dirty
	void setEnablePitchAndSpeedShiftingHack(bool enableHack) {m_bisSpeedAndPitchHackEnabled = enableHack;}
	void clear();

	struct SOUND_PROC_USERDATA
	{
		SOUNDHANDLE originalSampleChannel;
		long long offset;
	};

private:
	virtual void init();
	virtual void initAsync();
	virtual void destroy();

	SOUND_PROC_USERDATA *m_soundProcUserData;

	float m_fVolume;
	SOUNDHANDLE m_HSTREAM;
	SOUNDHANDLE m_HSTREAMBACKUP;
	SOUNDHANDLE m_HCHANNEL;
	SOUNDHANDLE m_HCHANNELBACKUP;

	bool m_bStream;
	bool m_bIs3d;
	bool m_bIsLooped;
	bool m_bIsOverlayable;

	bool m_bisSpeedAndPitchHackEnabled;
};

#endif

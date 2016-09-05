//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		handles sounds, bass library wrapper atm
//
// $NoKeywords: $snd
//===============================================================================//

#ifndef SOUNDENGINE_H
#define SOUNDENGINE_H

#include "cbase.h"

class Sound;

class SoundEngine
{
public:
	SoundEngine();
	~SoundEngine();

	bool play(Sound *snd);
	bool play3d(Sound *snd, Vector3 pos);
	void stop(Sound *snd);
	void pause(Sound *snd);

	void setVolume(float volume);
	void set3dPosition(Vector3 headPos, Vector3 viewDir, Vector3 viewUp);

	inline int getLatency() const {return m_iLatency;}
	float getAmplitude(Sound *snd);

private:
	bool m_bReady;

	int m_iLatency;
};

#endif

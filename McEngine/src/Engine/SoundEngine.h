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
	void pause(Sound *snd);
	void stop(Sound *snd);

	void setOutputDevice(UString outputDeviceName);
	void setVolume(float volume);
	void set3dPosition(Vector3 headPos, Vector3 viewDir, Vector3 viewUp);

	inline std::vector<UString> getOutputDevices() const {return m_outputDeviceNames;}
	inline UString getOutputDevice() {return m_sCurrentOutputDevice;}
	inline int getLatency() const {return m_iLatency;}
	float getAmplitude(Sound *snd);

private:
	bool initializeOutputDevice(int id);

	bool m_bReady;
	int m_iLatency;

	struct OUTPUT_DEVICE
	{
		int id;
		UString name;
	};
	std::vector<OUTPUT_DEVICE> m_outputDevices;
	std::vector<UString> m_outputDeviceNames;
	int m_iCurrentOutputDevice;
	UString m_sCurrentOutputDevice;
};

#endif

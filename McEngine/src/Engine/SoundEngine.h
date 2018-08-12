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

	void update();

	bool play(Sound *snd, float pan = 0.0f);
	bool play3d(Sound *snd, Vector3 pos);
	void pause(Sound *snd);
	void stop(Sound *snd);

	void setOnOutputDeviceChange(std::function<void()> callback);

	void setOutputDevice(UString outputDeviceName);
	void setVolume(float volume);
	void set3dPosition(Vector3 headPos, Vector3 viewDir, Vector3 viewUp);

	std::vector<UString> getOutputDevices();
	inline UString getOutputDevice() const {return m_sCurrentOutputDevice;}
	inline int getLatency() const {return m_iLatency;}
	float getAmplitude(Sound *snd);

private:
	void updateOutputDevices(bool handleOutputDeviceChanges, bool printInfo);
	bool initializeOutputDevice(int id = -1);

	bool m_bReady;
	int m_iLatency;

	float m_fPrevOutputDeviceChangeCheckTime;

	struct OUTPUT_DEVICE
	{
		int id;
		bool enabled;
		bool isDefault;
		UString name;
	};
	std::function<void()> m_outputDeviceChangeCallback;
	std::vector<OUTPUT_DEVICE> m_outputDevices;

	int m_iCurrentOutputDevice;
	UString m_sCurrentOutputDevice;
};

#endif

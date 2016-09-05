//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		resource manager
//
// $NoKeywords: $rm
//===============================================================================//

#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include "cbase.h"

#include "Image.h"
#include "Font.h"
#include "Sound.h"

#include "pthread.h"
#include <atomic>

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	void update();

	void loadResource(Resource *rs) {requestNextLoadUnmanaged(); loadResource(rs, true);}
	void destroyResource(Resource *rs);
	void destroyResources();
	void reloadResources();

	void requestNextLoadAsync();
	void requestNextLoadUnmanaged();

	Image *loadImage(UString filepath, UString resourceName, bool mipmapped = false);
	Image *loadImageAbs(UString absoluteFilepath, UString resourceName, bool mipmapped = false);
	Image *createImage(int width, int height, bool clampToEdge = true);

	McFont *loadFont(UString filepath, UString resourceName, unsigned int fontSize = 16, bool antialiasing = true);

	Sound *loadSound(UString filepath, UString resourceName, bool stream = false, bool threeD = false, bool loop = false);
	Sound *loadSoundAbs(UString filepath, UString resourceName, bool stream = false, bool threeD = false, bool loop = false);

	Image *getImage(UString resourceName);
	McFont *getFont(UString resourceName);
	Sound *getSound(UString resourceName);

	int getNumResources() const {return m_vResources.size();}
	inline std::vector<Resource*> getResources() const {return m_vResources;}

	template<typename T>
	struct MobileAtomic
	{
		std::atomic<T> atomic;

		MobileAtomic() : atomic(T()) {}

		explicit MobileAtomic ( T const& v ) : atomic ( v ) {}
		explicit MobileAtomic ( std::atomic<T> const& a ) : atomic ( a.load() ) {}

		MobileAtomic ( MobileAtomic const&other ) : atomic( other.atomic.load() ) {}

		MobileAtomic& operator=( MobileAtomic const &other )
		{
			atomic.store( other.atomic.load() );
			return *this;
		}
	};
	typedef MobileAtomic<bool> MobileAtomicBool;

	struct LOAD_THREAD
	{
		pthread_t thread;
		Resource *resource;
		MobileAtomicBool finished;
	};

private:
	void loadResource(Resource *res, bool load);
	void doesntExistWarning(UString resourceName);
	Resource *exists(UString resourceName);

	std::vector<Resource*> m_vResources;
	std::vector<Resource*> m_vAsyncDestroy;
	std::vector<LOAD_THREAD*> m_threads;

	bool m_bNextLoadAsync;
	bool m_bNextLoadUnmanaged;

	std::vector<std::pair<Resource*, MobileAtomicBool>> m_loadingWork;
	std::vector<Resource*> m_loadingWorkAsyncDestroy;
	pthread_t m_loadingThread;
};

#endif

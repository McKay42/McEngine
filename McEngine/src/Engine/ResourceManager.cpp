//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		resource manager
//
// $NoKeywords: $rm
//===============================================================================//

#include "ResourceManager.h"
#include "Engine.h"
#include "ConVar.h"
#include "Timer.h"

#include <mutex>
#include "WinMinGW.Mutex.h"

#include "OpenGLImage.h"

#define RM_IMAGE_FOLDER "materials/"
#define RM_FONT_FOLDER "fonts/"
#define RM_SOUND_FOLDER "sounds/"

ConVar rm_warnings("rm_warnings", false);
ConVar rm_async_rand_delay("rm_async_rand_delay", 0.0f);
ConVar debug_rm("debug_rm", false);

extern bool g_bRunning;

std::mutex g_resourceManagerMutex;
std::mutex g_resourceManagerLoadingMutex;
std::mutex g_resourceManagerLoadingWorkMutex;

void *resourceLoadThread(void *data);
void *resourceLoaderThread(void *data);

ResourceManager::ResourceManager()
{
	m_bNextLoadAsync = false;
	m_bNextLoadUnmanaged = false;

	// build loading thread
	int ret = pthread_create(&m_loadingThread, NULL, resourceLoaderThread, (void*)&m_loadingWork);
	if (ret)
		engine->showMessageError("ResourceManager Error", UString::format("pthread_create() returned %i!", ret));

	// stop loading thread, wait for work
	g_resourceManagerLoadingMutex.lock();
}

ResourceManager::~ResourceManager()
{
	// release all resources
	destroyResources();

	if (m_loadingWork.size() < 1)
		g_resourceManagerLoadingMutex.unlock(); // let it exit
	pthread_join(m_loadingThread, NULL); // TODO: not the best solution. will block shutdown until the current element is loaded
}

void ResourceManager::update()
{
	if (debug_rm.getBool())
	{
		if (m_threads.size() > 0)
			debugLog("Resource Manager: %i active worker thread(s)\n", m_threads.size());
	}

	bool reLock = false;
	g_resourceManagerMutex.lock();
	{
		// handle load finish (and synchronous init())
		for (int i=0; i<m_loadingWork.size(); i++)
		{
			if (m_loadingWork[i].second.atomic)
			{
				if (debug_rm.getBool())
					debugLog("Resource Manager: Worker thread #%i finished.\n", i);

				// copy pointer, so we can stop everything before finishing
				Resource *rs = m_loadingWork[i].first;

				g_resourceManagerLoadingWorkMutex.lock();
					m_loadingWork.erase(m_loadingWork.begin()+i);
				g_resourceManagerLoadingWorkMutex.unlock();
				i--;

				// stop the worker thread if everything has been loaded
				if (m_loadingWork.size() < 1)
					g_resourceManagerLoadingMutex.lock();

				// unlock. this allows resources to trigger "recursive" loads within init()
				g_resourceManagerMutex.unlock();
				reLock = true;

				// finish (synchronous init())
				rs->load();

				break; // only allow 1 work item to finish per tick
			}
		}

	if (reLock)
	{
		g_resourceManagerMutex.lock();
	}

		// handle async destroy
		for (int i=0; i<m_loadingWorkAsyncDestroy.size(); i++)
		{
			bool canBeDestroyed = true;
			for (int w=0; w<m_loadingWork.size(); w++)
			{
				if (m_loadingWork[w].first == m_loadingWorkAsyncDestroy[i])
				{
					if (debug_rm.getBool())
						debugLog("Resource Manager: Waiting for async destroy of #%i ...\n", i);

					canBeDestroyed = false;
					break;
				}
			}

			if (canBeDestroyed)
			{
				if (debug_rm.getBool())
					debugLog("Resource Manager: Async destroy of #%i\n", i);

				m_loadingWorkAsyncDestroy[i]->release();
				delete m_loadingWorkAsyncDestroy[i];
				m_loadingWorkAsyncDestroy.erase(m_loadingWorkAsyncDestroy.begin()+i);
				i--;
			}
		}
	}
	g_resourceManagerMutex.unlock();
}

void ResourceManager::destroyResources()
{
	while (m_vResources.size() > 0)
	{
		destroyResource(m_vResources[0]);
	}
	m_vResources.clear();
}

void ResourceManager::destroyResource(Resource *rs)
{
	if (rs == NULL)
	{
		if (rm_warnings.getBool())
			debugLog("RESOURCE MANAGER Warning: destroyResource(NULL)!\n");
		return;
	}

	g_resourceManagerMutex.lock();
	{

	for (int i=0; i<m_vResources.size(); i++)
	{
		if (m_vResources[i] == rs)
		{
			// handle async destroy
			for (int w=0; w<m_loadingWork.size(); w++)
			{
				if (m_loadingWork[w].first == rs)
				{
					if (debug_rm.getBool())
						debugLog("Resource Manager: Scheduled async destroy of %s\n", rs->getName().toUtf8());

					m_loadingWorkAsyncDestroy.push_back(rs);
					m_vResources.erase(m_vResources.begin()+i);

					// HACKHACK: ugly
					g_resourceManagerMutex.unlock();
					return;
				}
			}

			// standard destroy
			rs->release();
			SAFE_DELETE(rs);
			m_vResources.erase(m_vResources.begin()+i);
			break;
		}
	}

	}
	g_resourceManagerMutex.unlock();
}

void ResourceManager::reloadResources()
{
	for (int i=0; i<m_vResources.size(); i++)
	{
		m_vResources[i]->reload();
	}
}

void ResourceManager::requestNextLoadAsync()
{
	m_bNextLoadAsync = true;
}

void ResourceManager::requestNextLoadUnmanaged()
{
	m_bNextLoadUnmanaged = true;
}

Image *ResourceManager::loadImage(UString filepath, UString resourceName, bool mipmapped)
{
	// check if it already exists
	{
		Resource *temp = exists(resourceName);
		if (temp != NULL)
			return dynamic_cast<Image*>(temp);
	}

	// create instance and load it
	filepath.insert(0, RM_IMAGE_FOLDER);
	Image *img = new OpenGLImage(this, filepath, mipmapped);
	img->setName(resourceName);

	loadResource(img, true);

	return img;
}

Image *ResourceManager::loadImageAbs(UString absoluteFilepath, UString resourceName, bool mipmapped)
{
	// check if it already exists
	{
		Resource *temp = exists(resourceName);
		if (temp != NULL)
			return dynamic_cast<Image*>(temp);
	}

	// create instance and load it
	Image *img = new OpenGLImage(this, absoluteFilepath, mipmapped);
	img->setName(resourceName);

	loadResource(img, true);

	return img;
}

Image *ResourceManager::createImage(int width, int height, bool clampToEdge)
{
	if (width < 1 || height < 1 || width > 4096 || height > 4096)
	{
		engine->showMessageError("Resource Manager Error", UString::format("Invalid parameters in createImage(%i, %i, %i)!\n", width, height, (int)clampToEdge));
		return NULL;
	}

	// create instance and load it
	Image *img = new OpenGLImage(width, height, clampToEdge);
	img->setName("<CREATED_IMAGE>");

	loadResource(img, false);

	return img;
}

McFont *ResourceManager::loadFont(UString filepath, UString resourceName, unsigned int fontSize, bool antialiasing)
{
	// check if it already exists
	{
		Resource *temp = exists(resourceName);
		if (temp != NULL)
			return dynamic_cast<McFont*>(temp);
	}

	// create instance and load it
	filepath.insert(0, RM_FONT_FOLDER);
	McFont *fnt = new McFont(this, filepath, fontSize, antialiasing);
	fnt->setName(resourceName);

	loadResource(fnt, true);

	return fnt;
}

Sound *ResourceManager::loadSound(UString filepath, UString resourceName, bool stream, bool threeD, bool loop)
{
	// check if it already exists
	{
		Resource *temp = exists(resourceName);
		if (temp != NULL)
			return dynamic_cast<Sound*>(temp);
	}

	// create instance and load it
	filepath.insert(0, RM_SOUND_FOLDER);
	Sound *snd = new Sound(this, filepath, stream, threeD, loop);
	snd->setName(resourceName);

	loadResource(snd, true);

	return snd;
}

Sound *ResourceManager::loadSoundAbs(UString filepath, UString resourceName, bool stream, bool threeD, bool loop)
{
	// check if it already exists
	{
		Resource *temp = exists(resourceName);
		if (temp != NULL)
			return dynamic_cast<Sound*>(temp);
	}

	// create instance and load it
	Sound *snd = new Sound(this, filepath, stream, threeD, loop);
	snd->setName(resourceName);

	loadResource(snd, true);

	return snd;
}

Image *ResourceManager::getImage(UString resourceName)
{
	for (int i=0; i<m_vResources.size(); i++)
	{
		if (m_vResources[i]->getName() == resourceName)
			return dynamic_cast<Image*>(m_vResources[i]);
	}

	doesntExistWarning(resourceName);
	return NULL;
}

McFont *ResourceManager::getFont(UString resourceName)
{
	for (int i=0; i<m_vResources.size(); i++)
	{
		if (m_vResources[i]->getName() == resourceName)
			return dynamic_cast<McFont*>(m_vResources[i]);
	}

	doesntExistWarning(resourceName);
	return NULL;
}

Sound *ResourceManager::getSound(UString resourceName)
{
	for (int i=0; i<m_vResources.size(); i++)
	{
		if (m_vResources[i]->getName() == resourceName)
			return dynamic_cast<Sound*>(m_vResources[i]);
	}

	doesntExistWarning(resourceName);
	return NULL;
}

void ResourceManager::loadResource(Resource *res, bool load)
{
	if (!m_bNextLoadUnmanaged)
		m_vResources.push_back(res); // add managed resource
	else
		m_bNextLoadUnmanaged = false;

	if (!load) return;

	if (!m_bNextLoadAsync)
	{
		// load normally
		res->loadAsync();
		res->load();
	}
	else
	{
		m_bNextLoadAsync = false;

		g_resourceManagerMutex.lock();
		{
			// add work to loading thread
			std::pair<Resource*, MobileAtomicBool> work;
			work.first = res;
			work.second = MobileAtomic<bool>(false);

			g_resourceManagerLoadingWorkMutex.lock();
			m_loadingWork.push_back(work);

			// let the loading thread run
			if (m_loadingWork.size() == 1)
				g_resourceManagerLoadingMutex.unlock();
			g_resourceManagerLoadingWorkMutex.unlock();
		}
		g_resourceManagerMutex.unlock();
	}
}

void ResourceManager::doesntExistWarning(UString resourceName)
{
	if (rm_warnings.getBool())
	{
		UString errormsg = "Resource \"";
		errormsg.append(resourceName);
		errormsg.append("\" does not exist!");
		engine->showMessageWarning("RESOURCE MANAGER: ", errormsg);
	}
}

Resource *ResourceManager::exists(UString resourceName)
{
	for (int i=0; i<m_vResources.size(); i++)
	{
		if (m_vResources[i]->getName() == resourceName)
		{
			if (rm_warnings.getBool())
			{
				UString errormsg = "Resource \"";
				errormsg.append(resourceName);
				errormsg.append("\" already loaded!");
				debugLog("RESOURCE MANAGER: %s\n",errormsg.toUtf8());
			}

			return m_vResources[i];
		}
	}

	return NULL;
}



void *resourceLoadThread(void *data)
{
	// debugging
	if (rm_async_rand_delay.getInt() > 0)
	{
		Timer sleepTimer;
		sleepTimer.start();
		double randSleepSecs = rand() % rm_async_rand_delay.getInt();
		while (sleepTimer.getElapsedTime() < randSleepSecs)
		{
			sleepTimer.update();
		}
	}

	ResourceManager::LOAD_THREAD *loader = (ResourceManager::LOAD_THREAD*)data;
	loader->resource->loadAsync();
	loader->finished = ResourceManager::MobileAtomic<bool>(true);
	return NULL;
}

void *resourceLoaderThread(void *data)
{
	std::vector<std::pair<Resource*, ResourceManager::MobileAtomicBool>> *todo = (std::vector<std::pair<Resource*, ResourceManager::MobileAtomicBool>>*)data;

	while (g_bRunning)
	{
		// wait for work
		g_resourceManagerLoadingMutex.lock(); // thread will wait here
		g_resourceManagerLoadingMutex.unlock();

		// quickly check if there is work to do (this can potentially cause engine lag!)
		g_resourceManagerLoadingWorkMutex.lock();
		int size = todo->size();
		std::pair<Resource*, ResourceManager::MobileAtomicBool> work;
		if (size > 0)
			work = (*todo)[0];
		g_resourceManagerLoadingWorkMutex.unlock();

		// if we have work, do it after unlocking the work mutex
		if (size > 0)
		{
			if (!(work.second.atomic)) // if we need to get loaded
			{
				// debugging
				if (rm_async_rand_delay.getInt() > 0)
				{
					Timer sleepTimer;
					sleepTimer.start();
					double randSleepSecs = rand() % rm_async_rand_delay.getInt();
					while (sleepTimer.getElapsedTime() < randSleepSecs)
					{
						sleepTimer.update();
					}
				}

				work.first->loadAsync();

				// very quickly signal that we are done
				g_resourceManagerLoadingWorkMutex.lock();
				(*todo)[0].second = ResourceManager::MobileAtomic<bool>(true);
				g_resourceManagerLoadingWorkMutex.unlock();
			}
		}
	}

	return NULL;
}

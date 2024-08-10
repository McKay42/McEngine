//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		resource manager
//
// $NoKeywords: $rm
//===============================================================================//

#include "ResourceManager.h"

#include "Environment.h"
#include "Engine.h"
#include "ConVar.h"
#include "Timer.h"
#include "Thread.h"

#ifdef MCENGINE_FEATURE_MULTITHREADING

#include <mutex>
#include "WinMinGW.Mutex.h"

static std::mutex g_resourceManagerMutex;				// internal lock for nested async loads
static std::mutex g_resourceManagerLoadingWorkMutex;	// work vector lock across all threads

static void *_resourceLoaderThread(void *data);

#endif

class ResourceManagerLoaderThread
{
public:
#ifdef MCENGINE_FEATURE_MULTITHREADING

	// self
	McThread *thread;

	// wait lock
	std::mutex loadingMutex;

	// args
	std::atomic<size_t> threadIndex;
	std::atomic<bool> running;
	std::vector<ResourceManager::LOADING_WORK> *loadingWork;

#endif
};

ConVar rm_numthreads("rm_numthreads", 3, FCVAR_NONE, "how many parallel resource loader threads are spawned once on startup (!), and subsequently used during runtime");
ConVar rm_warnings("rm_warnings", false, FCVAR_NONE);
ConVar rm_debug_async_delay("rm_debug_async_delay", 0.0f, FCVAR_CHEAT);
ConVar rm_interrupt_on_destroy("rm_interrupt_on_destroy", true, FCVAR_CHEAT);
ConVar debug_rm_("debug_rm", false, FCVAR_NONE);

ConVar *ResourceManager::debug_rm = &debug_rm_;

// HACKHACK: do this with env->getOS() or something
#ifdef __SWITCH__

const char *ResourceManager::PATH_DEFAULT_IMAGES = "romfs:/materials/";
const char *ResourceManager::PATH_DEFAULT_FONTS = "romfs:/fonts/";
const char *ResourceManager::PATH_DEFAULT_SOUNDS = "romfs:/sounds/";
const char *ResourceManager::PATH_DEFAULT_SHADERS = "romfs:/shaders/";

#else

const char *ResourceManager::PATH_DEFAULT_IMAGES = "materials/";
const char *ResourceManager::PATH_DEFAULT_FONTS = "fonts/";
const char *ResourceManager::PATH_DEFAULT_SOUNDS = "sounds/";
const char *ResourceManager::PATH_DEFAULT_SHADERS = "shaders/";

#endif

ResourceManager::ResourceManager()
{
	m_bNextLoadAsync = false;
	m_iNumResourceInitPerFrameLimit = 1;

	m_loadingWork.reserve(32);

	// OS specific engine settings/overrides
	if (env->getOS() == Environment::OS::OS_HORIZON)
	{
		rm_numthreads.setValue(1.0f);
		rm_numthreads.setDefaultFloat(1.0f);
	}

	// create loader threads
#ifdef MCENGINE_FEATURE_MULTITHREADING

	for (int i=0; i<rm_numthreads.getInt(); i++)
	{
		ResourceManagerLoaderThread *loaderThread = new ResourceManagerLoaderThread();

		loaderThread->loadingMutex.lock(); // stop loader thread immediately, wait for work
		loaderThread->threadIndex = i;
		loaderThread->running = true;
		loaderThread->loadingWork = &m_loadingWork;

		loaderThread->thread = new McThread(_resourceLoaderThread, (void*)loaderThread);
		if (!loaderThread->thread->isReady())
		{
			engine->showMessageError("ResourceManager Error", "Couldn't create thread!");
			SAFE_DELETE(loaderThread->thread);
			SAFE_DELETE(loaderThread);
		}
		else
			m_threads.push_back(loaderThread);
	}

#endif
}

ResourceManager::~ResourceManager()
{
	// release all not-currently-being-loaded resources (1)
	destroyResources();

	// let all loader threads exit
#ifdef MCENGINE_FEATURE_MULTITHREADING

	for (size_t i=0; i<m_threads.size(); i++)
	{
		m_threads[i]->running = false;
	}

	for (size_t i=0; i<m_threads.size(); i++)
	{
		const size_t threadIndex = m_threads[i]->threadIndex.load();

		bool hasLoadingWork = false;
		for (size_t w=0; w<m_loadingWork.size(); w++)
		{
			if (m_loadingWork[w].threadIndex.atomic.load() == threadIndex)
			{
				hasLoadingWork = true;
				break;
			}
		}

		if (!hasLoadingWork)
			m_threads[i]->loadingMutex.unlock();
	}

	// wait for threads to stop
	for (size_t i=0; i<m_threads.size(); i++)
	{
		delete m_threads[i]->thread;
	}

	m_threads.clear();

#endif

	// cleanup leftovers (can only do that after loader threads have exited) (2)
	for (size_t i=0; i<m_loadingWorkAsyncDestroy.size(); i++)
	{
		delete m_loadingWorkAsyncDestroy[i];
	}
	m_loadingWorkAsyncDestroy.clear();
}

void ResourceManager::update()
{
	bool reLock = false;

#ifdef MCENGINE_FEATURE_MULTITHREADING

	g_resourceManagerMutex.lock();

#endif
	{
		// handle load finish (and synchronous init())
		size_t numResourceInitCounter = 0;
		for (size_t i=0; i<m_loadingWork.size(); i++)
		{
			if (m_loadingWork[i].done.atomic.load())
			{
				if (debug_rm->getBool())
					debugLog("Resource Manager: Worker thread #%i finished.\n", i);

				// copy pointer, so we can stop everything before finishing
				Resource *rs = m_loadingWork[i].resource.atomic.load();
				const size_t threadIndex = m_loadingWork[i].threadIndex.atomic.load();

#ifdef MCENGINE_FEATURE_MULTITHREADING

				g_resourceManagerLoadingWorkMutex.lock();

#endif

				{
					m_loadingWork.erase(m_loadingWork.begin() + i);
				}

#ifdef MCENGINE_FEATURE_MULTITHREADING

				g_resourceManagerLoadingWorkMutex.unlock();

#endif

				i--;

#ifdef MCENGINE_FEATURE_MULTITHREADING

				// stop this worker thread if everything has been loaded
				int numLoadingWorkForThreadIndex = 0;
				for (size_t w=0; w<m_loadingWork.size(); w++)
				{
					if (m_loadingWork[w].threadIndex.atomic.load() == threadIndex)
						numLoadingWorkForThreadIndex++;
				}

				if (numLoadingWorkForThreadIndex < 1)
				{
					if (m_threads.size() > 0)
						m_threads[threadIndex]->loadingMutex.lock();
				}

				// unlock. this allows resources to trigger "recursive" loads within init()
				g_resourceManagerMutex.unlock();

#endif

				reLock = true;

				// check if this was an async destroy, can skip load() if that is the case
				// TODO: this will probably break stuff, needs in depth testing before change, think more about this
				/*
				bool isAsyncDestroy = false;
				for (size_t a=0; a<m_loadingWorkAsyncDestroy.size(); a++)
				{
					if (m_loadingWorkAsyncDestroy[a] == rs)
					{
						isAsyncDestroy = true;
						break;
					}
				}
				*/

				// finish (synchronous init())
				//if (!isAsyncDestroy)

				rs->load();
				numResourceInitCounter++;

				//else if (debug_rm->getBool())
				//	debugLog("Resource Manager: Skipping load() due to async destroy of #%i\n", (i + 1));

				if (m_iNumResourceInitPerFrameLimit > 0 && numResourceInitCounter >= m_iNumResourceInitPerFrameLimit)
					break; // NOTE: only allow 1 work item to finish per frame (avoid stutters for e.g. texture uploads)
				else
				{
					if (reLock)
					{
						reLock = false;
						g_resourceManagerMutex.lock();
					}
				}
			}
		}

#ifdef MCENGINE_FEATURE_MULTITHREADING

	if (reLock)
	{
		g_resourceManagerMutex.lock();
	}

#endif

		// handle async destroy
		for (size_t i=0; i<m_loadingWorkAsyncDestroy.size(); i++)
		{
			bool canBeDestroyed = true;
			for (size_t w=0; w<m_loadingWork.size(); w++)
			{
				if (m_loadingWork[w].resource.atomic.load() == m_loadingWorkAsyncDestroy[i])
				{
					if (debug_rm->getBool())
						debugLog("Resource Manager: Waiting for async destroy of #%i ...\n", i);

					canBeDestroyed = false;
					break;
				}
			}

			if (canBeDestroyed)
			{
				if (debug_rm->getBool())
					debugLog("Resource Manager: Async destroy of #%i\n", i);

				delete m_loadingWorkAsyncDestroy[i]; // implicitly calls release() through the Resource destructor
				m_loadingWorkAsyncDestroy.erase(m_loadingWorkAsyncDestroy.begin() + i);
				i--;
			}
		}
	}
#ifdef MCENGINE_FEATURE_MULTITHREADING

	g_resourceManagerMutex.unlock();

#endif
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

	if (debug_rm->getBool())
		debugLog("ResourceManager: Destroying %s\n", rs->getName().toUtf8());

#ifdef MCENGINE_FEATURE_MULTITHREADING

	g_resourceManagerMutex.lock();

#endif
	{
		bool isManagedResource = false;
		int managedResourceIndex = -1;
		for (size_t i=0; i<m_vResources.size(); i++)
		{
			if (m_vResources[i] == rs)
			{
				isManagedResource = true;
				managedResourceIndex = i;
				break;
			}
		}

		// handle async destroy
		for (size_t w=0; w<m_loadingWork.size(); w++)
		{
			if (m_loadingWork[w].resource.atomic.load() == rs)
			{
				if (debug_rm->getBool())
					debugLog("Resource Manager: Scheduled async destroy of %s\n", rs->getName().toUtf8());

				if (rm_interrupt_on_destroy.getBool())
					rs->interruptLoad();

				m_loadingWorkAsyncDestroy.push_back(rs);
				if (isManagedResource)
					m_vResources.erase(m_vResources.begin() + managedResourceIndex);

				// NOTE: ugly
#ifdef MCENGINE_FEATURE_MULTITHREADING

				g_resourceManagerMutex.unlock();

#endif
				return; // we're done here
			}
		}

		// standard destroy
		SAFE_DELETE(rs);

		if (isManagedResource)
			m_vResources.erase(m_vResources.begin() + managedResourceIndex);
	}
#ifdef MCENGINE_FEATURE_MULTITHREADING

	g_resourceManagerMutex.unlock();

#endif
}

void ResourceManager::reloadResources()
{
	for (size_t i=0; i<m_vResources.size(); i++)
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
	m_nextLoadUnmanagedStack.push(true);
}

Image *ResourceManager::loadImage(UString filepath, UString resourceName, bool mipmapped, bool keepInSystemMemory)
{
	// check if it already exists
	if (resourceName.length() > 0)
	{
		Resource *temp = checkIfExistsAndHandle(resourceName);
		if (temp != NULL)
			return dynamic_cast<Image*>(temp);
	}

	// create instance and load it
	filepath.insert(0, PATH_DEFAULT_IMAGES);
	Image *img = engine->getGraphics()->createImage(filepath, mipmapped, keepInSystemMemory);
	img->setName(resourceName);

	loadResource(img, true);

	return img;
}

Image *ResourceManager::loadImageUnnamed(UString filepath, bool mipmapped, bool keepInSystemMemory)
{
	filepath.insert(0, PATH_DEFAULT_IMAGES);
	Image *img = engine->getGraphics()->createImage(filepath, mipmapped, keepInSystemMemory);

	loadResource(img, true);

	return img;
}

Image *ResourceManager::loadImageAbs(UString absoluteFilepath, UString resourceName, bool mipmapped, bool keepInSystemMemory)
{
	// check if it already exists
	if (resourceName.length() > 0)
	{
		Resource *temp = checkIfExistsAndHandle(resourceName);
		if (temp != NULL)
			return dynamic_cast<Image*>(temp);
	}

	// create instance and load it
	Image *img = engine->getGraphics()->createImage(absoluteFilepath, mipmapped, keepInSystemMemory);
	img->setName(resourceName);

	loadResource(img, true);

	return img;
}

Image *ResourceManager::loadImageAbsUnnamed(UString absoluteFilepath, bool mipmapped, bool keepInSystemMemory)
{
	Image *img = engine->getGraphics()->createImage(absoluteFilepath, mipmapped, keepInSystemMemory);

	loadResource(img, true);

	return img;
}

Image *ResourceManager::createImage(unsigned int width, unsigned int height, bool mipmapped, bool keepInSystemMemory)
{
	if (width > 8192 || height > 8192)
	{
		engine->showMessageError("Resource Manager Error", UString::format("Invalid parameters in createImage(%i, %i, %i)!\n", width, height, (int)mipmapped));
		return NULL;
	}

	Image *img = engine->getGraphics()->createImage(width, height, mipmapped, keepInSystemMemory);

	loadResource(img, false);

	return img;
}

McFont *ResourceManager::loadFont(UString filepath, UString resourceName, int fontSize, bool antialiasing, int fontDPI)
{
	// check if it already exists
	if (resourceName.length() > 0)
	{
		Resource *temp = checkIfExistsAndHandle(resourceName);
		if (temp != NULL)
			return dynamic_cast<McFont*>(temp);
	}

	// create instance and load it
	filepath.insert(0, PATH_DEFAULT_FONTS);
	McFont *fnt = new McFont(filepath, fontSize, antialiasing, fontDPI);
	fnt->setName(resourceName);

	loadResource(fnt, true);

	return fnt;
}

McFont *ResourceManager::loadFont(UString filepath, UString resourceName, std::vector<wchar_t> characters, int fontSize, bool antialiasing, int fontDPI)
{
	// check if it already exists
	if (resourceName.length() > 0)
	{
		Resource *temp = checkIfExistsAndHandle(resourceName);
		if (temp != NULL)
			return dynamic_cast<McFont*>(temp);
	}

	// create instance and load it
	filepath.insert(0, PATH_DEFAULT_FONTS);
	McFont *fnt = new McFont(filepath, characters, fontSize, antialiasing, fontDPI);
	fnt->setName(resourceName);

	loadResource(fnt, true);

	return fnt;
}

Sound *ResourceManager::loadSound(UString filepath, UString resourceName, bool stream, bool threeD, bool loop, bool prescan)
{
	// check if it already exists
	if (resourceName.length() > 0)
	{
		Resource *temp = checkIfExistsAndHandle(resourceName);
		if (temp != NULL)
			return dynamic_cast<Sound*>(temp);
	}

	// create instance and load it
	filepath.insert(0, PATH_DEFAULT_SOUNDS);
	Sound *snd = new Sound(filepath, stream, threeD, loop, prescan);
	snd->setName(resourceName);

	loadResource(snd, true);

	return snd;
}

Sound *ResourceManager::loadSoundAbs(UString filepath, UString resourceName, bool stream, bool threeD, bool loop, bool prescan)
{
	// check if it already exists
	if (resourceName.length() > 0)
	{
		Resource *temp = checkIfExistsAndHandle(resourceName);
		if (temp != NULL)
			return dynamic_cast<Sound*>(temp);
	}

	// create instance and load it
	Sound *snd = new Sound(filepath, stream, threeD, loop, prescan);
	snd->setName(resourceName);

	loadResource(snd, true);

	return snd;
}

Shader *ResourceManager::loadShader(UString vertexShaderFilePath, UString fragmentShaderFilePath, UString resourceName)
{
	// check if it already exists
	if (resourceName.length() > 0)
	{
		Resource *temp = checkIfExistsAndHandle(resourceName);
		if (temp != NULL)
			return dynamic_cast<Shader*>(temp);
	}

	// create instance and load it
	vertexShaderFilePath.insert(0, PATH_DEFAULT_SHADERS);
	fragmentShaderFilePath.insert(0, PATH_DEFAULT_SHADERS);
	Shader *shader = engine->getGraphics()->createShaderFromFile(vertexShaderFilePath, fragmentShaderFilePath);
	shader->setName(resourceName);

	loadResource(shader, true);

	return shader;
}

Shader *ResourceManager::loadShader(UString vertexShaderFilePath, UString fragmentShaderFilePath)
{
	vertexShaderFilePath.insert(0, PATH_DEFAULT_SHADERS);
	fragmentShaderFilePath.insert(0, PATH_DEFAULT_SHADERS);
	Shader *shader = engine->getGraphics()->createShaderFromFile(vertexShaderFilePath, fragmentShaderFilePath);

	loadResource(shader, true);

	return shader;
}

Shader *ResourceManager::createShader(UString vertexShader, UString fragmentShader, UString resourceName)
{
	// check if it already exists
	if (resourceName.length() > 0)
	{
		Resource *temp = checkIfExistsAndHandle(resourceName);
		if (temp != NULL)
			return dynamic_cast<Shader*>(temp);
	}

	// create instance and load it
	Shader *shader = engine->getGraphics()->createShaderFromSource(vertexShader, fragmentShader);
	shader->setName(resourceName);

	loadResource(shader, true);

	return shader;
}

Shader *ResourceManager::createShader(UString vertexShader, UString fragmentShader)
{
	Shader *shader = engine->getGraphics()->createShaderFromSource(vertexShader, fragmentShader);

	loadResource(shader, true);

	return shader;
}

Shader *ResourceManager::loadShader2(UString shaderFilePath, UString resourceName)
{
	// check if it already exists
	if (resourceName.length() > 0)
	{
		Resource *temp = checkIfExistsAndHandle(resourceName);
		if (temp != NULL)
			return dynamic_cast<Shader*>(temp);
	}

	// create instance and load it
	shaderFilePath.insert(0, PATH_DEFAULT_SHADERS);
	Shader *shader = engine->getGraphics()->createShaderFromFile(shaderFilePath);
	shader->setName(resourceName);

	loadResource(shader, true);

	return shader;
}

Shader *ResourceManager::loadShader2(UString shaderFilePath)
{
	shaderFilePath.insert(0, PATH_DEFAULT_SHADERS);
	Shader *shader = engine->getGraphics()->createShaderFromFile(shaderFilePath);

	loadResource(shader, true);

	return shader;
}

Shader *ResourceManager::createShader2(UString shaderSource, UString resourceName)
{
	// check if it already exists
	if (resourceName.length() > 0)
	{
		Resource *temp = checkIfExistsAndHandle(resourceName);
		if (temp != NULL)
			return dynamic_cast<Shader*>(temp);
	}

	// create instance and load it
	Shader *shader = engine->getGraphics()->createShaderFromSource(shaderSource);
	shader->setName(resourceName);

	loadResource(shader, true);

	return shader;
}

Shader *ResourceManager::createShader2(UString shaderSource)
{
	Shader *shader = engine->getGraphics()->createShaderFromSource(shaderSource);

	loadResource(shader, true);

	return shader;
}

RenderTarget *ResourceManager::createRenderTarget(int x, int y, int width, int height, Graphics::MULTISAMPLE_TYPE multiSampleType)
{
	RenderTarget *rt = engine->getGraphics()->createRenderTarget(x, y, width, height, multiSampleType);
	rt->setName(UString::format("_RT_%ix%i", width, height));

	loadResource(rt, true);

	return rt;
}

RenderTarget *ResourceManager::createRenderTarget(int width, int height, Graphics::MULTISAMPLE_TYPE multiSampleType)
{
	return createRenderTarget(0, 0, width, height, multiSampleType);
}

TextureAtlas *ResourceManager::createTextureAtlas(int width, int height)
{
	TextureAtlas *ta = new TextureAtlas(width, height);
	ta->setName(UString::format("_TA_%ix%i", width, height));

	loadResource(ta, false);

	return ta;
}

VertexArrayObject *ResourceManager::createVertexArrayObject(Graphics::PRIMITIVE primitive, Graphics::USAGE_TYPE usage, bool keepInSystemMemory)
{
	VertexArrayObject *vao = engine->getGraphics()->createVertexArrayObject(primitive, usage, keepInSystemMemory);

	loadResource(vao, false);

	return vao;
}

Image *ResourceManager::getImage(UString resourceName) const
{
	for (size_t i=0; i<m_vResources.size(); i++)
	{
		if (m_vResources[i]->getName() == resourceName)
			return dynamic_cast<Image*>(m_vResources[i]);
	}

	doesntExistWarning(resourceName);
	return NULL;
}

McFont *ResourceManager::getFont(UString resourceName) const
{
	for (size_t i=0; i<m_vResources.size(); i++)
	{
		if (m_vResources[i]->getName() == resourceName)
			return dynamic_cast<McFont*>(m_vResources[i]);
	}

	doesntExistWarning(resourceName);
	return NULL;
}

Sound *ResourceManager::getSound(UString resourceName) const
{
	for (size_t i=0; i<m_vResources.size(); i++)
	{
		if (m_vResources[i]->getName() == resourceName)
			return dynamic_cast<Sound*>(m_vResources[i]);
	}

	doesntExistWarning(resourceName);
	return NULL;
}

Shader *ResourceManager::getShader(UString resourceName) const
{
	for (size_t i=0; i<m_vResources.size(); i++)
	{
		if (m_vResources[i]->getName() == resourceName)
			return dynamic_cast<Shader*>(m_vResources[i]);
	}

	doesntExistWarning(resourceName);
	return NULL;
}

bool ResourceManager::isLoading() const
{
	return (m_loadingWork.size() > 0);
}

bool ResourceManager::isLoadingResource(Resource *rs) const
{
	for (size_t i=0; i<m_loadingWork.size(); i++)
	{
		if (m_loadingWork[i].resource.atomic.load() == rs)
			return true;
	}

	return false;
}

void ResourceManager::loadResource(Resource *res, bool load)
{
	// handle flags
	if (m_nextLoadUnmanagedStack.size() < 1 || !m_nextLoadUnmanagedStack.top())
		m_vResources.push_back(res); // add managed resource

	const bool isNextLoadAsync = m_bNextLoadAsync;

	// flags must be reset on every load, to not carry over
	resetFlags();

	if (!load) return;

	if (!isNextLoadAsync)
	{
		// load normally
		res->loadAsync();
		res->load();
	}
	else
	{
#if defined(MCENGINE_FEATURE_MULTITHREADING)

		if (rm_numthreads.getInt() > 0)
		{
			g_resourceManagerMutex.lock();
			{
				// TODO: prefer thread which currently doesn't have anything to do (i.e. allow n-1 "permanent" background tasks without blocking)

				// split work evenly/linearly across all threads
				static size_t threadIndexCounter = 0;
				const size_t threadIndex = threadIndexCounter;

				// add work to loading thread
				LOADING_WORK work;

				work.resource = MobileAtomicResource(res);
				work.threadIndex = MobileAtomicSizeT(threadIndex);
				work.done = MobileAtomicBool(false);

				threadIndexCounter = (threadIndexCounter + 1) % (std::min(m_threads.size(), (size_t)std::max(rm_numthreads.getInt(), 1)));

				g_resourceManagerLoadingWorkMutex.lock();
				{
					m_loadingWork.push_back(work);

					int numLoadingWorkForThreadIndex = 0;
					for (size_t i=0; i<m_loadingWork.size(); i++)
					{
						if (m_loadingWork[i].threadIndex.atomic.load() == threadIndex)
							numLoadingWorkForThreadIndex++;
					}

					// let the loading thread run
					if (numLoadingWorkForThreadIndex == 1) // only necessary if thread is waiting (otherwise it will already be picked up by the next iteration)
					{
						if (m_threads.size() > 0)
							m_threads[threadIndex]->loadingMutex.unlock();
					}
				}
				g_resourceManagerLoadingWorkMutex.unlock();
			}
			g_resourceManagerMutex.unlock();
		}
		else
		{
			// load normally (threading disabled)
			res->loadAsync();
			res->load();
		}

#else

		// load normally (on platforms which don't support multithreading)
		res->loadAsync();
		res->load();

#endif
	}
}

void ResourceManager::doesntExistWarning(UString resourceName) const
{
	if (rm_warnings.getBool())
	{
		UString errormsg = "Resource \"";
		errormsg.append(resourceName);
		errormsg.append("\" does not exist!");
		engine->showMessageWarning("RESOURCE MANAGER: ", errormsg);
	}
}

Resource *ResourceManager::checkIfExistsAndHandle(UString resourceName)
{
	for (size_t i=0; i<m_vResources.size(); i++)
	{
		if (m_vResources[i]->getName() == resourceName)
		{
			if (rm_warnings.getBool())
				debugLog("RESOURCE MANAGER: Resource \"%s\" already loaded!\n", resourceName.toUtf8());

			// handle flags (reset them)
			resetFlags();

			return m_vResources[i];
		}
	}

	return NULL;
}

void ResourceManager::resetFlags()
{
	if (m_nextLoadUnmanagedStack.size() > 0)
		m_nextLoadUnmanagedStack.pop();

	m_bNextLoadAsync = false;
}



#ifdef MCENGINE_FEATURE_MULTITHREADING

static void *_resourceLoaderThread(void *data)
{
	ResourceManagerLoaderThread *self = (ResourceManagerLoaderThread*)data;

	const size_t threadIndex = self->threadIndex.load();

	while (self->running.load())
	{
		// wait for work
		self->loadingMutex.lock(); // thread will wait here if locked by engine
		self->loadingMutex.unlock();

		Resource *resourceToLoad = NULL;

		// quickly check if there is work to do (this can potentially cause engine lag!)
		// NOTE: we can't keep references to shared loadingWork objects (vector realloc/erase/etc.)
		g_resourceManagerLoadingWorkMutex.lock();
		{
			for (size_t i=0; i<self->loadingWork->size(); i++)
			{
				if ((*self->loadingWork)[i].threadIndex.atomic.load() == threadIndex && !(*self->loadingWork)[i].done.atomic.load())
				{
					resourceToLoad = (*self->loadingWork)[i].resource.atomic.load();
					break;
				}
			}
		}
		g_resourceManagerLoadingWorkMutex.unlock();

		// if we have work
		if (resourceToLoad != NULL)
		{
			// debug
			if (rm_debug_async_delay.getFloat() > 0.0f)
				env->sleep(rm_debug_async_delay.getFloat() * 1000 * 1000);

			// asynchronous initAsync()
			resourceToLoad->loadAsync();

			// very quickly signal that we are done
			g_resourceManagerLoadingWorkMutex.lock();
			{
				for (size_t i=0; i<self->loadingWork->size(); i++)
				{
					if ((*self->loadingWork)[i].threadIndex.atomic.load() == threadIndex && (*self->loadingWork)[i].resource.atomic.load() == resourceToLoad)
					{
						(*self->loadingWork)[i].done = ResourceManager::MobileAtomicBool(true);
						break;
					}
				}
			}
			g_resourceManagerLoadingWorkMutex.unlock();
		}
		else
			env->sleep(1000); // 1000 Hz sanity limit until locked again
	}

	return NULL;
}

#endif

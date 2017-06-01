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
#include "Shader.h"
#include "RenderTarget.h"
#include "VertexArrayObject.h"

#include "pthread.h"

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

	// images
	Image *loadImage(UString filepath, UString resourceName, bool mipmapped = false);
	Image *loadImageUnnamed(UString filepath, bool mipmapped = false);
	Image *loadImageAbs(UString absoluteFilepath, UString resourceName, bool mipmapped = false);
	Image *loadImageAbsUnnamed(UString absoluteFilepath, bool mipmapped = false);
	Image *createImage(unsigned int width, unsigned int height, bool mipmapped = false);

	// fonts
	McFont *loadFont(UString filepath, UString resourceName, unsigned int fontSize = 16, bool antialiasing = true);

	// sounds
	Sound *loadSound(UString filepath, UString resourceName, bool stream = false, bool threeD = false, bool loop = false, bool prescan = false);
	Sound *loadSoundAbs(UString filepath, UString resourceName, bool stream = false, bool threeD = false, bool loop = false, bool prescan = false);

	// shaders
	Shader *loadShader(UString vertexShaderFilePath, UString fragmentShaderFilePath, UString resourceName);
	Shader *loadShader(UString vertexShaderFilePath, UString fragmentShaderFilePath);
	Shader *createShader(UString vertexShader, UString fragmentShader, UString resourceName);
	Shader *createShader(UString vertexShader, UString fragmentShader);

	// rendertargets
	RenderTarget *createRenderTarget(int x, int y, int width, int height, Graphics::MULTISAMPLE_TYPE multiSampleType = Graphics::MULTISAMPLE_TYPE::MULTISAMPLE_0X);
	RenderTarget *createRenderTarget(int width, int height, Graphics::MULTISAMPLE_TYPE multiSampleType = Graphics::MULTISAMPLE_TYPE::MULTISAMPLE_0X);

	// models/meshes
	VertexArrayObject *createVertexArrayObject(Graphics::PRIMITIVE primitive = Graphics::PRIMITIVE::PRIMITIVE_TRIANGLES, Graphics::USAGE_TYPE usage = Graphics::USAGE_TYPE::USAGE_STATIC);

	// resource access by name // TODO: should probably use generics for this
	Image *getImage(UString resourceName);
	McFont *getFont(UString resourceName);
	Sound *getSound(UString resourceName);
	Shader *getShader(UString resourceName);

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

//================ Copyright (c) 2012, PG, All rights reserved. =================//
//
// Purpose:		base class for resources
//
// $NoKeywords: $res
//===============================================================================//

#include "Resource.h"
#include "Engine.h"
#include "Environment.h"

Resource::Resource(ResourceManager *loader, UString filepath)
{
	if (loader == NULL)
		engine->showMessageError("Resource Error", "Do not create resources without using a ResourceManager, you idiot >:(");
	if (filepath.length() > 0 && !env->fileExists(filepath))
	{
		UString errorMessage = "File does not exist: ";
		errorMessage.append(filepath);
		debugLog("Resource Warning: File %s does not exist!\n", filepath.toUtf8());
		///engine->showMessageError("Resource Error", errorMessage);
	}

	m_resourceManager = loader;
	m_sFilePath = filepath;
	m_bReady = false;
	m_bAsyncReady = false;
}

Resource::Resource()
{
	m_resourceManager = NULL;
	m_bReady = false;
	m_bAsyncReady = false;
}

void Resource::load()
{
	init();
}

void Resource::loadAsync()
{
	initAsync();
}

void Resource::reload()
{
	release();
	loadAsync(); // HACKHACK: this should also be reloaded asynchronously if it was initially loaded so, maybe
	load();
}

void Resource::release()
{
	destroy();
	m_bReady = false;
	m_bAsyncReady = false;
}


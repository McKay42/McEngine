//================ Copyright (c) 2012, PG, All rights reserved. =================//
//
// Purpose:		base class for resources
//
// $NoKeywords: $res
//===============================================================================//

#include "Resource.h"
#include "Engine.h"
#include "Environment.h"

Resource::Resource(UString filepath)
{
	m_sFilePath = filepath;

	if (filepath.length() > 0 && !env->fileExists(filepath))
	{
		UString errorMessage = "File does not exist: ";
		errorMessage.append(filepath);
		debugLog("Resource Warning: File %s does not exist!\n", filepath.toUtf8());
		///engine->showMessageError("Resource Error", errorMessage);

#if defined __linux__

		// if we are on linux, try loading a toLower() version of the filename (+ extension) of the string
		// HACKHACK: better than not doing/trying anything though
		char *utf8String = (char*)filepath.toUtf8();
		for (int i=filepath.length()-1; i>=0; i--)
		{
			if (utf8String[i] == '/')
			{
				for (int c=i; c<filepath.length(); c++)
				{
					utf8String[c] = std::tolower(utf8String[c]);
				}
				break;
			}
		}
		if (utf8String != NULL)
			m_sFilePath = UString(utf8String);
#endif

	}

	m_bReady = false;
	m_bAsyncReady = false;
	m_bInterrupted = false;
}

Resource::Resource()
{
	m_bReady = false;
	m_bAsyncReady = false;
	m_bInterrupted = false;
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

void Resource::interruptLoad()
{
	m_bInterrupted = true;
}

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

		if (env->getOS() == Environment::OS::OS_LINUX) // TODO: probably better to have something like isFileSystemCaseSensitive()
		{
			// NOTE: this assumes that filepaths in code are always fully lowercase
			// better than not doing/trying anything though

			// try loading a toUpper() version of the file extension

			// search backwards from end to first dot, then toUpper() forwards till end of string
			char *utf8String = (char*)filepath.toUtf8();
			for (int s=filepath.length(); s>=0; s--)
			{
				if (utf8String[s] == '.')
				{
					for (int i=s+1; i<filepath.length(); i++)
					{
						utf8String[i] = std::toupper(utf8String[i]);
					}

					break;
				}
			}
			m_sFilePath = UString(utf8String);

			if (!env->fileExists(m_sFilePath))
			{
				// if still not found, try with toLower() filename (keeping uppercase extension)

				// search backwards from end to first dot, then toLower() everything until first slash
				bool foundFilenameStart = false;
				for (int s=filepath.length(); s>=0; s--)
				{
					if (foundFilenameStart)
					{
						if (utf8String[s] == '/')
							break;

						utf8String[s] = std::tolower(utf8String[s]);
					}

					if (utf8String[s] == '.')
						foundFilenameStart = true;
				}

				m_sFilePath = UString(utf8String);

				if (!env->fileExists(m_sFilePath))
				{
					// last chance, try with toLower() filename + extension

					// toLower() backwards until first slash
					for (int s=filepath.length(); s>=0; s--)
					{
						if (utf8String[s] == '/')
							break;

						utf8String[s] = std::tolower(utf8String[s]);
					}

					m_sFilePath = UString(utf8String);
				}
			}
		}
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
	loadAsync(); // TODO: this should also be reloaded asynchronously if it was initially loaded so, maybe
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

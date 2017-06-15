//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		file wrapper, for cross-platform unicode path support
//
// $NoKeywords: $file
//===============================================================================//

#include "File.h"
#include "ConVar.h"
#include "Engine.h"

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

#include "WinFile.h"

#elif defined __linux__

#endif

ConVar debug_file("debug_file", false);

ConVar *File::debug = &debug_file;

File::File(UString filePath)
{
	m_file = NULL;

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

	m_file = new WinFile(filePath);

#elif defined __linux__

	m_file = new StdFile(filePath);

#else

	m_file = new StdFile(filePath);

#endif
}

File::~File()
{
	SAFE_DELETE(m_file);
}

bool File::canRead() const
{
	return m_file->canRead();
}

UString File::readLine()
{
	return m_file->readLine();
}

const char *File::readFile()
{
	return m_file->readFile();
}

size_t File::getFileSize() const
{
	return m_file->getFileSize();
}



// std implementation of File
StdFile::StdFile(UString filePath)
{
	m_sFilePath = filePath;
	m_bReady = false;
	m_bRead = true;
	m_iFileSize = 0;

	m_ifstream.open(filePath.toUtf8(), std::ios::in | std::ios::binary);

	// check if we could open it at all
	if (!m_ifstream.good())
	{
		debugLog("File Error: Couldn't open() file %s\n", filePath.toUtf8());
		return;
	}

	// get and check filesize
	m_ifstream.seekg(0, std::ios::end);
	m_iFileSize = m_ifstream.tellg();
	m_ifstream.seekg(0, std::ios::beg);

	if (m_iFileSize < 1)
	{
		debugLog("File Error: FileSize is < 0\n");
		return;
	}
	else if (m_iFileSize > 1024*1024*200) // 200 MB sanity check
	{
		debugLog("File Error: FileSize is > 200 MB!!!\n");
		return;
	}

	// check if directory
	// on some operating systems, std::ifstream may also be a directory (!)
	// we need to call getline() before any error/fail bits are set, which we can then check with good()
	std::string tempLine;
	std::getline(m_ifstream, tempLine);
	if (!m_ifstream.good() && m_iFileSize < 1)
	{
		debugLog("File Error: File %s is a directory.\n", filePath.toUtf8());
		return;
	}
	m_ifstream.seekg(0, std::ios::beg);

	if (File::debug->getBool())
		debugLog("StdFile: Opening %s\n", filePath.toUtf8());

	m_bReady = true;
}

StdFile::~StdFile()
{
	m_ifstream.close(); // unnecessary
}

bool StdFile::canRead() const
{
	return m_bReady && m_ifstream.good() && m_bRead;
}

UString StdFile::readLine()
{
	if (!m_bReady)
		return "";

	m_bRead = (bool)std::getline(m_ifstream, m_sBuffer);
	return UString(m_sBuffer.c_str());
}

const char *StdFile::readFile()
{
	if (File::debug->getBool())
		debugLog("StdFile::readFile() on %s\n", m_sFilePath.toUtf8());

	if (!m_bReady || !canRead())
		return NULL;

	m_sBuffer.reserve(m_iFileSize);
	m_sBuffer.assign((std::istreambuf_iterator<char>(m_ifstream)), std::istreambuf_iterator<char>());

	return m_sBuffer.c_str();
}

size_t StdFile::getFileSize() const
{
	return m_iFileSize;
}

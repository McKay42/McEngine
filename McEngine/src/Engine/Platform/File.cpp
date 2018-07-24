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

#endif

ConVar debug_file("debug_file", false);

ConVar *File::debug = &debug_file;

File::File(UString filePath, TYPE type)
{
	m_file = NULL;

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

	m_file = new WinFile(filePath, type);

#else

	m_file = new StdFile(filePath, type);

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

bool File::canWrite() const
{
	return m_file->canWrite();
}

void File::write(const char *buffer, size_t size)
{
	m_file->write(buffer, size);
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
StdFile::StdFile(UString filePath, File::TYPE type)
{
	m_sFilePath = filePath;
	m_bRead = (type == File::TYPE::READ);

	m_bReady = false;
	m_iFileSize = 0;

	if (m_bRead)
	{
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
		m_ifstream.clear(); // clear potential error state due to the check above
		m_ifstream.seekg(0, std::ios::beg);
	}
	else // WRITE
	{
		m_ofstream.open(filePath.toUtf8(), std::ios::out | std::ios::trunc | std::ios::binary);

		// check if we could open it at all
		if (!m_ofstream.good())
		{
			debugLog("File Error: Couldn't open() file %s\n", filePath.toUtf8());
			return;
		}
	}

	if (File::debug->getBool())
		debugLog("StdFile: Opening %s\n", filePath.toUtf8());

	m_bReady = true;
}

StdFile::~StdFile()
{
	m_ofstream.close(); // unnecessary
	m_ifstream.close(); // unnecessary
}

bool StdFile::canRead() const
{
	return m_bReady && m_ifstream.good() && m_bRead;
}

bool StdFile::canWrite() const
{
	return m_bReady && m_ofstream.good() && !m_bRead;
}

void StdFile::write(const char *buffer, size_t size)
{
	if (!canWrite()) return;

	m_ofstream.write(buffer, size);
}

UString StdFile::readLine()
{
	if (!canRead()) return "";

	m_bRead = (bool)std::getline(m_ifstream, m_sBuffer);
	return UString(m_sBuffer.c_str());
}

const char *StdFile::readFile()
{
	if (File::debug->getBool())
		debugLog("StdFile::readFile() on %s\n", m_sFilePath.toUtf8());

	if (!m_bReady || !canRead()) return NULL;

	m_sBuffer.reserve(m_iFileSize);
	m_sBuffer.assign((std::istreambuf_iterator<char>(m_ifstream)), std::istreambuf_iterator<char>());

	return m_sBuffer.c_str();
}

size_t StdFile::getFileSize() const
{
	return m_iFileSize;
}

//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		fuck windows, doesn't even support utf8 filepaths
//
// $NoKeywords: $winfile
//===============================================================================//

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

#include "WinFile.h"
#include "Engine.h"
#include "ConVar.h"

//#define WINFILE_USE_MEMORY_MAPPING

WinFile::WinFile(UString filePath)
{
	m_sFilePath = filePath;
	m_bReady = false;

	// pointers
	m_buffer = NULL;
	m_handle = NULL;
	m_fullBuffer = NULL;
	m_handleMapping = NULL;
	m_memoryMappedView = NULL;

	// line reader
	m_iRead = 0;
	m_bNeedsRead = true;
	m_iLineIndex = 0;
	m_iLineBufferReadIndexOffset = 0;
	m_bCanRead = true;
	m_bEOF = false;

	// open file handle
	m_handle = CreateFileW(filePath.wc_str(), // file to open
			GENERIC_READ,					// open for reading
			FILE_SHARE_READ,				// share for reading
			NULL,							// default security
			OPEN_EXISTING,					// existing file only
			FILE_FLAG_SEQUENTIAL_SCAN,		// normal file
			NULL);							// no attr. template

	if (m_handle == INVALID_HANDLE_VALUE)
	{
		m_handle = NULL;
		debugLog("File Error: Couldn't CreateFileW(), GetLastError() = %i\n", GetLastError());
		return;
	}

	// check if file contains anything at all
	m_iFileSize = GetFileSize(m_handle, 0);
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

#ifdef WINFILE_USE_MEMORY_MAPPING

	m_handleMapping = CreateFileMappingW(
										m_handle,
										NULL,
										PAGE_READONLY,
										0,
										0,
										NULL);
	if (m_handleMapping == NULL)
	{
		debugLog("File Error: Couldn't CreateFileMappingW(), GetLastError() = %i\n", GetLastError());
		return;
	}

	m_memoryMappedView = reinterpret_cast<char*>(MapViewOfFile(m_handleMapping, FILE_MAP_READ, 0, 0, 0));

	if (!m_memoryMappedView)
	{
		debugLog("File Error: Couldn't MapViewOfFile(), GetLastError() = %i\n", GetLastError());
		return;
	}

#endif

	if (File::debug->getBool())
		debugLog("WinFile: Opening %s, m_iFileSize = %i\n", filePath.toUtf8(), m_iFileSize);

	m_bReady = true;
}

WinFile::~WinFile()
{
	if (m_memoryMappedView)
	{
		UnmapViewOfFile(m_memoryMappedView);
		m_buffer = NULL; // unlink!
	}
	if (m_handleMapping != NULL)
		CloseHandle(m_handleMapping);
	if (m_handle != NULL)
		CloseHandle(m_handle);
	if (m_fullBuffer != NULL)
	{
		delete[] m_fullBuffer;
		m_fullBuffer = NULL;
	}
	if (m_buffer != NULL)
	{
		delete[] m_buffer;
		m_buffer = NULL;
	}
}

bool WinFile::canRead() const
{
	return m_bReady && m_bCanRead;
}

bool WinFile::checkReadForLineBuffer()
{
	if (!m_bReady || !m_bNeedsRead || !m_bCanRead)
		return false;

	m_bNeedsRead = false;
	m_iLineIndex = 0;

#ifdef WINFILE_USE_MEMORY_MAPPING

	m_iRead = m_iFileSize;
	m_buffer = m_memoryMappedView; // link!
	if (m_iLineBufferReadIndexOffset > m_iFileSize)
	{
		// delayed m_bCanRead to simulate ReadFile() behaviour
		if (m_bEOF)
			m_bCanRead = false;
		m_bEOF = true;
		return false;
	}

#else

	// create/reset readline buffer
	if (m_buffer == NULL)
		m_buffer = new char[bufferSize];
	memset(m_buffer, '\0', bufferSize);

	// try to read and fill the buffer; (bufferSize-1) to always reserve 1 zero bit at the end
	if (!ReadFile(m_handle, m_buffer, bufferSize-1, &m_iRead, NULL) || m_iRead == 0)
	{
		// ERROR_HANDLE_EOF
		m_bCanRead = false;
		m_bEOF = true;
		return false;
	}

#endif

	return true;
}

bool WinFile::checkReadForFullBuffer()
{
	if (!m_bReady || !m_bNeedsRead)
		return false;

	m_bNeedsRead = false;

#ifdef WINFILE_USE_MEMORY_MAPPING

#else

	// create full buffer
	m_fullBuffer = new char[m_iFileSize];

	if (m_fullBuffer == NULL)
	{
		debugLog("File Error: Out of memory!!!\n");
		return false;
	}

	// try to read and fill the entire buffer
	if (!ReadFile(m_handle, m_fullBuffer, m_iFileSize, &m_iRead, NULL) || m_iRead == 0)
	{
		// ERROR_HANDLE_EOF
		m_bCanRead = false;
		return false;
	}

#endif

	m_bEOF = true;

	return true;
}

UString WinFile::readLine()
{
	if (!m_bReady)
		return "";

	bool foundLine = false;
	UString result;

	char lineBuffer[bufferSize+2]; // +2 for sanity
	while (!foundLine && m_bCanRead)
	{
		// check if we need to read some more
		checkReadForLineBuffer();

		// line detection loop
		unsigned int lineBufferIndex = 0;
		memset(lineBuffer, '\0', bufferSize); // two zero chars at the end should actually be enough, but whatever
		for (int i=m_iLineIndex; i<bufferSize && (i+m_iLineBufferReadIndexOffset)<m_iRead && lineBufferIndex<bufferSize; i++)
		{
			const BYTE c = ((BYTE*)m_buffer)[i+m_iLineBufferReadIndexOffset];

			// detect newline, then break and set next index for future readLine()s
			if (c == '\r')
			{
				// skip possibly following '\n' as well
				if (i+m_iLineBufferReadIndexOffset+1 < m_iRead && ((BYTE*)m_buffer)[i+m_iLineBufferReadIndexOffset+1] == '\n')
					i++;

				// set new line index for next iteration, +1 to skip the newline char
				foundLine = true;
				m_iLineIndex = i+1;
				break;
			}
			else if (c == '\n')
			{
				// set new line index for next iteration, +1 to skip the newline char
				foundLine = true;
				m_iLineIndex = i+1;
				break;
			}
			else // if not on newline, add to linebuffer
				lineBuffer[lineBufferIndex++] = m_buffer[i+m_iLineBufferReadIndexOffset];
		}

		// using memchr instead
		// doesn't seem to be faster, is actually a bit slower sometimes
		/*
		if (m_iLineIndex + m_iLineBufferReadIndexOffset < m_iRead && m_iLineIndex < bufferSize)
		{
			char *newlinePointer = (char*)memchr(m_buffer + m_iLineIndex + m_iLineBufferReadIndexOffset, '\n', bufferSize - m_iLineIndex);
			if (newlinePointer != NULL)
			{
				// found line
				foundLine = true;
				lineBufferIndex = (unsigned int)(newlinePointer - (m_buffer + m_iLineBufferReadIndexOffset));

				// copy everything from the last position to the position of the just found newline
				memcpy(lineBuffer, m_buffer + m_iLineIndex + m_iLineBufferReadIndexOffset, newlinePointer - (m_buffer + m_iLineIndex + m_iLineBufferReadIndexOffset));

				// update the index for the next iteration (+1 to skip the newline char)
				m_iLineIndex = lineBufferIndex+1;
			}
			else
			{
				// if no newline was found, read up to bufferSize to finish this block
				lineBufferIndex = bufferSize - m_iLineIndex;
				if (lineBufferIndex > 0)
					memcpy(lineBuffer, m_buffer + m_iLineIndex + m_iLineBufferReadIndexOffset, lineBufferIndex);
				m_iLineIndex += 1;
			}
		}
		*/

		// append everything read so far
		if (lineBufferIndex > 0 || foundLine)
		{
			// only manually correct lineIndex if we didn't find a line
			if (!foundLine)
				m_iLineIndex += lineBufferIndex; // increase read position for possible next line

			// only append if we have anything at all
			if (lineBufferIndex > 0)
				result.append(lineBuffer);
		}
		else // if no line could be read, check if we need to read more in the next iteration
		{
			m_bNeedsRead = true; // this triggers a file read in checkReadForLineBuffer()

#ifdef WINFILE_USE_MEMORY_MAPPING

			m_iLineBufferReadIndexOffset += bufferSize;

#endif
		}
	}

	return result;
}

const char *WinFile::readFile()
{
	if (File::debug->getBool())
		debugLog("WinFile::readFile() on %s\n", m_sFilePath.toUtf8());

#ifdef WINFILE_USE_MEMORY_MAPPING

	return m_memoryMappedView;

#else

	if (checkReadForFullBuffer())
		return m_fullBuffer;
	else
		return NULL;

#endif

}

size_t WinFile::getFileSize() const
{
	return m_iFileSize;
}

#endif

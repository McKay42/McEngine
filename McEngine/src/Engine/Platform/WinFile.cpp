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

WinFile::WinFile(UString filePath, File::TYPE type)
{
	m_sFilePath = filePath;
	m_bReady = false;

	m_bCanWrite = false;
	m_bCanRead = false;
	m_iFileSize = 0;

	m_buffer = NULL;
	m_handle = NULL;
	m_fullBuffer = NULL;

	if (type == File::TYPE::READ)
	{
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
	}
	else // WRITE
	{
		m_bCanWrite = true;

		// open file handle
		m_handle = CreateFileW(filePath.wc_str(), // file to open
				GENERIC_WRITE,					// open for writing
				0,								// exclusive access (not shared)
				NULL,							// default security
				CREATE_ALWAYS,					// overwrite everything
				FILE_ATTRIBUTE_NORMAL,			// normal file
				NULL);							// no attr. template

		if (m_handle == INVALID_HANDLE_VALUE)
		{
			m_handle = NULL;
			debugLog("File Error: Couldn't CreateFileW(), GetLastError() = %i\n", GetLastError());
			return;
		}
	}

	if (File::debug->getBool())
		debugLog("WinFile: Opening %s, m_iFileSize = %i\n", filePath.toUtf8(), m_iFileSize);

	m_bReady = true;
}

WinFile::~WinFile()
{
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

bool WinFile::canWrite() const
{
	return m_bReady && m_bCanWrite;
}

void WinFile::write(const char *buffer, size_t size)
{
	if (!canWrite()) return;

	DWORD bytesWritten;
	if (WriteFile(m_handle, buffer, size, &bytesWritten, NULL) == TRUE)
	{
		if (bytesWritten != size)
			debugLog("WinFile Warning: WriteFile() only wrote %lu/%lu bytes\n", bytesWritten, size);

		m_iFileSize += bytesWritten;

		LARGE_INTEGER absolutePosition;
		absolutePosition.QuadPart = m_iFileSize;

		SetFilePointerEx(m_handle, absolutePosition, NULL, FILE_BEGIN);
	}
	else
	{
		m_bCanWrite = false;
		debugLog("WinFile Error: Couldn't WriteFile(), GetLastError() = %i\n", GetLastError());
	}
}

bool WinFile::checkReadForLineBuffer()
{
	if (!m_bReady || !m_bNeedsRead || !m_bCanRead) return false;

	m_bNeedsRead = false;
	m_iLineIndex = 0;

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

	return true;
}

bool WinFile::checkReadForFullBuffer()
{
	if (!m_bReady || !m_bNeedsRead) return false;

	m_bNeedsRead = false;

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

	m_bEOF = true;

	return true;
}

UString WinFile::readLine()
{
	if (!canRead()) return "";

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
		for (int i=m_iLineIndex; (i < bufferSize) && ((i+m_iLineBufferReadIndexOffset) < m_iRead) && (lineBufferIndex < bufferSize); i++)
		{
			const unsigned char c = ((unsigned char*)m_buffer)[i+m_iLineBufferReadIndexOffset];

			// detect newline, then break and set next index for future readLine()s
			if (c == '\r')
			{
				// skip possibly following '\n' as well
				if (i+m_iLineBufferReadIndexOffset+1 < m_iRead && ((unsigned char*)m_buffer)[i+m_iLineBufferReadIndexOffset+1] == '\n')
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
			m_bNeedsRead = true; // this triggers a file read in checkReadForLineBuffer()
	}

	return result;
}

const char *WinFile::readFile()
{
	if (File::debug->getBool())
		debugLog("WinFile::readFile() on %s\n", m_sFilePath.toUtf8());

	if (checkReadForFullBuffer())
		return m_fullBuffer;
	else
		return NULL;
}

size_t WinFile::getFileSize() const
{
	return m_iFileSize;
}

#endif

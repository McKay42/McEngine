//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		fuck windows, doesn't even support utf8 filepaths
//
// $NoKeywords: $winfile
//===============================================================================//

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

#ifndef WINFILE_H
#define WINFILE_H

#include "File.h"

#include <Windows.h>

class WinFile : public BaseFile
{
public:
	WinFile(UString filePath, File::TYPE type);
	virtual ~WinFile();

	bool canRead() const;
	bool canWrite() const;

	void write(const char *buffer, size_t size);

	UString readLine();
	const char *readFile();

	size_t getFileSize() const;

private:
	static const DWORD bufferSize = 4096;

	bool checkReadForLineBuffer();
	bool checkReadForFullBuffer();

	UString m_sFilePath;

	bool m_bReady;
	bool m_bCanRead;
	bool m_bCanWrite;
	bool m_bEOF;

	HANDLE m_handle;
	size_t m_iFileSize;

	// line reader
	char *m_buffer;
	DWORD m_iRead;
	bool m_bNeedsRead;
	DWORD m_iLineIndex;

	// full reader
	char *m_fullBuffer;
	DWORD m_iLineBufferReadIndexOffset;
};

#endif

#endif

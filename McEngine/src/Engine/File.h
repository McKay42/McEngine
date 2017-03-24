//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		file wrapper, for cross-platform unicode path support
//
// $NoKeywords: $file $os
//===============================================================================//

#ifndef FILE_H
#define FILE_H

#include "cbase.h"

// TODO: reading/writing bool in constructor

class BaseFile;
class ConVar;

class File
{
public:
	static ConVar *debug;

public:
	File(UString filePath);
	virtual ~File();

	bool canRead() const;

	UString readLine();
	const char *readFile();

	size_t getFileSize() const;

private:
	BaseFile *m_file;
};

class BaseFile
{
public:
	virtual ~BaseFile() {;}

	virtual bool canRead() const = 0;

	virtual UString readLine() = 0;
	virtual const char *readFile() = 0;

	virtual size_t getFileSize() const = 0;
};



// std implementation of File
class StdFile : public BaseFile
{
public:
	StdFile(UString filePath);
	virtual ~StdFile();

	bool canRead() const;

	UString readLine();
	const char *readFile();

	size_t getFileSize() const;

private:
	bool m_bReady;
	bool m_bRead;

	std::ifstream m_ifstream;
	std::string m_sBuffer;
	size_t m_iFileSize;

	UString m_sFilePath;
};

#endif

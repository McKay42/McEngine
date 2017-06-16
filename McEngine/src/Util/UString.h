//============== Copyright (c) 2009, 2D Boy & PG, All rights reserved. ===============//
//
// Purpose:		unicode string class (modified)
//
// $NoKeywords: $ustring $string
//====================================================================================//

// TODO: this needs a lot more standard functionality

#ifndef USTRING_H
#define USTRING_H

#include "cbase.h"

class UString
{
public:
	static UString format(const char *utf8format, ...);

public:
	UString();
	UString(const wchar_t *str);
	UString(const char *utf8);
	UString(const UString &ustr);
	~UString();

	void clear();

	// get
	inline int length() const {return mLength;}
	inline const char *toUtf8() const {return mUtf8;}
	inline const wchar_t *wc_str() const {return mUnicode;} // WARNING: may return NULL for empty strings
	inline bool isAsciiOnly() const {return mIsAsciiOnly;}
	bool isWhitespaceOnly();

	int findChar(wchar_t ch, int start = 0, bool respectEscapeChars = false) const;
	int findChar(const UString &str, int start = 0, bool respectEscapeChars = false) const;
	int find(const UString &str, int start = 0) const;
	int find(const UString &str, int start, int end) const;
	int findLast(const UString &str, int start = 0) const;
	int findLast(const UString &str, int start, int end) const;

	// modifiers
	void collapseEscapes();
	void append(const UString &str);
	void insert(int offset, const UString &str);
	void insert(int offset, wchar_t ch);
	void erase(int offset, int count);

	// actions (non-modifying)
	UString substr(int offset, int charCount = -1) const;
	std::vector<UString> split(UString delim);
	UString trim();

	// conversions
	float toFloat() const;
	int toInt() const;
	long toLong() const;

	// operators
	wchar_t operator [] (int index) const;
	UString &operator = (const UString &ustr);
	bool operator == (const UString &ustr) const;
	bool operator != (const UString &ustr) const;
	bool operator < (const UString &ustr) const;

private:
	int fromUtf8(const char *utf8);

	int decode(const char *utf8, wchar_t *unicode);
	int encode(const wchar_t *unicode, int length, char *utf8, bool *isAsciiOnly) const;

	wchar_t getCodePoint(const char *utf8, int offset, int numBytes, unsigned char firstByteMask);

	void getUtf8(wchar_t ch, char *utf8, int numBytes, int firstByteValue) const;

	void updateUtf8();

private:
	int mLength;
	bool mIsAsciiOnly;

	wchar_t *mUnicode;
	char *mUtf8;
};

#endif

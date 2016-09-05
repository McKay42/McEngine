//============== Copyright (c) 2009, 2D Boy & PG, All rights reserved. ===============//
//
// Purpose:		unicode string class (modified)
//
// $NoKeywords: $ustring $string
//====================================================================================//

#include "UString.h"

#include <stdarg.h>
#include <wchar.h>
#include <wctype.h>
#include <string.h>
#include <cstdarg>

#define MASK_1BYTE  0x80 /* 1000 0000 */
#define VALUE_1BYTE 0x00 /* 0000 0000 */
#define MASK_2BYTE  0xE0 /* 1110 0000 */
#define VALUE_2BYTE 0xC0 /* 1100 0000 */
#define MASK_3BYTE  0xF0 /* 1111 0000 */
#define VALUE_3BYTE 0xE0 /* 1110 0000 */
#define MASK_4BYTE  0xF8 /* 1111 1000 */
#define VALUE_4BYTE 0xF0 /* 1111 0000 */
#define MASK_5BYTE  0xFC /* 1111 1100 */
#define VALUE_5BYTE 0xF8 /* 1111 1000 */
#define MASK_6BYTE  0xFE /* 1111 1110 */
#define VALUE_6BYTE 0xFC /* 1111 1100 */

#define MASK_MULTIBYTE 0x3F /* 0011 1111 */

#define ESCAPE_CHAR '\\'

UString::UString()
{
	mUnicode = NULL;
	mLength = 0;
	mUtf8 = NULL;
	mIsAsciiOnly = false;
}

UString::UString(const char *utf8)
{
	mUnicode = NULL;
	mLength = 0;
	mUtf8 = NULL;
	fromUtf8(utf8);
}

UString::UString(const UString &ustr)
{
	mUnicode = NULL;
	mLength = 0;
	mUtf8 = NULL;
	(*this) = ustr;
}

UString::UString(const wchar_t *str)
{
	// get length
	mLength = (int)wcslen(str);

	// allocate new mem for unicode data
	mUnicode = new wchar_t[mLength+1];

	// copy contents and null terminate
	memcpy(mUnicode, str, (mLength+1)*sizeof(wchar_t));

	// null out the utf version
	mUtf8 = NULL;

	mIsAsciiOnly = false;

	updateUtf8();
}

UString::~UString()
{
	if (mUnicode != NULL)
		delete[] mUnicode;
	if (mUtf8 != NULL)
		delete[] mUtf8;

	mLength = 0;
}

UString UString::format(const char *utf8format, ...)
{
	// decode the utf8 string
	UString formatted;
	int bufSize = formatted.fromUtf8(utf8format) + 2;

	// print the args to the format
	wchar_t *buf = NULL;
	int written = -1;
	while (true)
	{
		buf = new wchar_t[bufSize];

		va_list ap;
		va_start(ap, utf8format);
		written = vswprintf(buf, bufSize, formatted.mUnicode, ap);
		va_end(ap);

		// if we didn't use the entire buffer
		if (written > 0 && written < bufSize)
		{
			// cool, keep the formatted string and move on
			delete[] formatted.mUnicode;
			formatted.mUnicode = buf;
			formatted.mLength = written;
			formatted.updateUtf8();
			break;
		}
		else
		{
			// we need a larger buffer
			delete[] buf;
			bufSize *= 2;
		}
	}

	return formatted;
}

int UString::length() const
{
	return mLength;
}

const char *UString::toUtf8() const
{
	return mUtf8;
}

const wchar_t *UString::wc_str() const
{
	return mUnicode;
}

int UString::fromUtf8(const char *utf8)
{
	int startIndex = 0;
	if (utf8[0] == (char)0xef && utf8[1] == (char)0xbb && utf8[2] == (char)0xbf) // utf-8
		startIndex = 3;
	else
	{
		char c0 = utf8[0];
		char c1 = utf8[1];
		char c2 = utf8[2];
		char c3 = utf8[3];
		bool utf16le = (c0 == (char)0xff && c1 == (char)0xfe && c2 != (char)0x00);
		bool utf16be = (c0 == (char)0xfe && c1 == (char)0xff && c2 != (char)0x00);
		bool utf32le = (c0 == (char)0xff && c1 == (char)0xfe && c2 == (char)0x00 && c3 == (char)0x00);
		bool utf32be = (c0 == (char)0x00 && c1 == (char)0x00 && c2 == (char)0xfe && c3 == (char)0xff);

		if (utf16le || utf16be || utf32le || utf32be)
		{
			// TODO: UTF-16/32 not yet supported
			return 0;
		}
	}

	mLength = decode(&(utf8[startIndex]), NULL);
	mUnicode = new wchar_t[mLength+1];
	mUtf8 = NULL;
	int length = decode(&(utf8[startIndex]), mUnicode);

	// reencode to utf8
	updateUtf8();

	return length;
}

UString UString::substr(int offset, int charCount) const
{
	UString str;

	if (charCount < 0)
		charCount = mLength - offset;

	// allocate new mem:
	str.mLength = charCount;
	str.mUnicode = new wchar_t[charCount+1];

	// copy mem contents and null terminate:
	memcpy(str.mUnicode, &(mUnicode[offset]), charCount*sizeof(wchar_t));
	str.mUnicode[charCount] = 0;

	// update the substring's utf encoding:
	str.updateUtf8();

	return str;
}

int UString::findChar(wchar_t ch, int start, bool respectEscapeChars) const
{
	bool escaped = false;
	for (int i=start; i<mLength; i++)
	{
		// if we're respecting escape chars AND we are not in an escape
		// sequence AND we've found an escape character
		if (respectEscapeChars && !escaped && mUnicode[i] == ESCAPE_CHAR)
		{
			// now we're in an escape sequence
			escaped = true;
		}
		else
		{
			if (!escaped && mUnicode[i] == ch)
				return i;

			escaped = false;
		}
	}

	return -1;
}

int UString::findChar(const UString &str, int start, bool respectEscapeChars) const
{
	bool escaped = false;
	for (int i=start; i<mLength; i++)
	{
		// if we're respecting escape chars AND we are not in an escape
		// sequence AND we've found an escape character
		if (respectEscapeChars && !escaped && mUnicode[i] == ESCAPE_CHAR)
		{
			// now we're in an escape sequence
			escaped = true;
		}
		else
		{
			if (!escaped && str.findChar(mUnicode[i]) >= 0)
				return i;

			escaped = false;
		}
	}

	return -1;
}

int UString::find(const UString &str, int start) const
{
	int lastPossibleMatch = mLength - str.mLength;
	for (int i=start; i<=lastPossibleMatch; i++)
	{
		if (memcmp(&(mUnicode[i]), str.mUnicode, str.mLength * sizeof(*mUnicode)) == 0)
		{
			return i;
		}
	}

	return -1;
}

int UString::find(const UString &str, int start, int end) const
{
	int lastPossibleMatch = mLength - str.mLength;
	for (int i=start; i<=lastPossibleMatch && i<end; i++)
	{
		if (memcmp(&(mUnicode[i]), str.mUnicode, str.mLength * sizeof(*mUnicode)) == 0)
		{
			return i;
		}
	}

	return -1;
}

int UString::findLast(const UString &str, int start) const
{
	int lastI = -1;
	for (int i=start; i<mLength; i++)
	{
		if (memcmp(&(mUnicode[i]), str.mUnicode, str.mLength * sizeof(*mUnicode)) == 0)
		{
			lastI = i;
		}
	}

	return lastI;
}

int UString::findLast(const UString &str, int start, int end) const
{
	int lastI = -1;
	for (int i=start; i<mLength && i<end; i++)
	{
		if (memcmp(&(mUnicode[i]), str.mUnicode, str.mLength * sizeof(*mUnicode)) == 0)
		{
			lastI = i;
		}
	}

	return lastI;
}

std::vector<UString> UString::split(UString delim)
{
	std::vector<UString> results;
	int start = 0;
	int end = 0;

	while ((end = find(delim, start)) != -1)
	{
		results.push_back(substr(start, end-start));
		start = end + delim.length();
	}
	results.push_back(substr(start, end-start));

	return results;
}

void UString::append(const UString &str)
{
	if (str.mLength == 0)
		return;

	// calculate new size
	int newSize = mLength + str.mLength;

	// allocate new data buffer
	wchar_t *newUnicode = new wchar_t[newSize+1];

	// copy existing data
	memcpy(newUnicode, mUnicode, mLength*sizeof(wchar_t));

	// copy appended data
	memcpy(&(newUnicode[mLength]), str.mUnicode, (str.mLength+1)*sizeof(wchar_t)); // +1 for null termination

	// replace the old values with the new
	delete[] mUnicode;
	mUnicode = newUnicode;
	mLength = newSize;

	// reencode to utf8
	updateUtf8();
}

void UString::insert(int offset, const UString &str)
{
	// calculate new size
	int newSize = mLength + str.mLength;

	// allocate new data buffer
	wchar_t *newUnicode = new wchar_t[newSize+1];

	// if we're not inserting at the beginning of the string
	if (offset > 0)
	{
		// copy first part of data
		memcpy(newUnicode, mUnicode, offset*sizeof(wchar_t));
	}

	// copy inserted string
	memcpy(&(newUnicode[offset]), str.mUnicode, str.mLength*sizeof(wchar_t));

	// if we're not inserting at the end of the string
	if (offset < mLength)
	{
		// copy rest of string
		memcpy(&(newUnicode[offset+str.mLength]), &(mUnicode[offset]), (mLength-offset+1)*sizeof(wchar_t)); // +1 to include null termination
	}
	else
	{
		// just null terminate it
		newUnicode[newSize] = 0;
	}

	// replace the old values with the new
	delete[] mUnicode;
	mUnicode = newUnicode;
	mLength = newSize;

	// reencode to utf8
	updateUtf8();
}

void UString::insert(int offset, wchar_t ch)
{
	// calculate new size
	int newSize = mLength + 1;

	// allocate new data buffer
	wchar_t *newUnicode = new wchar_t[newSize];

	// copy first part of data
	memcpy(newUnicode, mUnicode, offset*sizeof(wchar_t));

	// place the inserted char
	newUnicode[offset] = ch;

	// copy rest of string
	memcpy(&(newUnicode[offset+1]), &(mUnicode[offset]), (mLength-offset+1)*sizeof(wchar_t)); // +1 to null terminate

	// replace the old values with the new
	delete[] mUnicode;
	mUnicode = newUnicode;
	mLength = newSize;

	// reencode to utf8
	updateUtf8();
}

void UString::erase(int offset, int count)
{
	// calculate new size
	int newLength = mLength - count;

	// allocate new data buffer
	wchar_t *newUnicode = new wchar_t[newLength+1];

	// copy first part of data
	memcpy(newUnicode, mUnicode, offset*sizeof(wchar_t));

	// copy rest of string: (including terminating zero character)
	memcpy(&(newUnicode[offset]), &(mUnicode[offset+count]), (newLength-offset+1)*sizeof(wchar_t));

	// replace the old values with the new?
	delete[] mUnicode;
	mUnicode = newUnicode;
	mLength = newLength;

	// reencode to utf8
	updateUtf8();
}

void UString::clear()
{
	if (mUnicode != NULL)
	{
		delete[] mUnicode;
		mUnicode = NULL;
	}

	if (mUtf8 != NULL)
	{
		delete[] mUtf8;
		mUtf8 = NULL;
	}

	mLength = 0;
}

bool UString::isAsciiOnly()
{
	return mIsAsciiOnly;
}

bool UString::isWhitespaceOnly()
{
	int startPos = 0;
	while (startPos < mLength)
	{
		if (!iswspace(mUnicode[startPos]))
			return false;

		startPos++;
	}
	return true;
}

void UString::collapseEscapes()
{
	int writeIndex = 0;
	bool escaped = false;
	wchar_t *buf = new wchar_t[mLength];

	// iterate over the unicode string
	for (int readIndex=0; readIndex<mLength; readIndex++)
	{
		// if we're not already escaped and this is an escape char
		if (!escaped && mUnicode[readIndex] == ESCAPE_CHAR)
		{
			// we're escaped
			escaped = true;
		}
		else
		{
			// move this char over and increment the write index
			buf[writeIndex] = mUnicode[readIndex];
			writeIndex++;

			// we're no longer escaped
			escaped = false;
		}
	}

	// replace old data with new data
	delete[] mUnicode;
	mLength = writeIndex;
	mUnicode = new wchar_t[mLength];
	memcpy(mUnicode,buf,mLength*sizeof(wchar_t));

	// free the temporary buffer
	delete[] buf;

	// the utf encoding is out of date, update it
	updateUtf8();
}

UString UString::trim()
{
	int startPos = 0;
	while (startPos < mLength && iswspace(mUnicode[startPos]))
	{
		startPos++;
	}

	int endPos = mLength - 1;
	while ((endPos >= 0) && (endPos < mLength) && iswspace(mUnicode[endPos]))
	{
		endPos--;
	}
	return substr(startPos, endPos - startPos + 1);
}

float UString::toFloat() const
{
	return strtof(mUtf8, NULL);
}

int UString::toInt() const
{
	return (int)strtol(mUtf8, NULL, 0);
}

long UString::toLong() const
{
	return strtol(mUtf8, NULL, 0);
}

wchar_t UString::operator [] (int index) const
{
	if (mLength > 0)
		return mUnicode[clamp<int>(index, 0, mLength-1)];
	else
		return 0;
}

UString &UString::operator = (const UString &ustr)
{
	wchar_t *newUnicode = NULL;

	// if this is not a null string
	if (ustr.mLength > 0)
	{
		// allocate new mem for unicode data
		newUnicode = new wchar_t[ustr.mLength+1];

		// copy unicode mem contents
		memcpy(newUnicode,ustr.mUnicode,(ustr.mLength+1)*sizeof(wchar_t));
	}

	// deallocate old mem
	delete[] mUnicode;

	// init variables
	mLength = ustr.mLength;
	mUnicode = newUnicode;

	// reencode to utf8
	updateUtf8();

	return *this;
}

bool UString::operator == (const UString &ustr) const
{
	if (mLength != ustr.mLength)
		return false;
	if (mUnicode == NULL && ustr.mUnicode == NULL)
		return true;
	else if (mUnicode == NULL || ustr.mUnicode == NULL)
		return false;

	return memcmp(mUnicode, ustr.mUnicode, mLength*sizeof(wchar_t)) == 0;
}

bool UString::operator != (const UString &ustr) const
{
	bool equal = (*this == ustr);
	return !equal;
}

bool UString::operator < (const UString &ustr) const
{
	for (int i=0; i<mLength && i<ustr.mLength; i++)
	{
		if (mUnicode[i] != ustr.mUnicode[i])
		{
			return mUnicode[i] < ustr.mUnicode[i];
		}
	}

	if (mLength == ustr.mLength)
		return false;

	return mLength < ustr.mLength;
}

int UString::decode(const char *utf8, wchar_t *unicode)
{
	int length = 0;
	for (int i=0; utf8[i]!=0; i++)
	{
		char b = utf8[i];
		if ((b & MASK_1BYTE) == VALUE_1BYTE) // if this is a single byte code point
		{
			if (unicode!=NULL)
			{
				unicode[length] = b;
			}
		}
		else if ((b & MASK_2BYTE) == VALUE_2BYTE) // if this is a 2 byte code point
		{
			if (unicode!=NULL)
			{
				unicode[length] = getCodePoint(utf8,i,2,(unsigned char)(~MASK_2BYTE));
			}
			i += 1;
		}
		else if ((b & MASK_3BYTE) == VALUE_3BYTE) // if this is a 3 byte code point
		{
			if (unicode!=NULL)
			{
				unicode[length] = getCodePoint(utf8,i,3,(unsigned char)(~MASK_3BYTE));
			}
			i += 2;
		}
		else if ((b & MASK_4BYTE) == VALUE_4BYTE) // if this is a 4 byte code point
		{
			if (unicode!=NULL)
			{
				unicode[length] = getCodePoint(utf8,i,4,(unsigned char)(~MASK_4BYTE));
			}
			i += 3;
		}
		else if ((b & MASK_5BYTE) == VALUE_5BYTE) // if this is a 5 byte code point
		{
			if (unicode!=NULL)
			{
				unicode[length] = getCodePoint(utf8,i,5,(unsigned char)(~MASK_5BYTE));
			}
			i += 4;
		}
		else if ((b & MASK_6BYTE) == VALUE_6BYTE) // if this is a 6 byte code point
		{
			if (unicode!=NULL)
			{
				unicode[length] = getCodePoint(utf8,i,6,(unsigned char)(~MASK_6BYTE));
			}
			i += 5;
		}

		length++;
	}

	if (unicode != NULL)
	{
		// null terminate
		unicode[length] = 0;
	}

	return length;
}

int UString::encode(const wchar_t *unicode, int length, char *utf8, bool *isAsciiOnly) const
{
	int utf8len = 0;
	bool foundMultibyte = false;
	for (int i=0; i<length; i++)
	{
		wchar_t ch = unicode[i];

		///assert(ch >= 0);

		if (ch < 0x00000080) // 1 byte
		{
			if (utf8 != NULL)
			{
				utf8[utf8len] = (char)ch;
			}
			utf8len += 1;
		}
		else if (ch < 0x00000800) // 2 bytes
		{
			if (utf8 != NULL)
			{
				getUtf8(ch, &(utf8[utf8len]), 2, VALUE_2BYTE);
			}
			utf8len += 2;
			foundMultibyte = true;
		}
		else if (ch < 0x00010000) // 3 bytes
		{
			if (utf8 != NULL)
			{
				getUtf8(ch, &(utf8[utf8len]), 3, VALUE_3BYTE);
			}
			utf8len += 3;
			foundMultibyte = true;
		}
		else if (ch < 0x00200000) // 4 bytes
		{
			if (utf8 != NULL)
			{
				getUtf8(ch, &(utf8[utf8len]), 4, VALUE_4BYTE);
			}
			utf8len += 4;
			foundMultibyte = true;
		}
		else if (ch < 0x04000000) // 5 bytes
		{
			if (utf8 != NULL)
			{
				getUtf8(ch, &(utf8[utf8len]), 5, VALUE_5BYTE);
			}
			utf8len += 5;
			foundMultibyte = true;
		}
		else // 6 bytes
		{
			if (utf8 != NULL)
			{
				getUtf8(ch, &(utf8[utf8len]), 6, VALUE_6BYTE);
			}
			utf8len += 6;
			foundMultibyte = true;
		}
	}

	if (isAsciiOnly != NULL)
	{
		*isAsciiOnly = !foundMultibyte;
	}

	return utf8len;
}

wchar_t UString::getCodePoint(const char *utf8, int offset, int numBytes, unsigned char firstByteMask)
{
	// get the bits out of the first byte
	wchar_t wc = utf8[offset] & firstByteMask;

	// iterate over the rest of the bytes
	for (int i=1; i<numBytes; i++)
	{
		// shift the code point bits to make room for the new bits
		wc = wc << 6;

		// add the new bits
		wc |= utf8[offset+i] & MASK_MULTIBYTE;
	}

	// return the code point
	return wc;
}

void UString::getUtf8(wchar_t ch, char *utf8, int numBytes, int firstByteValue) const
{
	for (int i=numBytes-1; i>0; i--)
	{
		// store the lowest bits in a utf8 byte
		utf8[i] = (ch & MASK_MULTIBYTE) | 0x80;
		ch >>= 6;
	}

	// store the remaining bits
	*utf8 = (firstByteValue | ch);
}

void UString::updateUtf8()
{
	if (mUtf8 != NULL)
	{
		delete[] mUtf8;
		mUtf8 = NULL;
	}

	int size = encode(mUnicode, mLength, NULL, &mIsAsciiOnly);
	if (size > 0)
	{
		mUtf8 = new char[size+1];
		encode(mUnicode, mLength, mUtf8, NULL);
		mUtf8[size] = 0; // null terminate
	}
	else
	{
		mUtf8 = new char[1];
		mUtf8[0] = 0; // null terminate
	}
}

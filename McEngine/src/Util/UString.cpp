//============== Copyright (c) 2009, 2D Boy & PG, All rights reserved. ===============//
//
// Purpose:		unicode string class (modified)
//
// $NoKeywords: $ustring $string
//====================================================================================//

#include "UString.h"

#include <wchar.h>
#include <wctype.h>
#include <string.h>

#define USTRING_MASK_1BYTE  0x80 /* 1000 0000 */
#define USTRING_VALUE_1BYTE 0x00 /* 0000 0000 */
#define USTRING_MASK_2BYTE  0xE0 /* 1110 0000 */
#define USTRING_VALUE_2BYTE 0xC0 /* 1100 0000 */
#define USTRING_MASK_3BYTE  0xF0 /* 1111 0000 */
#define USTRING_VALUE_3BYTE 0xE0 /* 1110 0000 */
#define USTRING_MASK_4BYTE  0xF8 /* 1111 1000 */
#define USTRING_VALUE_4BYTE 0xF0 /* 1111 0000 */
#define USTRING_MASK_5BYTE  0xFC /* 1111 1100 */
#define USTRING_VALUE_5BYTE 0xF8 /* 1111 1000 */
#define USTRING_MASK_6BYTE  0xFE /* 1111 1110 */
#define USTRING_VALUE_6BYTE 0xFC /* 1111 1100 */

#define USTRING_MASK_MULTIBYTE 0x3F /* 0011 1111 */

#define USTRING_ESCAPE_CHAR '\\'

constexpr char UString::nullString[];
constexpr wchar_t UString::nullWString[];

UString::UString()
{
	mLength = 0;
	mIsAsciiOnly = false;
	mUnicode = (wchar_t*)nullWString;
	mUtf8 = (char*)nullString;
}

UString::UString(const char *utf8)
{
	mLength = 0;
	mUnicode = (wchar_t*)nullWString;
	mUtf8 = (char*)nullString;

	fromUtf8(utf8);
}

UString::UString(const UString &ustr)
{
	mLength = 0;
	mUnicode = (wchar_t*)nullWString;
	mUtf8 = (char*)nullString;

	(*this) = ustr;
}

UString::UString(UString &&ustr)
{
	// move
	mLength = ustr.mLength;
	mIsAsciiOnly = ustr.mIsAsciiOnly;
	mUnicode = ustr.mUnicode;
	mUtf8 = ustr.mUtf8;

	// reset source
	ustr.mLength = 0;
	ustr.mIsAsciiOnly = false;
	ustr.mUnicode = NULL;
	ustr.mUtf8 = NULL;
}

UString::UString(const wchar_t *str)
{
	// get length
	mLength = (str != NULL ? (int)std::wcslen(str) : 0);

	// allocate new mem for unicode data
	mUnicode = new wchar_t[mLength+1]; // +1 for null termination later

	// copy contents and null terminate
	if (mLength > 0)
		memcpy(mUnicode, str, (mLength)*sizeof(wchar_t));

	mUnicode[mLength] = '\0'; // null terminate

	// null out and rebuild the utf version
	mUtf8 = NULL;
	mIsAsciiOnly = false;

	updateUtf8();
}

UString::~UString()
{
	mLength = 0;
	mIsAsciiOnly = false;

	deleteUnicode();
	deleteUtf8();
}

void UString::clear()
{
	mLength = 0;
	mIsAsciiOnly = false;

	deleteUnicode();
	deleteUtf8();
}

UString UString::format(const char *utf8format, ...)
{
	// decode the utf8 string
	UString formatted;
	int bufSize = formatted.fromUtf8(utf8format) + 1; // +1 for default heuristic (no args, but null char). arguments will need multiple iterations and allocations anyway

	if (formatted.mLength == 0) return formatted;

	// print the args to the format
	wchar_t *buf = NULL;
	int written = -1;
	while (true)
	{
		if (bufSize >= 1024*1024)
		{
			printf("WARNING: Potential vswprintf(%s, ...) infinite loop, stopping ...\n", utf8format);
			return formatted;
		}

		buf = new wchar_t[bufSize];

		va_list ap;
		va_start(ap, utf8format);
		written = vswprintf(buf, bufSize, formatted.mUnicode, ap);
		va_end(ap);

		// if we didn't use the entire buffer
		if (written > 0 && written < bufSize)
		{
			// cool, keep the formatted string and we're done
			if (!formatted.isUnicodeNull())
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

bool UString::isWhitespaceOnly() const
{
	int startPos = 0;
	while (startPos < mLength)
	{
		if (!std::iswspace(mUnicode[startPos]))
			return false;

		startPos++;
	}

	return true;
}

int UString::findChar(wchar_t ch, int start, bool respectEscapeChars) const
{
	bool escaped = false;
	for (int i=start; i<mLength; i++)
	{
		// if we're respecting escape chars AND we are not in an escape
		// sequence AND we've found an escape character
		if (respectEscapeChars && !escaped && mUnicode[i] == USTRING_ESCAPE_CHAR)
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
		if (respectEscapeChars && !escaped && mUnicode[i] == USTRING_ESCAPE_CHAR)
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

void UString::collapseEscapes()
{
	if (mLength == 0) return;

	int writeIndex = 0;
	bool escaped = false;
	wchar_t *buf = new wchar_t[mLength];

	// iterate over the unicode string
	for (int readIndex=0; readIndex<mLength; readIndex++)
	{
		// if we're not already escaped and this is an escape char
		if (!escaped && mUnicode[readIndex] == USTRING_ESCAPE_CHAR)
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
	deleteUnicode();
	mLength = writeIndex;
	mUnicode = new wchar_t[mLength];
	memcpy(mUnicode, buf, mLength*sizeof(wchar_t));

	// free the temporary buffer
	delete[] buf;

	// the utf encoding is out of date, update it
	updateUtf8();
}

void UString::append(const UString &str)
{
	if (str.mLength == 0) return;

	// calculate new size
	const int newSize = mLength + str.mLength;

	// allocate new data buffer
	wchar_t *newUnicode = new wchar_t[newSize+1]; // +1 for null termination later

	// copy existing data
	if (mLength > 0)
		memcpy(newUnicode, mUnicode, mLength*sizeof(wchar_t));

	// copy appended data
	memcpy(&(newUnicode[mLength]), str.mUnicode, (str.mLength+1)*sizeof(wchar_t)); // +1 to also copy the null char from the old string

	// replace the old values with the new
	deleteUnicode();
	mUnicode = newUnicode;
	mLength = newSize;

	// reencode to utf8
	updateUtf8();
}

void UString::insert(int offset, const UString &str)
{
	if (str.mLength == 0) return;

	offset = clamp<int>(offset, 0, mLength);

	// calculate new size
	const int newSize = mLength + str.mLength;

	// allocate new data buffer
	wchar_t *newUnicode = new wchar_t[newSize+1]; // +1 for null termination later

	// if we're not inserting at the beginning of the string
	if (offset > 0)
		memcpy(newUnicode, mUnicode, offset*sizeof(wchar_t)); // copy first part of data

	// copy inserted string
	memcpy(&(newUnicode[offset]), str.mUnicode, str.mLength*sizeof(wchar_t));

	// if we're not inserting at the end of the string
	if (offset < mLength)
	{
		// copy rest of string (including terminating null char)
		const int numRightChars = mLength - offset + 1;
		if (numRightChars > 0)
			memcpy(&(newUnicode[offset+str.mLength]), &(mUnicode[offset]), (numRightChars)*sizeof(wchar_t));
	}
	else
		newUnicode[newSize] = '\0';  // null terminate

	// replace the old values with the new
	deleteUnicode();
	mUnicode = newUnicode;
	mLength = newSize;

	// reencode to utf8
	updateUtf8();
}

void UString::insert(int offset, wchar_t ch)
{
	offset = clamp<int>(offset, 0, mLength);

	// calculate new size
	const int newSize = mLength + 1; // +1 for the added character

	// allocate new data buffer
	wchar_t *newUnicode = new wchar_t[newSize+1]; // and again +1 for null termination later

	// copy first part of data
	if (offset > 0)
		memcpy(newUnicode, mUnicode, offset*sizeof(wchar_t));

	// place the inserted char
	newUnicode[offset] = ch;

	// copy rest of string (including terminating null char)
	const int numRightChars = mLength - offset + 1;
	if (numRightChars > 0)
		memcpy(&(newUnicode[offset+1]), &(mUnicode[offset]), (numRightChars)*sizeof(wchar_t));

	// replace the old values with the new
	deleteUnicode();
	mUnicode = newUnicode;
	mLength = newSize;

	// reencode to utf8
	updateUtf8();
}

void UString::erase(int offset, int count)
{
	if (isUnicodeNull() || mLength == 0 || count == 0 || offset > mLength-1) return;

	offset = clamp<int>(offset, 0, mLength);
	count = clamp<int>(count, 0, mLength - offset);

	// calculate new size
	const int newLength = mLength - count;

	// allocate new data buffer
	wchar_t *newUnicode = new wchar_t[newLength+1]; // +1 for null termination later

	// copy first part of data
	if (offset > 0)
		memcpy(newUnicode, mUnicode, offset*sizeof(wchar_t));

	// copy rest of string (including terminating null char)
	const int numRightChars = newLength - offset + 1;
	if (numRightChars > 0)
		memcpy(&(newUnicode[offset]), &(mUnicode[offset+count]), (numRightChars)*sizeof(wchar_t));

	// replace the old values with the new
	deleteUnicode();
	mUnicode = newUnicode;
	mLength = newLength;

	// reencode to utf8
	updateUtf8();
}

UString UString::substr(int offset, int charCount) const
{
	offset = clamp<int>(offset, 0, mLength);

	if (charCount < 0)
		charCount = mLength - offset;

	charCount = clamp<int>(charCount, 0, mLength - offset);

	// allocate new mem
	UString str;
	str.mLength = charCount;
	str.mUnicode = new wchar_t[charCount+1]; // +1 for null termination later

	// copy mem contents
	if (charCount > 0)
		memcpy(str.mUnicode, &(mUnicode[offset]), charCount*sizeof(wchar_t));

	str.mUnicode[charCount] = '\0'; // null terminate

	// update the substring's utf encoding
	str.updateUtf8();

	return str;
}

std::vector<UString> UString::split(UString delim) const
{
	std::vector<UString> results;
	if (delim.length() < 1 || mLength < 1) return results;

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

UString UString::trim() const
{
	int startPos = 0;
	while (startPos < mLength && std::iswspace(mUnicode[startPos]))
	{
		startPos++;
	}

	int endPos = mLength - 1;
	while ((endPos >= 0) && (endPos < mLength) && std::iswspace(mUnicode[endPos]))
	{
		endPos--;
	}

	return substr(startPos, endPos - startPos + 1);
}

float UString::toFloat() const
{
	return !isUtf8Null() ? std::strtof(mUtf8, NULL) : 0;
}

int UString::toInt() const
{
	return !isUtf8Null() ? (int)std::strtol(mUtf8, NULL, 0) : 0;
}

long UString::toLong() const
{
	return !isUtf8Null() ? std::strtol(mUtf8, NULL, 0) : 0;
}

wchar_t UString::operator [] (int index) const
{
	if (mLength > 0)
		return mUnicode[clamp<int>(index, 0, mLength-1)];

	return (wchar_t)0;
}

UString &UString::operator = (const UString &ustr)
{
	wchar_t *newUnicode = (wchar_t*)nullWString;

	// if this is not a null string
	if (ustr.mLength > 0 && !ustr.isUnicodeNull())
	{
		// allocate new mem for unicode data
		newUnicode = new wchar_t[ustr.mLength+1];

		// copy unicode mem contents
		memcpy(newUnicode, ustr.mUnicode, (ustr.mLength+1)*sizeof(wchar_t));
	}

	// deallocate old mem
	if (!isUnicodeNull())
		delete[] mUnicode;

	// init variables
	mLength = ustr.mLength;
	mUnicode = newUnicode;

	// reencode to utf8
	updateUtf8();

	return *this;
}

UString &UString::operator = (UString &&ustr)
{
	if (this != &ustr)
	{
		// free ourself
		if (!isUnicodeNull())
			delete[] mUnicode;
		if (!isUtf8Null())
			delete[] mUtf8;

		// move
		mLength = ustr.mLength;
		mIsAsciiOnly = ustr.mIsAsciiOnly;
		mUnicode = ustr.mUnicode;
		mUtf8 = ustr.mUtf8;

		// reset source
		ustr.mLength = 0;
		ustr.mIsAsciiOnly = false;
		ustr.mUnicode = NULL;
		ustr.mUtf8 = NULL;
	}

	return *this;
}

bool UString::operator == (const UString &ustr) const
{
	if (mLength != ustr.mLength) return false;

	if (isUnicodeNull() && ustr.isUnicodeNull())
		return true;
	else if (isUnicodeNull() || ustr.isUnicodeNull())
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
			return mUnicode[i] < ustr.mUnicode[i];
	}

	if (mLength == ustr.mLength) return false;

	return mLength < ustr.mLength;
}

int UString::fromUtf8(const char *utf8)
{
	if (utf8 == NULL) return 0;

	size_t supposedStringSize = strlen(utf8) + 1; // +1 due to null char, since we're accessing the raw data below in the utf-8/16/32 check

	int startIndex = 0;
	if (supposedStringSize > 2)
	{
		if (utf8[0] == (char)0xef && utf8[1] == (char)0xbb && utf8[2] == (char)0xbf) // utf-8
			startIndex = 3;
		else
		{
			// check for utf-16
			char c0 = utf8[0];
			char c1 = utf8[1];
			char c2 = utf8[2];
			bool utf16le = (c0 == (char)0xff && c1 == (char)0xfe && c2 != (char)0x00);
			bool utf16be = (c0 == (char)0xfe && c1 == (char)0xff && c2 != (char)0x00);
			if (utf16le || utf16be)
			{
				// TODO: UTF-16 not yet supported
				return 0;
			}

			// check for utf-32
			// HACKHACK: TODO: this check will never work, due to the null characters reporting strlen() as too short (i.e. c1 == 0x00)
			if (supposedStringSize > 3)
			{
				char c3 = utf8[3];
				bool utf32le = (c0 == (char)0xff && c1 == (char)0xfe && c2 == (char)0x00 && c3 == (char)0x00);
				bool utf32be = (c0 == (char)0x00 && c1 == (char)0x00 && c2 == (char)0xfe && c3 == (char)0xff);

				if (utf32le || utf32be)
				{
					// TODO: UTF-32 not yet supported
					return 0;
				}
			}
		}
	}

	mLength = decode(&(utf8[startIndex]), NULL);
	mUnicode = new wchar_t[mLength+1]; // +1 for null termination later
	const int length = decode(&(utf8[startIndex]), mUnicode);

	// reencode to utf8
	updateUtf8();

	return length;
}

int UString::decode(const char *utf8, wchar_t *unicode)
{
	if (utf8 == NULL) return 0; // unicode is checked below

	int length = 0;
	for (int i=0; utf8[i]!=0; i++)
	{
		char b = utf8[i];
		if ((b & USTRING_MASK_1BYTE) == USTRING_VALUE_1BYTE) // if this is a single byte code point
		{
			if (unicode != NULL)
				unicode[length] = b;
		}
		else if ((b & USTRING_MASK_2BYTE) == USTRING_VALUE_2BYTE) // if this is a 2 byte code point
		{
			if (unicode != NULL)
				unicode[length] = getCodePoint(utf8, i, 2, (unsigned char)(~USTRING_MASK_2BYTE));

			i += 1;
		}
		else if ((b & USTRING_MASK_3BYTE) == USTRING_VALUE_3BYTE) // if this is a 3 byte code point
		{
			if (unicode != NULL)
				unicode[length] = getCodePoint(utf8, i, 3, (unsigned char)(~USTRING_MASK_3BYTE));

			i += 2;
		}
		else if ((b & USTRING_MASK_4BYTE) == USTRING_VALUE_4BYTE) // if this is a 4 byte code point
		{
			if (unicode != NULL)
				unicode[length] = getCodePoint(utf8, i, 4, (unsigned char)(~USTRING_MASK_4BYTE));

			i += 3;
		}
		else if ((b & USTRING_MASK_5BYTE) == USTRING_VALUE_5BYTE) // if this is a 5 byte code point
		{
			if (unicode != NULL)
				unicode[length] = getCodePoint(utf8, i, 5, (unsigned char)(~USTRING_MASK_5BYTE));

			i += 4;
		}
		else if ((b & USTRING_MASK_6BYTE) == USTRING_VALUE_6BYTE) // if this is a 6 byte code point
		{
			if (unicode != NULL)
				unicode[length] = getCodePoint(utf8, i, 6, (unsigned char)(~USTRING_MASK_6BYTE));

			i += 5;
		}

		length++;
	}

	if (unicode != NULL)
		unicode[length] = '\0'; // null terminate

	return length;
}

int UString::encode(const wchar_t *unicode, int length, char *utf8, bool *isAsciiOnly)
{
	if (unicode == NULL) return 0; // utf8 is checked below

	int utf8len = 0;
	bool foundMultiByte = false;
	for (int i=0; i<length; i++)
	{
		wchar_t ch = unicode[i];

		///assert(ch >= 0);

		if (ch < 0x00000080) // 1 byte
		{
			if (utf8 != NULL)
				utf8[utf8len] = (char)ch;

			utf8len += 1;
		}
		else if (ch < 0x00000800) // 2 bytes
		{
			foundMultiByte = true;

			if (utf8 != NULL)
				getUtf8(ch, &(utf8[utf8len]), 2, USTRING_VALUE_2BYTE);

			utf8len += 2;
		}
		else if (ch < 0x00010000) // 3 bytes
		{
			foundMultiByte = true;

			if (utf8 != NULL)
				getUtf8(ch, &(utf8[utf8len]), 3, USTRING_VALUE_3BYTE);

			utf8len += 3;
		}
		else if (ch < 0x00200000) // 4 bytes
		{
			foundMultiByte = true;

			if (utf8 != NULL)
				getUtf8(ch, &(utf8[utf8len]), 4, USTRING_VALUE_4BYTE);

			utf8len += 4;
		}
		else if (ch < 0x04000000) // 5 bytes
		{
			foundMultiByte = true;

			if (utf8 != NULL)
				getUtf8(ch, &(utf8[utf8len]), 5, USTRING_VALUE_5BYTE);

			utf8len += 5;
		}
		else // 6 bytes
		{
			foundMultiByte = true;

			if (utf8 != NULL)
				getUtf8(ch, &(utf8[utf8len]), 6, USTRING_VALUE_6BYTE);

			utf8len += 6;
		}
	}

	if (isAsciiOnly != NULL)
		*isAsciiOnly = !foundMultiByte;

	return utf8len;
}

wchar_t UString::getCodePoint(const char *utf8, int offset, int numBytes, unsigned char firstByteMask)
{
	if (utf8 == NULL) return (wchar_t)0;

	// get the bits out of the first byte
	wchar_t wc = utf8[offset] & firstByteMask;

	// iterate over the rest of the bytes
	for (int i=1; i<numBytes; i++)
	{
		// shift the code point bits to make room for the new bits
		wc = wc << 6;

		// add the new bits
		wc |= utf8[offset+i] & USTRING_MASK_MULTIBYTE;
	}

	// return the code point
	return wc;
}

void UString::getUtf8(wchar_t ch, char *utf8, int numBytes, int firstByteValue)
{
	if (utf8 == NULL) return;

	for (int i=numBytes-1; i>0; i--)
	{
		// store the lowest bits in a utf8 byte
		utf8[i] = (ch & USTRING_MASK_MULTIBYTE) | 0x80;
		ch >>= 6;
	}

	// store the remaining bits
	*utf8 = (firstByteValue | ch);
}

void UString::updateUtf8()
{
	// delete previous
	deleteUtf8();

	// rebuild
	const int size = encode(mUnicode, mLength, NULL, &mIsAsciiOnly);
	if (size > 0)
	{
		mUtf8 = new char[size+1];
		encode(mUnicode, mLength, mUtf8, NULL);
		mUtf8[size] = '\0'; // null terminate
	}
}

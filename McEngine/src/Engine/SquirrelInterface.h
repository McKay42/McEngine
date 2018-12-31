//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		scripting system
//
// $NoKeywords: $sqscript
//===============================================================================//

#ifndef SQUIRRELINTERFACE_H
#define SQUIRRELINTERFACE_H

#include "cbase.h"

#ifdef MCENGINE_FEATURE_SQUIRREL

#include <squirrel.h>

#endif

class SquirrelInterface
{
public:
	static void execScriptFile(UString filename);

#ifdef MCENGINE_FEATURE_SQUIRREL

	// API

	static bool createClassInstanceReference(HSQUIRRELVM v, const char *className, SQUserPointer nativeInstancePointer);

	static int createResourceReference(const char *resourceName);
	static UString getResourceNameByReference(int resourceID);

#endif

public:
	SquirrelInterface();
	virtual ~SquirrelInterface();

	void draw(Graphics *g);
	void update();

	int exec(UString script, UString name = "exec");

	void pushHookArgClass(const char *className, void *nativeInstancePointer);
	void callHook(UString hookName);
	void popHookArgs();

	void kill();

	inline bool isReady() const {return m_bReady;}

private:

	int exec(const char *script, size_t length, const char *name);

	bool m_bReady;

#ifdef MCENGINE_FEATURE_SQUIRREL

	void pushRoot();
	void pop();
	void pushTable(const char *tableName);
	void popTable();
	void pushClass(const char *className);
	void popClass();
	void addFunction(const char *functionName, SQFUNCTION function);

	static void squirrel_debugHook(HSQUIRRELVM v, SQInteger type, const SQChar *sourceName, SQInteger line, const SQChar *funcName);
	static void squirrel_printFunc(HSQUIRRELVM v, const SQChar *fmt, ...);
	static void squirrel_errorFunc(HSQUIRRELVM v, const SQChar *fmt, ...);
	static SQInteger squirrel_suspend(HSQUIRRELVM v);

	// vm
	HSQUIRRELVM m_vm;

	// hooks
	enum class HOOK_ARG_TYPE
	{
		CLASS
	};
	struct HOOK_ARG
	{
		HOOK_ARG_TYPE type;
		const char *className;
		SQUserPointer nativeInstancePointer;
	};
	std::vector<HOOK_ARG> m_hookArgs;

	// resources
	static std::unordered_map<std::string, int> m_resourceNameToRef;	// get
	static std::vector<UString> m_resourceRefToName;					// draw

#endif

};

extern SquirrelInterface *squirrel;

#ifdef MCENGINE_FEATURE_SQUIRREL

// API

class Squirrel_Class
{
public:
	static SQUserPointer getNativeInstancePointer(HSQUIRRELVM v, int offset = -1);
};

// classes/tables

class Squirrel_Math
{
public:
	static SQInteger sqrt(HSQUIRRELVM v);
	static SQInteger abs(HSQUIRRELVM v);
	static SQInteger sin(HSQUIRRELVM v);
	static SQInteger cos(HSQUIRRELVM v);
	static SQInteger asin(HSQUIRRELVM v);
	static SQInteger acos(HSQUIRRELVM v);
	static SQInteger log(HSQUIRRELVM v);
	static SQInteger log10(HSQUIRRELVM v);
	static SQInteger tan(HSQUIRRELVM v);
	static SQInteger atan(HSQUIRRELVM v);
	static SQInteger atan2(HSQUIRRELVM v);
	static SQInteger pow(HSQUIRRELVM v);
	static SQInteger floor(HSQUIRRELVM v);
	static SQInteger ceil(HSQUIRRELVM v);
	static SQInteger exp(HSQUIRRELVM v);
};

class Squirrel_Engine
{
public:
	static SQInteger getTime(HSQUIRRELVM v);
	static SQInteger getTimeReal(HSQUIRRELVM v);
	static SQInteger getFrameTime(HSQUIRRELVM v);
	static SQInteger getScreenWidth(HSQUIRRELVM v);
	static SQInteger getScreenHeight(HSQUIRRELVM v);
};

class Squirrel_ResourceManager
{
public:
	static SQInteger getImage(HSQUIRRELVM v);
	static SQInteger getSound(HSQUIRRELVM v);
	static SQInteger getFont(HSQUIRRELVM v);
};

class Squirrel_ConVar : public Squirrel_Class
{
public:
	static SQInteger createConVar(HSQUIRRELVM v);
	static SQInteger getConVarByName(HSQUIRRELVM v);

	static SQInteger setValue(HSQUIRRELVM v);

	static SQInteger getFloat(HSQUIRRELVM v);
	static SQInteger getInt(HSQUIRRELVM v);
	static SQInteger getBool(HSQUIRRELVM v);
	static SQInteger getString(HSQUIRRELVM v);
};

class Squirrel_Graphics : public Squirrel_Class
{
public:
	// color
	static SQInteger setColor(HSQUIRRELVM v);
	static SQInteger setAlpha(HSQUIRRELVM v);

	static SQInteger drawLine(HSQUIRRELVM v);
	static SQInteger fillRect(HSQUIRRELVM v);

	// 2d resource drawing
	static SQInteger drawImage(HSQUIRRELVM v);
	static SQInteger drawString(HSQUIRRELVM v);

	// matrices & transforms
	static SQInteger pushTransform(HSQUIRRELVM v);
	static SQInteger popTransform(HSQUIRRELVM v);

	// 2D
	static SQInteger translate(HSQUIRRELVM v);
	static SQInteger rotate(HSQUIRRELVM v);
	static SQInteger scale(HSQUIRRELVM v);
};

class Squirrel_Image : public Squirrel_Class
{
public:
	static SQInteger getWidth(HSQUIRRELVM v);
	static SQInteger getHeight(HSQUIRRELVM v);
};

class Squirrel_Sound : public Squirrel_Class
{
public:
	static SQInteger setPositionMS(HSQUIRRELVM v);
	static SQInteger setVolume(HSQUIRRELVM v);

	static SQInteger getPositionMS(HSQUIRRELVM v);
	static SQInteger getLengthMS(HSQUIRRELVM v);

	static SQInteger isPlaying(HSQUIRRELVM v);
};

class Squirrel_Font : public Squirrel_Class
{
public:
	static SQInteger getHeight(HSQUIRRELVM v);

	static SQInteger getStringWidth(HSQUIRRELVM v);
	static SQInteger getStringHeight(HSQUIRRELVM v);
};

#endif

#endif

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

#endif

public:
	SquirrelInterface();
	virtual ~SquirrelInterface();

	void draw(Graphics *g);
	void update();

	void exec(UString script, UString name = "exec");

	void callHook(UString hookName);

private:

	void exec(const char *script, size_t length, const char *name);

#ifdef MCENGINE_FEATURE_SQUIRREL

	void push();
	void pop();
	void pushTable(const char *tableName);
	void popTable();
	void pushClass(const char *className);
	void popClass();
	void addFunction(const char *functionName, SQFUNCTION function);

	static void squirrel_printFunc(HSQUIRRELVM v, const SQChar *fmt, ...);
	static void squirrel_errorFunc(HSQUIRRELVM v, const SQChar *fmt, ...);

	HSQUIRRELVM m_vm;

#endif

};

extern SquirrelInterface *squirrel;

#ifdef MCENGINE_FEATURE_SQUIRREL

// API

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
	static SQInteger getFrameTime(HSQUIRRELVM v);
	static SQInteger getScreenWidth(HSQUIRRELVM v);
	static SQInteger getScreenHeight(HSQUIRRELVM v);
};

class Squirrel_ConVar
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

class Squirrel_Graphics
{
public:
	static SQInteger drawLine(HSQUIRRELVM v);
	static SQInteger fillRect(HSQUIRRELVM v);

	static SQInteger setColor(HSQUIRRELVM v);
};

#endif

#endif

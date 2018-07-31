//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		scripting system
//
// $NoKeywords: $sqscript
//===============================================================================//

#include "SquirrelInterface.h"

#include "Engine.h"
#include "ConVar.h"
#include "File.h"

#ifdef MCENGINE_FEATURE_SQUIRREL

#include <sqstdio.h>
#include <sqstdaux.h>

#endif

#define RM_SCRIPTS_FOLDER "scripts/"

SquirrelInterface *squirrel = NULL;

SquirrelInterface::SquirrelInterface()
{
	squirrel = this;

#ifdef MCENGINE_FEATURE_SQUIRREL

	m_vm = sq_open(1024);
    sqstd_seterrorhandlers(m_vm);
    sq_setprintfunc(m_vm, squirrel_printFunc, squirrel_errorFunc);

    // API
    push();
    {
    	// global function tables
    	pushTable("math");
    	{
    		addFunction("sqrt", Squirrel_Math::sqrt);
    		addFunction("abs", Squirrel_Math::abs);
    		addFunction("sin", Squirrel_Math::sin);
    		addFunction("cos", Squirrel_Math::cos);
    		addFunction("asin", Squirrel_Math::asin);
    		addFunction("acos", Squirrel_Math::acos);
    		addFunction("log", Squirrel_Math::log);
    		addFunction("log10", Squirrel_Math::log10);
    		addFunction("tan", Squirrel_Math::tan);
    		addFunction("atan", Squirrel_Math::atan);
    		addFunction("atan2", Squirrel_Math::atan2);
    		addFunction("pow", Squirrel_Math::pow);
    		addFunction("floor", Squirrel_Math::floor);
    		addFunction("ceil", Squirrel_Math::ceil);
    		addFunction("exp", Squirrel_Math::exp);
    	}
    	popTable();

    	pushTable("engine");
    	{
    		addFunction("getTime", Squirrel_Engine::getTime);
    		addFunction("getFrameTime", Squirrel_Engine::getFrameTime);
    		addFunction("getScreenWidth", Squirrel_Engine::getScreenWidth);
    		addFunction("getScreenHeight", Squirrel_Engine::getScreenHeight);

    		addFunction("drawLine", Squirrel_Graphics::drawLine); // TODO: temp. remove later
    		addFunction("fillRect", Squirrel_Graphics::fillRect); // TODO: temp. remove later
    		addFunction("setColor", Squirrel_Graphics::setColor); // TODO: temp. remove later
    	}
    	popTable();

    	pushTable("convar");
    	{
    		addFunction("createConVar", Squirrel_ConVar::createConVar);
    		addFunction("getConVarByName", Squirrel_ConVar::getConVarByName);
    	}
    	popTable();



    	// classes
    	pushClass("ConVar");
    	{
    		addFunction("setValue", Squirrel_ConVar::setValue);

    		addFunction("getFloat", Squirrel_ConVar::getFloat);
    		addFunction("getInt", Squirrel_ConVar::getInt);
    		addFunction("getBool", Squirrel_ConVar::getBool);
    		addFunction("getString", Squirrel_ConVar::getString);
    	}
    	popClass();

    	pushClass("Graphics");
    	{
    		addFunction("drawLine", Squirrel_Graphics::drawLine);
    		addFunction("fillRect", Squirrel_Graphics::fillRect);

    		addFunction("setColor", Squirrel_Graphics::setColor);
    	}
    	popClass();
    }
    pop();

    debugLog("Squirrel: Version %i\n", sq_getversion());

#endif
}

SquirrelInterface::~SquirrelInterface()
{
#ifdef MCENGINE_FEATURE_SQUIRREL

	sq_close(m_vm);

#endif

	squirrel = NULL;
}

void SquirrelInterface::draw(Graphics *g)
{

}

void SquirrelInterface::update()
{

}

void SquirrelInterface::execScriptFile(UString filename)
{
	UString filepath = RM_SCRIPTS_FOLDER;
	filepath.append(filename);

	if (filepath.find(".") == -1) // env->getFileExtensionFromFilePath(filepath) != "nut"
		filepath.append(".nut");

	if (!env->fileExists(filepath))
	{
		debugLog("SquirrelInterface::execScriptFile() error, file \"%s\" not found!\n", filepath.toUtf8());
		return;
	}

	File file(filepath);
	if (file.canRead() && file.getFileSize() > 0)
		squirrel->exec(file.readFile(), file.getFileSize(), filepath.toUtf8());
	else
		debugLog("SquirrelInterface::execScriptFile() error, couldn't read file \"%s\"!\n", filepath.toUtf8());
}

void SquirrelInterface::exec(UString script, UString name)
{
	exec(script.toUtf8(), script.length(), name.toUtf8());
}

void SquirrelInterface::callHook(UString hookName)
{
#ifdef MCENGINE_FEATURE_SQUIRREL

	// TODO: this currently still breaks if no script is loaded
	push();
	{
		sq_pushstring(m_vm, hookName.toUtf8(), hookName.length());
		sq_get(m_vm, -2);

		push();
		{
			sq_call(m_vm, 1, SQFalse, SQTrue);
		}
		pop();
	}
	pop();

#endif
}

void SquirrelInterface::exec(const char *script, size_t length, const char *name)
{
#ifdef MCENGINE_FEATURE_SQUIRREL

	SQInteger top = sq_gettop(m_vm);
	{
		if (SQ_SUCCEEDED(sq_compilebuffer(m_vm, script, length, name, SQTrue)))
		{
			sq_pushroottable(m_vm);
			sq_call(m_vm, 1, SQFalse, SQTrue);
			sq_pop(m_vm, 1);
		}
		else
			debugLog(0xffff0000, "SquirrelInterface::exec() failed to sq_compilebuffer()!\n");
	}
	sq_settop(m_vm, top); // TODO: what does this do, and is it necessary?

#endif
}

#ifdef MCENGINE_FEATURE_SQUIRREL

void SquirrelInterface::push()
{
	sq_pushroottable(m_vm);
}

void SquirrelInterface::pop()
{
	sq_pop(m_vm, 1);
}

void SquirrelInterface::pushTable(const char *tableName)
{
	sq_pushstring(m_vm, tableName, -1);
	sq_newtable(m_vm);
}

void SquirrelInterface::popTable()
{
	sq_newslot(m_vm, -3, SQFalse);
}

void SquirrelInterface::pushClass(const char *className)
{
	sq_pushstring(m_vm, className, -1);
	sq_newclass(m_vm, SQFalse);
}

void SquirrelInterface::popClass()
{
	sq_createslot(m_vm, -3);
}

void SquirrelInterface::addFunction(const char *functionName, SQFUNCTION function)
{
	sq_pushstring(m_vm, functionName, -1);
	sq_newclosure(m_vm, function, 0);
	sq_newslot(m_vm, -3, SQFalse);
}

bool SquirrelInterface::createClassInstanceReference(HSQUIRRELVM v, const char *className, SQUserPointer nativeInstancePointer)
{
	int oldTop = sq_gettop(v);

	sq_pushroottable(v);
	sq_pushstring(v, className, -1);

	if (SQ_FAILED(sq_rawget(v, -2)))
	{
		sq_settop(v, oldTop);
		return false;
	}

	if (SQ_FAILED(sq_createinstance(v, -1)))
	{
		sq_settop(v, oldTop);
		return false;
	}

	sq_remove(v, -3); // removes the root table
	sq_remove(v, -2); // removes the class

	if (SQ_FAILED(sq_setinstanceup(v, -1, nativeInstancePointer)))
	{
		sq_settop(v, oldTop);
		return false;
	}

	return true;
}

void SquirrelInterface::squirrel_printFunc(HSQUIRRELVM vm, const SQChar *fmt, ...)
{
	if (fmt == NULL) return;

	va_list ap;
	va_start(ap, fmt);

	debugLog(fmt, ap);
	printf("\n");

	va_end(ap);
}

void SquirrelInterface::squirrel_errorFunc(HSQUIRRELVM v, const SQChar *fmt, ...)
{
	if (fmt == NULL) return;

	va_list ap;
	va_start(ap, fmt);

	debugLog(0xffff0000, fmt, ap);

	va_end(ap);
}



SQInteger Squirrel_Math::sqrt(HSQUIRRELVM v)
{
    SQFloat f;
    sq_getfloat(v, 2, &f);
    sq_pushfloat(v, (SQFloat)std::sqrt(f));
    return 1;
}

SQInteger Squirrel_Math::abs(HSQUIRRELVM v)
{
	SQObjectType type = sq_gettype(v, 2);
	if (type == OT_FLOAT)
	{
	    SQFloat f;
	    sq_getfloat(v, 2, &f);
	    sq_pushfloat(v, (SQFloat)std::abs(f));
	    return 1;
	}
	else if (type == OT_INTEGER)
	{
	    SQInteger i;
	    sq_getinteger(v, 2, &i);
	    sq_pushinteger(v, (SQInteger)std::abs(i));
	    return 1;
	}
	return 0;
}

SQInteger Squirrel_Math::sin(HSQUIRRELVM v)
{
    SQFloat f;
    sq_getfloat(v, 2, &f);
    sq_pushfloat(v, (SQFloat)std::sin(f));
    return 1;
}

SQInteger Squirrel_Math::cos(HSQUIRRELVM v)
{
    SQFloat f;
    sq_getfloat(v, 2, &f);
    sq_pushfloat(v, (SQFloat)std::cos(f));
    return 1;
}

SQInteger Squirrel_Math::asin(HSQUIRRELVM v)
{
    SQFloat f;
    sq_getfloat(v, 2, &f);
    sq_pushfloat(v, (SQFloat)std::asin(f));
    return 1;
}

SQInteger Squirrel_Math::acos(HSQUIRRELVM v)
{
    SQFloat f;
    sq_getfloat(v, 2, &f);
    sq_pushfloat(v, (SQFloat)std::acos(f));
    return 1;
}

SQInteger Squirrel_Math::log(HSQUIRRELVM v)
{
    SQFloat f;
    sq_getfloat(v, 2, &f);
    sq_pushfloat(v, (SQFloat)std::log(f));
    return 1;
}

SQInteger Squirrel_Math::log10(HSQUIRRELVM v)
{
    SQFloat f;
    sq_getfloat(v, 2, &f);
    sq_pushfloat(v, (SQFloat)std::log10(f));
    return 1;
}

SQInteger Squirrel_Math::tan(HSQUIRRELVM v)
{
    SQFloat f;
    sq_getfloat(v, 2, &f);
    sq_pushfloat(v, (SQFloat)std::tan(f));
    return 1;
}

SQInteger Squirrel_Math::atan(HSQUIRRELVM v)
{
    SQFloat f;
    sq_getfloat(v, 2, &f);
    sq_pushfloat(v, (SQFloat)std::atan(f));
    return 1;
}

SQInteger Squirrel_Math::atan2(HSQUIRRELVM v)
{
    SQFloat f1, f2;
    sq_getfloat(v, 2, &f1);
    sq_getfloat(v, 3, &f2);
    sq_pushfloat(v, (SQFloat)std::atan2(f1, f2));
    return 1;
}

SQInteger Squirrel_Math::pow(HSQUIRRELVM v)
{
    SQFloat f1, f2;
    sq_getfloat(v, 2, &f1);
    sq_getfloat(v, 3, &f2);
    sq_pushfloat(v, (SQFloat)std::pow(f1, f2));
    return 1;
}

SQInteger Squirrel_Math::floor(HSQUIRRELVM v)
{
    SQFloat f;
    sq_getfloat(v, 2, &f);
    sq_pushfloat(v, (SQFloat)std::floor(f));
    return 1;
}

SQInteger Squirrel_Math::ceil(HSQUIRRELVM v)
{
    SQFloat f;
    sq_getfloat(v, 2, &f);
    sq_pushfloat(v, (SQFloat)std::ceil(f));
    return 1;
}

SQInteger Squirrel_Math::exp(HSQUIRRELVM v)
{
    SQFloat f;
    sq_getfloat(v, 2, &f);
    sq_pushfloat(v, (SQFloat)std::exp(f));
    return 1;
}



SQInteger Squirrel_Engine::getTime(HSQUIRRELVM v)
{
	sq_pushfloat(v, (SQFloat)engine->getTime());
	return 1;
}

SQInteger Squirrel_Engine::getFrameTime(HSQUIRRELVM v)
{
	sq_pushfloat(v, (SQFloat)engine->getFrameTime());
	return 1;
}

SQInteger Squirrel_Engine::getScreenWidth(HSQUIRRELVM v)
{
	sq_pushinteger(v, (SQInteger)engine->getScreenWidth());
	return 1;
}

SQInteger Squirrel_Engine::getScreenHeight(HSQUIRRELVM v)
{
	sq_pushinteger(v, (SQInteger)engine->getScreenHeight());
	return 1;
}



SQInteger Squirrel_ConVar::createConVar(HSQUIRRELVM v)
{
	const SQInteger argc = sq_gettop(v);
	void *result = NULL;
	if (argc > 1 && sq_gettype(v, 2) == OT_STRING)
	{
		const SQChar *name = NULL;
		if (SQ_SUCCEEDED(sq_getstring(v, 2, &name)) && name != NULL)
		{
			// check if the convar already exists, return the existing one if it does
			ConVar *cvar = convar->getConVarByName(UString(name), false);

			if (cvar != NULL)
				result = cvar;
			else
			{
				if (argc > 2 && sq_gettype(v, 3) == OT_STRING)
				{
					const SQChar *value = NULL;
					if (SQ_SUCCEEDED(sq_getstring(v, 3, &value)) && value != NULL)
					{
						if (argc > 3 && sq_gettype(v, 4) == OT_STRING)
						{
							const SQChar *help = NULL;
							if (SQ_SUCCEEDED(sq_getstring(v, 4, &help)) && help != NULL)
							{
								// name + value + helptext
								result = new ConVar(UString(name), value, UString(help));
							}
						}
						else // name + value
							result = new ConVar(UString(name), value);
					}
				}
				else // name
					result = new ConVar(UString(name));
			}
		}
	}

	if (result != NULL)
		SquirrelInterface::createClassInstanceReference(v, "ConVar", result);
	else
		sq_pushnull(v);

	return 1;
}

SQInteger Squirrel_ConVar::getConVarByName(HSQUIRRELVM v)
{
	const SQInteger argc = sq_gettop(v);
	void *result = NULL;
	if (argc == 2 && sq_gettype(v, 2) == OT_STRING)
	{
		const SQChar *str = NULL;
		if (SQ_SUCCEEDED(sq_getstring(v, 2, &str)) && str != NULL)
		{
			UString strCopy = UString(str);
			result = convar->getConVarByName(strCopy, false);
		}
	}

	if (result != NULL)
		SquirrelInterface::createClassInstanceReference(v, "ConVar", result);
	else
		sq_pushnull(v);

	return 1;
}

SQInteger Squirrel_ConVar::setValue(HSQUIRRELVM v)
{
	const int oldTop = sq_gettop(v);

	SQUserPointer nativeInstancePointer;
	if (SQ_FAILED(sq_getinstanceup(v, -2, &nativeInstancePointer, 0)))
	{
		sq_settop(v, oldTop);
		return false;
	}

	if (nativeInstancePointer != NULL)
	{
		ConVar *cvar = (ConVar*)(nativeInstancePointer);
		SQFloat fValue = 0.0f;
		SQInteger iValue = 0;
		const SQChar *sValue = NULL;
		if (SQ_SUCCEEDED(sq_getfloat(v, -1, &fValue)))
			cvar->setValue(fValue);
		else if (SQ_SUCCEEDED(sq_getstring(v, -1, &sValue)) && sValue != NULL)
			cvar->setValue(UString(sValue));
		else if (SQ_SUCCEEDED(sq_getinteger(v, -1, &iValue)))
			cvar->setValue(iValue);
	}

	return 0;
}

SQInteger Squirrel_ConVar::getFloat(HSQUIRRELVM v)
{
	const int oldTop = sq_gettop(v);

	SQUserPointer nativeInstancePointer;
	if (SQ_FAILED(sq_getinstanceup(v, -1, &nativeInstancePointer, 0)))
	{
		sq_settop(v, oldTop);
		return false;
	}

	if (nativeInstancePointer != NULL)
	{
		ConVar *cvar = (ConVar*)(nativeInstancePointer);
		sq_pushfloat(v, cvar->getFloat());
		return 1;
	}

	return 0;
}

SQInteger Squirrel_ConVar::getInt(HSQUIRRELVM v)
{
	const int oldTop = sq_gettop(v);

	SQUserPointer nativeInstancePointer;
	if (SQ_FAILED(sq_getinstanceup(v, -1, &nativeInstancePointer, 0)))
	{
		sq_settop(v, oldTop);
		return false;
	}

	if (nativeInstancePointer != NULL)
	{
		ConVar *cvar = (ConVar*)(nativeInstancePointer);
		sq_pushinteger(v, cvar->getInt());
		return 1;
	}

	return 0;
}

SQInteger Squirrel_ConVar::getBool(HSQUIRRELVM v)
{
	const int oldTop = sq_gettop(v);

	SQUserPointer nativeInstancePointer;
	if (SQ_FAILED(sq_getinstanceup(v, -1, &nativeInstancePointer, 0)))
	{
		sq_settop(v, oldTop);
		return false;
	}

	if (nativeInstancePointer != NULL)
	{
		ConVar *cvar = (ConVar*)(nativeInstancePointer);
		sq_pushbool(v, cvar->getBool());
		return 1;
	}

	return 0;
}

SQInteger Squirrel_ConVar::getString(HSQUIRRELVM v)
{
	const int oldTop = sq_gettop(v);

	SQUserPointer nativeInstancePointer;
	if (SQ_FAILED(sq_getinstanceup(v, -1, &nativeInstancePointer, 0)))
	{
		sq_settop(v, oldTop);
		return false;
	}

	if (nativeInstancePointer != NULL)
	{
		ConVar *cvar = (ConVar*)(nativeInstancePointer);
		sq_pushstring(v, cvar->getString().toUtf8(), cvar->getString().length());
		return 1;
	}

	return 0;
}



SQInteger Squirrel_Graphics::drawLine(HSQUIRRELVM v)
{
	SQInteger x1, y1, x2, y2;
	if (SQ_SUCCEEDED(sq_getinteger(v, 2, &x1)) && SQ_SUCCEEDED(sq_getinteger(v, 3, &y1)) && SQ_SUCCEEDED(sq_getinteger(v, 4, &x2)) && SQ_SUCCEEDED(sq_getinteger(v, 5, &y2)))
		engine->getGraphics()->drawLine(x1, y1, x2, y2);

	return 0;
}

SQInteger Squirrel_Graphics::fillRect(HSQUIRRELVM v)
{
	SQInteger x, y, width, height;
	if (SQ_SUCCEEDED(sq_getinteger(v, 2, &x)) && SQ_SUCCEEDED(sq_getinteger(v, 3, &y)) && SQ_SUCCEEDED(sq_getinteger(v, 4, &width)) && SQ_SUCCEEDED(sq_getinteger(v, 5, &height)))
		engine->getGraphics()->fillRect(x, y, width, height);

	return 0;
}

SQInteger Squirrel_Graphics::setColor(HSQUIRRELVM v)
{
	SQInteger color;
	if (SQ_SUCCEEDED(sq_getinteger(v, 2, &color)))
		engine->getGraphics()->setColor((Color)color);

	return 0;
}

#endif



//************************//
//	Squirrel ConCommands  //
//************************//

void _squirrel_exec(UString args)
{
	SquirrelInterface::execScriptFile(args);
}
void _squirrel_do(UString args)
{
	squirrel->exec(args);
}

ConVar _squirrel_exec_("squirrel_exec", _squirrel_exec);
ConVar _squirrel_do_("squirrel_do", _squirrel_do);

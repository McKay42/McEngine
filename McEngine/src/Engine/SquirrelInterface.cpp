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
#include "Console.h"
#include "ResourceManager.h"

#ifdef MCENGINE_FEATURE_SQUIRREL

#include <sqstdio.h>
#include <sqstdaux.h>

#endif

#define RM_SCRIPTS_FOLDER "scripts/"

ConVar debug_squirrel("debug_squirrel", false);
ConVar squirrel_timeout("squirrel_timeout", 2.0f);

SquirrelInterface *squirrel = NULL;

#ifdef MCENGINE_FEATURE_SQUIRREL

std::unordered_map<std::string, int> SquirrelInterface::m_resourceNameToRef;
std::vector<UString> SquirrelInterface::m_resourceRefToName;

#endif

// TODO: multi-script multi-vm handling, reload scripts during runtime
// TODO: finish hook system
// TODO: customize squirrel to allow killing infinite loops

SquirrelInterface::SquirrelInterface()
{
	squirrel = this;

	m_bReady = false;

#ifdef MCENGINE_FEATURE_SQUIRREL

	m_vm = sq_open(1024);
	sq_enabledebuginfo(m_vm, SQTrue);
	///sq_setnativedebughook(m_vm, squirrel_debugHook);
	///sqstd_seterrorhandlers(m_vm); // TODO: think about this
	sq_setprintfunc(m_vm, squirrel_printFunc, squirrel_errorFunc);

    // API
    pushRoot();
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
    		addFunction("getTimeReal", Squirrel_Engine::getTimeReal);
    		addFunction("getFrameTime", Squirrel_Engine::getFrameTime);
    		addFunction("getScreenWidth", Squirrel_Engine::getScreenWidth);
    		addFunction("getScreenHeight", Squirrel_Engine::getScreenHeight);
    	}
    	popTable();

    	pushTable("resources");
    	{
			addFunction("getImage", Squirrel_ResourceManager::getImage);
			addFunction("getSound", Squirrel_ResourceManager::getSound);
			addFunction("getFont", Squirrel_ResourceManager::getFont);
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
    		addFunction("setColor", Squirrel_Graphics::setColor);
    		addFunction("setAlpha", Squirrel_Graphics::setAlpha);

    		addFunction("drawLine", Squirrel_Graphics::drawLine);
    		addFunction("fillRect", Squirrel_Graphics::fillRect);

    		addFunction("drawImage", Squirrel_Graphics::drawImage);
    		addFunction("drawString", Squirrel_Graphics::drawString);

    		addFunction("pushTransform", Squirrel_Graphics::pushTransform);
    		addFunction("popTransform", Squirrel_Graphics::popTransform);

    		addFunction("translate", Squirrel_Graphics::translate);
    		addFunction("rotate", Squirrel_Graphics::rotate);
    		addFunction("scale", Squirrel_Graphics::scale);
    	}
    	popClass();

    	pushClass("Image");
    	{
    		addFunction("getWidth", Squirrel_Image::getWidth);
    		addFunction("getHeight", Squirrel_Image::getHeight);
    	}
    	popClass();

    	pushClass("Sound");
    	{
    		addFunction("setPositionMS", Squirrel_Sound::setPositionMS);
    		addFunction("setVolume", Squirrel_Sound::setVolume);

    		addFunction("getPositionMS", Squirrel_Sound::getPositionMS);
    		addFunction("getLengthMS", Squirrel_Sound::getLengthMS);

    		addFunction("isPlaying", Squirrel_Sound::isPlaying);
		}
		popClass();

		pushClass("Font");
		{
			addFunction("getHeight", Squirrel_Font::getHeight);

			addFunction("getStringWidth", Squirrel_Font::getStringWidth);
			addFunction("getStringHeight", Squirrel_Font::getStringHeight);
		}
		popClass();
	}
    pop();

	debugLog("Squirrel: Version %i\n", sq_getversion());

	m_bReady = true;



    // TEMP: DEBUG:
    Console::execConfigFile("autoexec");



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

int SquirrelInterface::exec(UString script, UString name)
{
	return exec(script.toUtf8(), script.length(), name.toUtf8());
}

void SquirrelInterface::pushHookArgClass(const char *className, void *nativeInstancePointer)
{
#ifdef MCENGINE_FEATURE_SQUIRREL

	if (m_hookArgs.size() > 0)
	{
		engine->showMessageErrorFatal("Squirrel Hook Arg Leak", "Make sure all push*() have a pop*()!");
		engine->shutdown();
		return;
	}

	HOOK_ARG arg;
	arg.type = HOOK_ARG_TYPE::CLASS;
	arg.nativeInstancePointer = nativeInstancePointer;
	arg.className = className;
	m_hookArgs.push_back(arg);

#endif
}

void SquirrelInterface::callHook(UString hookName)
{
#ifdef MCENGINE_FEATURE_SQUIRREL

	if (!m_bReady || sq_getvmstate(m_vm) == SQ_VMSTATE_SUSPENDED) return;

	pushRoot(); // main
	{
		sq_pushstring(m_vm, hookName.toUtf8(), hookName.length());
		SQRESULT res = sq_get(m_vm, -2); // get the function from the root table
		if (SQ_SUCCEEDED(res))
		{
			pushRoot(); //'this' (function environment object)
			{
				// args
				for (int i=0; i<m_hookArgs.size(); i++)
				{
					switch (m_hookArgs[i].type)
					{
					case HOOK_ARG_TYPE::CLASS:
						createClassInstanceReference(m_vm, m_hookArgs[i].className, m_hookArgs[i].nativeInstancePointer);
						break;
					}
				}

				// execute
				res = sq_call(m_vm, 1 + m_hookArgs.size(), SQFalse, SQTrue);
				if (SQ_FAILED(res))
					debugLog("SquirrelInterface::callHook( %s ) failed with %i (check your arguments)\n", hookName.toUtf8(), (int)res);
			}
			pop();
		}
		else if (debug_squirrel.getBool())
			debugLog("SquirrelInterface::callHook( %s ) failed with %i (not found)\n", hookName.toUtf8(), (int)res);
	}
	pop();

#endif
}

void SquirrelInterface::popHookArgs()
{
#ifdef MCENGINE_FEATURE_SQUIRREL

	m_hookArgs.clear();

#endif
}

void SquirrelInterface::kill()
{
#ifdef MCENGINE_FEATURE_SQUIRREL

	m_bReady = false;

#endif
}

int SquirrelInterface::exec(const char *script, size_t length, const char *name)
{
#ifdef MCENGINE_FEATURE_SQUIRREL

	if (!m_bReady) return 0;

	SQInteger top = sq_gettop(m_vm);
	{
		if (SQ_SUCCEEDED(sq_compilebuffer(m_vm, script, length, name, SQTrue))) // pushes entry point on the stack
		{
			sq_pushroottable(m_vm);
			sq_call(m_vm, 1, SQFalse, SQTrue);
			sq_pop(m_vm, 1);
		}
		else
			debugLog(0xffff0000, "SquirrelInterface::exec() failed to sq_compilebuffer()!\n");
	}
	sq_settop(m_vm, top); // top should be 0 here!

	// TODO: return entry point? (though settop will delete everything below it?) somehow return a value which allows us to execute functions within this script

#else

	return -1; // TODO

#endif
}

#ifdef MCENGINE_FEATURE_SQUIRREL

void SquirrelInterface::pushRoot()
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

int SquirrelInterface::createResourceReference(const char *resourceName)
{
	if (resourceName == NULL)
	{
		debugLog("ERROR: SquirrelInterface::createResourceReference(NULL)!\n");
		return -1;
	}

	const std::string stdResourceName(resourceName);

	if (stdResourceName.length() < 1) return -1;

	if (m_resourceNameToRef.find(stdResourceName) != m_resourceNameToRef.end())
		return m_resourceNameToRef[stdResourceName];
	else
	{
		const int resourceID = m_resourceRefToName.size() + 1;

		debugLog("DEBUG: SquirrelInterface::createResourceReference( %s ) = %i\n", resourceName, resourceID);

		m_resourceNameToRef[stdResourceName] = resourceID;
		m_resourceRefToName.push_back(UString(resourceName));
		return resourceID;
	}
}

UString SquirrelInterface::getResourceNameByReference(int resourceID)
{
	resourceID--;

	if (resourceID > -1 && resourceID < m_resourceRefToName.size())
		return m_resourceRefToName[resourceID];
	else
		return UString("nil");
}

void SquirrelInterface::squirrel_debugHook(HSQUIRRELVM v, SQInteger type, const SQChar *sourceName, SQInteger line, const SQChar *funcName)
{
	//debugLog("debugHook: type = %i, sourceName = %s, line = %i, funcName = %s\n", type, sourceName, line, funcName);
	// NOTE: can't use this for killing endless loops, must use custom squirrel
}

void SquirrelInterface::squirrel_printFunc(HSQUIRRELVM vm, const SQChar *fmt, ...)
{
	if (fmt == NULL) return;

	va_list ap;
	va_start(ap, fmt);

	// TODO: check for format string vulnerabilities
	debugLog(fmt, ap);
	printf("\n");

	va_end(ap);
}

void SquirrelInterface::squirrel_errorFunc(HSQUIRRELVM v, const SQChar *fmt, ...)
{
	if (fmt == NULL) return;

	va_list ap;
	va_start(ap, fmt);

	// TODO: check for format string vulnerabilities
	debugLog(0xffff0000, fmt, ap);

	va_end(ap);
}

SQInteger SquirrelInterface::squirrel_suspend(HSQUIRRELVM v)
{
	return sq_suspendvm(v);
}



SQUserPointer Squirrel_Class::getNativeInstancePointer(HSQUIRRELVM v, int offset)
{
	const int oldTop = sq_gettop(v);

	SQUserPointer nativeInstancePointer;
	if (SQ_FAILED(sq_getinstanceup(v, offset, &nativeInstancePointer, 0)))
	{
		sq_settop(v, oldTop);
		return NULL;
	}

	return nativeInstancePointer;
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

SQInteger Squirrel_Engine::getTimeReal(HSQUIRRELVM v)
{
	sq_pushfloat(v, (SQFloat)engine->getTimeReal());
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



SQInteger Squirrel_ResourceManager::getImage(HSQUIRRELVM v)
{
	const SQInteger argc = sq_gettop(v);
	void *result = NULL;
	if (argc == 2 && sq_gettype(v, 2) == OT_STRING)
	{
		const SQChar *str = NULL;
		if (SQ_SUCCEEDED(sq_getstring(v, 2, &str)) && str != NULL)
			result = (void*)SquirrelInterface::createResourceReference(str);
	}

	if (result != NULL)
		SquirrelInterface::createClassInstanceReference(v, "Image", result);
	else
		sq_pushnull(v);

	return 1;
}

SQInteger Squirrel_ResourceManager::getSound(HSQUIRRELVM v)
{
	const SQInteger argc = sq_gettop(v);
	void *result = NULL;
	if (argc == 2 && sq_gettype(v, 2) == OT_STRING)
	{
		const SQChar *str = NULL;
		if (SQ_SUCCEEDED(sq_getstring(v, 2, &str)) && str != NULL)
			result = (void*)SquirrelInterface::createResourceReference(str);
	}

	if (result != NULL)
		SquirrelInterface::createClassInstanceReference(v, "Sound", result);
	else
		sq_pushnull(v);

	return 1;
}

SQInteger Squirrel_ResourceManager::getFont(HSQUIRRELVM v)
{
	const SQInteger argc = sq_gettop(v);
	void *result = NULL;
	if (argc == 2 && sq_gettype(v, 2) == OT_STRING)
	{
		const SQChar *str = NULL;
		if (SQ_SUCCEEDED(sq_getstring(v, 2, &str)) && str != NULL)
			result = (void*)SquirrelInterface::createResourceReference(str);
	}

	if (result != NULL)
		SquirrelInterface::createClassInstanceReference(v, "Font", result);
	else
		sq_pushnull(v);

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
			result = convar->getConVarByName(UString(str), false);
	}

	if (result != NULL)
		SquirrelInterface::createClassInstanceReference(v, "ConVar", result);
	else
		sq_pushnull(v);

	return 1;
}

SQInteger Squirrel_ConVar::setValue(HSQUIRRELVM v)
{
	SQUserPointer nativeInstancePointer = getNativeInstancePointer(v);

	if (nativeInstancePointer != NULL)
	{
		ConVar *cvar = (ConVar*)(nativeInstancePointer);
		SQFloat fValue = 0.0f;
		SQInteger iValue = 0;
		const SQChar *sValue = NULL;
		if (SQ_SUCCEEDED(sq_getfloat(v, -1, &fValue)))
			cvar->setValue((float)fValue);
		else if (SQ_SUCCEEDED(sq_getstring(v, -1, &sValue)) && sValue != NULL)
			cvar->setValue(UString(sValue));
		else if (SQ_SUCCEEDED(sq_getinteger(v, -1, &iValue)))
			cvar->setValue((int)iValue);
	}

	return 0;
}

SQInteger Squirrel_ConVar::getFloat(HSQUIRRELVM v)
{
	SQUserPointer nativeInstancePointer = getNativeInstancePointer(v);

	if (nativeInstancePointer != NULL)
	{
		ConVar *cvar = (ConVar*)(nativeInstancePointer);
		sq_pushfloat(v, (SQFloat)cvar->getFloat());
		return 1;
	}

	return 0;
}

SQInteger Squirrel_ConVar::getInt(HSQUIRRELVM v)
{
	SQUserPointer nativeInstancePointer = getNativeInstancePointer(v);

	if (nativeInstancePointer != NULL)
	{
		ConVar *cvar = (ConVar*)(nativeInstancePointer);
		sq_pushinteger(v, (SQInteger)cvar->getInt());
		return 1;
	}

	return 0;
}

SQInteger Squirrel_ConVar::getBool(HSQUIRRELVM v)
{
	SQUserPointer nativeInstancePointer = getNativeInstancePointer(v);

	if (nativeInstancePointer != NULL)
	{
		ConVar *cvar = (ConVar*)(nativeInstancePointer);
		sq_pushbool(v, (SQBool)cvar->getBool());
		return 1;
	}

	return 0;
}

SQInteger Squirrel_ConVar::getString(HSQUIRRELVM v)
{
	SQUserPointer nativeInstancePointer = getNativeInstancePointer(v);

	if (nativeInstancePointer != NULL)
	{
		ConVar *cvar = (ConVar*)(nativeInstancePointer);
		sq_pushstring(v, (const SQChar*)cvar->getString().toUtf8(), cvar->getString().length());
		return 1;
	}

	return 0;
}



SQInteger Squirrel_Graphics::setColor(HSQUIRRELVM v)
{
	SQInteger color;
	if (SQ_SUCCEEDED(sq_getinteger(v, 2, &color)))
		engine->getGraphics()->setColor((Color)color);

	return 0;
}

SQInteger Squirrel_Graphics::setAlpha(HSQUIRRELVM v)
{
	SQFloat alpha;
	if (SQ_SUCCEEDED(sq_getfloat(v, 2, &alpha)))
		engine->getGraphics()->setAlpha((float)alpha);

	return 0;
}

SQInteger Squirrel_Graphics::drawLine(HSQUIRRELVM v)
{
	SQInteger x1, y1, x2, y2;
	if (SQ_SUCCEEDED(sq_getinteger(v, 2, &x1)) && SQ_SUCCEEDED(sq_getinteger(v, 3, &y1)) && SQ_SUCCEEDED(sq_getinteger(v, 4, &x2)) && SQ_SUCCEEDED(sq_getinteger(v, 5, &y2)))
		engine->getGraphics()->drawLine((int)x1, (int)y1, (int)x2, (int)y2);

	return 0;
}

SQInteger Squirrel_Graphics::fillRect(HSQUIRRELVM v)
{
	SQInteger x, y, width, height;
	if (SQ_SUCCEEDED(sq_getinteger(v, 2, &x)) && SQ_SUCCEEDED(sq_getinteger(v, 3, &y)) && SQ_SUCCEEDED(sq_getinteger(v, 4, &width)) && SQ_SUCCEEDED(sq_getinteger(v, 5, &height)))
		engine->getGraphics()->fillRect((int)x, (int)y, (int)width, (int)height);

	return 0;
}

SQInteger Squirrel_Graphics::drawImage(HSQUIRRELVM v)
{
	SQUserPointer nativeInstancePointer = getNativeInstancePointer(v);

	if (nativeInstancePointer != NULL)
	{
		Image *nativeRes = engine->getResourceManager()->getImage(SquirrelInterface::getResourceNameByReference((int)nativeInstancePointer));
		if (nativeRes != NULL)
			engine->getGraphics()->drawImage(nativeRes);
	}

	return 0;
}

SQInteger Squirrel_Graphics::drawString(HSQUIRRELVM v)
{
	SQUserPointer nativeInstancePointer = getNativeInstancePointer(v, -2);

	if (nativeInstancePointer != NULL)
	{
		const SQInteger argc = sq_gettop(v);

		if (argc == 3 && sq_gettype(v, 3) == OT_STRING)
		{
			const SQChar *str = NULL;
			if (SQ_SUCCEEDED(sq_getstring(v, 3, &str)) && str != NULL)
			{
				McFont *nativeRes = engine->getResourceManager()->getFont(SquirrelInterface::getResourceNameByReference((int)nativeInstancePointer));
				if (nativeRes != NULL)
					engine->getGraphics()->drawString(nativeRes, UString(str));
			}
		}
	}

	return 0;
}

SQInteger Squirrel_Graphics::pushTransform(HSQUIRRELVM v)
{
	engine->getGraphics()->pushTransform();
	return 0;
}

SQInteger Squirrel_Graphics::popTransform(HSQUIRRELVM v)
{
	engine->getGraphics()->popTransform();
	return 0;
}

SQInteger Squirrel_Graphics::translate(HSQUIRRELVM v)
{
	const SQInteger argc = sq_gettop(v);

	SQFloat x, y, z;
	if (SQ_SUCCEEDED(sq_getfloat(v, 2, &x)) && SQ_SUCCEEDED(sq_getfloat(v, 3, &y)))
	{
		if (argc > 3 && SQ_SUCCEEDED(sq_getfloat(v, 4, &z)))
			engine->getGraphics()->translate(x, y, z);
		else
			engine->getGraphics()->translate(x, y);
	}

	return 0;
}

SQInteger Squirrel_Graphics::rotate(HSQUIRRELVM v)
{
	const SQInteger argc = sq_gettop(v);

	SQFloat deg, x, y, z;
	if (SQ_SUCCEEDED(sq_getfloat(v, 2, &deg)))
	{
		if (argc > 4 && SQ_SUCCEEDED(sq_getfloat(v, 3, &x)) && SQ_SUCCEEDED(sq_getfloat(v, 4, &y)) && SQ_SUCCEEDED(sq_getfloat(v, 4, &z)))
			engine->getGraphics()->rotate(deg, x, y, z);
		else
			engine->getGraphics()->rotate(deg);
	}

	return 0;
}

SQInteger Squirrel_Graphics::scale(HSQUIRRELVM v)
{
	const SQInteger argc = sq_gettop(v);

	SQFloat x, y, z;
	if (SQ_SUCCEEDED(sq_getfloat(v, 2, &x)) && SQ_SUCCEEDED(sq_getfloat(v, 3, &y)))
	{
		if (argc > 3 && SQ_SUCCEEDED(sq_getfloat(v, 4, &z)))
			engine->getGraphics()->scale(x, y, z);
		else
			engine->getGraphics()->scale(x, y);
	}

	return 0;
}



SQInteger Squirrel_Image::getWidth(HSQUIRRELVM v)
{
	SQUserPointer nativeInstancePointer = getNativeInstancePointer(v);

	if (nativeInstancePointer != NULL)
	{
		Image *nativeRes = engine->getResourceManager()->getImage(SquirrelInterface::getResourceNameByReference((int)nativeInstancePointer));
		if (nativeRes != NULL)
		{
			sq_pushinteger(v, (SQInteger)nativeRes->getWidth());
			return 1;
		}
	}

	return 0;
}

SQInteger Squirrel_Image::getHeight(HSQUIRRELVM v)
{
	SQUserPointer nativeInstancePointer = getNativeInstancePointer(v);

	if (nativeInstancePointer != NULL)
	{
		Image *nativeRes = engine->getResourceManager()->getImage(SquirrelInterface::getResourceNameByReference((int)nativeInstancePointer));
		if (nativeRes != NULL)
		{
			sq_pushinteger(v, (SQInteger)nativeRes->getHeight());
			return 1;
		}
	}

	return 0;
}



SQInteger Squirrel_Sound::setPositionMS(HSQUIRRELVM v)
{
	SQUserPointer nativeInstancePointer = getNativeInstancePointer(v);

	if (nativeInstancePointer != NULL)
	{
		SQInteger iValue = 0;
		if (SQ_SUCCEEDED(sq_getinteger(v, -1, &iValue)))
		{
			Sound *nativeRes = engine->getResourceManager()->getSound(SquirrelInterface::getResourceNameByReference((int)nativeInstancePointer));
			if (nativeRes != NULL)
				nativeRes->setPositionMS((unsigned long)iValue);
		}
	}

	return 0;
}

SQInteger Squirrel_Sound::setVolume(HSQUIRRELVM v)
{
	SQUserPointer nativeInstancePointer = getNativeInstancePointer(v);

	if (nativeInstancePointer != NULL)
	{
		SQFloat fValue = 0.0f;
		if (SQ_SUCCEEDED(sq_getfloat(v, -1, &fValue)))
		{
			Sound *nativeRes = engine->getResourceManager()->getSound(SquirrelInterface::getResourceNameByReference((int)nativeInstancePointer));
			if (nativeRes != NULL)
				nativeRes->setPositionMS((float)fValue);
		}
	}

	return 0;
}

SQInteger Squirrel_Sound::getPositionMS(HSQUIRRELVM v)
{
	SQUserPointer nativeInstancePointer = getNativeInstancePointer(v);

	if (nativeInstancePointer != NULL)
	{
		Sound *nativeRes = engine->getResourceManager()->getSound(SquirrelInterface::getResourceNameByReference((int)nativeInstancePointer));
		if (nativeRes != NULL)
		{
			sq_pushinteger(v, (SQInteger)nativeRes->getPositionMS());
			return 1;
		}
	}

	return 0;
}

SQInteger Squirrel_Sound::getLengthMS(HSQUIRRELVM v)
{
	SQUserPointer nativeInstancePointer = getNativeInstancePointer(v);

	if (nativeInstancePointer != NULL)
	{
		Sound *nativeRes = engine->getResourceManager()->getSound(SquirrelInterface::getResourceNameByReference((int)nativeInstancePointer));
		if (nativeRes != NULL)
		{
			sq_pushinteger(v, (SQInteger)nativeRes->getLengthMS());
			return 1;
		}
	}

	return 0;
}

SQInteger Squirrel_Sound::isPlaying(HSQUIRRELVM v)
{
	SQUserPointer nativeInstancePointer = getNativeInstancePointer(v);

	if (nativeInstancePointer != NULL)
	{
		Sound *nativeRes = engine->getResourceManager()->getSound(SquirrelInterface::getResourceNameByReference((int)nativeInstancePointer));
		if (nativeRes != NULL)
		{
			sq_pushbool(v, (SQBool)nativeRes->isPlaying());
			return 1;
		}
	}

	return 0;
}



SQInteger Squirrel_Font::getHeight(HSQUIRRELVM v)
{
	SQUserPointer nativeInstancePointer = getNativeInstancePointer(v);

	if (nativeInstancePointer != NULL)
	{
		McFont *nativeRes = engine->getResourceManager()->getFont(SquirrelInterface::getResourceNameByReference((int)nativeInstancePointer));
		if (nativeRes != NULL)
		{
			sq_pushfloat(v, (SQFloat)nativeRes->getHeight());
			return 1;
		}
	}

	return 0;
}

SQInteger Squirrel_Font::getStringWidth(HSQUIRRELVM v)
{
	SQUserPointer nativeInstancePointer = getNativeInstancePointer(v, -2);

	if (nativeInstancePointer != NULL)
	{
		const SQInteger argc = sq_gettop(v);

		if (argc == 3 && sq_gettype(v, 3) == OT_STRING)
		{
			const SQChar *str = NULL;
			if (SQ_SUCCEEDED(sq_getstring(v, 3, &str)) && str != NULL)
			{
				McFont *nativeRes = engine->getResourceManager()->getFont(SquirrelInterface::getResourceNameByReference((int)nativeInstancePointer));
				if (nativeRes != NULL)
				{
					sq_pushfloat(v, (SQFloat)nativeRes->getStringWidth(UString(str)));
					return 1;
				}
			}
		}
	}

	return 0;
}

SQInteger Squirrel_Font::getStringHeight(HSQUIRRELVM v)
{
	SQUserPointer nativeInstancePointer = getNativeInstancePointer(v, -2);

	if (nativeInstancePointer != NULL)
	{
		const SQInteger argc = sq_gettop(v);

		if (argc == 3 && sq_gettype(v, 3) == OT_STRING)
		{
			const SQChar *str = NULL;
			if (SQ_SUCCEEDED(sq_getstring(v, 3, &str)) && str != NULL)
			{
				McFont *nativeRes = engine->getResourceManager()->getFont(SquirrelInterface::getResourceNameByReference((int)nativeInstancePointer));
				if (nativeRes != NULL)
				{
					sq_pushfloat(v, (SQFloat)nativeRes->getStringHeight(UString(str)));
					return 1;
				}
			}
		}
	}

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

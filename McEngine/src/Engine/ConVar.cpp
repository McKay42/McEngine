//================ Copyright (c) 2011, PG, All rights reserved. =================//
//
// Purpose:		console variables
//
// $NoKeywords: $convar
//===============================================================================//

#include "ConVar.h"
#include "Engine.h"

std::vector<ConVar*> &getGlobalConVarArray() // if we don't do it like this, the random order of static initializations will rape us
{
	static std::vector<ConVar*> g_vConVarArray; // (singleton)
	return g_vConVarArray;
}

void addConVar(ConVar *c)
{
	getGlobalConVarArray().push_back(c);
}



void ConVar::init()
{
	m_callbackfunc = NULL;
	m_callbackfuncargs = NULL;
	m_changecallback = NULL;

	m_fValue = 0.0f;
	m_fDefaultValue = 0.0f;

	m_bHasValue = true;
}

void ConVar::init(UString name)
{
	init();

	m_sName = name;

	m_bHasValue = false;
}

void ConVar::init(UString name, ConVarCallback callback)
{
	init();

	m_sName = name;
	m_callbackfunc = callback;

	m_bHasValue = false;
}

void ConVar::init(UString name, ConVarCallbackArgs callbackARGS)
{
	init();

	m_sName = name;
	m_callbackfuncargs = callbackARGS;

	m_bHasValue = false;
}

void ConVar::init(UString name, float defaultValue, UString helpString, ConVarChangeCallback callback)
{
	init();

	m_sName = name;
	m_fDefaultValue = defaultValue;
	setValue(defaultValue);
	m_sHelpString = helpString;
	m_changecallback = callback;
}

void ConVar::init(UString name, UString defaultValue, UString helpString, ConVarChangeCallback callback)
{
	init();

	m_sName = name;
	m_sDefaultValue = defaultValue;
	setValue(defaultValue);
	m_sHelpString = helpString;
	m_changecallback = callback;
}

ConVar::ConVar(UString name)
{
	init(name);
	addConVar(this);
}

ConVar::ConVar(UString name, ConVarCallback callback)
{
	init(name, callback);
	addConVar(this);
}

ConVar::ConVar(UString name, ConVarCallbackArgs callbackARGS)
{
	init(name, callbackARGS);
	addConVar(this);
}

ConVar::ConVar(UString name, float fDefaultValue)
{
	init(name, fDefaultValue, "", NULL);
	addConVar(this);
}

ConVar::ConVar(UString name, float fDefaultValue, ConVarChangeCallback callback)
{
	init(name, fDefaultValue, "", callback);
	addConVar(this);
}

ConVar::ConVar(UString name, float fDefaultValue, UString helpString)
{
	init(name, fDefaultValue, helpString, NULL);
	addConVar(this);
}

ConVar::ConVar(UString name, float fDefaultValue, UString helpString, ConVarChangeCallback callback)
{
	init(name, fDefaultValue, helpString, callback);
	addConVar(this);
}

ConVar::ConVar(UString name, int iDefaultValue)
{
	init(name, (float)iDefaultValue, "", NULL);
	addConVar(this);
}

ConVar::ConVar(UString name, int iDefaultValue, ConVarChangeCallback callback)
{
	init(name, (float)iDefaultValue, "", callback);
	addConVar(this);
}

ConVar::ConVar(UString name, int iDefaultValue, UString helpString)
{
	init(name, (float)iDefaultValue, helpString, NULL);
	addConVar(this);
}

ConVar::ConVar(UString name, int iDefaultValue, UString helpString, ConVarChangeCallback callback)
{
	init(name, (float)iDefaultValue, helpString, callback);
	addConVar(this);
}

ConVar::ConVar(UString name, bool bDefaultValue)
{
	init(name, bDefaultValue ? 1.0f : 0.0f, "", NULL);
	addConVar(this);
}

ConVar::ConVar(UString name, bool bDefaultValue, ConVarChangeCallback callback)
{
	init(name, bDefaultValue ? 1.0f : 0.0f, "", callback);
	addConVar(this);
}

ConVar::ConVar(UString name, bool bDefaultValue, UString helpString)
{
	init(name, bDefaultValue ? 1.0f : 0.0f, helpString, NULL);
	addConVar(this);
}

ConVar::ConVar(UString name, bool bDefaultValue, UString helpString, ConVarChangeCallback callback)
{
	init(name, bDefaultValue ? 1.0f : 0.0f, helpString, callback);
	addConVar(this);
}

ConVar::ConVar(UString name, const char *sDefaultValue)
{
	init(name, UString(sDefaultValue), "", NULL);
	addConVar(this);
}

ConVar::ConVar(UString name, const char *sDefaultValue, UString helpString)
{
	init(name, UString(sDefaultValue), helpString, NULL);
	addConVar(this);
}

ConVar::ConVar(UString name, const char *sDefaultValue, ConVarChangeCallback callback)
{
	init(name, UString(sDefaultValue), "", callback);
	addConVar(this);
}

ConVar::ConVar(UString name, const char *sDefaultValue, UString helpString, ConVarChangeCallback callback)
{
	init(name, UString(sDefaultValue), helpString, callback);
	addConVar(this);
}

void ConVar::exec()
{
	if (m_callbackfunc != NULL)
		m_callbackfunc();
}

void ConVar::execArgs(UString args)
{
	if (m_callbackfuncargs != NULL)
		m_callbackfuncargs(args);
}

void ConVar::setValue(float value)
{
	// backup previous value
	const float oldValue = m_fValue;

	// then set the new value
	m_fValue = value;
	m_sValue = UString::format("%g", m_fValue);

	// possible void callback
	exec();

	// possible change callback
	if (m_changecallback != NULL)
		m_changecallback(UString::format("%g", oldValue), UString::format("%g", value));

	// possible arg callback
	execArgs(UString::format("%g", value));
}

void ConVar::setValue(UString sValue)
{
	// backup previous value
	UString oldValue = sValue;

	// possible void callback
	exec();

	// then set the new value
	m_sValue = sValue;

	if (sValue.length() > 0)
		m_fValue = sValue.toFloat();

	// possible change callback
	if (m_changecallback != NULL)
		m_changecallback(oldValue, sValue);

	// possible arg callback
	execArgs(sValue);
}

void ConVar::setCallback(NativeConVarCallback callback)
{
	m_callbackfunc = callback;
}

void ConVar::setCallback(NativeConVarCallbackArgs callback)
{
	m_callbackfuncargs = callback;
}

void ConVar::setCallback(NativeConVarChangeCallback callback)
{
	m_changecallback = callback;
}

bool ConVar::hasCallbackArgs()
{
	return m_callbackfuncargs != NULL || m_changecallback != NULL;
}



//********************************//
//  ConVarHandler Implementation  //
//********************************//

ConVar _emptyDummyConVar("emptyDummyConVar", 42.0f);

ConVar *ConVarHandler::emptyDummyConVar = &_emptyDummyConVar;

ConVarHandler *convar = new ConVarHandler();

ConVarHandler::ConVarHandler()
{
}

ConVarHandler::~ConVarHandler()
{
	convar = NULL;
}

std::vector<ConVar*> ConVarHandler::getConVarArray()
{
	std::vector<ConVar*> convars;
	for (int i=0; i<getGlobalConVarArray().size(); i++)
	{
		convars.push_back((getGlobalConVarArray()[i]));
	}
	return convars;
}

int ConVarHandler::getNumConVars()
{
	return getGlobalConVarArray().size();
}

ConVar *ConVarHandler::getConVarByName(UString name, bool warnIfNotFound)
{
	for (int i=0; i<getGlobalConVarArray().size(); i++)
	{
		if (getGlobalConVarArray()[i]->getName() == name)
			return (getGlobalConVarArray()[i]);
	}

	if (warnIfNotFound)
	{
		UString errormsg = UString("ENGINE: ConVar \"");
		errormsg.append(name);
		errormsg.append("\" does not exist...\n");
		debugLog(errormsg.toUtf8());
		engine->showMessageWarning("Engine Error", errormsg);
	}

	if (!warnIfNotFound)
		return NULL;
	else
		return emptyDummyConVar;
}

std::vector<ConVar*> ConVarHandler::getConVarByLetter(UString letters)
{
	std::vector<ConVar*> temp;

	if (letters.length() < 1)
		return temp;

	for (int i=0; i<getGlobalConVarArray().size(); i++)
	{
		if (getGlobalConVarArray()[i]->getName().find(letters, 0, letters.length()) == 0)
			temp.push_back((getGlobalConVarArray()[i]));
	}
	return temp;
}



//*****************************//
//	ConVarHandler ConCommands  //
//*****************************//

void _find(UString args)
{
	if (args.length() < 1)
	{
		debugLog("Usage:  find <string>");
		return;
	}

	std::vector<ConVar*> temp;

	std::vector<ConVar*> convars = convar->getConVarArray();
	for (int i=0; i<convars.size(); i++)
	{
		UString curcvar = convars[i]->getName();
		if (curcvar.find(args, 0, curcvar.length()) != -1)
			temp.push_back(convars[i]);
	}

	if (temp.size() < 1)
	{
		UString thelog = "No commands found containing \"";
		thelog.append(args);
		thelog.append("\".\n");
		debugLog("%s", thelog.toUtf8());
		return;
	}

	debugLog("----------------------------------------------\n");
	UString thelog = "[ find : ";
	thelog.append(args);
	thelog.append(" ]\n");
	debugLog("%s", thelog.toUtf8());

	for (int i=0; i<temp.size(); i++)
	{
		UString tstring = temp[i]->getName();
		tstring.append("\n");
		debugLog("%s", tstring.toUtf8());
	}

	debugLog("----------------------------------------------\n");
}

void _help(UString args)
{
	if (args.length() < 1)
	{
		debugLog("Usage:  help <cvarname>");
		return;
	}

	std::vector<ConVar*> temp = convar->getConVarByLetter(args);

	if (temp.size() < 1)
	{
		UString thelog = "ConVar \"";
		thelog.append(args);
		thelog.append("\" does not exist.\n");
		debugLog("%s", thelog.toUtf8());
		return;
	}

	if (temp[0]->getHelpstring().length() < 1)
	{
		UString thelog = "ConVar \"";
		thelog.append(args);
		thelog.append("\" does not have a helpstring.\n");
		debugLog("%s", thelog.toUtf8());
		return;
	}

	UString thelog = temp[0]->getName();
	thelog.append(" : ");
	thelog.append(temp[0]->getHelpstring());

	debugLog("%s", thelog.toUtf8());
}

void _listcommands(void)
{
	std::vector<ConVar*> temp;

	debugLog("----------------------------------------------\n");

	std::vector<ConVar*> convars = convar->getConVarArray();
	for (int i=0; i<convars.size(); i++)
	{
		UString tstring = convars[i]->getName();
		if (convars[i]->getHelpstring().length() > 0)
		{
			tstring.append(" : ");
			tstring.append(convars[i]->getHelpstring());
		}
		tstring.append("\n");
		debugLog("%s", tstring.toUtf8());
	}

	debugLog("----------------------------------------------\n");
}

ConVar _find_("find", _find);
ConVar _help_("help", _help);
ConVar _listcommands_("listcommands", _listcommands);

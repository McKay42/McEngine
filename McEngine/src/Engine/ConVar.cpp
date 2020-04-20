//================ Copyright (c) 2011, PG, All rights reserved. =================//
//
// Purpose:		console variables
//
// $NoKeywords: $convar
//===============================================================================//

#include "ConVar.h"
#include "Engine.h"



static std::vector<ConVar*> &_getGlobalConVarArray()
{
	static std::vector<ConVar*> g_vConVars; // (singleton)
	return g_vConVars;
}

static std::unordered_map<std::string, ConVar*> &_getGlobalConVarMap()
{
	static std::unordered_map<std::string, ConVar*> g_vConVarMap; // (singleton)
	return g_vConVarMap;
}

static void _addConVar(ConVar *c)
{
	if (_getGlobalConVarArray().size() < 1)
		_getGlobalConVarArray().reserve(1024);

	_getGlobalConVarArray().push_back(c);
	_getGlobalConVarMap()[std::string(c->getName().toUtf8(), c->getName().length())] = c;
}

static ConVar *_getConVar(const UString &name)
{
	const auto result = _getGlobalConVarMap().find(std::string(name.toUtf8(), name.length()));
	if (result != _getGlobalConVarMap().end())
		return result->second;
	else
		return NULL;
}



void ConVar::init()
{
	m_callbackfunc = NULL;
	m_callbackfuncargs = NULL;
	m_changecallback = NULL;

	m_fValue = 0.0f;
	m_fDefaultValue = 0.0f;

	m_bHasValue = true;
	m_type = CONVAR_TYPE::CONVAR_TYPE_FLOAT;
}

void ConVar::init(UString name)
{
	init();

	m_sName = name;

	m_bHasValue = false;
	m_type = CONVAR_TYPE::CONVAR_TYPE_STRING;
}

void ConVar::init(UString name, ConVarCallback callback)
{
	init();

	m_sName = name;
	m_callbackfunc = callback;

	m_bHasValue = false;
	m_type = CONVAR_TYPE::CONVAR_TYPE_STRING;
}

void ConVar::init(UString name, ConVarCallbackArgs callbackARGS)
{
	init();

	m_sName = name;
	m_callbackfuncargs = callbackARGS;

	m_bHasValue = false;
	m_type = CONVAR_TYPE::CONVAR_TYPE_STRING;
}

void ConVar::init(UString name, float defaultValue, UString helpString, ConVarChangeCallback callback)
{
	init();

	m_type = CONVAR_TYPE::CONVAR_TYPE_FLOAT;
	m_sName = name;
	setDefaultFloat(defaultValue);
	{
		setValue(defaultValue);
	}
	m_sHelpString = helpString;
	m_changecallback = callback;
}

void ConVar::init(UString name, UString defaultValue, UString helpString, ConVarChangeCallback callback)
{
	init();

	m_type = CONVAR_TYPE::CONVAR_TYPE_STRING;
	m_sName = name;
	setDefaultString(defaultValue);
	{
		setValue(defaultValue);
	}
	m_sHelpString = helpString;
	m_changecallback = callback;
}

ConVar::ConVar(UString name)
{
	init(name);
	_addConVar(this);
}

ConVar::ConVar(UString name, ConVarCallback callback)
{
	init(name, callback);
	_addConVar(this);
}

ConVar::ConVar(UString name, ConVarCallbackArgs callbackARGS)
{
	init(name, callbackARGS);
	_addConVar(this);
}

ConVar::ConVar(UString name, float fDefaultValue)
{
	init(name, fDefaultValue, "", NULL);
	_addConVar(this);
}

ConVar::ConVar(UString name, float fDefaultValue, ConVarChangeCallback callback)
{
	init(name, fDefaultValue, "", callback);
	_addConVar(this);
}

ConVar::ConVar(UString name, float fDefaultValue, UString helpString)
{
	init(name, fDefaultValue, helpString, NULL);
	_addConVar(this);
}

ConVar::ConVar(UString name, float fDefaultValue, UString helpString, ConVarChangeCallback callback)
{
	init(name, fDefaultValue, helpString, callback);
	_addConVar(this);
}

ConVar::ConVar(UString name, int iDefaultValue)
{
	init(name, (float)iDefaultValue, "", NULL);
	m_type = CONVAR_TYPE::CONVAR_TYPE_INT;
	_addConVar(this);
}

ConVar::ConVar(UString name, int iDefaultValue, ConVarChangeCallback callback)
{
	init(name, (float)iDefaultValue, "", callback);
	m_type = CONVAR_TYPE::CONVAR_TYPE_INT;
	_addConVar(this);
}

ConVar::ConVar(UString name, int iDefaultValue, UString helpString)
{
	init(name, (float)iDefaultValue, helpString, NULL);
	m_type = CONVAR_TYPE::CONVAR_TYPE_INT;
	_addConVar(this);
}

ConVar::ConVar(UString name, int iDefaultValue, UString helpString, ConVarChangeCallback callback)
{
	init(name, (float)iDefaultValue, helpString, callback);
	m_type = CONVAR_TYPE::CONVAR_TYPE_INT;
	_addConVar(this);
}

ConVar::ConVar(UString name, bool bDefaultValue)
{
	init(name, bDefaultValue ? 1.0f : 0.0f, "", NULL);
	m_type = CONVAR_TYPE::CONVAR_TYPE_BOOL;
	_addConVar(this);
}

ConVar::ConVar(UString name, bool bDefaultValue, ConVarChangeCallback callback)
{
	init(name, bDefaultValue ? 1.0f : 0.0f, "", callback);
	m_type = CONVAR_TYPE::CONVAR_TYPE_BOOL;
	_addConVar(this);
}

ConVar::ConVar(UString name, bool bDefaultValue, UString helpString)
{
	init(name, bDefaultValue ? 1.0f : 0.0f, helpString, NULL);
	m_type = CONVAR_TYPE::CONVAR_TYPE_BOOL;
	_addConVar(this);
}

ConVar::ConVar(UString name, bool bDefaultValue, UString helpString, ConVarChangeCallback callback)
{
	init(name, bDefaultValue ? 1.0f : 0.0f, helpString, callback);
	m_type = CONVAR_TYPE::CONVAR_TYPE_BOOL;
	_addConVar(this);
}

ConVar::ConVar(UString name, const char *sDefaultValue)
{
	init(name, UString(sDefaultValue), "", NULL);
	_addConVar(this);
}

ConVar::ConVar(UString name, const char *sDefaultValue, UString helpString)
{
	init(name, UString(sDefaultValue), helpString, NULL);
	_addConVar(this);
}

ConVar::ConVar(UString name, const char *sDefaultValue, ConVarChangeCallback callback)
{
	init(name, UString(sDefaultValue), "", callback);
	_addConVar(this);
}

ConVar::ConVar(UString name, const char *sDefaultValue, UString helpString, ConVarChangeCallback callback)
{
	init(name, UString(sDefaultValue), helpString, callback);
	_addConVar(this);
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

void ConVar::setDefaultFloat(float defaultValue)
{
	m_fDefaultValue = defaultValue;
	m_sDefaultValue = UString::format("%g", defaultValue);
}

void ConVar::setDefaultString(UString defaultValue)
{
	m_sDefaultValue = defaultValue;
}

void ConVar::setValue(float value)
{
	// backup previous value
	const float oldValue = m_fValue.load();

	// then set the new value
	const UString newStringValue = UString::format("%g", value);
	{
		m_fValue = value;
		m_sValue = newStringValue;
	}

	// handle callbacks
	{
		// possible void callback
		exec();

		// possible change callback
		if (m_changecallback != NULL)
			m_changecallback(UString::format("%g", oldValue), newStringValue);

		// possible arg callback
		execArgs(newStringValue);
	}
}

void ConVar::setValue(UString sValue)
{
	// backup previous value
	const UString oldValue = m_sValue;

	// then set the new value
	{
		m_sValue = sValue;

		if (sValue.length() > 0)
			m_fValue = sValue.toFloat();
	}

	// handle callbacks
	{
		// possible void callback
		exec();

		// possible change callback
		if (m_changecallback != NULL)
			m_changecallback(oldValue, sValue);

		// possible arg callback
		execArgs(sValue);
	}
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

void ConVar::setHelpString(UString helpString)
{
	m_sHelpString = helpString;
}

bool ConVar::hasCallbackArgs() const
{
	return (m_callbackfuncargs || m_changecallback);
}



//********************************//
//  ConVarHandler Implementation  //
//********************************//

ConVar _emptyDummyConVar("emptyDummyConVar", 42.0f);

ConVar *ConVarHandler::emptyDummyConVar = &_emptyDummyConVar;

ConVarHandler *convar = new ConVarHandler();

ConVarHandler::ConVarHandler()
{
	convar = this;
}

ConVarHandler::~ConVarHandler()
{
	convar = NULL;
}

const std::vector<ConVar*> &ConVarHandler::getConVarArray() const
{
	return _getGlobalConVarArray();
}

int ConVarHandler::getNumConVars() const
{
	return _getGlobalConVarArray().size();
}

ConVar *ConVarHandler::getConVarByName(UString name, bool warnIfNotFound) const
{
	ConVar *found = _getConVar(name);
	if (found != NULL)
		return found;

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

std::vector<ConVar*> ConVarHandler::getConVarByLetter(UString letters) const
{
	std::vector<ConVar*> matchingConVars;

	if (letters.length() < 1)
		return matchingConVars;

	const std::vector<ConVar*> &convars = getConVarArray();

	for (int i=0; i<convars.size(); i++)
	{
		if (convars[i]->getName().find(letters, 0, letters.length()) == 0)
			matchingConVars.push_back(convars[i]);
	}

	return matchingConVars;
}



//*****************************//
//	ConVarHandler ConCommands  //
//*****************************//

static void _find(UString args)
{
	if (args.length() < 1)
	{
		debugLog("Usage:  find <string>");
		return;
	}

	const std::vector<ConVar*> &convars = convar->getConVarArray();

	std::vector<ConVar*> matchingConVars;
	for (int i=0; i<convars.size(); i++)
	{
		UString curcvar = convars[i]->getName();
		if (curcvar.find(args, 0, curcvar.length()) != -1)
			matchingConVars.push_back(convars[i]);
	}

	if (matchingConVars.size() < 1)
	{
		UString thelog = "No commands found containing \"";
		thelog.append(args);
		thelog.append("\".\n");
		debugLog("%s", thelog.toUtf8());
		return;
	}

	debugLog("----------------------------------------------\n");
	{
		UString thelog = "[ find : ";
		thelog.append(args);
		thelog.append(" ]\n");
		debugLog("%s", thelog.toUtf8());

		for (int i=0; i<matchingConVars.size(); i++)
		{
			UString tstring = matchingConVars[i]->getName();
			tstring.append("\n");
			debugLog("%s", tstring.toUtf8());
		}
	}
	debugLog("----------------------------------------------\n");
}

static void _help(UString args)
{
	if (args.length() < 1)
	{
		debugLog("Usage:  help <cvarname>");
		return;
	}

	const std::vector<ConVar*> matches = convar->getConVarByLetter(args);

	if (matches.size() < 1)
	{
		UString thelog = "ConVar \"";
		thelog.append(args);
		thelog.append("\" does not exist.\n");
		debugLog("%s", thelog.toUtf8());
		return;
	}

	// use closest match
	int index = 0;
	for (int i=0; i<matches.size(); i++)
	{
		if (matches[i]->getName() == args)
		{
			index = i;
			break;
		}
	}

	const ConVar *match = matches[index];

	if (match->getHelpstring().length() < 1)
	{
		UString thelog = "ConVar \"";
		thelog.append(match->getName());
		thelog.append("\" does not have a helpstring.\n");
		debugLog("%s", thelog.toUtf8());
		return;
	}

	UString thelog = match->getName();
	{
		if (match->hasValue())
			thelog.append(UString::format(" = %s ( def. \"%s\" )", match->getString().toUtf8(), match->getDefaultString().toUtf8()));

		thelog.append(" - ");
		thelog.append(match->getHelpstring());
	}

	debugLog("%s", thelog.toUtf8());
}

static void _listcommands(void)
{
	debugLog("----------------------------------------------\n");
	{
		const std::vector<ConVar*> &convars = convar->getConVarArray();
		for (int i=0; i<convars.size(); i++)
		{
			UString tstring = convars[i]->getName();

			if (convars[i]->hasValue())
			{
				tstring.append(UString::format(" = %s ( def. \"%s\" )", convars[i]->getString().toUtf8(), convars[i]->getDefaultString().toUtf8()));
			}

			if (convars[i]->getHelpstring().length() > 0)
			{
				tstring.append(" - ");
				tstring.append(convars[i]->getHelpstring());
			}

			tstring.append("\n");

			debugLog("%s", tstring.toUtf8());
		}
	}
	debugLog("----------------------------------------------\n");
}

ConVar _find_("find", _find);
ConVar _help_("help", _help);
ConVar _listcommands_("listcommands", _listcommands);

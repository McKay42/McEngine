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
	_getGlobalConVarMap()[std::string(c->getName().toUtf8(), c->getName().lengthUtf8())] = c;
}

static ConVar *_getConVar(const UString &name)
{
	const auto result = _getGlobalConVarMap().find(std::string(name.toUtf8(), name.lengthUtf8()));
	if (result != _getGlobalConVarMap().end())
		return result->second;
	else
		return NULL;
}



UString ConVar::typeToString(CONVAR_TYPE type)
{
	switch (type)
	{
	case ConVar::CONVAR_TYPE::CONVAR_TYPE_BOOL:
		return "bool";
	case ConVar::CONVAR_TYPE::CONVAR_TYPE_INT:
		return "int";
	case ConVar::CONVAR_TYPE::CONVAR_TYPE_FLOAT:
		return "float";
	case ConVar::CONVAR_TYPE::CONVAR_TYPE_STRING:
		return "string";
	}

	return "";
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

void ConVar::init(UString &name)
{
	init();

	m_sName = name;

	m_bHasValue = false;
	m_type = CONVAR_TYPE::CONVAR_TYPE_STRING;
}

void ConVar::init(UString &name, ConVarCallback callback)
{
	init();

	m_sName = name;
	m_callbackfunc = callback;

	m_bHasValue = false;
	m_type = CONVAR_TYPE::CONVAR_TYPE_STRING;
}

void ConVar::init(UString &name, UString helpString, ConVarCallback callback)
{
	init(name, callback);

	m_sHelpString = helpString;
}

void ConVar::init(UString &name, ConVarCallbackArgs callbackARGS)
{
	init();

	m_sName = name;
	m_callbackfuncargs = callbackARGS;

	m_bHasValue = false;
	m_type = CONVAR_TYPE::CONVAR_TYPE_STRING;
}

void ConVar::init(UString &name, UString helpString, ConVarCallbackArgs callbackARGS)
{
	init(name, callbackARGS);

	m_sHelpString = helpString;
}

void ConVar::init(UString &name, float defaultValue, UString helpString, ConVarChangeCallback callback)
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

void ConVar::init(UString &name, UString defaultValue, UString helpString, ConVarChangeCallback callback)
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

ConVar::ConVar(UString name, const char *helpString, ConVarCallback callback)
{
	init(name, helpString, callback);
	_addConVar(this);
}

ConVar::ConVar(UString name, ConVarCallbackArgs callbackARGS)
{
	init(name, callbackARGS);
	_addConVar(this);
}

ConVar::ConVar(UString name, const char *helpString, ConVarCallbackArgs callbackARGS)
{
	init(name, helpString, callbackARGS);
	_addConVar(this);
}

ConVar::ConVar(UString name, float fDefaultValue)
{
	init(name, fDefaultValue, UString(""), NULL);
	_addConVar(this);
}

ConVar::ConVar(UString name, float fDefaultValue, ConVarChangeCallback callback)
{
	init(name, fDefaultValue, UString(""), callback);
	_addConVar(this);
}

ConVar::ConVar(UString name, float fDefaultValue, const char *helpString)
{
	init(name, fDefaultValue, UString(helpString), NULL);
	_addConVar(this);
}

ConVar::ConVar(UString name, float fDefaultValue, const char *helpString, ConVarChangeCallback callback)
{
	init(name, fDefaultValue, UString(helpString), callback);
	_addConVar(this);
}

ConVar::ConVar(UString name, int iDefaultValue)
{
	init(name, (float)iDefaultValue, "", NULL);
	{
		m_type = CONVAR_TYPE::CONVAR_TYPE_INT;
	}
	_addConVar(this);
}

ConVar::ConVar(UString name, int iDefaultValue, ConVarChangeCallback callback)
{
	init(name, (float)iDefaultValue, "", callback);
	{
		m_type = CONVAR_TYPE::CONVAR_TYPE_INT;
	}
	_addConVar(this);
}

ConVar::ConVar(UString name, int iDefaultValue, const char *helpString)
{
	init(name, (float)iDefaultValue, UString(helpString), NULL);
	{
		m_type = CONVAR_TYPE::CONVAR_TYPE_INT;
	}
	_addConVar(this);
}

ConVar::ConVar(UString name, int iDefaultValue, const char *helpString, ConVarChangeCallback callback)
{
	init(name, (float)iDefaultValue, UString(helpString), callback);
	{
		m_type = CONVAR_TYPE::CONVAR_TYPE_INT;
	}
	_addConVar(this);
}

ConVar::ConVar(UString name, bool bDefaultValue)
{
	init(name, bDefaultValue ? 1.0f : 0.0f, "", NULL);
	{
		m_type = CONVAR_TYPE::CONVAR_TYPE_BOOL;
	}
	_addConVar(this);
}

ConVar::ConVar(UString name, bool bDefaultValue, ConVarChangeCallback callback)
{
	init(name, bDefaultValue ? 1.0f : 0.0f, "", callback);
	{
		m_type = CONVAR_TYPE::CONVAR_TYPE_BOOL;
	}
	_addConVar(this);
}

ConVar::ConVar(UString name, bool bDefaultValue, const char *helpString)
{
	init(name, bDefaultValue ? 1.0f : 0.0f, UString(helpString), NULL);
	{
		m_type = CONVAR_TYPE::CONVAR_TYPE_BOOL;
	}
	_addConVar(this);
}

ConVar::ConVar(UString name, bool bDefaultValue, const char *helpString, ConVarChangeCallback callback)
{
	init(name, bDefaultValue ? 1.0f : 0.0f, UString(helpString), callback);
	{
		m_type = CONVAR_TYPE::CONVAR_TYPE_BOOL;
	}
	_addConVar(this);
}

ConVar::ConVar(UString name, const char *sDefaultValue)
{
	init(name, UString(sDefaultValue), UString(""), NULL);
	_addConVar(this);
}

ConVar::ConVar(UString name, const char *sDefaultValue, const char *helpString)
{
	init(name, UString(sDefaultValue), UString(helpString), NULL);
	_addConVar(this);
}

ConVar::ConVar(UString name, const char *sDefaultValue, ConVarChangeCallback callback)
{
	init(name, UString(sDefaultValue), UString(""), callback);
	_addConVar(this);
}

ConVar::ConVar(UString name, const char *sDefaultValue, const char *helpString, ConVarChangeCallback callback)
{
	init(name, UString(sDefaultValue), UString(helpString), callback);
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
	// TODO: make this less unsafe in multithreaded environments (for float convars at least)

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

ConVar _emptyDummyConVar("emptyDummyConVar", 42.0f, "this placeholder convar is returned by convar->getConVarByName() if no matching convar is found");

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
		return &_emptyDummyConVar;
}

std::vector<ConVar*> ConVarHandler::getConVarByLetter(UString letters) const
{
	std::unordered_set<std::string> matchingConVarNames;
	std::vector<ConVar*> matchingConVars;
	{
		if (letters.length() < 1)
			return matchingConVars;

		const std::vector<ConVar*> &convars = getConVarArray();

		// first try matching exactly
		for (size_t i=0; i<convars.size(); i++)
		{
			if (convars[i]->getName().find(letters, 0, letters.length()) == 0)
			{
				if (letters.length() > 1)
					matchingConVarNames.insert(std::string(convars[i]->getName().toUtf8(), convars[i]->getName().lengthUtf8()));

				matchingConVars.push_back(convars[i]);
			}
		}

		// then try matching substrings
		if (letters.length() > 1)
		{
			for (size_t i=0; i<convars.size(); i++)
			{
				if (convars[i]->getName().find(letters) != -1)
				{
					std::string stdName(convars[i]->getName().toUtf8(), convars[i]->getName().lengthUtf8());
					if (matchingConVarNames.find(stdName) == matchingConVarNames.end())
					{
						matchingConVarNames.insert(stdName);
						matchingConVars.push_back(convars[i]);
					}
				}
			}
		}

		// (results should be displayed in vector order)
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
	for (size_t i=0; i<convars.size(); i++)
	{
		UString curcvar = convars[i]->getName();
		if (curcvar.find(args, 0, curcvar.length()) != -1)
			matchingConVars.push_back(convars[i]);
	}

	if (matchingConVars.size() > 0)
	{
		struct CONVAR_SORT_COMPARATOR
		{
			bool operator () (ConVar const *var1, ConVar const *var2)
			{
				return (var1->getName() < var2->getName());
			}
		};
		std::sort(matchingConVars.begin(), matchingConVars.end(), CONVAR_SORT_COMPARATOR());
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

		for (size_t i=0; i<matchingConVars.size(); i++)
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
	args = args.trim();

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
	size_t index = 0;
	for (size_t i=0; i<matches.size(); i++)
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
		{
			thelog.append(UString::format(" = %s ( def. \"%s\" , ", match->getString().toUtf8(), match->getDefaultString().toUtf8()));
			thelog.append(ConVar::typeToString(match->getType()));
			thelog.append(" )");
		}

		thelog.append(" - ");
		thelog.append(match->getHelpstring());
	}
	debugLog("%s", thelog.toUtf8());
}

static void _listcommands(void)
{
	debugLog("----------------------------------------------\n");
	{
		std::vector<ConVar*> convars = convar->getConVarArray();
		struct CONVAR_SORT_COMPARATOR
		{
			bool operator () (ConVar const *var1, ConVar const *var2)
			{
				return (var1->getName() < var2->getName());
			}
		};
		std::sort(convars.begin(), convars.end(), CONVAR_SORT_COMPARATOR());

		for (size_t i=0; i<convars.size(); i++)
		{
			const ConVar *var = convars[i];

			UString tstring = var->getName();
			{
				if (var->hasValue())
				{
					tstring.append(UString::format(" = %s ( def. \"%s\" , ", var->getString().toUtf8(), var->getDefaultString().toUtf8()));
					tstring.append(ConVar::typeToString(var->getType()));
					tstring.append(" )");
				}

				if (var->getHelpstring().length() > 0)
				{
					tstring.append(" - ");
					tstring.append(var->getHelpstring());
				}

				tstring.append("\n");
			}
			debugLog("%s", tstring.toUtf8());
		}
	}
	debugLog("----------------------------------------------\n");
}

ConVar _find_("find", _find);
ConVar _help_("help", _help);
ConVar _listcommands_("listcommands", _listcommands);

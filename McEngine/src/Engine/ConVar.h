//================ Copyright (c) 2011, PG, All rights reserved. =================//
//
// Purpose:		console variables
//
// $NoKeywords: $convar
//===============================================================================//

#ifndef CONVAR_H
#define CONVAR_H

#include "cbase.h"

class ConVars
{
public:
	// shared engine-wide cross-game convars for convenience access (for convars which don't fit anywhere else)
	static ConVar sv_cheats;
};

#define FCVAR_NONE				0		// the default, no flags at all
#define FCVAR_UNREGISTERED		(1<<0)	// not added to g_vConVars list (not settable/gettable via console/help), not visible in find/listcommands results etc.
#define FCVAR_DEVELOPMENTONLY	(1<<1)	// hidden in released products (like FCVAR_HIDDEN, but flag gets compiled out if ALLOW_DEVELOPMENT_CONVARS is defined)
#define FCVAR_HARDCODED			(1<<2)	// if this is set then the value can't and shouldn't ever be changed
#define FCVAR_HIDDEN			(1<<3)	// not visible in find/listcommands results etc., but is settable/gettable via console/help
#define FCVAR_CHEAT				(1<<4)	// always return default value and ignore callbacks unless sv_cheats is enabled (default value is changeable via setDefault*())

class ConVar
{
public:
	enum class CONVAR_TYPE
	{
		CONVAR_TYPE_BOOL,
		CONVAR_TYPE_INT,
		CONVAR_TYPE_FLOAT,
		CONVAR_TYPE_STRING
	};

	// raw callbacks
	typedef void (*ConVarCallback)(void);
	typedef void (*ConVarChangeCallback)(UString oldValue, UString newValue);
	typedef void (*ConVarCallbackArgs)(UString args);

	// delegate callbacks
	typedef fastdelegate::FastDelegate0<> NativeConVarCallback;
	typedef fastdelegate::FastDelegate1<UString> NativeConVarCallbackArgs;
	typedef fastdelegate::FastDelegate2<UString, UString> NativeConVarChangeCallback;

public:
	static UString typeToString(CONVAR_TYPE type);

public:
	explicit ConVar(UString name);

	explicit ConVar(UString name, int flags, ConVarCallback callback);
	explicit ConVar(UString name, int flags, const char *helpString, ConVarCallback callback);

	explicit ConVar(UString name, int flags, ConVarCallbackArgs callbackARGS);
	explicit ConVar(UString name, int flags, const char *helpString, ConVarCallbackArgs callbackARGS);

	explicit ConVar(UString name, float defaultValue, int flags);
	explicit ConVar(UString name, float defaultValue, int flags, ConVarChangeCallback callback);
	explicit ConVar(UString name, float defaultValue, int flags, const char *helpString);
	explicit ConVar(UString name, float defaultValue, int flags, const char *helpString, ConVarChangeCallback callback);

	explicit ConVar(UString name, int defaultValue, int flags);
	explicit ConVar(UString name, int defaultValue, int flags, ConVarChangeCallback callback);
	explicit ConVar(UString name, int defaultValue, int flags, const char *helpString);
	explicit ConVar(UString name, int defaultValue, int flags, const char *helpString, ConVarChangeCallback callback);

	explicit ConVar(UString name, bool defaultValue, int flags);
	explicit ConVar(UString name, bool defaultValue, int flags, ConVarChangeCallback callback);
	explicit ConVar(UString name, bool defaultValue, int flags, const char *helpString);
	explicit ConVar(UString name, bool defaultValue, int flags, const char *helpString, ConVarChangeCallback callback);

	explicit ConVar(UString name, const char *defaultValue, int flags);
	explicit ConVar(UString name, const char *defaultValue, int flags, const char *helpString);
	explicit ConVar(UString name, const char *defaultValue, int flags, ConVarChangeCallback callback);
	explicit ConVar(UString name, const char *defaultValue, int flags, const char *helpString, ConVarChangeCallback callback);

	// callbacks
	void exec();
	void execArgs(UString args);

	// set
	void setDefaultFloat(float defaultValue);
	void setDefaultString(UString defaultValue);

	void setValue(float value);
	void setValue(UString sValue);

	void setCallback(NativeConVarCallback callback);
	void setCallback(NativeConVarCallbackArgs callback);
	void setCallback(NativeConVarChangeCallback callback);

	void setHelpString(UString helpString);

	// get
	inline float getDefaultFloat() const {return m_fDefaultValue.load();}
	inline const UString &getDefaultString() const {return m_sDefaultValue;}

	inline bool getBool() const				{return		((isFlagSet(FCVAR_CHEAT) && !ConVars::sv_cheats.getBool() ? m_fDefaultValue.load() : m_fValue.load()) > 0);}
	inline float getFloat() const			{return		 (isFlagSet(FCVAR_CHEAT) && !ConVars::sv_cheats.getBool() ? m_fDefaultValue.load() : m_fValue.load());}
	inline int getInt() const				{return (int)(isFlagSet(FCVAR_CHEAT) && !ConVars::sv_cheats.getBool() ? m_fDefaultValue.load() : m_fValue.load());}
	inline const UString &getString() const	{return		 (isFlagSet(FCVAR_CHEAT) && !ConVars::sv_cheats.getBool() ? m_sDefaultValue : m_sValue);}

	inline const UString &getHelpstring() const {return m_sHelpString;}
	inline const UString &getName() const {return m_sName;}
	inline CONVAR_TYPE getType() const {return m_type;}
	inline int getFlags() const {return m_iFlags;}

	inline bool hasValue() const {return m_bHasValue;}
	inline bool hasCallbackArgs() const {return (m_callbackfuncargs || m_changecallback);}
	inline bool isFlagSet(int flag) const {return (bool)(m_iFlags & flag);}

private:
	void init(int flags);
	void init(UString &name, int flags);
	void init(UString &name, int flags, ConVarCallback callback);
	void init(UString &name, int flags, UString helpString, ConVarCallback callback);
	void init(UString &name, int flags, ConVarCallbackArgs callbackARGS);
	void init(UString &name, int flags, UString helpString, ConVarCallbackArgs callbackARGS);
	void init(UString &name, float defaultValue, int flags, UString helpString, ConVarChangeCallback callback);
	void init(UString &name, UString defaultValue, int flags, UString helpString, ConVarChangeCallback callback);

	void setDefaultFloatInt(float defaultValue);
	void setDefaultStringInt(UString defaultValue);

	void setValueInt(float value);
	void setValueInt(UString sValue);

private:
	bool m_bHasValue;
	CONVAR_TYPE m_type;
	int m_iFlags;

	UString m_sName;
	UString m_sHelpString;

	std::atomic<float> m_fValue;
	std::atomic<float> m_fDefaultValue;

	UString m_sValue;
	UString m_sDefaultValue;

	NativeConVarCallback m_callbackfunc;
	NativeConVarCallbackArgs m_callbackfuncargs;
	NativeConVarChangeCallback m_changecallback;
};



//*******************//
//  Searching/Lists  //
//*******************//

class ConVarHandler
{
public:
	static UString flagsToString(int flags);

public:
	ConVarHandler();
	~ConVarHandler();

	const std::vector<ConVar*> &getConVarArray() const;
	int getNumConVars() const;

	ConVar *getConVarByName(UString name, bool warnIfNotFound = true) const;
	std::vector<ConVar*> getConVarByLetter(UString letters) const;
};

extern ConVarHandler *convar;

#endif

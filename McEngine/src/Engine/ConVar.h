//================ Copyright (c) 2011, PG, All rights reserved. =================//
//
// Purpose:		console variables
//
// $NoKeywords: $convar
//===============================================================================//

#ifndef CONVAR_H
#define CONVAR_H

#include "cbase.h"

// raw callbacks
typedef void (*ConVarCallback)(void);
typedef void (*ConVarChangeCallback)(UString oldValue, UString newValue);
typedef void (*ConVarCallbackArgs)(UString args);

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

public:
	// delegate callbacks
	typedef fastdelegate::FastDelegate0<> NativeConVarCallback;
	typedef fastdelegate::FastDelegate1<UString> NativeConVarCallbackArgs;
	typedef fastdelegate::FastDelegate2<UString, UString> NativeConVarChangeCallback;

	ConVar(UString name);

	ConVar(UString name, ConVarCallback callback);
	ConVar(UString name, ConVarCallbackArgs callbackARGS);

	ConVar(UString name, float defaultValue);
	ConVar(UString name, float defaultValue, ConVarChangeCallback callback);
	ConVar(UString name, float defaultValue, UString helpString);
	ConVar(UString name, float defaultValue, UString helpString, ConVarChangeCallback callback);

	ConVar(UString name, int defaultValue);
	ConVar(UString name, int defaultValue, ConVarChangeCallback callback);
	ConVar(UString name, int defaultValue, UString helpString);
	ConVar(UString name, int defaultValue, UString helpString, ConVarChangeCallback callback);

	ConVar(UString name, bool defaultValue);
	ConVar(UString name, bool defaultValue, ConVarChangeCallback callback);
	ConVar(UString name, bool defaultValue, UString helpString);
	ConVar(UString name, bool defaultValue, UString helpString, ConVarChangeCallback callback);

	ConVar(UString name, const char *defaultValue);
	ConVar(UString name, const char *defaultValue, UString helpString);
	ConVar(UString name, const char *defaultValue, ConVarChangeCallback callback);
	ConVar(UString name, const char *defaultValue, UString helpString, ConVarChangeCallback callback);

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
	inline const UString getDefaultString() const {return m_sDefaultValue;}

	inline bool getBool() const {return (m_fValue.load() > 0);}
	inline float getFloat() const {return m_fValue.load();}
	inline int getInt() const {return (int)(m_fValue.load());}
	inline const UString getString() const {return m_sValue;}

	inline const UString getHelpstring() const {return m_sHelpString;}
	inline const UString getName() const {return m_sName;}
	inline CONVAR_TYPE getType() const {return m_type;}

	inline bool hasValue() const {return m_bHasValue.load();}
	bool hasCallbackArgs() const;

private:
	void init();
	void init(UString name);
	void init(UString name, ConVarCallback callback);
	void init(UString name, ConVarCallbackArgs callbackARGS);
	void init(UString name, float defaultValue, UString helpString, ConVarChangeCallback callback);
	void init(UString name, UString defaultValue, UString helpString, ConVarChangeCallback callback);

	std::atomic<bool> m_bHasValue;
	CONVAR_TYPE m_type;

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
	ConVarHandler();
	~ConVarHandler();

	const std::vector<ConVar*> &getConVarArray() const;
	int getNumConVars() const;

	ConVar *getConVarByName(UString name, bool warnIfNotFound = true) const;
	std::vector<ConVar*> getConVarByLetter(UString letters) const;

private:
	static ConVar *emptyDummyConVar;
};

extern ConVarHandler *convar;

#endif

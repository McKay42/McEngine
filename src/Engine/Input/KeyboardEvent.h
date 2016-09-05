//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		a wrapper for consumable keyboard events
//
// $NoKeywords: $key
//===============================================================================//

#ifndef KEYBOARDEVENT_H
#define KEYBOARDEVENT_H

typedef unsigned long KEYCODE;

class KeyboardEvent
{
public:
	KeyboardEvent(KEYCODE keyCode);

	void consume();

	inline bool isConsumed() const {return m_bConsumed;}
	inline KEYCODE getKeyCode() const {return m_keyCode;}
	inline KEYCODE getCharCode() const {return m_keyCode;}

    bool operator == (const KEYCODE &rhs) const;
    bool operator != (const KEYCODE &rhs) const;

private:
	KEYCODE m_keyCode;
	bool m_bConsumed;
};



inline bool KeyboardEvent::operator == (const KEYCODE &rhs) const
{
    return m_keyCode == rhs;
}

inline bool KeyboardEvent::operator != (const KEYCODE &rhs) const
{
    return m_keyCode != rhs;
}

#endif

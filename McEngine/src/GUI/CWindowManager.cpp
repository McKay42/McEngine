//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		handles multiple window interactions
//
// $NoKeywords: $
//===============================================================================//

#include "CWindowManager.h"

#include "Engine.h"
#include "ResourceManager.h"
#include "CBaseUIWindow.h"

CWindowManager::CWindowManager()
{
	m_bVisible = true;
	m_bEnabled = true;
	m_iLastEnabledWindow = 0;
	m_iCurrentEnabledWindow = 0;
}

CWindowManager::~CWindowManager()
{
	for (int i=0; i<m_windows.size(); i++)
	{
		delete m_windows[i];
	}
}

void CWindowManager::draw(Graphics *g)
{
	if (!m_bVisible) return;

	for (int i=m_windows.size()-1; i>=0; i--)
	{
		m_windows[i]->draw(g);
	}

	/*
	for (int i=0; i<m_windows.size(); i++)
	{
		UString text = UString::format(" -%i-  ", i);
		text.append(m_windows[i]->getName());
		g->pushTransform();
			g->translate(20, i*40+350);
			g->drawString(engine->getResourceManager()->getFont("FONT_DEFAULT"), text);
		g->popTransform();
	}

	g->pushTransform();
	g->translate(20, 500);
	g->drawString(engine->getResourceManager()->getFont("FONT_DEFAULT"), UString::format("currentEnabled = %i, lastEnabled = %i, topIndex = %i", m_iCurrentEnabledWindow, m_iLastEnabledWindow, getTopMouseWindowIndex()));
	g->popTransform();
	*/
}

void CWindowManager::update()
{
	if (!m_bVisible || m_windows.size() == 0) return;

	// update all windows, detect depth changes
	for (int i=0; i<m_windows.size(); i++)
	{
		m_windows[i]->update();
	}
	int topMouseWindowIndex = getTopMouseWindowIndex();

	// don't event think about switching if we are disabled
	if (!m_bEnabled)
		return;

	// TODO: the logic here is fucked
	// if the topmost window isn't doing anything, we may be switching
	if (!m_windows[0]->isBusy())
	{
		if (topMouseWindowIndex < m_windows.size())
			m_iCurrentEnabledWindow = topMouseWindowIndex;

		// enable the closest window under the cursor on a change, disable the others
		if (m_iCurrentEnabledWindow != m_iLastEnabledWindow)
		{
			m_iLastEnabledWindow = m_iCurrentEnabledWindow;
			if (m_iLastEnabledWindow < m_windows.size())
			{
				m_windows[m_iLastEnabledWindow]->setEnabled(true);
				//debugLog("enabled %s @%f\n", m_windows[m_iLastEnabledWindow]->getName().toUtf8(), engine->getTime());
				for (int i=0; i<m_windows.size(); i++)
				{
					if (i != m_iLastEnabledWindow)
						m_windows[i]->setEnabled(false);
				}
			}
		}

		// now check if we have to switch any windows
		bool topSwitch = false;
		int newTop = 0;
		for (int i=0; i<m_windows.size(); i++)
		{
			if (m_windows[i]->isBusy() && !topSwitch)
			{
				topSwitch = true;
				newTop = i;
			}
		}

		// switch top window to m_windows[0], all others get pushed down
		if (topSwitch && newTop != 0)
		{
			//debugLog("top switch @%f\n", engine->getTime());
			if (m_windows.size() > 1)
			{
				CBaseUIWindow *newTopWindow = m_windows[newTop];
				newTopWindow->setEnabled(true);

				for (int i=newTop-1; i>=0; i--)
				{
					m_windows[i+1] = m_windows[i];
				}

				m_windows[0] = newTopWindow;
				m_iCurrentEnabledWindow = m_iLastEnabledWindow = 0;
			}
		}
	}
}

int CWindowManager::getTopMouseWindowIndex()
{
	int tempEnabledWindow = m_windows.size()-1;
	for (int i=0; i<m_windows.size(); i++)
	{
		if (m_windows[i]->isMouseInside() && i < tempEnabledWindow)
			tempEnabledWindow = i;
	}
	return tempEnabledWindow;
}

void CWindowManager::addWindow(CBaseUIWindow *window)
{
	if (window == NULL)
		engine->showMessageError("Window Manager Error", "addWindow(NULL), you maggot!");

	m_windows.insert(m_windows.begin(), window);

	// disable all other windows
	for (int i=1; i<(int)(m_windows.size()-1); i++)
	{
		m_windows[i]->setEnabled(false);
	}
}

void CWindowManager::onResolutionChange(Vector2 newResolution)
{
	for (int i=0; i<m_windows.size(); i++)
	{
		m_windows[i]->onResolutionChange(newResolution);
	}
}

void CWindowManager::setFocus(CBaseUIWindow *window)
{
	if (window == NULL)
		engine->showMessageError("Window Manager Error", "setFocus(NULL), you noodle!");

	for (int i=0; i<m_windows.size(); i++)
	{
		if (m_windows[i] == window && i != 0)
		{
			CBaseUIWindow *newTopWindow = m_windows[i];
			for (int i2=i-1; i2>=0; i2--)
			{
				m_windows[i2+1] = m_windows[i2];
			}
			m_windows[0] = newTopWindow;
			break;
		}
	}
}

void CWindowManager::openAll()
{
	for (int i=0; i<m_windows.size(); i++)
	{
		m_windows[i]->open();
	}
}

void CWindowManager::closeAll()
{
	for (int i=0; i<m_windows.size(); i++)
	{
		m_windows[i]->close();
	}
}

bool CWindowManager::isMouseInside()
{
	for (int i=0; i<m_windows.size(); i++)
	{
		if (m_windows[i]->isMouseInside())
			return true;
	}
	return false;
}

bool CWindowManager::isVisible()
{
	if (!m_bVisible)
		return false;

	for (int i=0; i<m_windows.size(); i++)
	{
		if (m_windows[i]->isVisible())
			return true;
	}
	return false;
}

bool CWindowManager::isActive()
{
	if (!m_bVisible)
		return false;
	for (int i=0; i<m_windows.size(); i++)
	{
		if (m_windows[i]->isActive() || m_windows[i]->isBusy()) // TODO: is this correct? (busy)
			return true;
	}
	return false;
}

void CWindowManager::onKeyDown(KeyboardEvent &e)
{
	if (!m_bVisible) return;

	for (int i=0; i<m_windows.size(); i++)
	{
		m_windows[i]->onKeyDown(e);
	}
}

void CWindowManager::onKeyUp(KeyboardEvent &e)
{
	if (!m_bVisible) return;

	for (int i=0; i<m_windows.size(); i++)
	{
		m_windows[i]->onKeyUp(e);
	}
}

void CWindowManager::onChar(KeyboardEvent &e)
{
	if (!m_bVisible) return;

	for (int i=0; i<m_windows.size(); i++)
	{
		m_windows[i]->onChar(e);
	}
}

void CWindowManager::setEnabled(bool enabled)
{
	m_bEnabled = enabled;
	if (!m_bEnabled)
	{
		for (int i=0; i<m_windows.size(); i++)
		{
			m_windows[i]->setEnabled(false);
		}
	}
	else
		m_windows[m_iCurrentEnabledWindow]->setEnabled(true);
}

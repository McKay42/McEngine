//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		base class for windows
//
// $NoKeywords: $
//===============================================================================//

#include "CBaseUIWindow.h"

#include "Engine.h"
#include "ConVar.h"
#include "Mouse.h"
#include "Cursors.h"
#include "Environment.h"
#include "ResourceManager.h"
#include "AnimationHandler.h"
#include "RenderTarget.h"

#include "CBaseUIContainer.h"
#include "CBaseUIButton.h"
#include "CBaseUIBoxShadow.h"

ConVar ui_window_animspeed("ui_window_animspeed", 0.29f);
ConVar ui_window_shadow_radius("ui_window_shadow_radius", 13.0f);

CBaseUIWindow::CBaseUIWindow(float xPos, float yPos, float xSize, float ySize, UString name) : CBaseUIElement(xPos, yPos, xSize, ySize, name)
{
	ResourceManager *rc = engine->getResourceManager();

	int titleBarButtonSize = 13;
	int titleBarButtonGap = 6;

	// titlebar
	m_bDrawTitleBarLine = true;
	m_titleFont = rc->loadFont("weblysleekuisb.ttf", "FONT_WINDOW_TITLE", 13.0f);
	m_iTitleBarHeight = m_titleFont->getHeight()+12;
	if (m_iTitleBarHeight < titleBarButtonSize)
		m_iTitleBarHeight = titleBarButtonSize + 4;
	m_titleBarContainer = new CBaseUIContainer(m_vPos.x, m_vPos.y, m_vSize.x, m_iTitleBarHeight, "titlebarcontainer");

	m_closeButton = new CBaseUIButton(m_vSize.x - titleBarButtonSize - (m_iTitleBarHeight-titleBarButtonSize)/2.0f, m_iTitleBarHeight/2.0f - titleBarButtonSize/2.0f, titleBarButtonSize, titleBarButtonSize, "", "");
	m_closeButton->setClickCallback( fastdelegate::MakeDelegate(this, &CBaseUIWindow::close) );
	m_closeButton->setDrawFrame(false);

	m_minimizeButton = new CBaseUIButton(m_vSize.x - titleBarButtonSize*2 - (m_iTitleBarHeight-titleBarButtonSize)/2.0f - titleBarButtonGap, m_iTitleBarHeight/2.0f - titleBarButtonSize/2.0f, titleBarButtonSize, titleBarButtonSize, "", "");
	m_minimizeButton->setVisible(false);
	m_minimizeButton->setDrawFrame(false);
	m_minimizeButton->setClickCallback( fastdelegate::MakeDelegate(this, &CBaseUIWindow::minimize) );

	m_titleBarContainer->addBaseUIElement(m_minimizeButton);
	m_titleBarContainer->addBaseUIElement(m_closeButton);

	// main container
	m_container = new CBaseUIContainer(m_vPos.x, m_vPos.y + m_titleBarContainer->getSize().y, m_vSize.x, m_vSize.y - m_titleBarContainer->getSize().y, "maincontainer");

	// colors
	m_frameColor = 0xffffffff;
	m_backgroundColor = 0xff000000;
	m_frameBrightColor = 0;
	m_frameDarkColor = 0;
	m_titleColor = 0xffffffff;

	// events
	m_vResizeLimit = Vector2(100,90);
	m_bMoving = false;
	m_bResizing = false;
	m_iResizeType = 0; //1 == top left, 2 == left, 3 == bottom left, 4 == bottom, 5 = bottom right, 6 == right, 7 == top right, 8 == top

	// window properties
	m_bIsOpen = false;
	m_bAnimIn = false;
	m_bResizeable = true;
	m_bCoherenceMode = false;
	m_fAnimation = 0.0f;

	m_bDrawFrame = true;
	m_bDrawBackground = true;
	m_bRoundedRectangle = false;

	// test features
	//m_rt = engine->getResourceManager()->createRenderTarget(m_vPos.x, m_vPos.y, m_vSize.x+1, m_vSize.y+1);
	//float shadowRadius = ui_window_shadow_radius.getInt();
	///m_shadow = new CBaseUIBoxShadow(0xff000000, shadowRadius, m_vPos.x-shadowRadius, m_vPos.y-shadowRadius, m_vSize.x+shadowRadius*2, m_vSize.y+shadowRadius*2+4, "windowshadow");

	setTitle(name);
	setVisible(false);

	// for very small resolutions on engine start
	if (m_vPos.y + m_vSize.y > engine->getScreenHeight())
	{
		setSizeY(engine->getScreenHeight() - 12);
	}
}

CBaseUIWindow::~CBaseUIWindow()
{
	///SAFE_DELETE(m_shadow);
	SAFE_DELETE(m_container);
	SAFE_DELETE(m_titleBarContainer);
}

void CBaseUIWindow::draw(Graphics *g)
{
	if (!m_bVisible) return;

	// TODO: structure
	/*
	if (!anim->isAnimating(&m_fAnimation))
		m_shadow->draw(g);
	else
	{
		m_shadow->setColor(COLOR((int)((m_fAnimation)*255.0f), 255, 255, 255));

		// HACKHACK: shadows can't render inside a 3DScene
		m_shadow->renderOffscreen(g);

		g->push3DScene(McRect(m_vPos.x, m_vPos.y, m_vSize.x, m_vSize.y));
			g->rotate3DScene(0, (m_bAnimIn ? -1 : 1) * (1-m_fAnimation)*10, 0);
			g->translate3DScene(0, 0, -(1-m_fAnimation)*100);
			m_shadow->draw(g);
		g->pop3DScene();
	}
	*/

	// draw window
	//if (anim->isAnimating(&m_fAnimation) && !m_bCoherenceMode)
	//	m_rt->enable();

	{
		// draw background
		if (m_bDrawBackground)
		{
			g->setColor(m_backgroundColor);

			if (m_bRoundedRectangle)
			{
				//int border = 0;
				g->fillRoundedRect(m_vPos.x, m_vPos.y, m_vSize.x+1, m_vSize.y+1, 6);
			}
			else
				g->fillRect(m_vPos.x, m_vPos.y, m_vSize.x+1, m_vSize.y+1);
		}

		// draw frame
		if (m_bDrawFrame)
		{
			if (m_frameDarkColor != 0 || m_frameBrightColor != 0)
				g->drawRect(m_vPos.x,m_vPos.y,m_vSize.x,m_vSize.y, m_frameDarkColor, m_frameBrightColor, m_frameBrightColor, m_frameDarkColor);
			else
			{
				g->setColor(/*m_bEnabled ? 0xffffff00 : */m_frameColor);
				g->drawRect(m_vPos.x,m_vPos.y,m_vSize.x,m_vSize.y);
			}
		}

		// draw window contents
		g->pushClipRect(McRect(m_vPos.x+1, m_vPos.y+2, m_vSize.x-1, m_vSize.y-1));
		{
			// draw main container
			g->pushClipRect(McRect(m_vPos.x+1, m_vPos.y+2, m_vSize.x-1, m_vSize.y-1));
				m_container->draw(g);
				drawCustomContent(g);
			g->popClipRect();

			// draw title bar background
			if (m_bDrawBackground && !m_bRoundedRectangle)
			{
				g->setColor(m_backgroundColor);
				g->fillRect(m_vPos.x, m_vPos.y, m_vSize.x, m_iTitleBarHeight);
			}

			// draw title bar line
			if (m_bDrawTitleBarLine)
			{
				g->setColor(m_frameColor);
				g->drawLine(m_vPos.x, m_vPos.y + m_iTitleBarHeight, m_vPos.x + m_vSize.x, m_vPos.y + m_iTitleBarHeight);
			}

			// draw title
			g->setColor(m_titleColor);
			g->pushTransform();
			g->translate((int)(m_vPos.x + m_vSize.x/2.0f - m_fTitleFontWidth/2.0f), (int)(m_vPos.y + m_fTitleFontHeight/2.0f + m_iTitleBarHeight/2.0f));
				g->drawString(m_titleFont, m_sTitle);
			g->popTransform();

			// draw title bar container
			g->pushClipRect(McRect(m_vPos.x+1, m_vPos.y+2, m_vSize.x-1, m_iTitleBarHeight));
				m_titleBarContainer->draw(g);
			g->popClipRect();

			// draw close button 'x'
			g->setColor(m_closeButton->getFrameColor());
			g->drawLine(m_closeButton->getPos().x+1, m_closeButton->getPos().y+1, m_closeButton->getPos().x + m_closeButton->getSize().x, m_closeButton->getPos().y + m_closeButton->getSize().y);
			g->drawLine(m_closeButton->getPos().x+1, m_closeButton->getPos().y + m_closeButton->getSize().y - 1, m_closeButton->getPos().x + m_closeButton->getSize().x, m_closeButton->getPos().y);

			// draw minimize button '_'
			if (m_minimizeButton->isVisible())
			{
				g->setColor(m_minimizeButton->getFrameColor());
				g->drawLine(m_minimizeButton->getPos().x+2, m_minimizeButton->getPos().y + m_minimizeButton->getSize().y - 2, m_minimizeButton->getPos().x + m_minimizeButton->getSize().x - 1, m_minimizeButton->getPos().y + m_minimizeButton->getSize().y - 2);
			}
		}
		g->popClipRect();
	}

	// TODO: structure
	if (anim->isAnimating(&m_fAnimation) && !m_bCoherenceMode)
	{
		/*
		m_rt->disable();


		m_rt->setColor(COLOR((int)(m_fAnimation*255.0f), 255, 255, 255));

		g->push3DScene(McRect(m_vPos.x, m_vPos.y, m_vSize.x, m_vSize.y));
			g->rotate3DScene((m_bAnimIn ? -1 : 1) * (1-m_fAnimation)*10, 0, 0);
			g->translate3DScene(0, 0, -(1-m_fAnimation)*100);
			m_rt->draw(g, m_vPos.x, m_vPos.y);
		g->pop3DScene();
		*/
	}
}

void CBaseUIWindow::update()
{
	CBaseUIElement::update();
	if (!m_bVisible) return;

	// after the close animation is finished, set invisible
	if (m_fAnimation == 0.0f && m_bVisible)
	{
		setVisible(false);
	}

	// window logic comes first
	if (!m_titleBarContainer->isBusy() && !m_container->isBusy() && m_bMouseInside && m_bEnabled)
		updateWindowLogic();

	// the main two containers
	m_titleBarContainer->update();
	m_container->update();

	// moving
	if (m_bMoving)
		setPos(m_vLastPos + (engine->getMouse()->getPos() - m_vMousePosBackup));

	// resizing
	if (m_bResizing)
	{
		switch(m_iResizeType)
		{
		case 1:
			setPos(clamp<float>(m_vLastPos.x + (engine->getMouse()->getPos().x - m_vMousePosBackup.x), -m_vSize.x, m_vLastPos.x + m_vLastSize.x - m_vResizeLimit.x),
				   clamp<float>(m_vLastPos.y + (engine->getMouse()->getPos().y - m_vMousePosBackup.y), -m_vSize.y, m_vLastPos.y + m_vLastSize.y - m_vResizeLimit.y));
			setSize(clamp<float>(m_vLastSize.x + (m_vMousePosBackup.x - engine->getMouse()->getPos().x), m_vResizeLimit.x, engine->getScreenWidth()),
					clamp<float>(m_vLastSize.y + (m_vMousePosBackup.y - engine->getMouse()->getPos().y), m_vResizeLimit.y, engine->getScreenHeight()));
			break;
		case 2:
			setPosX(clamp<float>(m_vLastPos.x + (engine->getMouse()->getPos().x - m_vMousePosBackup.x), -m_vSize.x, m_vLastPos.x + m_vLastSize.x - m_vResizeLimit.x));
			setSizeX(clamp<float>(m_vLastSize.x + (m_vMousePosBackup.x - engine->getMouse()->getPos().x), m_vResizeLimit.x, engine->getScreenWidth()));
			break;
		case 3:
			setPosX(clamp<float>(m_vLastPos.x + (engine->getMouse()->getPos().x - m_vMousePosBackup.x), -m_vSize.x, m_vLastPos.x + m_vLastSize.x - m_vResizeLimit.x));
			setSizeX(clamp<float>(m_vLastSize.x + (m_vMousePosBackup.x - engine->getMouse()->getPos().x), m_vResizeLimit.x, engine->getScreenWidth()));
			setSizeY(clamp<float>(m_vLastSize.y + (engine->getMouse()->getPos().y - m_vMousePosBackup.y), m_vResizeLimit.y, engine->getScreenHeight()));
			break;
		case 4:
			setSizeY(clamp<float>(m_vLastSize.y + (engine->getMouse()->getPos().y - m_vMousePosBackup.y), m_vResizeLimit.y, engine->getScreenHeight()));
			break;
		case 5:
			setSize(clamp<float>(m_vLastSize.x + (engine->getMouse()->getPos().x - m_vMousePosBackup.x), m_vResizeLimit.x, engine->getScreenWidth()),
					clamp<float>(m_vLastSize.y + (engine->getMouse()->getPos().y - m_vMousePosBackup.y), m_vResizeLimit.y, engine->getScreenHeight()));
			break;
		case 6:
			setSizeX(clamp<float>(m_vLastSize.x + (engine->getMouse()->getPos().x - m_vMousePosBackup.x), m_vResizeLimit.x, engine->getScreenWidth()));
			break;
		case 7:
			setPosY(clamp<float>(m_vLastPos.y + (engine->getMouse()->getPos().y - m_vMousePosBackup.y), -m_vSize.y, m_vLastPos.y + m_vLastSize.y - m_vResizeLimit.y));
			setSizeY(clamp<float>(m_vLastSize.y + (m_vMousePosBackup.y - engine->getMouse()->getPos().y), m_vResizeLimit.y, engine->getScreenHeight()));
			setSizeX(clamp<float>(m_vLastSize.x + (engine->getMouse()->getPos().x - m_vMousePosBackup.x), m_vResizeLimit.x, engine->getScreenWidth()));
			break;
		case 8:
			setPosY(clamp<float>(m_vLastPos.y + (engine->getMouse()->getPos().y - m_vMousePosBackup.y), -m_vSize.y, m_vLastPos.y + m_vLastSize.y - m_vResizeLimit.y));
			setSizeY(clamp<float>(m_vLastSize.y + (m_vMousePosBackup.y - engine->getMouse()->getPos().y), m_vResizeLimit.y, engine->getScreenHeight()));
			break;
		}
	}
}

void CBaseUIWindow::onKeyDown(KeyboardEvent &e)
{
	if (!m_bVisible) return;
	m_container->onKeyDown(e);
}

void CBaseUIWindow::onKeyUp(KeyboardEvent &e)
{
	if (!m_bVisible) return;
	m_container->onKeyUp(e);
}

void CBaseUIWindow::onChar(KeyboardEvent &e)
{
	if (!m_bVisible) return;
	m_container->onChar(e);
}

CBaseUIWindow *CBaseUIWindow::setTitle(UString text)
{
	m_sTitle = text;
	updateTitleBarMetrics();
	return this;
}

void CBaseUIWindow::updateWindowLogic()
{
	if (!engine->getMouse()->isLeftDown())
	{
		m_bMoving = false;
		m_bResizing = false;
	}

	// handle resize & move cursor
	if (!m_titleBarContainer->isBusy() && !m_container->isBusy() && !m_bResizing && !m_bMoving)
	{
		if (!engine->getMouse()->isLeftDown())
			udpateResizeAndMoveLogic(false);
	}
}

void CBaseUIWindow::udpateResizeAndMoveLogic(bool captureMouse)
{
	if (m_bCoherenceMode) return; // resizing in coherence mode is handled in main.cpp

	// backup
	m_vLastSize = m_vSize;
	m_vMousePosBackup = engine->getMouse()->getPos();
	m_vLastPos = m_vPos;

	if (m_bResizeable)
	{
		// reset
		m_iResizeType = 0;

		int resizeHandleSize = 5;
		McRect resizeTopLeft = McRect(m_vPos.x, m_vPos.y, resizeHandleSize, resizeHandleSize);
		McRect resizeLeft = McRect(m_vPos.x, m_vPos.y, resizeHandleSize, m_vSize.y);
		McRect resizeBottomLeft = McRect(m_vPos.x, m_vPos.y + m_vSize.y - resizeHandleSize, resizeHandleSize, resizeHandleSize);
		McRect resizeBottom = McRect(m_vPos.x, m_vPos.y + m_vSize.y - resizeHandleSize, m_vSize.x, resizeHandleSize);
		McRect resizeBottomRight = McRect(m_vPos.x + m_vSize.x - resizeHandleSize, m_vPos.y + m_vSize.y - resizeHandleSize, resizeHandleSize, resizeHandleSize);
		McRect resizeRight = McRect(m_vPos.x + m_vSize.x - resizeHandleSize, m_vPos.y, resizeHandleSize, m_vSize.y);
		McRect resizeTopRight = McRect(m_vPos.x + m_vSize.x - resizeHandleSize, m_vPos.y, resizeHandleSize, resizeHandleSize);
		McRect resizeTop = McRect(m_vPos.x, m_vPos.y, m_vSize.x, resizeHandleSize);

		if (resizeTopLeft.contains(m_vMousePosBackup))
		{
			if (captureMouse)
				m_iResizeType = 1;
			engine->getMouse()->setCursorType(CURSOR_SIZE_VH);
		}
		else if (resizeBottomLeft.contains(m_vMousePosBackup))
		{
			if (captureMouse)
				m_iResizeType = 3;
			engine->getMouse()->setCursorType(CURSOR_SIZE_HV);
		}
		else if (resizeBottomRight.contains(m_vMousePosBackup))
		{
			if (captureMouse)
				m_iResizeType = 5;
			engine->getMouse()->setCursorType(CURSOR_SIZE_VH);
		}
		else if (resizeTopRight.contains(m_vMousePosBackup))
		{
			if (captureMouse)
				m_iResizeType = 7;
			engine->getMouse()->setCursorType(CURSOR_SIZE_HV);
		}
		else if (resizeLeft.contains(m_vMousePosBackup))
		{
			if (captureMouse)
				m_iResizeType = 2;
			engine->getMouse()->setCursorType(CURSOR_SIZE_H);
		}
		else if (resizeRight.contains(m_vMousePosBackup))
		{
			if (captureMouse)
				m_iResizeType = 6;
			engine->getMouse()->setCursorType(CURSOR_SIZE_H);
		}
		else if (resizeBottom.contains(m_vMousePosBackup))
		{
			if (captureMouse)
				m_iResizeType = 4;
			engine->getMouse()->setCursorType(CURSOR_SIZE_V);
		}
		else if (resizeTop.contains(m_vMousePosBackup))
		{
			if (captureMouse)
				m_iResizeType = 8;
			engine->getMouse()->setCursorType(CURSOR_SIZE_V);
		}
	}

	// handle resizing
	if (m_iResizeType > 0)
		m_bResizing = true;
	else if (captureMouse)
	{
		// handle moving
		McRect titleBarGrab = McRect(m_vPos.x, m_vPos.y, m_vSize.x, m_iTitleBarHeight);
		if (titleBarGrab.contains(m_vMousePosBackup))
			m_bMoving = true;
	}

	// resizing and moving have priority over window contents
	if (m_bResizing || m_bMoving)
		m_container->stealFocus();
}

void CBaseUIWindow::close()
{
	if (anim->isAnimating(&m_fAnimation)) return;

	m_bAnimIn = false;
	m_fAnimation = 1.0f;
	anim->moveQuadInOut(&m_fAnimation, 0.0f, ui_window_animspeed.getFloat());

	onClosed();
}

void CBaseUIWindow::open()
{
	if (anim->isAnimating(&m_fAnimation) || m_bVisible) return;

	setVisible(true);

	if (!m_bCoherenceMode)
	{
		m_bAnimIn = true;
		m_fAnimation = 0.001f;
		anim->moveQuadOut(&m_fAnimation, 1.0f, ui_window_animspeed.getFloat());
	}
	else
		m_fAnimation = 1.0f;
}

void CBaseUIWindow::minimize()
{
	if (m_bCoherenceMode)
		env->minimize();
}

CBaseUIWindow *CBaseUIWindow::setSizeToContent(int horizontalBorderSize, int verticalBorderSize)
{
	std::vector<CBaseUIElement*> *elements = m_container->getAllBaseUIElementsPointer();
	if (elements->size() < 1)
		return this;

	Vector2 newSize = Vector2(horizontalBorderSize, verticalBorderSize);

	for (int i=0; i<elements->size(); i++)
	{
		CBaseUIElement *el = (*elements)[i];
		int xReach = el->getRelPos().x + el->getSize().x + horizontalBorderSize;
		int yReach = el->getRelPos().y + el->getSize().y + verticalBorderSize;
		if (xReach > newSize.x)
			newSize.x = xReach;
		if (yReach > newSize.y)
			newSize.y = yReach;
	}
	newSize.y = newSize.y + m_titleBarContainer->getSize().y;

	setSize(newSize);

	return this;
}

CBaseUIWindow *CBaseUIWindow::enableCoherenceMode()
{
	m_bCoherenceMode = true;
	m_minimizeButton->setVisible(true);
	setPos(0,0);
	env->setWindowSize(m_vSize.x+1, m_vSize.y+1);

	return this;
}

void CBaseUIWindow::onMouseDownInside()
{
	m_bBusy = true;
	m_titleBarContainer->update();
	if (!m_titleBarContainer->isBusy())
		udpateResizeAndMoveLogic(true);
}

void CBaseUIWindow::onMouseUpInside()
{
	m_bBusy = false;
	m_bResizing = false;
	m_bMoving = false;
}

void CBaseUIWindow::onMouseUpOutside()
{
	m_bBusy = false;
	m_bResizing = false;
	m_bMoving = false;
}

void CBaseUIWindow::updateTitleBarMetrics()
{
	m_closeButton->setRelPos(m_vSize.x - m_closeButton->getSize().x - (m_iTitleBarHeight-m_closeButton->getSize().x)/2.0f, m_iTitleBarHeight/2.0f - m_closeButton->getSize().y/2.0f);
	m_minimizeButton->setRelPos(m_vSize.x - m_minimizeButton->getSize().x*2 - (m_iTitleBarHeight-m_minimizeButton->getSize().x)/2.0f - 6, m_iTitleBarHeight/2.0f - m_minimizeButton->getSize().y/2.0f);

	m_fTitleFontWidth = m_titleFont->getStringWidth(m_sTitle);
	m_fTitleFontHeight = m_titleFont->getHeight();
	m_titleBarContainer->setSize(m_vSize.x, m_iTitleBarHeight);
}

void CBaseUIWindow::onMoved()
{
	m_titleBarContainer->setPos(m_vPos);
	m_container->setPos(m_vPos.x, m_vPos.y + m_titleBarContainer->getSize().y);
	updateTitleBarMetrics();

	//if (!m_bCoherenceMode)
	//	m_rt->setPos(m_vPos);
	///m_shadow->setPos(m_vPos.x-m_shadow->getRadius(), m_vPos.y-m_shadow->getRadius());
}

void CBaseUIWindow::onResized()
{
	updateTitleBarMetrics();
	m_container->setSize(m_vSize.x, m_vSize.y - m_titleBarContainer->getSize().y);

	//if (!m_bCoherenceMode)
	//	m_rt->rebuild(m_vPos.x, m_vPos.y, m_vSize.x+1, m_vSize.y+1);
	///m_shadow->setSize(m_vSize.x+m_shadow->getRadius()*2, m_vSize.y+m_shadow->getRadius()*2+4);
}

void CBaseUIWindow::onResolutionChange(Vector2 newResolution)
{
	if (m_bCoherenceMode)
		setSize(newResolution.x-1, newResolution.y-1);
}

void CBaseUIWindow::onEnabled()
{
	m_container->setEnabled(true);
	m_titleBarContainer->setEnabled(true);
}

void CBaseUIWindow::onDisabled()
{
	m_bBusy = false;
	m_container->setEnabled(false);
	m_titleBarContainer->setEnabled(false);
}

void CBaseUIWindow::onClosed()
{
	if (m_bCoherenceMode)
		engine->shutdown();
}

bool CBaseUIWindow::isBusy()
{
	return (m_bBusy || m_titleBarContainer->isBusy() || m_container->isBusy()) && m_bVisible;
}

bool CBaseUIWindow::isActive()
{
	return (m_titleBarContainer->isActive() || m_container->isActive()) && m_bVisible;
}

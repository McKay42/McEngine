//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		engine framework test app
//
// $NoKeywords: $test
//===============================================================================//

#include "FrameworkTest.h"

#include "Engine.h"
#include "Mouse.h"
#include "Gamepad.h"
#include "ResourceManager.h"
#include "ConVar.h"

#include "File.h"

#include "CBaseUIButton.h"

class FrameworkTestButton : public CBaseUIButton
{
public:
	FrameworkTestButton(float xPos, float yPos, float xSize, float ySize, UString name, UString text) : CBaseUIButton(xPos, yPos, xSize, ySize, name, text) {;}

	virtual void onMouseInside() {debugLog("CBaseUIButton::onMouseInside()\n");}
	virtual void onMouseOutside() {debugLog("CBaseUIButton::onMouseOutside()\n");}
	virtual void onMouseDownInside() {debugLog("CBaseUIButton::onMouseDownInside()\n");}
	virtual void onMouseDownOutside() {debugLog("CBaseUIButton::onMouseDownOutside()\n");}
	virtual void onMouseUpInside() {debugLog("CBaseUIButton::onMouseUpInside()\n");}
	virtual void onMouseUpOutside() {debugLog("CBaseUIButton::onMouseUpOutside()\n");}
};

FrameworkTest::FrameworkTest()
{
	debugLog("FrameworkTest::FrameworkTest()\n");

	engine->getMouse()->addListener(this);
	engine->getGamepad()->addListener(this);

	m_testButton = new FrameworkTestButton(300, 600, 200, 25, "CBaseUIButton", "CBaseUIButton");

	// load resource
	engine->getResourceManager()->loadImage("block.png", "TESTIMAGE");
}

FrameworkTest::~FrameworkTest()
{
	debugLog("FrameworkTest::~FrameworkTest()\n");

	SAFE_DELETE(m_testButton);
}

void FrameworkTest::draw(Graphics *g)
{
	McFont *testFont = engine->getResourceManager()->getFont("FONT_DEFAULT");

	// test general drawing
	g->setColor(0xffff0000);
	int blockSize = 100;
	g->fillRect(engine->getScreenWidth()/2 - blockSize/2 + std::sin(engine->getTime()*3)*100, engine->getScreenHeight()/2 - blockSize/2 + std::sin(engine->getTime()*3*1.5f)*100, blockSize, blockSize);

	// test font texture atlas
	g->setColor(0xffffffff);
	g->pushTransform();
	g->translate(100, 100);
	testFont->drawTextureAtlas(g);
	g->popTransform();

	// test image
	Image *testImage = engine->getResourceManager()->getImage("TESTIMAGE");
	g->setColor(0xffffffff);
	g->pushTransform();
	g->translate(testImage->getWidth()/2 + 50, testImage->getHeight()/2 + 100);
	g->drawImage(testImage);
	g->popTransform();

	// test button
	m_testButton->draw(g);

	// test gamepad
	if (engine->getGamepad()->isConnected())
	{
		const int width = 150;
		Vector2 origin = Vector2(900, 200);
		Vector2 origin2 = origin + Vector2(width*1.5f, 0);
		g->setColor(0xffffffff);
		g->drawRect(origin.x-width/2, origin.y-width/2, width, width);
		g->drawRect(origin2.x-width/2, origin2.y-width/2, width, width);
		g->fillRect(origin.x-width/2, origin.y-width/2, width, width*engine->getGamepad()->getLeftTrigger());
		g->fillRect(origin2.x-width/2, origin2.y-width/2, width, width*engine->getGamepad()->getRightTrigger());
		g->setColor(0xff00ff00);
		g->drawLine(origin, origin - Vector2(-engine->getGamepad()->getLeftStick().x, engine->getGamepad()->getLeftStick().y)*width*0.5f);
		g->drawLine(origin2, origin2 - Vector2(-engine->getGamepad()->getRightStick().x, engine->getGamepad()->getRightStick().y)*width*0.5f);
	}

	// test text
	UString testText = "It's working!";
	g->push3DScene(McRect(800, 300, testFont->getStringWidth(testText), testFont->getHeight()));
	g->rotate3DScene(0, engine->getTime()*200, 0);
		g->pushTransform();
			g->translate(800, 300 + testFont->getHeight());
			g->drawString(testFont, testText);
		g->popTransform();
	g->pop3DScene();
}

void FrameworkTest::update()
{
	m_testButton->update();
}

void FrameworkTest::onResolutionChanged(Vector2 newResolution)
{
	debugLog("FrameworkTest::onResolutionChanged( (%f, %f) )\n", newResolution.x, newResolution.y);
}

void FrameworkTest::onFocusGained()
{
	debugLog("FrameworkTest::onFocusGained()\n");
}

void FrameworkTest::onFocusLost()
{
	debugLog("FrameworkTest::onFocusLost()\n");
}

void FrameworkTest::onMinimized()
{
	debugLog("FrameworkTest::onMinimized()\n");
}

void FrameworkTest::onRestored()
{
	debugLog("FrameworkTest::onRestored()\n");
}



void FrameworkTest::onKeyDown(KeyboardEvent &e)
{
	debugLog("FrameworkTest::onKeyDown( %ld )\n", e.getKeyCode());
}

void FrameworkTest::onKeyUp(KeyboardEvent &e)
{
	debugLog("FrameworkTest::onKeyUp( %ld )\n", e.getKeyCode());
}

void FrameworkTest::onChar(KeyboardEvent &e)
{
	debugLog("FrameworkTest::onChar( %ld )\n", e.getKeyCode());
}



void FrameworkTest::onLeftChange(bool down)
{
	debugLog("FrameworkTest::onLeftChange(%i)\n", (int)down);
}

void FrameworkTest::onMiddleChange(bool down)
{
	debugLog("FrameworkTest::onMiddleChange(%i)\n", (int)down);
}

void FrameworkTest::onRightChange(bool down)
{
	debugLog("FrameworkTest::onRightChange(%i)\n", (int)down);
}

void FrameworkTest::onWheelVertical(int delta)
{
	debugLog("FrameworkTest::onWheelVertical(%i)\n", delta);
}

void FrameworkTest::onWheelHorizontal(int delta)
{
	debugLog("FrameworkTest::onWheelHorizontal(%i)\n", delta);
}



void FrameworkTest::onButtonDown(GAMEPADBUTTON b)
{
	debugLog("FrameworkTest::onButtonDown(%i)\n", (int)b);
}

void FrameworkTest::onButtonUp(GAMEPADBUTTON b)
{
	debugLog("FrameworkTest::onButtonUp(%i)\n", (int)b);
}

void FrameworkTest::onConnected()
{
	debugLog("FrameworkTest::onConnected()\n");
}

void FrameworkTest::onDisconnected()
{
	debugLog("FrameworkTest::onDisconnected()\n");
}

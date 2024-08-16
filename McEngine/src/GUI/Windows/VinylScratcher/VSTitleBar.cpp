//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		3D flip bar used for music scrolling/searching/play history
//
// $NoKeywords: $
//===============================================================================//

#include "VSTitleBar.h"

#include "Engine.h"
#include "ResourceManager.h"
#include "AnimationHandler.h"
#include "Mouse.h"
#include "ConVar.h"

#include "CBaseUIContainer.h"
#include "CBaseUIButton.h"

ConVar vs_percent("vs_percent", 0.0f, FCVAR_NONE);



class VSTitleBarButton : public CBaseUIButton
{
public:
	VSTitleBarButton(float xPos, float yPos, float xSize, float ySize, UString name, UString text) : CBaseUIButton(xPos, yPos, xSize, ySize, name, text) {;}
	virtual ~VSTitleBarButton() {;}

	virtual void draw(Graphics *g)
	{
		if (!m_bVisible) return;

		// default background gradient
		{
			const Color top = COLOR(255, 244, 244, 244);
			const Color bottom = COLOR(255, 221, 221, 221);

			g->fillGradient(m_vPos.x + 1, m_vPos.y + 1, m_vSize.x - 1, m_vSize.y, top, top, bottom, bottom);
		}

		// blue seekbar overlay
		{
			const float seekBarPercent = vs_percent.getFloat();
			if (seekBarPercent > 0.0f)
			{
				const Color middle = COLOR(255, 0, 50, 119);
				const Color third = COLOR(255, 0, 113 - 50, 207 - 50);
				const Color top = COLOR(255, 0, 196, 223);

				const float sizeThird = m_vSize.y / 3.0f;

				g->fillGradient(m_vPos.x + 1, m_vPos.y + 1, 											(m_vSize.x-2)*seekBarPercent,	sizeThird,						top, top, third, third);
				g->fillGradient(m_vPos.x + 1, m_vPos.y + 1 + sizeThird,									(m_vSize.x-2)*seekBarPercent,	std::round(sizeThird/2.0f) + 1,	third, third, middle, middle);
				g->fillGradient(m_vPos.x + 1, m_vPos.y + 1 + sizeThird + std::round(sizeThird/2.0f),	(m_vSize.x-2)*seekBarPercent,	std::round(sizeThird/2.0f) + 1,	middle, middle, third, third);
				g->fillGradient(m_vPos.x + 1, m_vPos.y + 1 + 2*sizeThird,								(m_vSize.x-2)*seekBarPercent,	sizeThird,						third, third, top, top);
			}
		}

		// bottom line
		{
			g->setColor(COLOR(255, 204, 204, 204));
			g->drawLine(m_vPos.x, m_vPos.y + m_vSize.y, m_vPos.x + m_vSize.x, m_vPos.y + m_vSize.y);
		}

		drawText(g);
	}
};



VSTitleBar::VSTitleBar(int x, int y, int xSize, McFont *font) : CBaseUIElement(x, y, xSize, 44 * env->getDPIScale(), "")
{
	m_font = font;

	const Color textColor = COLOR(215, 55, 55, 55);

	m_container = new CBaseUIContainer(0, 0, m_vSize.x, m_vSize.y, "");

	m_title = new VSTitleBarButton(0, 0, xSize - 1, m_vSize.y, "", "");
	m_title->setDrawBackground(false);
	m_title->setDrawFrame(false);
	m_title->setTextColor(textColor);
	m_title->setFont(m_font);
	m_container->addBaseUIElement(m_title);

	m_title2 = new VSTitleBarButton(0, 0, xSize - 1, m_vSize.y, "", "Ready");
	m_title2->setDrawBackground(true);
	m_title2->setDrawFrame(false);
	m_title2->setTextColor(textColor);
	m_title2->setFont(m_font);
	m_container->addBaseUIElement(m_title2);

	// vars
	m_fRot = 0.0f;
	m_iFlip = 0;

	m_bIsSeeking = false;
}

VSTitleBar::~VSTitleBar()
{
	SAFE_DELETE(m_container);
}

void VSTitleBar::draw(Graphics *g)
{
	if (m_iFlip != 0)
	{
		// 3d flip effect
		if ((m_fRot > -45.0f && m_iFlip == 1) || (m_iFlip == 2 && m_fRot < 45.0f))
		{
			g->push3DScene(McRect(m_vPos.x, m_vPos.y, m_title->getSize().x, m_title->getSize().y));
			{
				g->offset3DScene(0, 0, m_title->getSize().y/2);
				g->rotate3DScene(m_fRot + (m_iFlip == 1 ? 90 : -90), 0, 0);
				drawTitle1(g);
			}
			g->pop3DScene();

			g->push3DScene(McRect(m_vPos.x, m_vPos.y, m_title2->getSize().x, m_title2->getSize().y));
			{
				g->offset3DScene(0, 0, m_title2->getSize().y/2);
				g->rotate3DScene(m_fRot, 0, 0);
				drawTitle2(g);
			}
			g->pop3DScene();
		}
		else
		{
			g->push3DScene(McRect(m_vPos.x, m_vPos.y, m_title2->getSize().x, m_title2->getSize().y));
			{
				g->offset3DScene(0, 0, m_title2->getSize().y/2);
				g->rotate3DScene(m_fRot, 0, 0);

				drawTitle2(g);
			}
			g->pop3DScene();

			g->push3DScene(McRect(m_vPos.x, m_vPos.y, m_title->getSize().x, m_title->getSize().y));
			{
				g->offset3DScene(0, 0, m_title->getSize().y/2);
				g->rotate3DScene(m_fRot + (m_iFlip == 1 ? 90 : -90), 0, 0);

				drawTitle1(g);
			}
			g->pop3DScene();
		}
	}
	else
		drawTitle2(g);
}

void VSTitleBar::drawTitle1(Graphics *g)
{
	m_title->draw(g);
	if (vs_percent.getFloat() > 0)
	{
		m_title->setTextColor(0xffffffff);
		g->pushClipRect(McRect(m_vPos.x, m_vPos.y, vs_percent.getFloat()*m_vSize.x, m_title2->getSize().y));
		{
			m_title->draw(g);
		}
		g->popClipRect();
		m_title->setTextColor(COLOR(255, 55, 55, 55));
	}
}

void VSTitleBar::drawTitle2(Graphics *g)
{
	m_title2->draw(g);
	if (vs_percent.getFloat() > 0)
	{
		m_title2->setTextColor(0xffffffff);
		g->pushClipRect(McRect(m_vPos.x, m_vPos.y, vs_percent.getFloat()*m_vSize.x, m_title2->getSize().y));
		{
			m_title2->draw(g);
		}
		g->popClipRect();
		m_title2->setTextColor(COLOR(255, 55, 55, 55));
	}
}

void VSTitleBar::update()
{
	CBaseUIElement::update();
	if (!m_bVisible) return;

	m_container->update();

	// handle 3d flip logic
	if (m_iFlip != 0)
	{
		if (std::abs(m_fRot) == 90.0f)
		{
			anim->deleteExistingAnimation(&m_fRot);
			m_fRot = 0.0f;
			m_iFlip = 0;

			// text switch
			UString backupt = m_title2->getText();
			{
				m_title2->setText(m_title->getText());
			}
			m_title->setText(backupt);
		}
	}

	// handle scrubbing
	if (m_title2->isActive() && m_bActive)
	{
		m_bIsSeeking = true;
		const float percent = clamp<float>((engine->getMouse()->getPos().x + 1 - m_vPos.x) / m_title->getSize().x, 0.0f, 1.0f);
		vs_percent.setValue(percent);
	}
	else
	{
		// fire seek callback once scrubbing stops
		if (m_bIsSeeking)
		{
			m_bIsSeeking = false;
			if (m_seekCallback != NULL && !engine->getMouse()->isRightDown())
				m_seekCallback();
		}
	}
}

void VSTitleBar::onResized()
{
	m_container->setSize(m_vSize);

	m_title->setSize(m_vSize.x - 1, m_vSize.y);
	m_title2->setSize(m_vSize.x - 1, m_vSize.y);
}

void VSTitleBar::onMoved()
{
	// forward
	m_container->setPos(m_vPos);
}

void VSTitleBar::onFocusStolen()
{
	// forward
	m_container->onFocusStolen();
}

void VSTitleBar::setTitle(UString title, bool reverse)
{
	m_title->setText(title);
	if (anim->isAnimating(&m_fRot)) return;

	m_iFlip = 1;
	if (reverse)
		m_iFlip = 2;

	anim->moveQuadInOut(&m_fRot, reverse == false ? -90.0f : 90.0f, 0.45f);
}

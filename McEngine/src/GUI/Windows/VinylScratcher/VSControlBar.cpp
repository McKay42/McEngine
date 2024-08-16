//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		play/pause/forward/shuffle/repeat/eq/settings/volume/time bar
//
// $NoKeywords: $
//===============================================================================//

#include "VSControlBar.h"

#include "Engine.h"
#include "Keyboard.h"
#include "ConVar.h"
#include "ResourceManager.h"
#include "AnimationHandler.h"

#include "CBaseUIContainer.h"
#include "CBaseUICheckbox.h"
#include "CBaseUISlider.h"

ConVar vs_repeat("vs_repeat", false, FCVAR_NONE);
ConVar vs_shuffle("vs_shuffle", false, FCVAR_NONE);
ConVar vs_volume("vs_volume", 1.0f, FCVAR_NONE);



class VSControlBarButton : public CBaseUIButton
{
public:
	VSControlBarButton(float xPos, float yPos, float xSize, float ySize, UString name, UString text) : CBaseUIButton(xPos, yPos, xSize, ySize, name, text) {;}
	virtual ~VSControlBarButton() {;}

	virtual void draw(Graphics *g)
	{
		if (!m_bVisible) return;

		const Color top = COLOR(255, 244, 244, 244);
		const Color bottom = COLOR(255, 221, 221, 221);

		g->fillGradient(m_vPos.x + 1, m_vPos.y + 1, m_vSize.x - 1, m_vSize.y, top, top, bottom, bottom);

		g->setColor(COLOR(255, 204, 204, 204));
		g->drawRect(m_vPos.x, m_vPos.y, m_vSize.x, m_vSize.y);

		drawText(g);
	}

protected:
	virtual void drawText(Graphics *g)
	{
		if (m_font != NULL && m_sText.length() > 0)
		{
			const int textPressedAdd = (m_bActive ? 1 : 0);

			g->pushClipRect(McRect(m_vPos.x + 1, m_vPos.y + 1, m_vSize.x - 1, m_vSize.y - 1));
			{
				g->setColor(m_textColor);
				g->pushTransform();
				{
					g->translate((int)(m_vPos.x + m_vSize.x/2.0f - m_fStringWidth/2.0f), (int)(m_vPos.y + m_vSize.y/2.0f + m_fStringHeight/2.0f));
					g->translate((int)(1 + textPressedAdd), (int)(1 + textPressedAdd));

					if (m_textDarkColor != 0)
						g->setColor(m_textDarkColor);
					else
						g->setColor(COLOR_INVERT(m_textColor));

					g->drawString(m_font, m_sText);
				}
				g->popTransform();
				g->pushTransform();
				{
					g->translate((int)(m_vPos.x + m_vSize.x/2.0f - m_fStringWidth/2.0f + textPressedAdd), (int)(m_vPos.y + m_vSize.y/2.0f + m_fStringHeight/2.0f + textPressedAdd));

					if (m_textBrightColor != 0)
						g->setColor(m_textBrightColor);
					else
						g->setColor(m_textColor);

					g->drawString(m_font, m_sText);
				}
				g->popTransform();
			}
			g->popClipRect();
		}
	}
};

class VSControlBarSlider : public CBaseUISlider
{
public:
	VSControlBarSlider(float xPos, float yPos, float xSize, float ySize, UString name) : CBaseUISlider(xPos, yPos, xSize, ySize, name) {;}
	virtual ~VSControlBarSlider() {;}

	virtual void draw(Graphics *g)
	{
		CBaseUISlider::draw(g);
		if (!m_bVisible) return;

		g->drawQuad(Vector2(m_vPos.x+std::round(m_vBlockPos.x)+1, m_vPos.y+std::round(m_vBlockPos.y)+1),
					Vector2(m_vPos.x+std::round(m_vBlockPos.x)+1 + m_vBlockSize.x-1, m_vPos.y+std::round(m_vBlockPos.y)+1),
					Vector2(m_vPos.x+std::round(m_vBlockPos.x)+1 + m_vBlockSize.x-1, m_vPos.y+std::round(m_vBlockPos.y)+1 + m_vBlockSize.y/2),
					Vector2(m_vPos.x+std::round(m_vBlockPos.x)+1,  m_vPos.y+std::round(m_vBlockPos.y)+1 + m_vBlockSize.y/2),
					COLOR(255, 205, 218, 243),
					COLOR(255, 205, 218, 243),
					COLOR(255, 141, 188, 238),
					COLOR(255, 141, 188, 238));

		g->drawQuad(Vector2(m_vPos.x+std::round(m_vBlockPos.x)+1, m_vPos.y+std::round(m_vBlockPos.y)+1+std::round(m_vBlockSize.y/2.0f)),
					Vector2(m_vPos.x+std::round(m_vBlockPos.x)+1 + m_vBlockSize.x-1, m_vPos.y+std::round(m_vBlockPos.y)+1+std::round(m_vBlockSize.y/2.0f)),
					Vector2(m_vPos.x+std::round(m_vBlockPos.x)+1 + m_vBlockSize.x-1, m_vPos.y+std::round(m_vBlockPos.y)+1+std::round(m_vBlockSize.y/2.0f) + m_vBlockSize.y-(std::round(m_vBlockSize.y/2.0f))),
					Vector2(m_vPos.x+std::round(m_vBlockPos.x)+1, m_vPos.y+std::round(m_vBlockPos.y)+1+std::round(m_vBlockSize.y/2.0f) + m_vBlockSize.y-(std::round(m_vBlockSize.y/2.0f))),
					COLOR(255, 105, 173, 243),
					COLOR(255, 105, 173, 243),
					COLOR(255, 185, 253, 254),
					COLOR(255, 185, 253, 254));
	}
};

class VSControlBarCheckbox : public CBaseUICheckbox
{
public:
	VSControlBarCheckbox(float xPos, float yPos, float xSize, float ySize, UString name, UString text) : CBaseUICheckbox(xPos, yPos, xSize, ySize, name, text) {;}
	virtual ~VSControlBarCheckbox() {;}

	virtual void draw(Graphics *g)
	{
		if (!m_bVisible) return;

		const Color top = (m_bChecked ? COLOR(255, 178, 237, 171) : COLOR(255, 244, 244, 244));
		const Color bottom = (m_bChecked ? COLOR(255, 117, 211, 114) : COLOR(255, 221, 221, 221));

		g->fillGradient(m_vPos.x + 1, m_vPos.y + 1, m_vSize.x - 1, m_vSize.y, top, top, bottom, bottom);

		g->setColor(COLOR(255, 204, 204, 204));
		g->drawRect(m_vPos.x, m_vPos.y, m_vSize.x, m_vSize.y);

		drawText(g);
	}
};



VSControlBar::VSControlBar(int x, int y, int xSize, int ySize, McFont *font) : CBaseUIElement(x, y, xSize, ySize, "")
{
	vs_volume.setCallback(fastdelegate::MakeDelegate(this, &VSControlBar::onVolumeChanged));

	const float dpiScale = env->getDPIScale();
	const int height = 22 * dpiScale;
	const Color textColor = COLOR(215, 55, 55, 55);

	m_container = new CBaseUIContainer(0, 0, engine->getScreenWidth(), engine->getScreenHeight(), "");

	m_play = new VSControlBarButton(m_vSize.x/2.0f - height, 0, height*2, (height*2), "", ">");
	m_play->setTextColor(textColor);
	m_play->setFont(font);
	m_container->addBaseUIElement(m_play);

	m_prev = new VSControlBarButton(m_play->getRelPos().x - m_play->getSize().x, 0, height*2, (height*2), "", "<<");
	m_prev->setTextColor(textColor);
	m_prev->setFont(font);
	m_container->addBaseUIElement(m_prev);

	m_next = new VSControlBarButton(m_play->getRelPos().x + m_play->getSize().x, 0, height*2, (height*2), "", ">>");
	m_next->setTextColor(textColor);
	m_next->setFont(font);
	m_container->addBaseUIElement(m_next);

	m_volume = new VSControlBarSlider(0, 0, m_prev->getRelPos().x, height*2, "");
	m_volume->setOrientation(true);
	m_volume->setDrawBackground(false);
	m_volume->setFrameColor(COLOR(255, 204, 204, 204));
	m_volume->setBackgroundColor(0xffffffff);
	m_volume->setBounds(0.0f, 1.0f);
	m_volume->setInitialValue(vs_volume.getFloat());
	m_volume->setLiveUpdate(true);
	m_container->addBaseUIElement(m_volume);

	m_info = new VSControlBarButton(m_next->getRelPos().x + m_next->getSize().x, 0, m_vSize.x - 2*height - (m_next->getRelPos().x + m_next->getSize().x), height*2, "", "");
	m_info->setTextColor(textColor);
	m_info->setFont(font);
	m_info->setTextLeft(true);
	m_container->addBaseUIElement(m_info);

	m_shuffle = new VSControlBarCheckbox(m_vSize.x-height, 0, height, height, "", "");
	m_shuffle->setChangeCallback(fastdelegate::MakeDelegate(this, &VSControlBar::onShuffleCheckboxChanged));
	m_shuffle->setTextColor(textColor);
	m_shuffle->setFont(font);
	m_container->addBaseUIElement(m_shuffle);

	m_repeat = new VSControlBarCheckbox(m_vSize.x-2*height, 0, height, height, "", "r");
	m_repeat->setChangeCallback(fastdelegate::MakeDelegate(this, &VSControlBar::onRepeatCheckboxChanged));
	m_repeat->setTextColor(textColor);
	m_repeat->setFont(font);
	m_container->addBaseUIElement(m_repeat);

	m_eq = new VSControlBarCheckbox(m_vSize.x-2*height, height, height, height, "", "");
	m_eq->setTextColor(textColor);
	m_eq->setFont(font);
	m_container->addBaseUIElement(m_eq);

	m_settings = new VSControlBarCheckbox(m_vSize.x-height, height, height, height, "", "");
	m_settings->setTextColor(textColor);
	m_settings->setFont(font);
	m_container->addBaseUIElement(m_settings);
}

VSControlBar::~VSControlBar()
{
	SAFE_DELETE(m_container);
}

void VSControlBar::draw(Graphics *g)
{
	if (!m_bVisible) return;

	// draw background gradient
	{
		const Color top = COLOR(255, 244, 244, 244);
		const Color bottom = COLOR(255, 221, 221, 221);

		g->fillGradient(m_vPos.x, m_vPos.y, m_vSize.x, m_vSize.y, top, top, bottom, bottom);
	}

	m_container->draw(g);
}

void VSControlBar::update()
{
	CBaseUIElement::update();
	if (!m_bVisible) return;

	m_container->update();
}

void VSControlBar::onRepeatCheckboxChanged(CBaseUICheckbox *box)
{
	vs_repeat.setValue((float)box->isChecked());
}

void VSControlBar::onShuffleCheckboxChanged(CBaseUICheckbox *box)
{
	vs_shuffle.setValue((float)box->isChecked());
}

void VSControlBar::onVolumeChanged(UString oldValue, UString newValue)
{
	m_volume->setValue(vs_volume.getFloat());
}

void VSControlBar::onMoved()
{
	m_container->setPos(m_vPos);
}

void VSControlBar::onFocusStolen()
{
	// forward
	m_container->stealFocus();
}

void VSControlBar::onEnabled()
{
	// forward
	m_container->setEnabled(true);
}

void VSControlBar::onDisabled()
{
	// forward
	m_container->setEnabled(false);
}

void VSControlBar::onResized()
{
	m_play->setRelPosX(m_vSize.x/2 - m_play->getSize().x/2);
	m_prev->setRelPosX(m_play->getRelPos().x - m_prev->getSize().x);
	m_next->setRelPosX(m_play->getRelPos().x + m_play->getSize().x);
	m_info->setRelPosX(m_next->getRelPos().x + m_next->getSize().x);
	m_volume->setSizeX(m_prev->getRelPos().x);

	m_shuffle->setRelPosX(m_vSize.x - m_shuffle->getSize().x);
	m_repeat->setRelPosX(m_vSize.x - 2*m_repeat->getSize().x);
	m_eq->setRelPos(m_vSize.x - 2*m_eq->getSize().x, m_eq->getSize().y);
	m_settings->setRelPos(m_vSize.x - m_settings->getSize().x, m_settings->getSize().y);

	m_info->setSizeX(m_repeat->getRelPos().x - (m_next->getRelPos().x + m_next->getSize().x) + 1); // +1 fudge

	m_container->setSize(m_vSize);
}


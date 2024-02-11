//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		a not so simple textbox, revision 4
//
// $NoKeywords: $
//===============================================================================//

// TODO: support SHIFT + LEFT/RIGHT selection adjustments
// TODO: support CTRL + LEFT/RIGHT word caret jumping (to next space)
// TODO: support both SHIFT + CTRL + LEFT/RIGHT selection word jumping
// TODO: make scrolling anims fps independent

#include "CBaseUITextbox.h"

#include "Engine.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Cursors.h"
#include "ContextMenu.h"
#include "ResourceManager.h"
#include "ConVar.h"

ConVar ui_textbox_caret_blink_time("ui_textbox_caret_blink_time", 0.5f);
ConVar ui_textbox_text_offset_x("ui_textbox_text_offset_x", 3);

CBaseUITextbox::CBaseUITextbox(float xPos, float yPos, float xSize, float ySize, UString name) : CBaseUIElement(xPos, yPos, xSize, ySize, name)
{
	setKeepActive(true);

	m_font = engine->getResourceManager()->getFont("FONT_DEFAULT");

	m_textColor = m_frameColor = m_caretColor = 0xffffffff;
	m_backgroundColor = 0xff000000;
	m_frameBrightColor = 0;
	m_frameDarkColor = 0;

	m_bDrawFrame = true;
	m_bDrawBackground = true;

	m_iTextJustification = 0;
	m_iTextAddX = ui_textbox_text_offset_x.getInt();
	m_iTextAddY = 0;
	m_fTextScrollAddX = 0;
	m_iSelectX = 0;
	m_iCaretX = 0;
	m_iCaretWidth = 2;

	m_bHitenter = false;
	m_bContextMouse = false;
	m_bBlockMouse = false;
	m_bCatchMouse = false;
	m_bSelectCheck = false;
	m_iSelectStart = 0;
	m_iSelectEnd = 0;

	m_bLine = false;
	m_fLinetime = 0.0f;
	m_fTextWidth = 0.0f;
	m_iCaretPosition = 0;
}

void CBaseUITextbox::draw(Graphics *g)
{
	if (!m_bVisible) return;

	const float dpiScale = ((float)m_font->getDPI() / 96.0f); // NOTE: abusing font dpi

	// draw background
	if (m_bDrawBackground)
	{
		g->setColor(m_backgroundColor);
		g->fillRect(m_vPos.x + 1, m_vPos.y + 1, m_vSize.x - 1, m_vSize.y - 1);
	}

	// draw base frame
	if (m_bDrawFrame)
	{
		if (m_frameDarkColor != 0 || m_frameBrightColor != 0)
			g->drawRect(m_vPos.x, m_vPos.y, m_vSize.x, m_vSize.y, m_frameDarkColor, m_frameBrightColor, m_frameBrightColor, m_frameDarkColor);
		else
		{
			g->setColor(m_frameColor);
			g->drawRect(m_vPos.x, m_vPos.y, m_vSize.x, m_vSize.y);
		}
	}

	// draw text

	if (m_font == NULL) return;

	g->pushClipRect(McRect(m_vPos.x + 1, m_vPos.y + 1, m_vSize.x - 1, m_vSize.y));
	{
		// draw selection box
		if (hasSelectedText())
		{
			const int xpos1 = m_vPos.x + m_iTextAddX + m_iCaretX + m_fTextScrollAddX;
			const int xpos2 = m_vPos.x + m_iSelectX + m_iTextAddX + m_fTextScrollAddX;

			g->setColor(0xff56bcff);
			if (xpos1 > xpos2)
				g->fillRect(xpos2, m_vPos.y + 1, xpos1 - xpos2 + 2, m_vSize.y - 1);
			else
				g->fillRect(xpos1, m_vPos.y + 1, xpos2 - xpos1, m_vSize.y - 1);
		}

		drawText(g);

		// draw caret
		if (m_bActive && m_bLine)
		{
			const int caretWidth = std::round((float)m_iCaretWidth * dpiScale);
			const int height = std::round(m_vSize.y - 2*3 * dpiScale);
			const float yadd = std::round(height / 2.0f);

			g->setColor(m_caretColor);
			g->fillRect((int)(m_vPos.x + m_iTextAddX + m_iCaretX + m_fTextScrollAddX), (int)(m_vPos.y + m_vSize.y/2.0f - yadd), caretWidth, height);
		}
	}
	g->popClipRect();
}

void CBaseUITextbox::drawText(Graphics *g)
{
	g->setColor(m_textColor);
	g->pushTransform();
	{
		g->translate((int)(m_vPos.x + m_iTextAddX + m_fTextScrollAddX), (int)(m_vPos.y + m_iTextAddY));
		g->drawString(m_font, m_sText);
	}
	g->popTransform();
}

void CBaseUITextbox::update()
{
	CBaseUIElement::update();
	if (!m_bVisible) return;

	const Vector2 mousepos = engine->getMouse()->getPos();
	const bool mleft = engine->getMouse()->isLeftDown();
	const bool mright = engine->getMouse()->isRightDown();

	// HACKHACK: should do this with the proper events! this will only work properly though if we can event.consume() charDown's
	if (!m_bEnabled && m_bActive && mleft && !m_bMouseInside)
		m_bActive = false;

	if ((m_bMouseInside || (m_bBusy && (mleft || mright))) && (m_bActive || (!mleft && !mright)) && m_bEnabled)
		engine->getMouse()->setCursorType(CURSORTYPE::CURSOR_TEXT);

	// update caret blinking
	{
		if (!m_bActive)
		{
			m_bLine = true;
			m_fLinetime = engine->getTime();
		}
		else
		{
			if ((engine->getTime() - m_fLinetime) >= ui_textbox_caret_blink_time.getFloat() && !m_bLine)
			{
				m_bLine = true;
				m_fLinetime = engine->getTime();
			}

			if ((engine->getTime() - m_fLinetime) >= ui_textbox_caret_blink_time.getFloat() && m_bLine)
			{
				m_bLine = false;
				m_fLinetime = engine->getTime();
			}
		}
	}

	// handle mouse input
	{
		if (!m_bMouseInside && mleft && !m_bActive)
			m_bBlockMouse = true;

		if (m_bMouseInside && (mleft || mright) && m_bActive)
		{
			m_bCatchMouse = true;
			tickCaret();

			if (mright)
				m_bContextMouse = true;
		}

		if (!mleft)
		{
			m_bCatchMouse = false;
			m_bBlockMouse = false;
			m_bSelectCheck = false;
		}
	}

	// handle selecting and scrolling
	if (m_bBusy && m_bActive && (mleft || mright) && !m_bBlockMouse && m_sText.length() > 0)
	{
		tickCaret();

		const int mouseX = mousepos.x - m_vPos.x;

		// handle scrolling
		if (mleft)
		{
			if (m_fTextWidth > m_vSize.x)
			{
				if (mouseX < m_vSize.x*0.15f)
				{
					const int scrollspeed = mouseX < 0 ? std::abs(mouseX)/2 + 1 : 3;

					// TODO: animations which don't suck for usability
					m_fTextScrollAddX = clamp<int>(m_fTextScrollAddX + scrollspeed, 0, m_fTextWidth - m_vSize.x + ui_textbox_text_offset_x.getInt()*2);
					///animation->moveSmoothEnd(&m_fTextScrollAddX, clampi(m_fTextScrollAddX+scrollspeed, 0, m_fTextWidth-m_vSize.x+ui_textbox_text_offset_x.getInt()*2), 1);
				}

				if (mouseX > m_vSize.x*0.85f)
				{
					const int scrollspeed = mouseX > m_vSize.x ? std::abs(mouseX - m_vSize.x)/2 + 1 : 1;

					// TODO: animations which don't suck for usability
					m_fTextScrollAddX = clamp<int>(m_fTextScrollAddX - scrollspeed, 0, m_fTextWidth - m_vSize.x + ui_textbox_text_offset_x.getInt()*2);
					///animation->moveSmoothEnd(&m_fTextScrollAddX, clampi(m_fTextScrollAddX-scrollspeed, 0, m_fTextWidth-m_vSize.x+ui_textbox_text_offset_x.getInt()*2), 1);
				}
			}

			// handle selecting begin, once per grab
			if (!m_bSelectCheck)
			{
				m_bSelectCheck = true;
				for (int i=0; i<=m_sText.length(); i++)
				{
					const float curGlyphWidth = m_font->getStringWidth(m_sText.substr(i-1 > 0 ? i-1 : 0, 1)) / 2;
					if (mouseX >= m_font->getStringWidth(m_sText.substr(0, i)) + m_iTextAddX + m_fTextScrollAddX - curGlyphWidth)
						m_iSelectStart = i;
				}
				m_iSelectX = m_font->getStringWidth(m_sText.substr(0, m_iSelectStart));
			}

			// handle selecting end
			m_iSelectEnd = 0;
			for (int i=0; i<=m_sText.length(); i++)
			{
				const float curGlyphWidth = m_font->getStringWidth(m_sText.substr(i-1 > 0 ? i-1 : 0, 1)) / 2;
				if (mouseX >= m_font->getStringWidth(m_sText.substr(0, i)) + m_iTextAddX + m_fTextScrollAddX - curGlyphWidth)
					m_iSelectEnd = i;
			}
			m_iCaretPosition = m_iSelectEnd;
		}
		else
		{
			if (!hasSelectedText())
			{
				for (int i=0; i<=m_sText.length(); i++)
				{
					const float curGlyphWidth = m_font->getStringWidth(m_sText.substr(i-1 > 0 ? i-1 : 0, 1)) / 2;
					if (mouseX >= m_font->getStringWidth(m_sText.substr(0, i)) + m_iTextAddX + m_fTextScrollAddX - curGlyphWidth)
						m_iCaretPosition = i;
				}
			}
		}

		// update caretx
		updateCaretX();
	}
	
	// handle context menu
	if (!mright && m_bContextMouse && isMouseInside())
	{
		m_bContextMouse = false;
		/*
		engine->getMouse()->setCursorType(CURSORTYPE::CURSOR_NORMAL);
		cmenu->begin();
		{
			cmenu->addItem("Clear", 5);
			cmenu->addSeparator();
			cmenu->addItem("Paste", 4);

			if (hasSelectedText())
			{
				cmenu->addItem("Copy", 3);
				cmenu->addItem("Cut", 2);
				cmenu->addSeparator();
				cmenu->addItem("Delete", 1);
			}
		}
		const int item = cmenu->end();

		switch (item)
		{
		case 5: // clear
			clear();
			break;

		case 4: // paste
			handleDeleteSelectedText();
			insertTextFromClipboard();
			break;

		case 3: // copy
			//envDebugLog("selected copy text: %s\n",getSelectedText().toUtf8());
			env->setClipBoardText(getSelectedText());
			break;

		case 2: // cut
			env->setClipBoardText(getSelectedText());
			handleDeleteSelectedText();
			//envDebugLog("selected cut text: %s\n",getSelectedText().toUtf8());
			break;

		case 1: // delete
			handleDeleteSelectedText();
			break;
		}
		*/
	}
}

void CBaseUITextbox::onKeyDown(KeyboardEvent &e)
{
	if (!m_bActive || !m_bVisible) return;

	e.consume();

	switch (e.getKeyCode())
	{
	case KEY_DELETE:
		if (m_sText.length() > 0)
		{
			if (hasSelectedText())
				handleDeleteSelectedText();
			else if (m_iCaretPosition < m_sText.length())
			{
				m_sText.erase(m_iCaretPosition, 1);

				setText(m_sText);
				updateTextPos();
			}
		}
		tickCaret();
		break;

	case KEY_RETURN:
		m_bHitenter = true;
		break;

	case KEY_ESCAPE:
		m_bActive = false;
		break;

	case KEY_BACKSPACE:
		if (m_sText.length() > 0)
		{
			if (hasSelectedText())
				handleDeleteSelectedText();
			else if (m_iCaretPosition-1 >= 0)
			{
				if (engine->getKeyboard()->isControlDown())
				{
					// delete everything from the current caret position to the left, until after the first non-space character (but including it)
					bool foundNonSpaceChar = false;
					while (m_sText.length() > 0 && (m_iCaretPosition - 1) >= 0)
					{
						UString curChar = m_sText.substr(m_iCaretPosition - 1, 1);

						if (foundNonSpaceChar && curChar.isWhitespaceOnly())
							break;

						if (!curChar.isWhitespaceOnly())
							foundNonSpaceChar = true;

						m_sText.erase(m_iCaretPosition - 1, 1);
						m_iCaretPosition--;
					}
				}
				else
				{
					m_sText.erase(m_iCaretPosition - 1, 1);
					m_iCaretPosition--;
				}

				setText(m_sText);
				updateTextPos();
			}
		}
		tickCaret();
		break;

	case KEY_LEFT:
		{
			const bool hadSelectedText = hasSelectedText();
			const int prevSelectPos = std::min(m_iSelectStart, m_iSelectEnd);

			deselectText();

			if (!hadSelectedText)
				m_iCaretPosition = clamp<int>(m_iCaretPosition - 1, 0, m_sText.length());
			else
				m_iCaretPosition = clamp<int>(prevSelectPos, 0, m_sText.length());

			tickCaret();
			handleCaretKeyboardMove();
			updateCaretX();
		}
		break;

	case KEY_RIGHT:
		{
			const bool hadSelectedText = hasSelectedText();
			const int prevSelectPos = std::max(m_iSelectStart, m_iSelectEnd);

			deselectText();

			if (!hadSelectedText)
				m_iCaretPosition = clamp<int>(m_iCaretPosition + 1, 0, m_sText.length());
			else
				m_iCaretPosition = clamp<int>(prevSelectPos, 0, m_sText.length());

			tickCaret();
			handleCaretKeyboardMove();
			updateCaretX();
		}
		break;

	case KEY_C:
		if (engine->getKeyboard()->isControlDown())
			env->setClipBoardText(getSelectedText());
		break;

	case KEY_V:
		if (engine->getKeyboard()->isControlDown())
			insertTextFromClipboard();
		break;

	case KEY_A:
		if (engine->getKeyboard()->isControlDown())
		{
			// HACKHACK: make proper setSelectedText() function
			m_iSelectStart = 0;
			m_iSelectEnd = m_sText.length();

			m_iCaretPosition = m_iSelectEnd;
			m_iSelectX = m_font->getStringWidth(m_sText);
			m_iCaretX = 0;
			m_fTextScrollAddX = m_fTextWidth < m_vSize.x ? 0 : m_fTextWidth - m_vSize.x + ui_textbox_text_offset_x.getInt()*2;
		}
		break;

	case KEY_X:
		if (engine->getKeyboard()->isControlDown() && hasSelectedText())
		{
			env->setClipBoardText(getSelectedText());
			handleDeleteSelectedText();
		}
		break;

	case KEY_HOME:
		deselectText();
		m_iCaretPosition = 0;
		tickCaret();
		handleCaretKeyboardMove();
		updateCaretX();
		break;

	case KEY_END:
		deselectText();
		m_iCaretPosition = m_sText.length();
		tickCaret();
		handleCaretKeyboardMove();
		updateCaretX();
		break;
	}
}

void CBaseUITextbox::onChar(KeyboardEvent &e)
{
	if (!m_bActive || !m_bVisible) return;

	e.consume();

	// ignore any control characters, we only want text
	// funny story: Windows 10 still has this bug even today, where when editing the name of any shortcut/folder on the desktop, hitting CTRL + BACKSPACE will insert an invalid character
	if (e.getCharCode() < 32 || (engine->getKeyboard()->isControlDown() && !engine->getKeyboard()->isAltDown())) return;

	// delete any potentially selected text
	handleDeleteSelectedText();

	// add the pressed letter to the text
	{
		const KEYCODE charCode = e.getCharCode();

		m_sText.insert(m_iCaretPosition, (wchar_t)charCode);
		m_iCaretPosition++;

		setText(m_sText);
	}

	tickCaret();
}

void CBaseUITextbox::handleCaretKeyboardMove()
{
	const int caretPosition = m_iTextAddX + m_font->getStringWidth(m_sText.substr(0, m_iCaretPosition)) + m_fTextScrollAddX;
	if (caretPosition < 0)
		m_fTextScrollAddX += std::abs(caretPosition) + ui_textbox_text_offset_x.getInt();
	else if (caretPosition > (m_vSize.x - ui_textbox_text_offset_x.getInt()))
		m_fTextScrollAddX -= std::abs(caretPosition-m_vSize.x) + ui_textbox_text_offset_x.getInt();
}

void CBaseUITextbox::handleCaretKeyboardDelete()
{
	if (m_fTextWidth > m_vSize.x)
	{
		const int caretPosition = m_iTextAddX + m_font->getStringWidth(m_sText.substr(0, m_iCaretPosition)) + m_fTextScrollAddX;
		if (caretPosition < (m_vSize.x - ui_textbox_text_offset_x.getInt()))
			m_fTextScrollAddX += std::abs(m_vSize.x - ui_textbox_text_offset_x.getInt() - caretPosition);
	}
}

void CBaseUITextbox::tickCaret()
{
	m_bLine = true;
	m_fLinetime = engine->getTime();
}

bool CBaseUITextbox::hitEnter()
{
	if (m_bHitenter)
	{
		m_bHitenter = false;
		return true;
	}
	else
		return false;
}

bool CBaseUITextbox::hasSelectedText() const
{
	return ((m_iSelectStart - m_iSelectEnd) != 0);
}

void CBaseUITextbox::clear()
{
	deselectText();
	setText("");
}

CBaseUITextbox *CBaseUITextbox::setFont(McFont *font)
{
	m_font = font;
	setText(m_sText);

	return this;
}

CBaseUITextbox *CBaseUITextbox::setText(UString text)
{
	m_sText = text;
	m_iCaretPosition = clamp<int>(m_iCaretPosition, 0, text.length());

	// handle text justification
	m_fTextWidth = m_font->getStringWidth(m_sText);
	switch(m_iTextJustification)
	{
	case 0: // left
		m_iTextAddX = ui_textbox_text_offset_x.getInt();
		break;

	case 1: // middle
		m_iTextAddX = -(m_fTextWidth-m_vSize.x)/2.0f;
		m_iTextAddX = m_iTextAddX > 0 ? m_iTextAddX : ui_textbox_text_offset_x.getInt();
		break;

	case 2: // right
		m_iTextAddX = (m_vSize.x - m_fTextWidth) - ui_textbox_text_offset_x.getInt();
		m_iTextAddX = m_iTextAddX > 0 ? m_iTextAddX : ui_textbox_text_offset_x.getInt();
		break;
	}

	// handle over-text
	if (m_fTextWidth > m_vSize.x)
	{
		m_iTextAddX -= m_fTextWidth - m_vSize.x + ui_textbox_text_offset_x.getInt()*2;
		handleCaretKeyboardMove();
	}
	else
		m_fTextScrollAddX = 0;

	// TODO: force stop animation, it will fuck shit up
	///animation->moveSmoothEnd(&m_fTextScrollAddX, m_fTextScrollAddX, 0.1f);

	// center vertically
	const float addY = std::round(m_vSize.y/2.0f + m_font->getHeight()/2.0f);
	m_iTextAddY = addY > 0 ? addY : 0;

	updateCaretX();

	return this;
}

void CBaseUITextbox::setCursorPosRight()
{
	m_iCaretPosition = m_sText.length();
	{
		updateCaretX();
		tickCaret();
	}
	m_fTextScrollAddX = 0;
}

void CBaseUITextbox::updateCaretX()
{
	m_iCaretX = m_font->getStringWidth(m_sText.substr(0, m_iCaretPosition));
}

void CBaseUITextbox::handleDeleteSelectedText()
{
	if (!hasSelectedText()) return;

	const int selectedTextLength = (m_iSelectStart < m_iSelectEnd ? m_iSelectEnd - m_iSelectStart : m_iSelectStart - m_iSelectEnd);
	m_sText.erase(m_iSelectStart < m_iSelectEnd ? m_iSelectStart : m_iSelectEnd, selectedTextLength);

	if (m_iSelectEnd > m_iSelectStart)
		m_iCaretPosition -= selectedTextLength;

	setText(m_sText);

	// scroll back if empty white space
	updateTextPos();

	deselectText();

	setText(m_sText);
}

void CBaseUITextbox::insertTextFromClipboard()
{
	const UString clipstring = env->getClipBoardText();

	/*
	debugLog("got clip string: %s\n", clipstring.toUtf8());
	for (int i=0; i<clipstring.length(); i++)
	{
		debugLog("char #%i = %i\n", i, clipstring[i]);
	}
	*/

	if (clipstring.length() > 0)
	{
		handleDeleteSelectedText();
		{
			m_sText.insert(m_iCaretPosition, clipstring);
			m_iCaretPosition = m_iCaretPosition + clipstring.length();
		}
		setText(m_sText);
	}
}

void CBaseUITextbox::updateTextPos()
{
	if (m_iTextJustification == 0)
	{
		if ((m_iTextAddX + m_fTextScrollAddX) > ui_textbox_text_offset_x.getInt())
		{
			if (hasSelectedText() && m_iCaretPosition == 0)
			{
				// TODO: animations? don't like it as it is
				m_fTextScrollAddX = ui_textbox_text_offset_x.getInt() - m_iTextAddX;
				///animation->moveSmoothEnd(&m_fTextScrollAddX, ui_textbox_text_offset_x.getInt() - m_iTextAddX, 1);
			}
			else
				m_fTextScrollAddX = ui_textbox_text_offset_x.getInt() - m_iTextAddX;
		}
	}
}

void CBaseUITextbox::deselectText()
{
	m_iSelectStart = 0;
	m_iSelectEnd = 0;
}

UString CBaseUITextbox::getSelectedText()
{
	const int selectedTextLength = (m_iSelectStart < m_iSelectEnd ? m_iSelectEnd - m_iSelectStart : m_iSelectStart - m_iSelectEnd);

	if (selectedTextLength > 0)
		return m_sText.substr(m_iSelectStart < m_iSelectEnd ? m_iSelectStart : m_iSelectEnd, selectedTextLength);
	else
		return "";
}

void CBaseUITextbox::onResized()
{
	CBaseUIElement::onResized();

	// HACKHACK: brute force fix layout
	setText(m_sText);
}

void CBaseUITextbox::onMouseDownInside()
{
	CBaseUIElement::onMouseDownInside();

	// force busy, can't drag scroll release (textbox requires full focus due to text selection)
	m_bBusy = true;
}

void CBaseUITextbox::onMouseDownOutside()
{
	CBaseUIElement::onMouseDownOutside();

	m_bBusy = false;
	m_bActive = false;

	deselectText();
}

void CBaseUITextbox::onMouseUpInside()
{
	CBaseUIElement::onMouseUpInside();

	m_bBusy = false;
}

void CBaseUITextbox::onMouseUpOutside()
{
	CBaseUIElement::onMouseUpOutside();

	m_bBusy = false;
}

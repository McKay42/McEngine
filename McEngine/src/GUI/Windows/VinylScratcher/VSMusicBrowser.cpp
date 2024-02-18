//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		a simple drive and file selector
//
// $NoKeywords: $
//===============================================================================//

#include "VSMusicBrowser.h"
#include "VinylScratcher.h"

#include "Engine.h"
#include "Keyboard.h"
#include "ConVar.h"
#include "ResourceManager.h"
#include "AnimationHandler.h"

#include "CBaseUIContainer.h"
#include "CBaseUIButton.h"
#include "CBaseUIScrollView.h"

ConVar vs_browser_animspeed("vs_browser_animspeed", 0.15f, FCVAR_NONE);



struct VSMusicBrowserNaturalSortStringComparator
{
	// heavily modified version of https://github.com/scopeInfinity/NaturalSort

	static bool isDigit(const std::string::value_type &x)
	{
		return isdigit(x);
	}
	static bool isNotDigit(const std::string::value_type &x)
	{
		return !isDigit(x);
	}

	static bool compareCharactersLessThanIgnoreCase(const std::string::value_type &lhs, const std::string::value_type &rhs)
	{
		return (tolower(lhs) < tolower(rhs));
	}

	static int compareIteratorsLessThanIgnoreCase(const std::string::const_iterator &lhs, const std::string::const_iterator &rhs)
	{
		if (compareCharactersLessThanIgnoreCase(*lhs, *rhs))
			return -1;
		if (compareCharactersLessThanIgnoreCase(*rhs, *lhs))
			return 1;

		return 0;
	}

	static int compareNumbers(std::string::const_iterator lhsBegin, std::string::const_iterator lhsEnd, bool isFractionalPart1, std::string::const_iterator rhsBegin, std::string::const_iterator rhsEnd, bool isFractionalPart2)
	{
		if (isFractionalPart1 && !isFractionalPart2)
			return true;
		if (!isFractionalPart1 && isFractionalPart2)
			return false;

		if (isFractionalPart1)
		{
			while (lhsBegin < lhsEnd && rhsBegin < rhsEnd)
			{
				const int result = compareIteratorsLessThanIgnoreCase(lhsBegin, rhsBegin);
				if (result != 0)
					return result;

				lhsBegin++;
				rhsBegin++;
			}

			// skip intermediate zeroes
			while (lhsBegin < lhsEnd && *lhsBegin == '0')
			{
				lhsBegin++;
			}
			while (rhsBegin < rhsEnd && *rhsBegin == '0')
			{
				rhsBegin++;
			}

			if (lhsBegin == lhsEnd && rhsBegin != rhsEnd)
				return -1;
			else if (lhsBegin != lhsEnd && rhsBegin == rhsEnd)
				return 1;
			else
				return 0;
		}
		else
		{
			// skip initial zeroes
			while (lhsBegin < lhsEnd && *lhsBegin == '0')
			{
				lhsBegin++;
			}
			while (rhsBegin < rhsEnd && *rhsBegin == '0')
			{
				rhsBegin++;
			}

			// compare length of both strings
			if (lhsEnd - lhsBegin < rhsEnd - rhsBegin)
				return -1;
			if (lhsEnd - lhsBegin > rhsEnd - rhsBegin)
				return 1;

			// (equal in length, continue as normal)
			while (lhsBegin < lhsEnd)
			{
				const int result = compareIteratorsLessThanIgnoreCase(lhsBegin, rhsBegin);
				if (result != 0)
					return result;

				lhsBegin++;
				rhsBegin++;
			}

			return 0;
		}
	}

	static bool compareStringsNatural(const std::string &first, const std::string &second)
	{
		const std::string::const_iterator &lhsBegin = first.begin();
		const std::string::const_iterator &lhsEnd = first.end();
		const std::string::const_iterator &rhsBegin = second.begin();
		const std::string::const_iterator &rhsEnd = second.end();

		std::string::const_iterator current1 = lhsBegin;
		std::string::const_iterator current2 = rhsBegin;

		bool foundSpace1 = false;
		bool foundSpace2 = false;

		while (current1 != lhsEnd && current2 != rhsEnd)
		{
			// ignore more than one continuous space character
			{
				while (foundSpace1 && current1 != lhsEnd && *current1 == ' ')
				{
					current1++;
				}
				foundSpace1 = (*current1 == ' ');

				while (foundSpace2 && current2 != rhsEnd && *current2 == ' ')
				{
					current2++;
				}
				foundSpace2 = (*current2 == ' ');
			}

			if (!isDigit(*current1) || !isDigit(*current2))
			{
				// normal comparison for non-digit characters
				if (compareCharactersLessThanIgnoreCase(*current1, *current2))
					return true;
				if (compareCharactersLessThanIgnoreCase(*current2, *current1))
					return false;

				current1++;
				current2++;
			}
			else
			{
				// comparison for digit characters (including well formed fractions)
				std::string::const_iterator lastNonDigit1 = std::find_if(current1, lhsEnd, isNotDigit);
				std::string::const_iterator lastNonDigit2 = std::find_if(current2, rhsEnd, isNotDigit);

				const int result = compareNumbers(current1, lastNonDigit1, (current1 > lhsBegin && *(current1 - 1) == '.'),
												  current2, lastNonDigit2, (current2 > rhsBegin && *(current2 - 1) == '.'));
				if (result < 0)
					return true;
				if (result > 0)
					return false;

				current1 = lastNonDigit1;
				current2 = lastNonDigit2;
			}
		}

		if (current1 == lhsEnd && current2 == rhsEnd)
			return false;
		else
			return (current1 == lhsEnd);
	}

	bool operator() (UString const &a, UString const &b) const
	{
		const std::string aa = a.toUtf8();
		const std::string bb = b.toUtf8();

		return compareStringsNatural(aa, bb);
	}
};



class VSMusicBrowserButton : public CBaseUIButton
{
public:
	VSMusicBrowserButton(float xPos, float yPos, float xSize, float ySize, UString name, UString text) : CBaseUIButton(xPos, yPos, xSize, ySize, name, text)
	{
		m_bSelected = false;
		m_bIsDirectory = false;
		m_bPlaying = false;

		m_fSelectionAnim = 0.0f;
	}

	virtual ~VSMusicBrowserButton()
	{
		anim->deleteExistingAnimation(&m_fSelectionAnim);
	}

	virtual void draw(Graphics *g)
	{
		if (!m_bVisible) return;

		const bool isAnimatingSelectionAnim = anim->isAnimating(&m_fSelectionAnim);

		// draw regular line
		if (!m_bSelected && !isAnimatingSelectionAnim)
		{
			g->setColor(m_frameColor);
			g->drawLine(m_vPos.x + m_vSize.x, m_vPos.y, m_vPos.x + m_vSize.x, m_vPos.y + m_vSize.y);
			g->drawLine(m_vPos.x + m_vSize.x  - 1, m_vPos.y, m_vPos.x + m_vSize.x - 1, m_vPos.y + m_vSize.y);
		}

		// draw animated arrow and/or animated splitting line
		// NOTE: the arrow relies on the height being evenly divisible by 2 in order for the lines to not appear crooked/aliased
		if (m_bSelected || isAnimatingSelectionAnim)
		{
			g->setColor(m_frameColor);
			g->setAlpha(m_fSelectionAnim * m_fSelectionAnim);

			g->pushClipRect(McRect(m_vPos.x + m_vSize.x/2.0f, m_vPos.y, m_vSize.x/2.0f + 2, m_vSize.y + 1));
			{
				float xAdd = (1.0f - m_fSelectionAnim) * (m_vSize.y/2.0f);

				g->drawLine(m_vPos.x + m_vSize.x + xAdd, m_vPos.y, m_vPos.x + m_vSize.x - m_vSize.y/2.0f + xAdd, m_vPos.y + m_vSize.y/2.0f);
				g->drawLine(m_vPos.x + m_vSize.x - m_vSize.y/2.0f + xAdd, m_vPos.y + m_vSize.y/2.0f, m_vPos.x + m_vSize.x + xAdd, m_vPos.y + m_vSize.y);

				g->drawLine(m_vPos.x + m_vSize.x - 1 + xAdd, m_vPos.y, m_vPos.x + m_vSize.x - m_vSize.y/2.0f - 1 + xAdd, m_vPos.y + m_vSize.y/2.0f);
				g->drawLine(m_vPos.x + m_vSize.x - m_vSize.y/2.0f - 1 + xAdd, m_vPos.y + m_vSize.y/2.0f, m_vPos.x + m_vSize.x - 1 + xAdd, m_vPos.y + m_vSize.y);

				if (m_fSelectionAnim < 1.0f)
				{
					g->setAlpha(1.0f);

					g->drawLine(m_vPos.x + m_vSize.x, m_vPos.y, m_vPos.x + m_vSize.x, m_vPos.y + (m_vSize.y/2.0f)*(1.0f - m_fSelectionAnim));
					g->drawLine(m_vPos.x + m_vSize.x - 1, m_vPos.y, m_vPos.x + m_vSize.x - 1, m_vPos.y + (m_vSize.y/2.0f)*(1.0f - m_fSelectionAnim));

					g->drawLine(m_vPos.x + m_vSize.x, m_vPos.y + m_vSize.y/2.0f + (m_vSize.y/2.0f)*(m_fSelectionAnim), m_vPos.x + m_vSize.x, m_vPos.y + m_vSize.y);
					g->drawLine(m_vPos.x + m_vSize.x - 1, m_vPos.y + m_vSize.y/2.0f + (m_vSize.y/2.0f)*(m_fSelectionAnim), m_vPos.x + m_vSize.x - 1, m_vPos.y + m_vSize.y);
				}
			}
			g->popClipRect();
		}

		// highlight currently playing
		if (m_bPlaying)
		{
			float alpha = std::abs(std::sin(engine->getTime()*3));

			// don't animate unnecessarily while not in focus/foreground
			if (!engine->hasFocus())
				alpha = 0.75f;

			g->setColor(COLOR((int)(alpha*50.0f), 0,196,223));
			g->fillRect(m_vPos.x, m_vPos.y - 1, m_vSize.x, m_vSize.y + 2);
		}

		//g->setColor(0xffff0000);
		//g->drawRect(m_vPos.x, m_vPos.y, m_vSize.x, m_vSize.y);

		drawText(g);
	}

	void setSelected(bool selected)
	{
		if (selected && !m_bSelected)
			anim->moveQuadInOut(&m_fSelectionAnim, 1.0f, vs_browser_animspeed.getFloat(), 0.0f, true);
		else if (!selected)
			anim->moveQuadInOut(&m_fSelectionAnim, 0.0f, vs_browser_animspeed.getFloat(), 0.0f, true);

		m_bSelected = selected;
	}

	void setDirectory(bool directory) {m_bIsDirectory = directory;}
	void setPlaying(bool playing) {m_bPlaying = playing;}

	inline bool isDirectory() const {return m_bIsDirectory;}

private:
	bool m_bSelected;
	bool m_bIsDirectory;
	bool m_bPlaying;

	float m_fSelectionAnim;
};

class VSMusicBrowserColumnScrollView : public CBaseUIScrollView
{
public:
	VSMusicBrowserColumnScrollView(float xPos, float yPos, float xSize, float ySize, UString name) : CBaseUIScrollView(xPos, yPos, xSize, ySize, name)
	{
		m_fAnim = 0.0f;

		// spawn animation
		anim->moveQuadInOut(&m_fAnim, 1.0f, vs_browser_animspeed.getFloat(), 0.0f, true);
	}

	virtual ~VSMusicBrowserColumnScrollView()
	{
		anim->deleteExistingAnimation(&m_fAnim);
	}

	virtual void draw(Graphics *g)
	{
		if (anim->isAnimating(&m_fAnim))
		{
			g->push3DScene(McRect(m_vPos.x, m_vPos.y, m_vSize.x, m_vSize.y));
			{
				g->offset3DScene(-m_vSize.x/2, 0, 0);
				g->rotate3DScene(0, 100 - m_fAnim*100, 0);
				CBaseUIScrollView::draw(g);
			}
			g->pop3DScene();
		}
		else
			CBaseUIScrollView::draw(g);
	}

private:
	float m_fAnim;
};



VSMusicBrowser::VSMusicBrowser(int x, int y, int xSize, int ySize, McFont *font) : CBaseUIElement(x, y, xSize, ySize, "")
{
	m_font = font;

	m_defaultTextColor = COLOR(215, 55, 55, 55);
	m_playingTextBrightColor = COLOR(255, 0, 196, 223);
	m_playingTextDarkColor = COLOR(150, 0, 80, 130);

	m_mainContainer = new CBaseUIScrollView(x, y, xSize, ySize, "");
	m_mainContainer->setDrawBackground(false);
	m_mainContainer->setDrawFrame(false);
	m_mainContainer->setVerticalScrolling(false);
	m_mainContainer->setScrollbarColor(0x99666666);

	updateDrives();
}

VSMusicBrowser::~VSMusicBrowser()
{
	SAFE_DELETE(m_mainContainer);
}

void VSMusicBrowser::draw(Graphics *g)
{
	m_mainContainer->draw(g);
}

void VSMusicBrowser::update()
{
	CBaseUIElement::update();
	if (!m_bVisible) return;

	m_mainContainer->update();
}

void VSMusicBrowser::onButtonClicked(CBaseUIButton *button)
{
	VSMusicBrowserButton *btn = dynamic_cast<VSMusicBrowserButton*>(button);
	if (btn == NULL) return; // sanity

	// set flags / next song
	if (btn->isDirectory())
		btn->setSelected(true);
	else
	{
		m_previousActiveSong = m_activeSong;
		m_activeSong = btn->getName();

		btn->setPlaying(true);
		btn->setTextDarkColor(m_playingTextDarkColor);
		btn->setTextBrightColor(m_playingTextBrightColor);
	}

	// get column this press came from and deselect all others, also rebuild all columns > current
	bool fromInvalidSelection = true;
	for (size_t i=0; i<m_columns.size(); i++)
	{
		for (size_t b=0; b<m_columns[i].buttons.size(); b++)
		{
			if (m_columns[i].buttons[b]->getName() == btn->getName())
			{
				if (btn->isDirectory())
				{
					// rebuild it
					updateFolder(btn->getName(), i);

					// reset selection flag of all buttons in THIS column except the one we just clicked
					for (size_t r=0; r<m_columns[i].buttons.size(); r++)
					{
						if (m_columns[i].buttons[r] != btn)
							m_columns[i].buttons[r]->setSelected(false);
					}
				}
				else if (m_fileClickedCallback != NULL)
				{
					// rebuild playlist, but only if this click would result in a successful play
					if (VinylScratcher::tryPlayFile(btn->getName()))
					{
						fromInvalidSelection = false;

						m_playlist.clear();
						for (size_t p=0; p<m_columns[i].buttons.size(); p++)
						{
							if (!m_columns[i].buttons[p]->isDirectory())
								m_playlist.push_back(m_columns[i].buttons[p]->getName());
						}
					}

					// fire play event
					m_fileClickedCallback(btn->getName(), false);
				}

				break;
			}
		}
	}

	// update button highlight
	updatePlayingSelection(fromInvalidSelection);
}

void VSMusicBrowser::updatePlayingSelection(bool fromInvalidSelection)
{
	for (size_t i=0; i<m_columns.size(); i++)
	{
		for (size_t b=0; b<m_columns[i].buttons.size(); b++)
		{
			VSMusicBrowserButton *button = m_columns[i].buttons[b];

			if (button->getName() != m_activeSong)
			{
				button->setTextColor(m_defaultTextColor);
				button->setPlaying(false);
			}
			else
			{
				button->setTextBrightColor(m_playingTextBrightColor);
				button->setTextDarkColor(m_playingTextDarkColor);
				button->setPlaying(true);

				if (!fromInvalidSelection)
				{
					const float relativeButtonY = button->getPos().y - m_columns[i].view->getPos().y;
					if (relativeButtonY <= 0 || relativeButtonY + button->getSize().y >= m_columns[i].view->getSize().y)
						m_columns[i].view->scrollToElement(button, 0, m_columns[i].view->getSize().y/2 - button->getSize().y/2);
				}
			}
		}
	}
}

void VSMusicBrowser::updateFolder(UString baseFolder, size_t fromDepth)
{
	if (m_columns.size() < 1) return;

	// remove all columns > fromDepth
	if (fromDepth + 1 < m_columns.size())
	{
		for (size_t i=fromDepth+1; i<m_columns.size(); i++)
		{
			m_mainContainer->getContainer()->deleteBaseUIElement(m_columns[i].view);

			m_columns.erase(m_columns.begin() + i);
			i--;
		}
	}

	// create column
	COLUMN col;
	{
		const CBaseUIScrollView *previousView = m_columns[m_columns.size() - 1].view;
		const int xPos = previousView->getRelPos().x + previousView->getSize().x;

		const float dpiScale = env->getDPIScale();
		const int border = 20 * dpiScale;
		const int height = 28 * dpiScale;
		const Color frameColor = COLOR(255, 150, 150, 150);

		col.view = new VSMusicBrowserColumnScrollView(xPos, -1, 100, m_vSize.y, "");
		col.view->setScrollMouseWheelMultiplier((1/3.5f)*0.5f);
		col.view->setDrawBackground(false);
		col.view->setFrameColor(0xffff0000);
		col.view->setDrawFrame(false);
		col.view->setHorizontalScrolling(false);
		col.view->setScrollbarColor(0x99666666);

		float maxWidthCounter = 1;

		// go through the file system
		std::vector<UString> folders = env->getFoldersInFolder(baseFolder);
		std::vector<UString> files = env->getFilesInFolder(baseFolder);

		// sort both lists naturally
		std::sort(folders.begin(), folders.end(), VSMusicBrowserNaturalSortStringComparator());
		std::sort(files.begin(), files.end(), VSMusicBrowserNaturalSortStringComparator());

		// first, add all folders
		int elementCounter = 0;
		for (const UString &folder : folders)
		{
			if (folder == "." || folder == "..") continue;

			UString completeName = baseFolder;
			completeName.append(folder);
			completeName.append("/");

			VSMusicBrowserButton *folderButton = new VSMusicBrowserButton(border, border + elementCounter*height, 50 * dpiScale, height, completeName, folder);
			folderButton->setClickCallback(fastdelegate::MakeDelegate(this, &VSMusicBrowser::onButtonClicked));
			folderButton->setTextColor(m_defaultTextColor);
			folderButton->setFrameColor(frameColor);
			folderButton->setSizeToContent(12 * dpiScale, 5 * dpiScale);
			folderButton->setSizeY(height);
			folderButton->setDirectory(true);

			if (folderButton->getSize().x > maxWidthCounter)
				maxWidthCounter = folderButton->getSize().x;

			col.view->getContainer()->addBaseUIElement(folderButton);
			col.buttons.push_back(folderButton);

			elementCounter++;
		}

		// then add all files
		for (const UString &file : files)
		{
			if (file == "." || file == "..") continue;

			UString completeName = baseFolder;
			completeName.append(file);

			VSMusicBrowserButton *fileButton = new VSMusicBrowserButton(border, border + elementCounter*height, 50 * dpiScale, height, completeName, file);
			fileButton->setClickCallback(fastdelegate::MakeDelegate(this, &VSMusicBrowser::onButtonClicked));
			fileButton->setDrawBackground(false);
			fileButton->setTextColor(m_defaultTextColor);
			fileButton->setFrameColor(frameColor);
			fileButton->setSizeToContent(12 * dpiScale, 5 * dpiScale);
			fileButton->setSizeY(height);
			fileButton->setDirectory(false);

			// preemptively updatePlayingSelection() manually here, code duplication ahoy
			if (completeName == m_activeSong)
			{
				fileButton->setTextBrightColor(m_playingTextBrightColor);
				fileButton->setTextDarkColor(m_playingTextDarkColor);
				fileButton->setPlaying(true);
			}

			if (fileButton->getSize().x > maxWidthCounter)
				maxWidthCounter = fileButton->getSize().x;

			col.view->getContainer()->addBaseUIElementBack(fileButton);
			col.buttons.push_back(fileButton);

			elementCounter++;
		}

		// normalize button width
		for (size_t i=0; i<col.buttons.size(); i++)
		{
			col.buttons[i]->setSizeX(maxWidthCounter);
			col.buttons[i]->setTextLeft(true);
		}

		// update & add everything
		col.view->setSizeX(border + maxWidthCounter + 20 * dpiScale);
		col.view->setScrollSizeToContent(20 * dpiScale);
	}
	m_columns.push_back(col);
	m_mainContainer->getContainer()->addBaseUIElement(col.view);
	m_mainContainer->setScrollSizeToContent(0);
	m_mainContainer->scrollToRight();
}

void VSMusicBrowser::updateDrives()
{
	// drive selection is always at column 0
	if (m_columns.size() < 1)
	{
		COLUMN col;
		{
			col.view = new CBaseUIScrollView(-1, -1, 100, m_vSize.y, "");
			col.view->setDrawBackground(false);
			col.view->setFrameColor(0xffff0000);
			col.view->setDrawFrame(false);
			col.view->setHorizontalScrolling(false);
			col.view->setScrollbarColor(0x99666666);

			m_mainContainer->getContainer()->addBaseUIElement(col.view);
		}
		m_columns.push_back(col);
	}
	COLUMN &driveColumn = m_columns[0];

	driveColumn.view->clear();

	const float dpiScale = env->getDPIScale();
	const int border = 20 * dpiScale;
	const int height = 28 * dpiScale;
	const Color frameColor = COLOR(255, 150, 150, 150);

	float maxWidthCounter = 1;

	const std::vector<UString> drives = env->getLogicalDrives();
	for (size_t i=0; i<drives.size(); i++)
	{
		const UString &drive = drives[i];

		if (drive.length() < 1) continue; // sanity

		VSMusicBrowserButton *driveButton = new VSMusicBrowserButton(border, border + i*height, 50, height, drive, drive.substr(0, 1));
		driveButton->setTextColor(m_defaultTextColor);
		driveButton->setClickCallback(fastdelegate::MakeDelegate(this, &VSMusicBrowser::onButtonClicked));
		driveButton->setDirectory(true);
		driveButton->setDrawBackground(false);
		driveButton->setFrameColor(frameColor);
		driveButton->setFont(m_font);
		driveButton->setSizeToContent(m_font->getHeight() / 1.5f, 5 * dpiScale);
		driveButton->setTextLeft(true);
		driveButton->setSizeY(height);

		driveColumn.view->getContainer()->addBaseUIElement(driveButton);
		driveColumn.buttons.push_back(driveButton);

		if (driveButton->getSize().x > maxWidthCounter)
			maxWidthCounter = driveButton->getSize().x;
	}

	// normalize button widths
	for (size_t i=0; i<driveColumn.buttons.size(); i++)
	{
		driveColumn.buttons[i]->setSizeX(maxWidthCounter);
	}

	// update & add everything
	driveColumn.view->setSizeX(border + maxWidthCounter + 15 * dpiScale);
	driveColumn.view->setScrollSizeToContent(15 * dpiScale);
	m_mainContainer->setScrollSizeToContent(0);
}

void VSMusicBrowser::fireNextSong(bool previous)
{
	for (int i=0; i<(int)m_playlist.size(); i++)
	{
		// find current position
		if (m_playlist[i] == m_activeSong)
		{
			// look for the next playable file, if we can find one
			int breaker = 0;
			while (true)
			{
				int nextSongIndex = ((i + 1) < m_playlist.size() ? (i + 1) : 0);
				if (previous)
					nextSongIndex = ((i - 1) >= 0 ? (i - 1) : m_playlist.size() - 1);

				// if we have found the next playable file
				if (VinylScratcher::tryPlayFile(m_playlist[nextSongIndex]))
				{
					// set it, update the possibly visible button of the current song (and reset all others), and play it
					m_previousActiveSong = m_activeSong;
					m_activeSong = m_playlist[nextSongIndex];

					updatePlayingSelection();

					// fire play event
					m_fileClickedCallback(m_activeSong, previous);

					return;
				}
				else
				{
					if (previous)
						i--;
					else
						i++;

					if (previous)
					{
						if (i < 0)
							i = m_playlist.size() - 1;
					}
					else
					{
						if (i >= m_playlist.size())
							i = 0;
					}
				}
				breaker++;

				if (breaker > 2*m_playlist.size())
					return;
			}

			return;
		}
	}
}

void VSMusicBrowser::onInvalidFile()
{
	m_activeSong = m_previousActiveSong;
	updatePlayingSelection(true);
}

void VSMusicBrowser::onMoved()
{
	m_mainContainer->setPos(m_vPos);
}

void VSMusicBrowser::onResized()
{
	for (size_t i=0; i<m_columns.size(); i++)
	{
		m_columns[i].view->setSizeY(m_vSize.y);
		m_columns[i].view->scrollToY(m_columns[i].view->getScrollPosY());
	}

	m_mainContainer->setSize(m_vSize);
	m_mainContainer->setScrollSizeToContent(0);
	m_mainContainer->scrollToX(m_mainContainer->getScrollPosX());
}

void VSMusicBrowser::onFocusStolen()
{
	// forward
	m_mainContainer->stealFocus();
}

void VSMusicBrowser::onEnabled()
{
	// forward
	m_mainContainer->setEnabled(true);
}

void VSMusicBrowser::onDisabled()
{
	// forward
	m_mainContainer->setEnabled(false);
}

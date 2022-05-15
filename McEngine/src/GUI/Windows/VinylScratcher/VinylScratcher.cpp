//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		a music player with style
//
// $NoKeywords: $vs
//===============================================================================//

#include "VinylScratcher.h"

#include "Engine.h"
#include "SoundEngine.h"
#include "ResourceManager.h"
#include "AnimationHandler.h"
#include "Keyboard.h"
#include "ConVar.h"

#include "CBaseUIContainer.h"
#include "CBaseUIButton.h"
#include "CBaseUISlider.h"

#include "VSTitleBar.h"
#include "VSControlBar.h"
#include "VSMusicBrowser.h"

Sound *VinylScratcher::m_stream2 = NULL;

VinylScratcher::VinylScratcher() : CBaseUIWindow(220, 90, 1000, 700, "Vinyl Scratcher")
{
	const float dpiScale = env->getDPIScale();
	const int baseDPI = 96;
	const int newDPI = dpiScale * baseDPI;

	m_vs_percent_ref = convar->getConVarByName("vs_percent");

	McFont *font = engine->getResourceManager()->getFont("FONT_DEFAULT");
	McFont *windowTitleFont = engine->getResourceManager()->loadFont("ubuntu.ttf", "FONT_VS_WINDOW_TITLE", 10.0f, true, newDPI);
	McFont *controlBarFont = font;
	McFont *musicBrowserFont = font;

	m_titleBar = new VSTitleBar(0, 0, m_vSize.x + 2, font);
	m_controlBar = new VSControlBar(0, m_vSize.y - getTitleBarHeight() - m_titleBar->getSize().y, m_vSize.x, m_titleBar->getSize().y, controlBarFont);
	m_musicBrowser = new VSMusicBrowser(0, m_titleBar->getRelPos().y + m_titleBar->getSize().y, m_vSize.x, m_vSize.y - m_controlBar->getSize().y - m_titleBar->getSize().y - getTitleBarHeight() - 1, musicBrowserFont);

	getContainer()->addBaseUIElement(m_musicBrowser);
	getContainer()->addBaseUIElement(m_controlBar);
	getContainer()->addBaseUIElement(m_titleBar);

	m_controlBar->getPlayButton()->setClickCallback(fastdelegate::MakeDelegate(this, &VinylScratcher::onPlayClicked));
	m_controlBar->getNextButton()->setClickCallback(fastdelegate::MakeDelegate(this, &VinylScratcher::onNextClicked));
	m_controlBar->getPrevButton()->setClickCallback(fastdelegate::MakeDelegate(this, &VinylScratcher::onPrevClicked));
	m_musicBrowser->setFileClickedCallback(fastdelegate::MakeDelegate(this, &VinylScratcher::onFileClicked));
	m_titleBar->setSeekCallback(fastdelegate::MakeDelegate(this, &VinylScratcher::onSeek));
	m_controlBar->getVolumeSlider()->setChangeCallback(fastdelegate::MakeDelegate(this, &VinylScratcher::onVolumeChanged));

	// vars
	m_stream = engine->getResourceManager()->loadSoundAbs("", "SND_VS_STREAM", true);
	m_stream2 = engine->getResourceManager()->loadSoundAbs("", "SND_VS_STREAM2", true);
	m_fReverseMessageTimer = 0.0f;

	// window colors
	setBackgroundColor(0xffffffff);
	setTitleColor(0xff000000);
	setFrameColor(0xffcccccc);
	setTitleColor(0xff555555);
	getCloseButton()->setFrameColor(0xff888888);
	getMinimizeButton()->setFrameColor(0xff888888);

	// window settings
	setResizeLimit(240 * dpiScale, 160 * dpiScale);
	setTitleFont(windowTitleFont);
	getCloseButton()->setDrawBackground(false);
	getMinimizeButton()->setDrawBackground(false);

	//open();
}

void VinylScratcher::update()
{
	CBaseUIWindow::update();
	{
		// NOTE: do this even if the window is not visible
		if (m_stream->isFinished())
			onFinished();
	}
	if (!m_bVisible) return;

	// restore title text after timer
	if (m_fReverseMessageTimer != 0.0f && engine->getTime() > m_fReverseMessageTimer)
	{
		m_fReverseMessageTimer = 0.0f;
		m_titleBar->setTitle(m_stream->getFilePath().length() > 1 ? env->getFileNameFromFilePath(m_stream->getFilePath()) : "Ready", true);
	}

	// update seekbar
	if (m_stream->isPlaying() && !m_titleBar->isSeeking())
		m_vs_percent_ref->setValue(m_stream->getPosition());

	// update info text
	{
		unsigned long lengthMS = m_stream->getLengthMS();
		unsigned long positionMS = m_stream->getPositionMS();
		if (m_titleBar->isSeeking())
			positionMS = (unsigned long)((float)lengthMS * m_vs_percent_ref->getFloat());

		m_controlBar->getInfoButton()->setText(UString::format("  %i:%02i / %i:%02i", (positionMS/1000) / 60, (positionMS/1000) % 60, (lengthMS/1000) / 60, (lengthMS/1000) % 60));
	}
}

void VinylScratcher::onKeyDown(KeyboardEvent &e)
{
	CBaseUIWindow::onKeyDown(e);

	if (!m_bVisible) return;

	// hotkeys
	if (e == KEY_LEFT || e == KEY_UP || e == KEY_A)
		onPrevClicked();
	if (e == KEY_SPACE || e == KEY_ENTER)
		onPlayClicked();
	if (e == KEY_RIGHT || e == KEY_DOWN || e == KEY_D)
		onNextClicked();
}

void VinylScratcher::onFinished()
{
	if (convar->getConVarByName("vs_repeat")->getBool())
		engine->getSound()->play(m_stream);
	else
	{
		// reset and stop (since we can't know yet if there even is a next song)
		m_stream->setPosition(0);
		engine->getSound()->pause(m_stream);
		m_controlBar->getPlayButton()->setText(">");

		// play the next song
		m_musicBrowser->fireNextSong(false);
	}

	// update seekbar
	m_vs_percent_ref->setValue(0.0f);
}

void VinylScratcher::onFileClicked(UString filepath, bool reverse)
{
	// check if the file is valid and can be played, if it's valid then play it
	if (tryPlayFile(filepath))
	{
		engine->getSound()->stop(m_stream);

		m_stream->rebuild(filepath);
		m_stream->setVolume(m_controlBar->getVolumeSlider()->getFloat());

		if (engine->getSound()->play(m_stream))
		{
			m_controlBar->getPlayButton()->setText("II");
			m_titleBar->setTitle(env->getFileNameFromFilePath(filepath), reverse);

			if (m_fReverseMessageTimer > 0.0f)
				m_fReverseMessageTimer = 0.0f;
		}
	}
	else
	{
		if (m_fReverseMessageTimer == 0.0f)
		{
			m_titleBar->setTitle("Not a valid audio file!");
			m_fReverseMessageTimer = engine->getTime() + 1.5f;
		}
		m_musicBrowser->onInvalidFile();
	}
}

void VinylScratcher::onVolumeChanged(CBaseUISlider *slider)
{
	m_stream->setVolume(slider->getFloat());
}

void VinylScratcher::onSeek()
{
	m_stream->setPosition(m_vs_percent_ref->getFloat());
}

void VinylScratcher::onPlayClicked()
{
	if (m_stream->isPlaying())
	{
		engine->getSound()->pause(m_stream);
		m_controlBar->getPlayButton()->setText(">");
	}
	else
	{
		if (engine->getSound()->play(m_stream))
			m_controlBar->getPlayButton()->setText("II");
	}
}

void VinylScratcher::onNextClicked()
{
	m_musicBrowser->fireNextSong(false);
}

void VinylScratcher::onPrevClicked()
{
	m_musicBrowser->fireNextSong(true);
}

void VinylScratcher::onResized()
{
	CBaseUIWindow::onResized();

	m_titleBar->setSizeX(m_vSize.x + 2);
	m_controlBar->setSizeX(m_vSize.x);
	m_controlBar->setRelPosY(m_vSize.y - getTitleBarHeight() - m_controlBar->getSize().y);
	m_musicBrowser->setSize(m_vSize.x, m_vSize.y - m_controlBar->getSize().y - m_titleBar->getSize().y - getTitleBarHeight());

	getContainer()->update_pos();
}



bool VinylScratcher::tryPlayFile(UString filepath)
{
	m_stream2->rebuild(filepath);

	if (engine->getSound()->play(m_stream2))
	{
		engine->getSound()->stop(m_stream2);
		return true;
	}

	return false;
}

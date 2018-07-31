//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		console window, also handles ConVar parsing
//
// $NoKeywords: $con
//===============================================================================//

#include "Console.h"

#include "Engine.h"
#include "ConVar.h"
#include "ResourceManager.h"

#include <mutex>
#include "WinMinGW.Mutex.h"

#include "CBaseUIContainer.h"
#include "CBaseUITextbox.h"
#include "CBaseUIScrollView.h"
#include "CBaseUITextField.h"
#include "CBaseUIButton.h"
#include "CBaseUILabel.h"

#define CFG_FOLDER "cfg/"

#define CONSOLE_BORDER 6

void DUMMY_ARGS_CLEAR() {;}

ConVar console_logging("console_logging", true);
ConVar _clear("clear", DUMMY_ARGS_CLEAR);

std::vector<UString> Console::g_commandQueue;

std::mutex g_consoleLogMutex;

Console::Console() : CBaseUIWindow(350, 100, 620, 550, "Console")
{
	// convar bindings
	_clear.setCallback( fastdelegate::MakeDelegate(this, &Console::clear) );

	// resources
	m_logFont = engine->getResourceManager()->getFont("FONT_CONSOLE");
	McFont *textboxFont = engine->getResourceManager()->loadFont("tahoma.ttf", "FONT_CONSOLE_TEXTBOX", 9.0f, false);
	McFont *titleFont = engine->getResourceManager()->loadFont("tahoma.ttf", "FONT_CONSOLE_TITLE", 10.0f, false);

	// colors
	//Color frameColor = 0xff9a9a9a;
	Color brightColor = 0xffb7b7b7;
	Color darkColor = 0xff343434;
	Color backgroundColor = 0xff555555;
	Color windowBackgroundColor = 0xff7b7b7b;

	setTitleFont(titleFont);
	setDrawTitleBarLine(false);
	setDrawFrame(false);
	setRoundedRectangle(true);

	setBackgroundColor(windowBackgroundColor);

	//setFrameColor(frameColor);
	setFrameDarkColor(0xff9a9a9a);
	setFrameBrightColor(0xff8a8a8a);

	getCloseButton()->setBackgroundColor(0xffbababa);
	getCloseButton()->setDrawBackground(false);

	int textboxHeight = 20;

	// log scrollview
	m_log = new CBaseUIScrollView(CONSOLE_BORDER, 0, m_vSize.x - 2*CONSOLE_BORDER, m_vSize.y - getTitleBarHeight() - 2*CONSOLE_BORDER - textboxHeight - 1, "consolelog");
	m_log->setHorizontalScrolling(false);
	m_log->setVerticalScrolling(true);
	m_log->setBackgroundColor(backgroundColor);
	m_log->setFrameDarkColor(darkColor);
	m_log->setFrameBrightColor(brightColor);
	getContainer()->addBaseUIElement(m_log);

	/*
	m_newLog = new CBaseUITextField(CONSOLE_BORDER, 0, m_vSize.x - 2*CONSOLE_BORDER, m_vSize.y - getTitleBarHeight() - 2*CONSOLE_BORDER - textboxHeight - 1, "newconsolelog", "");
	m_newLog->setFont(m_logFont);
	m_newLog->setHorizontalScrolling(false);
	m_newLog->setVerticalScrolling(true);
	m_newLog->setBackgroundColor(backgroundColor);
	m_newLog->setFrameDarkColor(darkColor);
	m_newLog->setFrameBrightColor(brightColor);
	getContainer()->addBaseUIElement(m_newLog);
	*/

	// textbox
	m_textbox = new CBaseUITextbox(CONSOLE_BORDER, m_vSize.y - getTitleBarHeight() - textboxHeight - CONSOLE_BORDER, m_vSize.x - 2*CONSOLE_BORDER, textboxHeight, "consoletextbox");
	m_textbox->setText("");
	m_textbox->setFont(textboxFont);
	m_textbox->setBackgroundColor(backgroundColor);
	m_textbox->setFrameDarkColor(darkColor);
	m_textbox->setFrameBrightColor(brightColor);
	m_textbox->setCaretWidth(1);
	getContainer()->addBaseUIElement(m_textbox);

	// notify engine, exec autoexec
	engine->setConsole(this);
	Console::execConfigFile("autoexec.cfg");
}

Console::~Console()
{
}

void Console::processCommand(UString command)
{
	if (command.length() < 1) return;

	// remove empty space at beginning if it exists
	if (command.find(" ", 0, 1) != -1)
		command.erase(0, 1);

	// handle multiple commands separated by semicolons
	if (command.find(";") != -1 && command.find("echo") == -1)
	{
		std::vector<UString> commands = command.split(";");
		for (int i=0; i<commands.size(); i++)
		{
			processCommand(commands[i]);
		}
		return;
	}

	// separate convar name and value
	std::vector<UString> tokens = command.split(" ");
	UString commandName;
	UString commandValue;
	for (int i=0; i<tokens.size(); i++)
	{
		if (i == 0)
			commandName = tokens[i];
		else
		{
			commandValue.append(tokens[i]);
			if (i < (int)(tokens.size()-1))
				commandValue.append(" ");
		}
	}

	// get convar
	ConVar *var = convar->getConVarByName(commandName, false);
	if (var == NULL)
	{
		UString temp = "Unknown command \"";
		temp.append(commandName);
		temp.append("\"!\n");
		debugLog("%s", temp.toUtf8());
		return;
	}

	// set new value (this handles all callbacks internally)
	if (commandValue.length() > 0)
		var->setValue(commandValue);
	else
		var->exec();

	// log
	if (console_logging.getBool())
	{
		UString logMessage;

		bool doLog = false;
		if (commandValue.length() < 1 && var->getHelpstring().length() > 0)
		{
			doLog = true;

			logMessage = commandName;
			logMessage.append(" : ");

			if (var->hasValue())
			{
				logMessage.append(var->getString());
				logMessage.append(" - ");
				logMessage.append(var->getHelpstring());
			}
			else
				logMessage.append(var->getHelpstring());

			logMessage.append("\n");
		}
		else if (var->hasValue())
		{
			doLog = true;

			logMessage = commandName;
			logMessage.append(" : ");
			logMessage.append(var->getString());

			logMessage.append("\n");
		}

		if (logMessage.length() > 0 && doLog)
			debugLog("%s", logMessage.toUtf8());
	}
}

void Console::execConfigFile(UString filename)
{
	// handle extension
	filename.insert(0, CFG_FOLDER);
	if (filename.find(".cfg", filename.length()-4, filename.length()) == -1)
		filename.append(".cfg");

	// open it
	std::ifstream inFile(filename.toUtf8());
	if (!inFile.good())
	{
		debugLog("Console::execConfigFile() error, file \"%s\" not found!\n", filename.toUtf8());
		return;
	}

	// go through every line
	std::string line;
	std::vector<UString> cmds;
	while (std::getline(inFile, line))
	{
		if (line.size() > 0)
		{
			// handle comments
			UString cmd = UString(line.c_str());
			if (cmd.find("//", 0, cmd.length()) != -1)
				cmd.erase(cmd.find("//", 0, cmd.length()), cmd.length()-cmd.find("//", 0, cmd.length()));

			// add command
			cmds.push_back(cmd);
		}
	}

	// process the collected commands
	for (int i=0; i<cmds.size(); i++)
	{
		processCommand(cmds[i]);
	}
}

void Console::update()
{
	CBaseUIWindow::update();
	if (!m_bVisible) return;

	// TODO: this needs proper callbacks in the textbox class
	if (m_textbox->hitEnter())
	{
		processCommand(m_textbox->getText());
		m_textbox->clear();
	}
}

void Console::log(UString text, Color textColor)
{
	std::lock_guard<std::mutex> lk(g_consoleLogMutex);

	if (text.length() < 1)
		return;

	// delete illegal characters
	int newline = text.find("\n", 0);
	while (newline != -1)
	{
		text.erase(newline, 1);
		newline = text.find("\n", 0);
	}

	// get index
	int index = m_log->getContainer()->getAllBaseUIElementsPointer()->size();
	int height = 13;

	// create new label, add it
	CBaseUILabel *logEntry = new CBaseUILabel(3, height*index - 1, 150, height, text, text);
	logEntry->setDrawFrame(false);
	logEntry->setDrawBackground(false);
	logEntry->setTextColor(textColor);
	logEntry->setFont(m_logFont);
	logEntry->setSizeToContent(1,4);
	m_log->getContainer()->addBaseUIElement(logEntry);

	// update scrollsize, scroll to bottom, clear textbox
	m_log->setScrollSizeToContent();
	m_log->scrollToBottom();

	///m_newLog->append(text);
	///m_newLog->scrollToBottom();
}

void Console::clear()
{
	m_log->clear();
	///m_newLog->clear();
}

void Console::onResized()
{
	CBaseUIWindow::onResized();

	m_log->setSize(m_vSize.x - 2*CONSOLE_BORDER, m_vSize.y - getTitleBarHeight() - 2*CONSOLE_BORDER - m_textbox->getSize().y - 1);
	///m_newLog->setSize(m_vSize.x - 2*CONSOLE_BORDER, m_vSize.y - getTitleBarHeight() - 2*CONSOLE_BORDER - m_textbox->getSize().y - 1);
	m_textbox->setSize(m_vSize.x - 2*CONSOLE_BORDER, m_textbox->getSize().y);
	m_textbox->setRelPosY(m_log->getRelPos().y + m_log->getSize().y + CONSOLE_BORDER + 1);

	m_log->scrollToY(m_log->getScrollPosY());
	//m_newLog->scrollY(m_newLog->getScrollPosY());
}



//***********************//
//	Console ConCommands  //
//***********************//

void _exec(UString args)
{
	Console::execConfigFile(args);
}

void _echo(UString args)
{
	if (args.length() > 0)
	{
		args.append("\n");
		debugLog("%s", args.toUtf8());
	}
}

void _fizzbuzz(void)
{
	for (int i=1; i<101; i++)
	{
		if (i % 3 == 0 && i % 5 == 0)
			debugLog("%i fizzbuzz\n",i);
		else if (i % 3 == 0)
			debugLog("%i fizz\n",i);
		else if (i % 5 == 0)
			debugLog("%i buzz\n",i);
		else
			debugLog("%i\n",i);
	}
}

ConVar _exec_("exec", _exec);
ConVar _echo_("echo", _echo);
ConVar _fizzbuzz_("fizzbuzz", _fizzbuzz);

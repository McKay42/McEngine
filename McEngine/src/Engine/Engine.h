//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		core
//
// $NoKeywords: $engine $os
//===============================================================================//

#ifndef ENGINE_H
#define ENGINE_H

#include "cbase.h"

class App;
class Timer;
class Mouse;
class ConVar;
class Gamepad;
class Keyboard;
class InputDevice;
class SoundEngine;
class ContextMenu;
class NetworkHandler;
class OpenCLInterface;
class OpenVRInterface;
class VulkanInterface;
class ResourceManager;
class AnimationHandler;
class SquirrelInterface;

class CBaseUIContainer;
class ConsoleBox;
class Console;

class Engine
{
public:
	static void debugLog(const char *fmt, va_list args);
	static void debugLog(Color color, const char *fmt, va_list args);
	static void debugLog(const char *fmt, ...);
	static void debugLog(Color color, const char *fmt, ...);

public:
	Engine(Environment *environment, const char *args = NULL);
	~Engine();

	// app
	void loadApp();

	// render/update
	void onPaint();
	void onUpdate();

	// window messages
	void onFocusGained();
	void onFocusLost();
	void onMinimized();
	void onMaximized();
	void onRestored();
	void onResolutionChange(Vector2 newResolution);
	void onShutdown();

	// primary mouse messages
	void onMouseRawMove(int xDelta, int yDelta, bool absolute = false, bool virtualDesktop = false);
	void onMouseWheelVertical(int delta);
	void onMouseWheelHorizontal(int delta);
	void onMouseLeftChange(bool mouseLeftDown);
	void onMouseMiddleChange(bool mouseMiddleDown);
	void onMouseRightChange(bool mouseRightDown);

	// primary keyboard messages
	void onKeyboardKeyDown(KEYCODE keyCode);
	void onKeyboardKeyUp(KEYCODE keyCode);
	void onKeyboardChar(KEYCODE charCode);

	// convenience functions (passthroughs)
	void shutdown();
	void restart();
	void sleep(unsigned int us);
	void focus();
	void center();
	void toggleFullscreen();
	void disableFullscreen();

	void showMessageInfo(UString title, UString message);
	void showMessageWarning(UString title, UString message);
	void showMessageError(UString title, UString message);
	void showMessageErrorFatal(UString title, UString message);

	// engine specifics
	void blackout() {m_bBlackout = true;}
	void addGamepad(Gamepad *gamepad);
	void removeGamepad(Gamepad *gamepad);

	// interfaces
	inline App *getApp() const {return m_app;}
	inline Graphics *getGraphics() const {return m_graphics;}
	inline SoundEngine *getSound() const {return m_sound;}
	inline ResourceManager *getResourceManager() const {return m_resourceManager;}
	inline Environment *getEnvironment() const {return m_environment;}
	inline NetworkHandler *getNetworkHandler() const {return m_networkHandler;}

	// input devices
	inline Mouse *getMouse() const {return m_mouse;}
	inline Keyboard *getKeyboard() const {return m_keyboard;}
	inline Gamepad *getGamepad() const {return m_gamepad;}
	inline int getNumMice() {return m_mice.size();}
	inline int getNumKeyboards() {return m_keyboards.size();}
	inline int getNumGamepads() {return m_gamepads.size();}
	inline std::vector<Mouse*> getMice() {return m_mice;}
	inline std::vector<Keyboard*> getKeyboards() {return m_keyboards;}
	inline std::vector<Gamepad*> getGamepads() {return m_gamepads;}

	// screen
	void requestResolutionChange(Vector2 newResolution) {if (newResolution != m_vNewScreenSize) {m_vNewScreenSize = newResolution; m_bResolutionChange = true;}}
	inline Vector2 getScreenSize() const {return m_vScreenSize;}
	inline int getScreenWidth() const {return (int)m_vScreenSize.x;}
	inline int getScreenHeight() const {return (int)m_vScreenSize.y;}

	// vars
	void setFrameTime(double delta);
	inline double getTime() const {return m_dTime;}
	double const getTimeReal();
	inline double getTimeRunning() const {return m_dRunTime;}
	inline double getFrameTime() const {return m_dFrameTime;}
	inline unsigned long getFrameCount() const {return m_iFrameCount;}
	UString getArgs() {return m_sArgs;}
	inline bool hasFocus() const {return m_bHasFocus;}
	inline bool isDrawing() const {return m_bDrawing;}
	inline bool isMinimized() const {return m_bIsMinimized;}

	// debugging/console
	inline ConsoleBox *getConsoleBox() {return m_consoleBox;}
	inline Console *getConsole() {return m_console;}
	void setConsole(Console *console) {m_console = console;}
	inline CBaseUIContainer *getGUI() {return m_guiContainer;}

private:
	// interfaces
	App *m_app;
	Graphics *m_graphics;
	SoundEngine *m_sound;
	OpenCLInterface *m_openCL;
	OpenVRInterface *m_openVR;
	VulkanInterface *m_vulkan;
	ContextMenu *m_contextMenu;
	Environment *m_environment;
	NetworkHandler *m_networkHandler;
	ResourceManager *m_resourceManager;
	AnimationHandler *m_animationHandler;
	SquirrelInterface *m_squirrel;

	// input devices
	Mouse *m_mouse;
	Keyboard *m_keyboard;
	Gamepad *m_gamepad;
	std::vector<Mouse*> m_mice;
	std::vector<Keyboard*> m_keyboards;
	std::vector<Gamepad*> m_gamepads;
	std::vector<InputDevice*> m_inputDevices;

	// timing
	Timer *m_timer;
	double m_dTime;
	double m_dRunTime;
	unsigned long m_iFrameCount;
	double m_dFrameTime;

	// primary screen
	Vector2 m_vScreenSize;
	Vector2 m_vNewScreenSize;
	bool m_bResolutionChange;

	// window
	bool m_bHasFocus;
	bool m_bIsMinimized;

	// engine gui, mostly for debugging
	CBaseUIContainer *m_guiContainer;
	static ConsoleBox *m_consoleBox;
	static Console *m_console;

	// engine
	UString m_sArgs;
	bool m_bBlackout;
	bool m_bDrawing;
};

extern Engine *engine;
#define debugLog(format, ...) Engine::debugLog( format , ##__VA_ARGS__ )

#endif

#ifdef __linux__

#ifndef LINUXWAYLANDENVIRONMENT_H
#define LINUXWAYLANDENVIRONMENT_H

#include <vector>

#include <wayland-client.h>
#include <wayland-egl.h>
#include <wayland-cursor.h>

#include <xkbcommon/xkbcommon.h>

#include "xdg-shell-client-protocol.h"
#include "pointer-constraints-unstable-v1-client-protocol.h"
#include "relative-pointer-unstable-v1-client-protocol.h"

#include "LinuxEnvironment.h"

class LinuxWaylandEnvironment : public LinuxEnvironment
{
public:
	LinuxWaylandEnvironment(struct wl_display *display, struct wl_surface *surface,
		struct xdg_toplevel *toplevel, struct wl_egl_window *window, struct wl_compositor *compositor,
		struct wl_shm *shm);
	~LinuxWaylandEnvironment();

	void update();

	// engine/factory
	Graphics *createRenderer();

	// system
	void shutdown();
	void restart();

	// clipboard
	UString getClipBoardText();
	void setClipBoardText(UString text);

	// window
	void focus();
	void center();
	void minimize();
	void maximize();
	void enableFullscreen();
	void disableFullscreen();
	void setWindowTitle(UString title);
	void setWindowPos(int x, int y);
	void setWindowSize(int width, int height);
	void setWindowResizable(bool resizable);
	void setWindowGhostCorporeal(bool corporeal);
	void setMonitor(int monitor);
	Vector2 getWindowPos();
	Vector2 getWindowSize();
	int getMonitor();
	std::vector<McRect> getMonitors();
	Vector2 getNativeScreenSize();
	McRect getVirtualScreenRect();
	McRect getDesktopRect();
	int getDPI();
	bool isFullscreen();
	bool isWindowResizable();

	// mouse
	bool isCursorInWindow();
	bool isCursorVisible();
	bool isCursorClipped();
	Vector2 getMousePos();
	McRect getCursorClip();
	CURSORTYPE getCursor();
	void setCursor(CURSORTYPE cur);
	void setCursorVisible(bool visible);
	void setMousePos(int x, int y);
	void setCursorClip(bool clip, McRect rect);

	// keyboard
	UString keyCodeToString(KEYCODE keyCode);

	friend int mainWayland(int argc, char *argv[], struct wl_display *display);

private:
	void reconfigure();

	void pointerButton(uint32_t button, bool state);

	struct wl_display *m_display;
	struct wl_surface *m_surface;
	struct xdg_toplevel *m_toplevel;
	struct wl_egl_window *m_window;
	struct wl_compositor *m_compositor;

	struct wl_cursor_theme *m_cursorTheme;
	struct wl_cursor *m_cursor;
	struct wl_surface *m_cursorSurface;

	Vector2 m_vWindowSize;
	bool m_bIsMaximized;
	bool m_bIsFullscreen;
	bool m_bIsActivated;

	bool m_bIsRestartScheduled;

	bool m_bMouseLeft;
	bool m_bMouseMiddle;
	bool m_bMouseRight;
	Vector2 m_vMousePos;

	struct wl_pointer *m_pointer;
	uint32_t m_uPointerSerial;
	bool m_bCursorVisible;
	int m_iCursorsInWindow;
	CURSORTYPE m_cursorType;

	bool m_bWindowResizable;

	struct xkb_context *m_xkbContext;
	struct xkb_state *m_xkbState;
	struct xkb_keymap *m_xkbKeymap;

	struct Output
	{
		int name;
		struct wl_output *output;
		int x, y, width, height;
		int dpi;
		McRect geometry;
	};

	std::vector<Output *> m_outputs;
	int m_iCurrentOutput;
	int m_iWantedOutput;

	struct zwp_pointer_constraints_v1 *m_pointerConstraints;
	struct zwp_confined_pointer_v1 *m_confinedPointer;
	McRect m_confinedRect;
	bool m_bPointerIsConfined;

	struct wl_seat *m_seat;
	struct wl_data_device_manager *m_dataDeviceManager;
	struct wl_data_device *m_dataDevice;
	uint32_t m_serial;
	UString m_selection;

	struct zwp_relative_pointer_manager_v1 *m_relativePointerManager;

	static const struct wl_surface_listener surfaceListener;
	static const struct wl_registry_listener registryListener;
	static const struct wl_seat_listener seatListener;
	static const struct wl_pointer_listener pointerListener;
	static const struct wl_keyboard_listener keyboardListener;
	static const struct wl_touch_listener touchListener;
	static const struct wl_output_listener outputListener;
	static const struct zwp_confined_pointer_v1_listener confinedPointerListener;
	static const struct wl_data_device_listener dataDeviceListener;
	static const struct wl_data_source_listener dataSourceListener;
	static const struct zwp_relative_pointer_v1_listener relativePointerListener;

	static void surfaceEnter(void *data, struct wl_surface *wl_surface, struct wl_output *output);
	static void surfaceLeave(void *data, struct wl_surface *wl_surface, struct wl_output *output);

	static void registryGlobal(void *data, struct wl_registry *wl_registry, uint32_t name, const char *interface, uint32_t version);
	static void registryGlobalRemove(void *data, struct wl_registry *wl_registry, uint32_t name);

	static void seatCapabilities(void *data, struct wl_seat *wl_seat, uint32_t capabilities);
	static void seatName(void *data, struct wl_seat *wl_seat, const char *name);

	static void pointerEnter(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t surface_x, wl_fixed_t surface_y);
	static void pointerLeave(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface);
	static void pointerMotion(void *data, struct wl_pointer *wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y);
	static void pointerButton(void *data, struct wl_pointer *wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
	static void pointerAxis(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value);
	static void pointerFrame(void *data, struct wl_pointer *wl_pointer);
	static void pointerAxisSource(void *data, struct wl_pointer *wl_pointer, uint32_t axis_source);
	static void pointerAxisStop(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis);
	static void pointerAxisDiscrete(void *data, struct wl_pointer *wl_pointer, uint32_t axis, int32_t discrete);

	static void keyboardKeymap(void *data, struct wl_keyboard *wl_keyboard, uint32_t format, int32_t fd, uint32_t size);
	static void keyboardEnter(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys);
	static void keyboardLeave(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface);
	static void keyboardKey(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
	static void keyboardModifiers(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group);
	static void keyboardRepeatInfo(void *data, struct wl_keyboard *wl_keyboard, int32_t rate, int32_t delay);

	static void touchDown(void *data, struct wl_touch *wl_touch, uint32_t serial, uint32_t time, struct wl_surface *surface, int32_t id, wl_fixed_t x, wl_fixed_t y);
	static void touchUp(void *data, struct wl_touch *wl_touch, uint32_t serial, uint32_t time, int32_t id);
	static void touchMotion(void *data, struct wl_touch *wl_touch, uint32_t time, int32_t id, wl_fixed_t x, wl_fixed_t y);
	static void touchFrame(void *data, struct wl_touch *wl_touch);
	static void touchCancel(void *data, struct wl_touch *wl_touch);
	static void touchShape(void *data, struct wl_touch *wl_touch, int32_t id, wl_fixed_t major, wl_fixed_t minor);
	static void touchOrientation(void *data, struct wl_touch *wl_touch, int32_t id, wl_fixed_t orientation);

	static void outputGeometry(void *data, struct wl_output *wl_output, int32_t x, int32_t y, int32_t physical_width, int32_t physical_height, int32_t subpixel, const char *make, const char *model, int32_t transform);
	static void outputMode(void *data, struct wl_output *wl_output, uint32_t flags, int32_t width, int32_t height, int32_t refresh);
	static void outputDone(void *data, struct wl_output *wl_output);
	static void outputScale(void *data, struct wl_output *wl_output, int32_t factor);

	static void confinedPointerConfined(void *data, struct zwp_confined_pointer_v1 *zwp_confined_pointer_v1);
	static void confinedPointerUnconfined(void *data, struct zwp_confined_pointer_v1 *zwp_confined_pointer_v1);

	static void dataDeviceDataOffer(void *data, struct wl_data_device *wl_data_device, struct wl_data_offer *id);
	static void dataDeviceEnter(void *data, struct wl_data_device *wl_data_device, uint32_t serial, struct wl_surface *surface, wl_fixed_t x, wl_fixed_t y, struct wl_data_offer *id);
	static void dataDeviceLeave(void *data, struct wl_data_device *wl_data_device);
	static void dataDeviceMotion(void *data, struct wl_data_device *wl_data_device, uint32_t time, wl_fixed_t x, wl_fixed_t y);
	static void dataDeviceDrop(void *data, struct wl_data_device *wl_data_device);
	static void dataDeviceSelection(void *data, struct wl_data_device *wl_data_device, struct wl_data_offer *id);

	static void dataSourceTarget(void *data, struct wl_data_source *wl_data_source, const char *mime_type);
	static void dataSourceSend(void *data, struct wl_data_source *wl_data_source, const char *mime_type, int32_t fd);
	static void dataSourceCancelled(void *data, struct wl_data_source *wl_data_source);
	static void dataSourceDndDropPerformed(void *data, struct wl_data_source *wl_data_source);
	static void dataSourceDndFinished(void *data, struct wl_data_source *wl_data_source);
	static void dataSourceAction(void *data, struct wl_data_source *wl_data_source, uint32_t dnd_action);

	static void relativePointerRelativeMotion(void *data, struct zwp_relative_pointer_v1 *zwp_relative_pointer_v1, uint32_t utime_hi, uint32_t utime_lo, wl_fixed_t dx, wl_fixed_t dy, wl_fixed_t dx_unaccel, wl_fixed_t dy_unaccel);
};

#endif

#endif

#ifdef __linux__

#include <wayland-client-protocol.h>
#include <linux/input-event-codes.h>
#include <unistd.h>
#include <sys/mman.h>
#include <limits.h>

#include "LinuxWaylandEnvironment.h"
#include "LinuxEGLLegacyInterface.h"
#include "Engine.h"

extern bool g_bRunning;

extern Engine *g_engine;
extern LinuxWaylandEnvironment *g_environment;

LinuxWaylandEnvironment::LinuxWaylandEnvironment(struct wl_display *display,
	struct wl_surface *surface, struct xdg_toplevel *toplevel,
	struct wl_egl_window *window, struct wl_compositor *compositor, struct wl_shm *shm)
	: LinuxEnvironment()
{
	g_environment = this;
	m_display = display;
	m_surface = surface;
	m_toplevel = toplevel;
	m_window = window;
	m_compositor = compositor; // TODO: bind this ourselves instead

	m_cursorTheme = wl_cursor_theme_load(NULL, 24, shm);
	m_cursorSurface = wl_compositor_create_surface(compositor);

	m_vWindowSize = Vector2(0, 0);
	m_bIsMaximized = false;
	m_bIsFullscreen = false;
	m_bIsActivated = false;

	m_bIsRestartScheduled = false;

	bool m_bMouseLeft;
	bool m_bMouseMiddle;
	bool m_bMouseRight;
	m_vMousePos = Vector2(0, 0);

	m_pointer = NULL;
	m_uPointerSerial = 0;
	m_bCursorVisible = false;
	m_iCursorsInWindow = 0;
	setCursor(CURSOR_NORMAL);

	m_bWindowResizable = true;

	m_xkbContext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	m_xkbState = NULL;
	m_xkbKeymap = NULL;

	m_iCurrentOutput = 0;
	m_iWantedOutput = 0;

	m_pointerConstraints = NULL;
	m_confinedPointer = NULL;
	m_bPointerIsConfined = false;

	m_seat = NULL;
	m_dataDeviceManager = NULL;
	m_dataDevice = NULL;
	m_serial = 0;
	m_selection = "";

	struct wl_registry *registry = wl_display_get_registry(m_display);
	wl_registry_add_listener(registry, &registryListener, NULL);
	wl_display_roundtrip(display);
	wl_display_roundtrip(display);
	wl_registry_destroy(registry);

	m_dataDevice = wl_data_device_manager_get_data_device(m_dataDeviceManager, m_seat);
	wl_data_device_add_listener(m_dataDevice, &dataDeviceListener, NULL);

	struct zwp_relative_pointer_v1 *relativePointer =
		zwp_relative_pointer_manager_v1_get_relative_pointer(g_environment->m_relativePointerManager, m_pointer);
	zwp_relative_pointer_v1_add_listener(relativePointer, &relativePointerListener, NULL);
}

LinuxWaylandEnvironment::~LinuxWaylandEnvironment()
{
	for (Output *output : m_outputs)
		delete output;
	wl_cursor_theme_destroy(m_cursorTheme);
}

void LinuxWaylandEnvironment::update()
{
}

Graphics *LinuxWaylandEnvironment::createRenderer()
{
	return new LinuxEGLLegacyInterface(m_display, m_window);
}

void LinuxWaylandEnvironment::shutdown()
{
	g_bRunning = false;
}

void LinuxWaylandEnvironment::restart()
{
	m_bIsRestartScheduled = true;
	shutdown();
}


UString LinuxWaylandEnvironment::getClipBoardText()
{
	// TODO
	return "";
}

void LinuxWaylandEnvironment::setClipBoardText(UString text)
{
	struct wl_data_source *source = wl_data_device_manager_create_data_source(m_dataDeviceManager);
	wl_data_source_offer(source, "text/plain;charset=utf-8");
	wl_data_device_set_selection(m_dataDevice, source, m_serial);
	wl_data_source_add_listener(source, &dataSourceListener, NULL);
	m_selection = text;
}


void LinuxWaylandEnvironment::focus()
{
	// Unsupported
}

void LinuxWaylandEnvironment::center()
{
	// Unsupported
}

void LinuxWaylandEnvironment::minimize()
{
	xdg_toplevel_set_minimized(m_toplevel);
}

void LinuxWaylandEnvironment::maximize()
{
	xdg_toplevel_set_maximized(m_toplevel);
}

void LinuxWaylandEnvironment::enableFullscreen()
{
	xdg_toplevel_set_fullscreen(m_toplevel, m_outputs[m_iWantedOutput]->output);
}

void LinuxWaylandEnvironment::disableFullscreen()
{
	xdg_toplevel_unset_fullscreen(m_toplevel);
}

void LinuxWaylandEnvironment::setWindowTitle(UString title)
{
	xdg_toplevel_set_title(m_toplevel, title.toUtf8());
}

void LinuxWaylandEnvironment::setWindowPos(int x, int y)
{
	// Unsupported
}

void LinuxWaylandEnvironment::setWindowSize(int width, int height)
{
	// Unsupported
}

void LinuxWaylandEnvironment::setWindowResizable(bool resizable)
{
	m_bWindowResizable = resizable;
	if (m_bWindowResizable)
	{
		xdg_toplevel_set_min_size(m_toplevel, m_vWindowSize.x, m_vWindowSize.y);
		xdg_toplevel_set_max_size(m_toplevel, m_vWindowSize.x, m_vWindowSize.y);
	}
	else
	{
		xdg_toplevel_set_min_size(m_toplevel, 0, 0);
		xdg_toplevel_set_max_size(m_toplevel, 0, 0);
	}
}

void LinuxWaylandEnvironment::setWindowGhostCorporeal(bool corporeal)
{
	// Unsupported
}

void LinuxWaylandEnvironment::setMonitor(int monitor)
{
	m_iWantedOutput = monitor;
	if (m_bIsFullscreen)
		enableFullscreen();
}

Vector2 LinuxWaylandEnvironment::getWindowPos()
{
	// Unsupported
	return Vector2(0, 0);
}

Vector2 LinuxWaylandEnvironment::getWindowSize()
{
	return m_vWindowSize;
}

int LinuxWaylandEnvironment::getMonitor()
{
	return m_iCurrentOutput;
}

std::vector<McRect> LinuxWaylandEnvironment::getMonitors()
{
	std::vector<McRect> rects;
	for (Output *output : m_outputs)
		rects.push_back(output->geometry);
	return rects;
}

Vector2 LinuxWaylandEnvironment::getNativeScreenSize()
{
	McRect rect = m_outputs[m_iCurrentOutput]->geometry;
	return Vector2(rect.getWidth(), rect.getHeight());
}

McRect LinuxWaylandEnvironment::getVirtualScreenRect()
{
	int min_x = INT_MAX, max_x = INT_MAX;
	int min_y = 0, max_y = 0;
	for (Output *output : m_outputs)
	{
		min_x = std::min(min_x, output->x);
		min_y = std::min(min_y, output->y);
		max_x = std::max(max_x, output->x + output->width);
		max_y = std::max(max_y, output->y + output->height);
	}
	return McRect(min_x, min_y, max_x - min_x, max_y - min_y);
}

McRect LinuxWaylandEnvironment::getDesktopRect()
{
	// Unsupported
	McRect rect = m_outputs[m_iCurrentOutput]->geometry;
	return McRect(0, 0, rect.getWidth(), rect.getHeight());
}

int LinuxWaylandEnvironment::getDPI()
{
	return m_outputs[m_iCurrentOutput]->dpi;
}

bool LinuxWaylandEnvironment::isFullscreen()
{
	return m_bIsFullscreen;
}

bool LinuxWaylandEnvironment::isWindowResizable()
{
	return m_bWindowResizable;
}

bool LinuxWaylandEnvironment::isCursorInWindow()
{
	return m_iCursorsInWindow > 0;
}

bool LinuxWaylandEnvironment::isCursorVisible()
{
	return m_bCursorVisible;
}

bool LinuxWaylandEnvironment::isCursorClipped()
{
	return m_bPointerIsConfined;
}

Vector2 LinuxWaylandEnvironment::getMousePos()
{
	return m_vMousePos;
}

McRect LinuxWaylandEnvironment::getCursorClip()
{
	return m_confinedRect;
}

CURSORTYPE LinuxWaylandEnvironment::getCursor()
{
	return m_cursorType;
}

void LinuxWaylandEnvironment::setCursor(CURSORTYPE cur)
{
	m_cursorType = cur;
	const char *cursorName = "left_ptr";
	switch (cur)
	{
	case CURSOR_NORMAL:
		cursorName = "left_ptr";
		break;
	case CURSOR_WAIT:
		cursorName = "circle";
		break;
	case CURSOR_SIZE_H:
		cursorName = "sb_h_double_arrow";
		break;
	case CURSOR_SIZE_V:
		cursorName = "sb_v_double_arrow";
		break;
	case CURSOR_SIZE_HV:
		cursorName = "bottom_left_corner";
		break;
	case CURSOR_SIZE_VH:
		cursorName = "bottom_right_corner";
		break;
	case CURSOR_TEXT:
		cursorName = "xterm";
		break;
	}
	m_cursor = wl_cursor_theme_get_cursor(m_cursorTheme, cursorName);
	setCursorVisible(m_bCursorVisible);
}

void LinuxWaylandEnvironment::setCursorVisible(bool visible)
{
	m_bCursorVisible = visible;
	if (m_pointer == NULL) return;
	if (m_bCursorVisible)
	{
		struct wl_cursor_image *image = m_cursor->images[0];
		struct wl_buffer *buffer = wl_cursor_image_get_buffer(image);
		wl_pointer_set_cursor(m_pointer, m_uPointerSerial, m_cursorSurface, image->hotspot_x, image->hotspot_y);
		wl_surface_attach(m_cursorSurface, buffer, 0, 0);
		wl_surface_damage(m_cursorSurface, 0, 0, image->width, image->height);
		wl_surface_commit(m_cursorSurface);
	}
	else
	{
		wl_pointer_set_cursor(m_pointer, m_uPointerSerial, NULL, 0, 0);
	}
}

void LinuxWaylandEnvironment::setMousePos(int x, int y)
{
	// Unsupported
}

void LinuxWaylandEnvironment::setCursorClip(bool clip, McRect rect)
{
	if (m_pointerConstraints == NULL) return;
	if (clip)
	{
		m_confinedRect = rect;
		struct wl_region *region = wl_compositor_create_region(m_compositor);
		wl_region_add(region, rect.getMinX(), rect.getMinY(), rect.getWidth(), rect.getHeight());
		if (m_confinedPointer == NULL)
		{
			m_confinedPointer = zwp_pointer_constraints_v1_confine_pointer(m_pointerConstraints, m_surface, m_pointer, region, ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_ONESHOT);
			zwp_confined_pointer_v1_add_listener(m_confinedPointer, &confinedPointerListener, NULL);
		}
		else
		{
			zwp_confined_pointer_v1_set_region(m_confinedPointer, region);
		}
		wl_region_destroy(region);
	}
	else
	{
		if (m_confinedPointer)
		{
			zwp_confined_pointer_v1_destroy(m_confinedPointer);
			m_confinedPointer = NULL;
		}
	}
}

UString LinuxWaylandEnvironment::keyCodeToString(KEYCODE keyCode)
{
	char buffer[64];
	xkb_keysym_get_name(keyCode, buffer, sizeof(buffer));
	return buffer;
}

void LinuxWaylandEnvironment::reconfigure()
{
	setWindowResizable(m_bWindowResizable);
}

void LinuxWaylandEnvironment::pointerButton(uint32_t button, bool state)
{
	switch (button)
	{
		case BTN_LEFT:
			if (g_environment->m_bMouseLeft != state)
				engine->onMouseLeftChange(state);
			g_environment->m_bMouseLeft = state;
			break;
		case BTN_MIDDLE:
			if (g_environment->m_bMouseMiddle != state)
				engine->onMouseMiddleChange(state);
			g_environment->m_bMouseMiddle = state;
			break;
		case BTN_RIGHT:
			if (g_environment->m_bMouseRight != state)
				engine->onMouseRightChange(state);
			g_environment->m_bMouseRight = state;
			break;
	}
}

const struct wl_surface_listener LinuxWaylandEnvironment::surfaceListener =
{
	surfaceEnter,
	surfaceLeave
};

const struct wl_registry_listener LinuxWaylandEnvironment::registryListener =
{
	registryGlobal,
	registryGlobalRemove
};

const struct wl_seat_listener LinuxWaylandEnvironment::seatListener =
{
	seatCapabilities,
	seatName
};

const struct wl_pointer_listener LinuxWaylandEnvironment::pointerListener =
{
	pointerEnter,
	pointerLeave,
	pointerMotion,
	pointerButton,
	pointerAxis,
	pointerFrame,
	pointerAxisSource,
	pointerAxisStop,
	pointerAxisDiscrete
};

const struct wl_keyboard_listener LinuxWaylandEnvironment::keyboardListener =
{
	keyboardKeymap,
	keyboardEnter,
	keyboardLeave,
	keyboardKey,
	keyboardModifiers,
	keyboardRepeatInfo
};

const struct wl_touch_listener LinuxWaylandEnvironment::touchListener =
{
	touchDown,
	touchUp,
	touchMotion,
	touchFrame,
	touchCancel,
	touchShape,
	touchOrientation
};

const struct wl_output_listener LinuxWaylandEnvironment::outputListener =
{
	outputGeometry,
	outputMode,
	outputDone,
	outputScale
};

const struct zwp_confined_pointer_v1_listener LinuxWaylandEnvironment::confinedPointerListener =
{
	confinedPointerConfined,
	confinedPointerUnconfined
};

const struct wl_data_device_listener LinuxWaylandEnvironment::dataDeviceListener =
{
	dataDeviceDataOffer,
	dataDeviceEnter,
	dataDeviceLeave,
	dataDeviceMotion,
	dataDeviceDrop,
	dataDeviceSelection
};

const struct wl_data_source_listener LinuxWaylandEnvironment::dataSourceListener =
{
	dataSourceTarget,
	dataSourceSend,
	dataSourceCancelled,
	dataSourceDndDropPerformed,
	dataSourceDndFinished,
	dataSourceAction
};

const struct zwp_relative_pointer_v1_listener LinuxWaylandEnvironment::relativePointerListener =
{
	relativePointerRelativeMotion
};

extern "C"
{

void LinuxWaylandEnvironment::surfaceEnter(void *data, struct wl_surface *wl_surface, struct wl_output *output)
{
	for (size_t i = 0; i < g_environment->m_outputs.size(); i++)
	{
		if (g_environment->m_outputs[i]->output == output)
		{
			g_environment->m_iCurrentOutput = i;
		}
	}
}

void LinuxWaylandEnvironment::surfaceLeave(void *data, struct wl_surface *wl_surface, struct wl_output *output)
{
}

void LinuxWaylandEnvironment::registryGlobal(void *data, struct wl_registry *wl_registry, uint32_t name, const char *interface, uint32_t version)
{
	if (strcmp(interface, wl_seat_interface.name) == 0)
	{
		struct wl_seat *seat = (wl_seat *)wl_registry_bind(wl_registry, name, &wl_seat_interface, 5);
		wl_seat_add_listener(seat, &seatListener, NULL);
	}
	else if (strcmp(interface, wl_output_interface.name) == 0)
	{
		Output *output = new Output();
		output->name = name;
		output->output = (wl_output *)wl_registry_bind(wl_registry, name, &wl_output_interface, 3);
		output->x = output->y = output->width = output->height = 0;
		output->dpi = 96;
		output->geometry = McRect();
		g_environment->m_outputs.push_back(output);
		wl_output_add_listener(output->output, &outputListener, output);
	}
	else if (strcmp(interface, zwp_pointer_constraints_v1_interface.name) == 0)
	{
		g_environment->m_pointerConstraints = (struct zwp_pointer_constraints_v1 *)
			wl_registry_bind(wl_registry, name, &zwp_pointer_constraints_v1_interface, 1);
	}
	else if (strcmp(interface, wl_data_device_manager_interface.name) == 0)
	{
		g_environment->m_dataDeviceManager = (struct wl_data_device_manager *)
			wl_registry_bind(wl_registry, name, &wl_data_device_manager_interface, 1);
	}
	else if (strcmp(interface, zwp_relative_pointer_manager_v1_interface.name) == 0)
	{
		g_environment->m_relativePointerManager = (struct zwp_relative_pointer_manager_v1 *)
			wl_registry_bind(wl_registry, name, &zwp_relative_pointer_manager_v1_interface, 1);
	}
}

void LinuxWaylandEnvironment::registryGlobalRemove(void *data, struct wl_registry *wl_registry, uint32_t name)
{
	for (auto it = g_environment->m_outputs.begin(); it != g_environment->m_outputs.end(); ++it)
	{
		if (name == (*it)->name)
		{
			g_environment->m_outputs.erase(it);
			delete *it;
		}
	}
}

void LinuxWaylandEnvironment::seatCapabilities(void *data, struct wl_seat *wl_seat, uint32_t capabilities)
{
	if (capabilities & WL_SEAT_CAPABILITY_POINTER)
	{
		g_environment->m_seat = wl_seat;
		struct wl_pointer *pointer = wl_seat_get_pointer(wl_seat);
		wl_pointer_add_listener(pointer, &pointerListener, NULL);
		g_environment->m_pointer = pointer;
	}
	if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD)
	{
		struct wl_keyboard *keyboard = wl_seat_get_keyboard(wl_seat);
		wl_keyboard_add_listener(keyboard, &keyboardListener, NULL);
	}
	if (capabilities & WL_SEAT_CAPABILITY_TOUCH)
	{
		struct wl_touch *touch = wl_seat_get_touch(wl_seat);
		wl_touch_add_listener(touch, &touchListener, NULL);
	}
}

void LinuxWaylandEnvironment::seatName(void *data, struct wl_seat *wl_seat, const char *name)
{
}

void LinuxWaylandEnvironment::pointerEnter(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t surface_x, wl_fixed_t surface_y)
{
	g_environment->m_vMousePos = Vector2(wl_fixed_to_double(surface_x), wl_fixed_to_double(surface_y));
	g_environment->m_iCursorsInWindow++;
	g_environment->m_uPointerSerial = serial;
	g_environment->setCursorVisible(g_environment->m_bCursorVisible);
}

void LinuxWaylandEnvironment::pointerLeave(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface)
{
	g_environment->m_iCursorsInWindow--;
}

void LinuxWaylandEnvironment::pointerMotion(void *data, struct wl_pointer *wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y)
{
	g_environment->m_vMousePos = Vector2(wl_fixed_to_double(surface_x), wl_fixed_to_double(surface_y));
}

void LinuxWaylandEnvironment::pointerButton(void *data, struct wl_pointer *wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
	g_environment->pointerButton(button, state);
	g_environment->m_serial = serial;
}

void LinuxWaylandEnvironment::pointerAxis(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value)
{
	int delta = -wl_fixed_to_double(value) * 10;
	if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL)
		g_engine->onMouseWheelVertical(delta);
	else if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL)
		g_engine->onMouseWheelHorizontal(delta);
}

void LinuxWaylandEnvironment::pointerFrame(void *data, struct wl_pointer *wl_pointer)
{
}

void LinuxWaylandEnvironment::pointerAxisSource(void *data, struct wl_pointer *wl_pointer, uint32_t axis_source)
{
}

void LinuxWaylandEnvironment::pointerAxisStop(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis)
{
}

void LinuxWaylandEnvironment::pointerAxisDiscrete(void *data, struct wl_pointer *wl_pointer, uint32_t axis, int32_t discrete)
{
}

void LinuxWaylandEnvironment::keyboardKeymap(void *data, struct wl_keyboard *wl_keyboard, uint32_t format, int32_t fd, uint32_t size)
{
	xkb_keymap_unref(g_environment->m_xkbKeymap);
	xkb_state_unref(g_environment->m_xkbState);
	char *map_shm = (char *)mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
	g_environment->m_xkbKeymap = xkb_keymap_new_from_string(g_environment->m_xkbContext, map_shm, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
	munmap(map_shm, size);
	close(fd);
	g_environment->m_xkbState = xkb_state_new(g_environment->m_xkbKeymap);
}

void LinuxWaylandEnvironment::keyboardEnter(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys)
{
	g_engine->onFocusGained();
}

void LinuxWaylandEnvironment::keyboardLeave(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface)
{
	g_engine->onFocusLost();
}

void LinuxWaylandEnvironment::keyboardKey(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
	xkb_keycode_t keycode = key + 8;

	// hack: make all keys act like shift is being held down, because that's what the x11 backend does...
	xkb_mod_mask_t mods_depressed = xkb_state_serialize_mods(g_environment->m_xkbState, XKB_STATE_MODS_DEPRESSED);
	xkb_mod_mask_t mods_latched = xkb_state_serialize_mods(g_environment->m_xkbState, XKB_STATE_MODS_LATCHED);
	xkb_mod_mask_t mods_locked = xkb_state_serialize_mods(g_environment->m_xkbState, XKB_STATE_MODS_LOCKED);
	xkb_mod_mask_t group = xkb_state_serialize_mods(g_environment->m_xkbState, XKB_STATE_LAYOUT_LOCKED);
	xkb_state_update_mask(g_environment->m_xkbState, mods_depressed|1, mods_latched, mods_locked, 0, 0, group);
	xkb_keysym_t keysym = xkb_state_key_get_one_sym(g_environment->m_xkbState, keycode);
	xkb_state_update_mask(g_environment->m_xkbState, mods_depressed, mods_latched, mods_locked, 0, 0, group);

	if (state == WL_KEYBOARD_KEY_STATE_PRESSED)
	{
		g_engine->onKeyboardKeyDown(keysym);
		char utf8[32];
		xkb_state_key_get_utf8(g_environment->m_xkbState, keycode, utf8, sizeof(utf8));
		for (char *p = utf8; *p; p++)
			g_engine->onKeyboardChar(*p);
	}
	else if (state == WL_KEYBOARD_KEY_STATE_RELEASED)
	{
		g_engine->onKeyboardKeyUp(keysym);
	}

	g_environment->m_serial = serial;
}

void LinuxWaylandEnvironment::keyboardModifiers(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group)
{
	xkb_state_update_mask(g_environment->m_xkbState, mods_depressed, mods_latched, mods_locked, 0, 0, group);
}

void LinuxWaylandEnvironment::keyboardRepeatInfo(void *data, struct wl_keyboard *wl_keyboard, int32_t rate, int32_t delay)
{
}

void LinuxWaylandEnvironment::touchDown(void *data, struct wl_touch *wl_touch, uint32_t serial, uint32_t time, struct wl_surface *surface, int32_t id, wl_fixed_t x, wl_fixed_t y)
{
	g_environment->pointerButton(BTN_LEFT, true);
}

void LinuxWaylandEnvironment::touchUp(void *data, struct wl_touch *wl_touch, uint32_t serial, uint32_t time, int32_t id)
{
	g_environment->pointerButton(BTN_LEFT, false);
}

void LinuxWaylandEnvironment::touchMotion(void *data, struct wl_touch *wl_touch, uint32_t time, int32_t id, wl_fixed_t x, wl_fixed_t y)
{
	g_environment->m_vMousePos = Vector2(wl_fixed_to_double(x), wl_fixed_to_double(y));
}

void LinuxWaylandEnvironment::touchFrame(void *data, struct wl_touch *wl_touch)
{
}

void LinuxWaylandEnvironment::touchCancel(void *data, struct wl_touch *wl_touch)
{
}

void LinuxWaylandEnvironment::touchShape(void *data, struct wl_touch *wl_touch, int32_t id, wl_fixed_t major, wl_fixed_t minor)
{
}

void LinuxWaylandEnvironment::touchOrientation(void *data, struct wl_touch *wl_touch, int32_t id, wl_fixed_t orientation)
{
}

void LinuxWaylandEnvironment::outputGeometry(void *data, struct wl_output *wl_output, int32_t x, int32_t y, int32_t physical_width, int32_t physical_height, int32_t subpixel, const char *make, const char *model, int32_t transform)
{
	auto output = (Output *)data;
	output->x = x;
	output->y = y;
}

void LinuxWaylandEnvironment::outputMode(void *data, struct wl_output *wl_output, uint32_t flags, int32_t width, int32_t height, int32_t refresh)
{
	auto output = (Output *)data;
	if (flags & WL_OUTPUT_MODE_CURRENT)
	{
		output->width = width;
		output->height = height;
	}
}

void LinuxWaylandEnvironment::outputDone(void *data, struct wl_output *wl_output)
{
	auto output = (Output *)data;
	output->geometry.set(output->x, output->y, output->width, output->height);
}

void LinuxWaylandEnvironment::outputScale(void *data, struct wl_output *wl_output, int32_t factor)
{
	auto output = (Output *)data;
	output->dpi = factor * 96;
}

void LinuxWaylandEnvironment::confinedPointerConfined(void *data, struct zwp_confined_pointer_v1 *zwp_confined_pointer_v1)
{
	g_environment->m_bPointerIsConfined = true;
}

void LinuxWaylandEnvironment::confinedPointerUnconfined(void *data, struct zwp_confined_pointer_v1 *zwp_confined_pointer_v1)
{
	g_environment->m_bPointerIsConfined = false;
}

void LinuxWaylandEnvironment::dataDeviceDataOffer(void *data, struct wl_data_device *wl_data_device, struct wl_data_offer *id)
{
}

void LinuxWaylandEnvironment::dataDeviceEnter(void *data, struct wl_data_device *wl_data_device, uint32_t serial, struct wl_surface *surface, wl_fixed_t x, wl_fixed_t y, struct wl_data_offer *id)
{
}

void LinuxWaylandEnvironment::dataDeviceLeave(void *data, struct wl_data_device *wl_data_device)
{
}

void LinuxWaylandEnvironment::dataDeviceMotion(void *data, struct wl_data_device *wl_data_device, uint32_t time, wl_fixed_t x, wl_fixed_t y)
{
}

void LinuxWaylandEnvironment::dataDeviceDrop(void *data, struct wl_data_device *wl_data_device)
{
}

void LinuxWaylandEnvironment::dataDeviceSelection(void *data, struct wl_data_device *wl_data_device, struct wl_data_offer *id)
{
	// TODO
}

void LinuxWaylandEnvironment::dataSourceTarget(void *data, struct wl_data_source *wl_data_source, const char *mime_type)
{
}

void LinuxWaylandEnvironment::dataSourceSend(void *data, struct wl_data_source *wl_data_source, const char *mime_type, int32_t fd)
{
	size_t length = g_environment->m_selection.length();
	const char *text = g_environment->m_selection.toUtf8();
	size_t n = write(fd, text, length);
	close(fd);
}

void LinuxWaylandEnvironment::dataSourceCancelled(void *data, struct wl_data_source *wl_data_source)
{
	wl_data_source_destroy(wl_data_source);
}

void LinuxWaylandEnvironment::dataSourceDndDropPerformed(void *data, struct wl_data_source *wl_data_source)
{
}

void LinuxWaylandEnvironment::dataSourceDndFinished(void *data, struct wl_data_source *wl_data_source)
{
}

void LinuxWaylandEnvironment::dataSourceAction(void *data, struct wl_data_source *wl_data_source, uint32_t dnd_action)
{
}

void LinuxWaylandEnvironment::relativePointerRelativeMotion(void *data, struct zwp_relative_pointer_v1 *zwp_relative_pointer_v1, uint32_t utime_hi, uint32_t utime_lo, wl_fixed_t dx, wl_fixed_t dy, wl_fixed_t dx_unaccel, wl_fixed_t dy_unaccel)
{
	g_engine->onMouseRawMove(wl_fixed_to_int(dx_unaccel), wl_fixed_to_int(dy_unaccel));
}

}

#endif

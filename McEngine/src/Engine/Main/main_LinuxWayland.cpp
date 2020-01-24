#ifdef __linux__

#include <string.h>

#include <poll.h>
#include <unistd.h>

#include <wayland-client.h>
#include <wayland-egl.h>

#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-unstable-v1-client-protocol.h"

#include "LinuxWaylandEnvironment.h"
#include "Engine.h"
#include "Timer.h"
#include "ConVar.h"

#undef wl_array_for_each
#define wl_array_for_each(pos, array) for (pos = (decltype(pos))(array)->data; (const char *)pos < ((const char *)(array)->data + (array)->size); (pos)++)

extern char **environ;

extern Engine *g_engine;
extern LinuxWaylandEnvironment *g_environment;

extern ConVar fps_max;
extern ConVar fps_max_background;
extern ConVar fps_unlimited;

bool g_bVSync;
bool g_bShouldDraw;
extern bool g_bRunning;

struct wl_compositor *g_compositor;
struct wl_shm *g_shm;
struct xdg_wm_base *g_xdgWmBase;
struct zxdg_decoration_manager_v1 *g_xdgDecorationManager;

struct
{
	bool bShouldConfigure;
	uint32_t uWidth;
	uint32_t uHeight;
	bool bMaximized;
	bool bFullscreen;
	bool bActivated;
} g_pendingState;

extern "C"
{

static void xdgToplevelConfigure(void *data, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height, struct wl_array *states)
{
	g_pendingState.uWidth = width ? width : 1280;
	g_pendingState.uHeight = height ? height : 720;
	g_pendingState.bMaximized = false;
	g_pendingState.bFullscreen = false;
	g_pendingState.bActivated = false;
	xdg_toplevel_state *state;
	wl_array_for_each (state, states)
	{
		if (*state == XDG_TOPLEVEL_STATE_MAXIMIZED)
			g_pendingState.bMaximized = true;
		if (*state == XDG_TOPLEVEL_STATE_FULLSCREEN)
			g_pendingState.bFullscreen = true;
		if (*state == XDG_TOPLEVEL_STATE_ACTIVATED)
			g_pendingState.bActivated = true;
	}
}

static void xdgToplevelClose(void *data, struct xdg_toplevel *xdg_toplevel)
{
	g_bRunning = false;
}

static void xdgSurfaceConfigure(void *data, struct xdg_surface *xdg_surface, uint32_t serial)
{
	xdg_surface_ack_configure(xdg_surface, serial);
	g_pendingState.bShouldConfigure = true;
}

static void xdgWmBasePing(void *data, struct xdg_wm_base *wm_base, uint32_t serial)
{
	xdg_wm_base_pong(wm_base, serial);
}

static void frameCallbackDone(void *data, struct wl_callback *wl_callback, uint32_t callback_data)
{
	g_bShouldDraw = true;
	wl_callback_destroy(wl_callback);
}

static void registryGlobal(void *data, struct wl_registry *wl_registry, uint32_t name, const char *interface, uint32_t version)
{
	if (strcmp(interface, wl_compositor_interface.name) == 0)
	{
		g_compositor = (struct wl_compositor *)
			wl_registry_bind(wl_registry, name, &wl_compositor_interface, 4);
	}
	else if (strcmp(interface, wl_shm_interface.name) == 0)
	{
		g_shm = (struct wl_shm *)
			wl_registry_bind(wl_registry, name, &wl_shm_interface, 1);
	}
	else if (strcmp(interface, xdg_wm_base_interface.name) == 0)
	{
		g_xdgWmBase = (struct xdg_wm_base *)
			wl_registry_bind(wl_registry, name, &xdg_wm_base_interface, 1);
	}
	else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0)
	{
		g_xdgDecorationManager = (struct zxdg_decoration_manager_v1 *)
			wl_registry_bind(wl_registry, name, &zxdg_decoration_manager_v1_interface, 1);
	}
}

static void registryGlobalRemove(void *data, struct wl_registry *wl_registry, uint32_t name)
{
}

}

const struct xdg_toplevel_listener xdgToplevelListener =
{
	xdgToplevelConfigure,
	xdgToplevelClose
};

const struct xdg_surface_listener xdgSurfaceListener =
{
	xdgSurfaceConfigure
};

const struct xdg_wm_base_listener xdgWmBaseListener =
{
	xdgWmBasePing
};

const struct wl_callback_listener frameCallbackListener =
{
	frameCallbackDone
};

const struct wl_registry_listener registryListener =
{
	registryGlobal,
	registryGlobalRemove
};

int mainWayland(int argc, char *argv[], struct wl_display *display)
{
	struct wl_registry *registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registryListener, NULL);
	wl_display_roundtrip(display);

	xdg_wm_base_add_listener(g_xdgWmBase, &xdgWmBaseListener, NULL);

	struct wl_surface *surface = wl_compositor_create_surface(g_compositor);
	wl_surface_add_listener(surface, &LinuxWaylandEnvironment::surfaceListener, NULL);

	struct xdg_surface *xdgSurface = xdg_wm_base_get_xdg_surface(g_xdgWmBase, surface);
	xdg_surface_add_listener(xdgSurface, &xdgSurfaceListener, NULL);

	struct xdg_toplevel *xdgToplevel = xdg_surface_get_toplevel(xdgSurface);
	xdg_toplevel_add_listener(xdgToplevel, &xdgToplevelListener, NULL);
	xdg_toplevel_set_app_id(xdgToplevel, "McEngine");
	xdg_toplevel_set_title(xdgToplevel, "McEngine");

	if (g_xdgDecorationManager)
	{
		struct zxdg_toplevel_decoration_v1 *toplevelDecoration =
			zxdg_decoration_manager_v1_get_toplevel_decoration(g_xdgDecorationManager, xdgToplevel);
		zxdg_toplevel_decoration_v1_set_mode(toplevelDecoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
	}

	wl_surface_commit(surface);

	struct wl_egl_window *eglWindow = wl_egl_window_create(surface, 1280, 720);

	struct wl_callback *frameCallback = wl_surface_frame(surface);
	wl_callback_add_listener(frameCallback, &frameCallbackListener, NULL);

	wl_display_roundtrip(display);

	g_environment = new LinuxWaylandEnvironment(display, surface, xdgToplevel, eglWindow, g_compositor, g_shm);
	g_engine = new Engine(g_environment, argc > 1 ? argv[1] : "");
	g_engine->loadApp();
	g_engine->onUpdate();

	Timer *timer = new Timer();
	timer->start();
	timer->update();

	pollfd fds[] =
	{
		{wl_display_get_fd(display), POLLIN},
	};

	while (g_bRunning)
	{
		if (g_pendingState.bShouldConfigure)
		{
			wl_egl_window_resize(eglWindow, g_pendingState.uWidth, g_pendingState.uHeight, 0, 0);
			g_engine->requestResolutionChange(Vector2(g_pendingState.uWidth, g_pendingState.uHeight));
			g_environment->m_vWindowSize = Vector2(g_pendingState.uWidth, g_pendingState.uHeight);
			g_environment->m_bIsMaximized = g_pendingState.bMaximized;
			g_environment->m_bIsFullscreen = g_pendingState.bFullscreen;
			g_environment->m_bIsActivated = g_pendingState.bActivated;
			g_pendingState.bShouldConfigure = false;
			g_environment->reconfigure();
			g_bShouldDraw = true;
		}

		if (!g_bVSync || g_bShouldDraw)
		{
			g_engine->onUpdate();
			timer->update();
			g_engine->setFrameTime(timer->getDelta());
			g_engine->onPaint();
			struct wl_callback *frameCallback = wl_surface_frame(surface);
			wl_callback_add_listener(frameCallback, &frameCallbackListener, NULL);
			g_bShouldDraw = false;
		}

		wl_display_flush(display);

		while (wl_display_prepare_read(display) != 0)
			wl_display_dispatch_pending(display);

		int polls = poll(fds, 1, 0);

		if (polls == 1 && fds[0].revents & POLLIN)
			wl_display_read_events(display);
		else
			wl_display_cancel_read(display);

		if (wl_display_dispatch_pending(display) == -1)
			break;
	}

	g_engine->onShutdown();
	bool restartScheduled = g_environment->m_bIsRestartScheduled;

	SAFE_DELETE(g_engine);

	wl_display_disconnect(display);

	if (restartScheduled)
		execve(argv[0], argv, environ);

	return 0;
}

#endif

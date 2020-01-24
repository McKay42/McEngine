#ifdef __linux__

#ifndef LINUXEGLLEGACYINTERFACE_H

#include "OpenGLLegacyInterface.h"

#include <wayland-client.h>
#include <wayland-egl.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#ifdef MCENGINE_FEATURE_OPENGL

class LinuxEGLLegacyInterface : public OpenGLLegacyInterface
{
public:
	LinuxEGLLegacyInterface(struct wl_display *display, struct wl_egl_window *window);
	virtual ~LinuxEGLLegacyInterface();

	void endScene();
	void setVSync(bool vsync);

private:
	EGLDisplay m_display;
	EGLSurface m_surface;
	EGLContext m_context;
};

#endif

#endif

#endif

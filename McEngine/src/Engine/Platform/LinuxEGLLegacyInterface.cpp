#ifdef __linux__

#include "LinuxEGLLegacyInterface.h"

#ifdef MCENGINE_FEATURE_OPENGL

extern bool g_bVSync;

LinuxEGLLegacyInterface::LinuxEGLLegacyInterface(struct wl_display *display,
	struct wl_egl_window *window) : OpenGLLegacyInterface()
{
	m_display = eglGetPlatformDisplay(EGL_PLATFORM_WAYLAND_KHR, display, NULL);
	eglInitialize(m_display, NULL, NULL);
	eglBindAPI(EGL_OPENGL_API);

	EGLint attributes[] = {
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 0,
		EGL_CONFORMANT, EGL_OPENGL_BIT,
		EGL_MIN_SWAP_INTERVAL, 0,
	EGL_NONE};
	EGLConfig config;
	EGLint num_config;
	eglChooseConfig(m_display, attributes, &config, 1, &num_config);

	m_context = eglCreateContext(m_display, config, EGL_NO_CONTEXT, NULL);
	m_surface = eglCreateWindowSurface(m_display, config, window, NULL);

	eglMakeCurrent(m_display, m_surface, m_surface, m_context);
}

LinuxEGLLegacyInterface::~LinuxEGLLegacyInterface()
{
	eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroyContext(m_display, m_context);
	eglTerminate(m_display);
}

void LinuxEGLLegacyInterface::endScene() {
	OpenGLLegacyInterface::endScene();
	eglSwapInterval(m_display, 0);
	eglSwapBuffers(m_display, m_surface);
}

void LinuxEGLLegacyInterface::setVSync(bool vsync) {
	g_bVSync = vsync;
}

#endif

#endif

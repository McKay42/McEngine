//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		individual engine components to compile with
//
// $NoKeywords: $feat
//===============================================================================//

#ifndef ENGINEFEATURES_H
#define ENGINEFEATURES_H

/*
 * std::thread/std::mutex support
 */
#define MCENGINE_FEATURE_MULTITHREADING

/*
 * pthread support
 */
#define MCENGINE_FEATURE_PTHREADS

/*
 * OpenGL graphics (Desktop, legacy + modern)
 */
#define MCENGINE_FEATURE_OPENGL

/*
 * OpenGL graphics (Mobile, ES/EGL)
 */
//#define MCENGINE_FEATURE_OPENGLES

/*
 * DirectX graphics
 */
//#define MCENGINE_FEATURE_DIRECTX

/*
 * ENet & CURL networking
 */
//#define MCENGINE_FEATURE_NETWORKING

/*
 * BASS sound
 */
#define MCENGINE_FEATURE_SOUND

/*
 * BASS WASAPI sound (Windows only)
 */
//#define MCENGINE_FEATURE_BASS_WASAPI

/*
 * XInput gamepads
 */
//#define MCENGINE_FEATURE_GAMEPAD

/*
 * OpenCL
 */
//#define MCENGINE_FEATURE_OPENCL

/*
 * Vulkan
 */
//#define MCENGINE_FEATURE_VULKAN

/*
 * OpenVR
 */
//#define MCENGINE_FEATURE_OPENVR

/*
 * Squirrel Scripting
 */
//#define MCENGINE_FEATURE_SQUIRREL

/*
 * SDL2 main + Environment
 */
//#define MCENGINE_FEATURE_SDL

/*
 * SDL2 mixer (audio)
 */
//#define MCENGINE_FEATURE_SDL_MIXER

/*
 * Discord RPC (rich presence)
 */
//#define MCENGINE_FEATURE_DISCORD

/*
 * Steam
 */
//#define MCENGINE_FEATURE_STEAMWORKS

#endif

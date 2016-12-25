//================ Copyright (c) 2012, PG, All rights reserved. =================//
//
// Purpose:		shader wrapper
//
// $NoKeywords: $shader
//===============================================================================//

#include "Shader.h"
#include "ConVar.h"

ConVar _debug_shaders("debug_shaders", false);
ConVar *Shader::debug_shaders = &_debug_shaders;



//================ Copyright (c) 2012, PG, All rights reserved. =================//
//
// Purpose:		shader wrapper
//
// $NoKeywords: $shader
//===============================================================================//

#include "Shader.h"

#include "Engine.h"
#include "ConVar.h"
#include "File.h"

#include <sstream>

ConVar _debug_shaders("debug_shaders", false, FCVAR_NONE);
ConVar *Shader::debug_shaders = &_debug_shaders;

Shader::SHADER_PARSE_RESULT Shader::parseShaderFromFileOrString(UString graphicsInterfaceAndShaderTypePrefix, UString shaderSourceOrFilePath, bool source)
{
	const UString shaderPrefix = "###";	// e.g. ###OpenGLLegacyInterface::VertexShader##########################################################################################
	const UString descPrefix = "##";	// e.g. ##D3D11_BUFFER_DESC::D3D11_BIND_CONSTANT_BUFFER::ModelViewProjectionConstantBuffer::mvp::float4x4
	const UString newline = "\n";

	SHADER_PARSE_RESULT result;
	{
		if (!source)
			debugLog("Shader: Loading %s %s ...\n", graphicsInterfaceAndShaderTypePrefix.toUtf8(), shaderSourceOrFilePath.toUtf8());

		File file(!source ? shaderSourceOrFilePath : "");
		if (!source && !file.canRead())
		{
			engine->showMessageError("Shader Error", UString::format("Failed to load/read file %s", shaderSourceOrFilePath.toUtf8()));
			return result;
		}
		std::istringstream ss(source ? shaderSourceOrFilePath.toUtf8() : "");

		bool foundGraphicsInterfaceAndShaderTypePrefixAtLeastOnce = false;
		bool foundGraphicsInterfaceAndShaderTypePrefix = false;
		std::string curLine;
		while (!source ? file.canRead() : static_cast<bool>(std::getline(ss, curLine)))
		{
			UString uCurLine;
			{
				if (!source)
					uCurLine = file.readLine();
				else
					uCurLine = UString(curLine.c_str(), curLine.length());
			}

			const bool isShaderPrefixLine = (uCurLine.find(shaderPrefix) == 0);

			if (!isShaderPrefixLine && foundGraphicsInterfaceAndShaderTypePrefix)
			{
				const bool isDescPrefixLine = (uCurLine.find(descPrefix) == 0);

				if (!isDescPrefixLine)
				{
					result.source.append(uCurLine);
					result.source.append(newline);
				}
				else
				{
					result.source.append(newline);

					result.descs.push_back(uCurLine.substr(descPrefix.length()));
				}
			}
			else
				result.source.append(newline);

			if (isShaderPrefixLine)
			{
				if (!foundGraphicsInterfaceAndShaderTypePrefix)
				{
					if (uCurLine.find(graphicsInterfaceAndShaderTypePrefix) == shaderPrefix.length())
					{
						foundGraphicsInterfaceAndShaderTypePrefix = true;
						foundGraphicsInterfaceAndShaderTypePrefixAtLeastOnce = true;
					}
				}
				else
					foundGraphicsInterfaceAndShaderTypePrefix = false;
			}
		}
		if (!foundGraphicsInterfaceAndShaderTypePrefixAtLeastOnce)
			engine->showMessageError("Shader Error", UString::format("Missing \"%s\" in file %s", graphicsInterfaceAndShaderTypePrefix.toUtf8(), shaderSourceOrFilePath.toUtf8()));
	}
	return result;
}

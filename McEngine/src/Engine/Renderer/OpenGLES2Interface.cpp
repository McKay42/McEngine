//================ Copyright (c) 2019, PG, All rights reserved. =================//
//
// Purpose:		raw opengl es 2.x graphics interface
//
// $NoKeywords: $gles2i
//===============================================================================//

#include "OpenGLES2Interface.h"

#ifdef MCENGINE_FEATURE_OPENGLES

#include "Engine.h"
#include "ConVar.h"
#include "Camera.h"

#include "Font.h"
#include "OpenGLImage.h"
#include "OpenGLRenderTarget.h"
#include "OpenGLES2Shader.h"
#include "OpenGLES2VertexArrayObject.h"

#include "OpenGLHeaders.h"

#define GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX			0x9047
#define GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX		0x9048
#define GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX	0x9049

#define VBO_FREE_MEMORY_ATI								0x87FB
#define TEXTURE_FREE_MEMORY_ATI							0x87FC
#define RENDERBUFFER_FREE_MEMORY_ATI					0x87FD

OpenGLES2Interface::OpenGLES2Interface() : NullGraphicsInterface()
{
	// renderer
	m_bInScene = false;
	m_vResolution = engine->getScreenSize(); // initial viewport size = window size

	m_shaderTexturedGeneric = NULL;
	m_iShaderTexturedGenericPrevType = 0;
	m_iShaderTexturedGenericAttribPosition = 0;
	m_iShaderTexturedGenericAttribUV = 1;
	m_iShaderTexturedGenericAttribCol = 2;
	m_iVBOVertices = 0;
	m_iVBOTexcoords = 0;
	m_iVBOTexcolors = 0;

	// persistent vars
	m_color = 0xffffffff;
}

OpenGLES2Interface::~OpenGLES2Interface()
{
	SAFE_DELETE(m_shaderTexturedGeneric);

	if (m_iVBOVertices != 0)
		glDeleteBuffers(1, &m_iVBOVertices);
	if (m_iVBOTexcoords != 0)
		glDeleteBuffers(1, &m_iVBOTexcoords);
	if (m_iVBOTexcolors != 0)
		glDeleteBuffers(1, &m_iVBOTexcolors);
}

void OpenGLES2Interface::init()
{
	// check GL version
	const GLubyte *version = glGetString(GL_VERSION);
	debugLog("OpenGL: OpenGL Version %s\n", version);

	// enable
	glEnable(GL_BLEND);

	// disable
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	// blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	// culling
	glFrontFace(GL_CCW);

	//setWireframe(true);

	UString texturedGenericV =	"#version 100\n"
								"\n"
								"attribute vec3 position;\n"
								"attribute vec2 uv;\n"
								"attribute vec4 vcolor;\n"
								"\n"
								"varying vec2 texcoords;\n"
								"varying vec4 texcolor;\n"
								"\n"
								"uniform float type;\n"
								"uniform mat4 mvp;\n"
								"\n"
								"void main() {\n"
								"	texcoords = uv;\n"
								"	texcolor = vcolor;\n"
								"	gl_Position = mvp * vec4(position, 1.0);\n"
								"}\n"
								"\n";

	UString texturedGenericP =	"#version 100\n"
								"precision highp float;\n"
								"\n"
								"varying vec2 texcoords;\n"
								"varying vec4 texcolor;\n"
								"\n"
								"uniform float type;\n"
								"uniform vec4 col;\n"
								"uniform sampler2D tex;\n"
								"\n"
								"void main() {\n"
								//"	gl_FragColor = col;\n"
								"	gl_FragColor = mix(col, mix(texture2D(tex, texcoords) * col, texcolor, clamp(type - 1.0, 0.0, 1.0)), clamp(type, 0.0, 1.0));\n"
								"}\n"
								"\n";
	m_shaderTexturedGeneric = (OpenGLES2Shader*)createShaderFromSource(texturedGenericV, texturedGenericP);
	m_shaderTexturedGeneric->load();

	glGenBuffers(1, &m_iVBOVertices);
	glGenBuffers(1, &m_iVBOTexcoords);
	glGenBuffers(1, &m_iVBOTexcolors);

	m_iShaderTexturedGenericAttribPosition = m_shaderTexturedGeneric->getAttribLocation("position");
	m_iShaderTexturedGenericAttribUV = m_shaderTexturedGeneric->getAttribLocation("uv");
	m_iShaderTexturedGenericAttribCol = m_shaderTexturedGeneric->getAttribLocation("vcolor");

	// TODO: handle cases where more than 16384 elements are in an unbaked vao
	glBindBuffer(GL_ARRAY_BUFFER, m_iVBOVertices);
	glVertexAttribPointer(m_iShaderTexturedGenericAttribPosition, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glBufferData(GL_ARRAY_BUFFER, 16384*sizeof(Vector3), NULL, GL_STREAM_DRAW);
	glEnableVertexAttribArray(m_iShaderTexturedGenericAttribPosition);

	glBindBuffer(GL_ARRAY_BUFFER, m_iVBOTexcoords);
	glVertexAttribPointer(m_iShaderTexturedGenericAttribUV, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glBufferData(GL_ARRAY_BUFFER, 16384*sizeof(Vector2), NULL, GL_STREAM_DRAW);
	glEnableVertexAttribArray(m_iShaderTexturedGenericAttribUV);

	glBindBuffer(GL_ARRAY_BUFFER, m_iVBOTexcolors);
	glVertexAttribPointer(m_iShaderTexturedGenericAttribCol, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glBufferData(GL_ARRAY_BUFFER, 16384*sizeof(Vector4), NULL, GL_STREAM_DRAW);
	glEnableVertexAttribArray(m_iShaderTexturedGenericAttribCol);
}

void OpenGLES2Interface::beginScene()
{
	m_bInScene = true;

	// enable default shader (must happen before any uniform calls)
	m_shaderTexturedGeneric->enable();

	Matrix4 defaultProjectionMatrix = Camera::buildMatrixOrtho2D(0, m_vResolution.x, m_vResolution.y, 0, -1.0f, 1.0f);

	// push main transforms
	pushTransform();
	setProjectionMatrix(defaultProjectionMatrix);
	translate(r_globaloffset_x->getFloat(), r_globaloffset_y->getFloat());

	// and apply them
	updateTransform();

	// set clear color and clear
	//glClearColor(1, 1, 1, 1);
	//glClearColor(0.9568f, 0.9686f, 0.9882f, 1);
	glClearColor(0, 0, 0, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// display any errors of previous frames
	handleGLErrors();
}

void OpenGLES2Interface::endScene()
{
	popTransform();

	checkStackLeaks();

	if (m_clipRectStack.size() > 0)
	{
		engine->showMessageErrorFatal("ClipRect Stack Leak", "Make sure all push*() have a pop*()!");
		engine->shutdown();
	}

	m_bInScene = false;
}

void OpenGLES2Interface::clearDepthBuffer()
{
	glClear(GL_DEPTH_BUFFER_BIT);
}

void OpenGLES2Interface::setColor(Color color)
{
	if (color == m_color) return;

	if (m_shaderTexturedGeneric->isActive())
	{
		m_color = color;
		m_shaderTexturedGeneric->setUniform4f("col", ((unsigned char)(m_color >> 16))  / 255.0f, ((unsigned char)(m_color >> 8)) / 255.0f, ((unsigned char)(m_color >> 0)) / 255.0f, ((unsigned char)(m_color >> 24)) / 255.0f);
	}
}

void OpenGLES2Interface::setAlpha(float alpha)
{
	Color tempColor = m_color;

	tempColor &= 0x00ffffff;
	tempColor |= ((int)(255.0f * alpha)) << 24;

	setColor(tempColor);
}

void OpenGLES2Interface::drawLine(int x1, int y1, int x2, int y2)
{
	updateTransform();

	VertexArrayObject vao(Graphics::PRIMITIVE::PRIMITIVE_LINES);
	vao.addVertex(x1 + 0.5f, y1 + 0.5f);
	vao.addVertex(x2 + 0.5f, y2 + 0.5f);
	drawVAO(&vao);
}

void OpenGLES2Interface::drawLine(Vector2 pos1, Vector2 pos2)
{
	drawLine(pos1.x, pos1.y, pos2.x, pos2.y);
}

void OpenGLES2Interface::drawRect(int x, int y, int width, int height)
{
	drawLine(x, y, x+width, y);
	drawLine(x, y, x, y+height);
	drawLine(x, y+height, x+width+1, y+height);
	drawLine(x+width, y, x+width, y+height);
}

void OpenGLES2Interface::drawRect(int x, int y, int width, int height, Color top, Color right, Color bottom, Color left)
{
	setColor(top);
	drawLine(x, y, x+width, y);
	setColor(left);
	drawLine(x, y, x, y+height);
	setColor(bottom);
	drawLine(x, y+height, x+width+1, y+height);
	setColor(right);
	drawLine(x+width, y, x+width, y+height);
}

void OpenGLES2Interface::fillRect(int x, int y, int width, int height)
{
	updateTransform();

	VertexArrayObject vao(Graphics::PRIMITIVE::PRIMITIVE_QUADS);
	vao.addVertex(x, y);
	vao.addVertex(x, y + height);
	vao.addVertex(x + width, y + height);
	vao.addVertex(x + width, y);
	drawVAO(&vao);
}

void OpenGLES2Interface::fillGradient(int x, int y, int width, int height, Color topLeftColor, Color topRightColor, Color bottomLeftColor, Color bottomRightColor)
{
	updateTransform();

	VertexArrayObject vao(Graphics::PRIMITIVE::PRIMITIVE_QUADS);
	vao.addVertex(x, y);
	vao.addColor(topLeftColor);
	vao.addVertex(x + width, y);
	vao.addColor(topRightColor);
	vao.addVertex(x + width, y + height);
	vao.addColor(bottomRightColor);
	vao.addVertex(x, y + height);
	vao.addColor(bottomLeftColor);
	drawVAO(&vao);
}

void OpenGLES2Interface::drawQuad(int x, int y, int width, int height)
{
	updateTransform();

	VertexArrayObject vao(Graphics::PRIMITIVE::PRIMITIVE_QUADS);
	vao.addVertex(x, y);
	vao.addTexcoord(0, 0);
	vao.addVertex(x, y + height);
	vao.addTexcoord(0, 1);
	vao.addVertex(x + width, y + height);
	vao.addTexcoord(1, 1);
	vao.addVertex(x + width, y);
	vao.addTexcoord(1, 0);
	drawVAO(&vao);
}

void OpenGLES2Interface::drawQuad(Vector2 topLeft, Vector2 topRight, Vector2 bottomRight, Vector2 bottomLeft, Color topLeftColor, Color topRightColor, Color bottomRightColor, Color bottomLeftColor)
{
	updateTransform();

	VertexArrayObject vao(Graphics::PRIMITIVE::PRIMITIVE_QUADS);
	vao.addVertex(topLeft.x, topLeft.y);
	vao.addColor(topLeftColor);
	vao.addTexcoord(0, 0);
	vao.addVertex(bottomLeft.x, bottomLeft.y);
	vao.addColor(bottomLeftColor);
	vao.addTexcoord(0, 1);
	vao.addVertex(bottomRight.x, bottomRight.y);
	vao.addColor(bottomRightColor);
	vao.addTexcoord(1, 1);
	vao.addVertex(topRight.x, topRight.y);
	vao.addColor(topRightColor);
	vao.addTexcoord(1, 0);
	drawVAO(&vao);
}

void OpenGLES2Interface::drawImage(Image *image)
{
	if (image == NULL)
	{
		debugLog("WARNING: Tried to draw image with NULL texture!\n");
		return;
	}
	if (!image->isReady())
		return;

	updateTransform();

	float width = image->getWidth();
	float height = image->getHeight();

	float x = -width/2;
	float y = -height/2;

	VertexArrayObject vao(Graphics::PRIMITIVE::PRIMITIVE_QUADS);
	vao.addVertex(x, y);
	vao.addTexcoord(0, 0);
	vao.addVertex(x, y + height);
	vao.addTexcoord(0, 1);
	vao.addVertex(x + width, y + height);
	vao.addTexcoord(1, 1);
	vao.addVertex(x + width, y);
	vao.addTexcoord(1, 0);

	image->bind();
	drawVAO(&vao);
	image->unbind();

	if (r_debug_drawimage->getBool())
	{
		setColor(0xbbff00ff);
		drawRect(x, y, width, height);
	}
}

void OpenGLES2Interface::drawString(McFont *font, UString text)
{
	if (font == NULL || text.length() < 1 || !font->isReady()) return;

	updateTransform();

	font->drawString(this, text);
}

void OpenGLES2Interface::drawVAO(VertexArrayObject *vao)
{
	if (vao == NULL) return;

	updateTransform();

	// if baked, then we can directly draw the buffer
	if (vao->isReady())
	{
		OpenGLES2VertexArrayObject *glvao = (OpenGLES2VertexArrayObject*)vao;

		// configure shader
		if (m_shaderTexturedGeneric->isActive())
		{
			if (glvao->getNumTexcoords0() > 0)
			{
				if (m_iShaderTexturedGenericPrevType != 1)
				{
					m_shaderTexturedGeneric->setUniform1f("type", 1.0f);
					m_iShaderTexturedGenericPrevType = 1;
				}
			}
			else if (m_iShaderTexturedGenericPrevType != 0)
			{
				m_shaderTexturedGeneric->setUniform1f("type", 0.0f);
				m_iShaderTexturedGenericPrevType = 0;
			}
		}

		// draw
		glvao->draw();
		return;
	}

	const std::vector<Vector3> &vertices = vao->getVertices();
	const std::vector<Vector3> &normals = vao->getNormals();
	const std::vector<std::vector<Vector2>> &texcoords = vao->getTexcoords();
	const std::vector<Color> &vcolors = vao->getColors();

	if (vertices.size() < 2) return;

	// TODO: separate draw for non-quads, update quad draws to triangle draws to avoid rewrite overhead here

	// no support for quads, because fuck you
	// rewrite all quads into triangles
	std::vector<Vector3> finalVertices = vertices;
	std::vector<std::vector<Vector2>> finalTexcoords = texcoords;
	std::vector<Vector4> colors;
	std::vector<Vector4> finalColors;

	for (size_t i=0; i<vcolors.size(); i++)
	{
		Vector4 color = Vector4(COLOR_GET_Rf(vcolors[i]), COLOR_GET_Gf(vcolors[i]), COLOR_GET_Bf(vcolors[i]), COLOR_GET_Af(vcolors[i]));
		colors.push_back(color);
		finalColors.push_back(color);
	}
	int maxColorIndex = colors.size() - 1;

	Graphics::PRIMITIVE primitive = vao->getPrimitive();
	if (primitive == Graphics::PRIMITIVE::PRIMITIVE_QUADS)
	{
		finalVertices.clear();
		for (size_t t=0; t<finalTexcoords.size(); t++)
		{
			finalTexcoords[t].clear();
		}
		finalColors.clear();
		primitive = Graphics::PRIMITIVE::PRIMITIVE_TRIANGLES;

		if (vertices.size() > 3)
		{
			for (size_t i=0; i<vertices.size(); i+=4)
			{
				finalVertices.push_back(vertices[i + 0]);
				finalVertices.push_back(vertices[i + 1]);
				finalVertices.push_back(vertices[i + 2]);

				for (size_t t=0; t<texcoords.size(); t++)
				{
					finalTexcoords[t].push_back(texcoords[t][i + 0]);
					finalTexcoords[t].push_back(texcoords[t][i + 1]);
					finalTexcoords[t].push_back(texcoords[t][i + 2]);
				}

				if (colors.size() > 0)
				{
					finalColors.push_back(colors[clamp<int>(i + 0, 0, maxColorIndex)]);
					finalColors.push_back(colors[clamp<int>(i + 1, 0, maxColorIndex)]);
					finalColors.push_back(colors[clamp<int>(i + 2, 0, maxColorIndex)]);
				}

				finalVertices.push_back(vertices[i + 0]);
				finalVertices.push_back(vertices[i + 2]);
				finalVertices.push_back(vertices[i + 3]);

				for (size_t t=0; t<texcoords.size(); t++)
				{
					finalTexcoords[t].push_back(texcoords[t][i + 0]);
					finalTexcoords[t].push_back(texcoords[t][i + 2]);
					finalTexcoords[t].push_back(texcoords[t][i + 3]);
				}

				if (colors.size() > 0)
				{
					finalColors.push_back(colors[clamp<int>(i + 0, 0, maxColorIndex)]);
					finalColors.push_back(colors[clamp<int>(i + 2, 0, maxColorIndex)]);
					finalColors.push_back(colors[clamp<int>(i + 3, 0, maxColorIndex)]);
				}
			}
		}
	}

	// upload vertices to gpu
	if (finalVertices.size() > 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_iVBOVertices);
		glBufferSubData(GL_ARRAY_BUFFER, 0, finalVertices.size() * sizeof(Vector3), &(finalVertices[0]));
	}

	// upload texcoords to gpu
	if (finalTexcoords.size() > 0 && finalTexcoords[0].size() > 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_iVBOTexcoords);
		glBufferSubData(GL_ARRAY_BUFFER, 0, finalTexcoords[0].size() * sizeof(Vector2), &(finalTexcoords[0][0]));
	}

	// upload vertex colors to gpu
	if (finalColors.size() > 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_iVBOTexcolors);
		glBufferSubData(GL_ARRAY_BUFFER, 0, finalColors.size() * sizeof(Vector4), &(finalColors[0]));
	}

	// configure shader
	if (m_shaderTexturedGeneric->isActive())
	{
		// TODO: multitexturing support
		if (finalTexcoords.size() > 0 && finalTexcoords[0].size() > 0)
		{
			if (m_iShaderTexturedGenericPrevType != 1)
			{
				m_shaderTexturedGeneric->setUniform1f("type", 1.0f);
				m_iShaderTexturedGenericPrevType = 1;
			}
		}
		else if (m_iShaderTexturedGenericPrevType != 0)
		{
			m_shaderTexturedGeneric->setUniform1f("type", 0.0f);
			m_iShaderTexturedGenericPrevType = 0;
		}

		if (finalColors.size() > 0)
		{
			if (m_iShaderTexturedGenericPrevType != 2)
			{
				m_shaderTexturedGeneric->setUniform1f("type", 2.0f);
				m_iShaderTexturedGenericPrevType = 2;
			}
		}
	}

	// draw it
	glDrawArrays(primitiveToOpenGL(primitive), 0, finalVertices.size());
}

void OpenGLES2Interface::setClipRect(McRect clipRect)
{
	if (r_debug_disable_cliprect->getBool()) return;
	//if (m_bIs3DScene) return; // HACKHACK:TODO:

	// HACKHACK: compensate for viewport changes caused by RenderTargets!
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	//debugLog("viewport = %i, %i, %i, %i\n", viewport[0], viewport[1], viewport[2], viewport[3]);

	glEnable(GL_SCISSOR_TEST);
	glScissor((int)clipRect.getX()+viewport[0], viewport[3]-((int)clipRect.getY()-viewport[1]-1+(int)clipRect.getHeight()), (int)clipRect.getWidth(), (int)clipRect.getHeight());

	//debugLog("scissor = %i, %i, %i, %i\n", (int)clipRect.getX()+viewport[0], viewport[3]-((int)clipRect.getY()-viewport[1]-1+(int)clipRect.getHeight()), (int)clipRect.getWidth(), (int)clipRect.getHeight());
}

void OpenGLES2Interface::pushClipRect(McRect clipRect)
{
	if (m_clipRectStack.size() > 0)
		m_clipRectStack.push(m_clipRectStack.top().intersect(clipRect));
	else
		m_clipRectStack.push(clipRect);

	setClipRect(m_clipRectStack.top());
}

void OpenGLES2Interface::popClipRect()
{
	m_clipRectStack.pop();

	if (m_clipRectStack.size() > 0)
		setClipRect(m_clipRectStack.top());
	else
		setClipping(false);
}

void OpenGLES2Interface::setClipping(bool enabled)
{
	if (enabled)
	{
		if (m_clipRectStack.size() > 0)
			glEnable(GL_SCISSOR_TEST);
	}
	else
		glDisable(GL_SCISSOR_TEST);
}

void OpenGLES2Interface::setAlphaTesting(bool enabled)
{
	if (enabled)
		glEnable(GL_ALPHA_TEST);
	else
		glDisable(GL_ALPHA_TEST);
}

void OpenGLES2Interface::setAlphaTestFunc(COMPARE_FUNC alphaFunc, float ref)
{
	glAlphaFunc(compareFuncToOpenGL(alphaFunc), ref);
}

void OpenGLES2Interface::setBlending(bool enabled)
{
	if (enabled)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);
}

void OpenGLES2Interface::setBlendMode(BLEND_MODE blendMode)
{
	switch (blendMode)
	{
	case BLEND_MODE::BLEND_MODE_ALPHA:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case BLEND_MODE::BLEND_MODE_ADDITIVE:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;
	case BLEND_MODE::BLEND_MODE_PREMUL_ALPHA:
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case BLEND_MODE::BLEND_MODE_PREMUL_COLOR:
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		break;
	}
}

void OpenGLES2Interface::setDepthBuffer(bool enabled)
{
	if (enabled)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
}

void OpenGLES2Interface::setCulling(bool culling)
{
	if (culling)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
}

void OpenGLES2Interface::setWireframe(bool enabled)
{
	// only GL_FILL is supported
}

int OpenGLES2Interface::getVRAMTotal()
{
	int nvidiaMemory[4];
	int atiMemory[4];
	
	for (int i=0; i<4; i++)
	{
		nvidiaMemory[i] = -1;
		atiMemory[i] = -1;
	}

	glGetIntegerv(GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, nvidiaMemory);
	glGetIntegerv(TEXTURE_FREE_MEMORY_ATI, atiMemory);

	glGetError(); // clear error state

	if (nvidiaMemory[0] < 1)
		return atiMemory[0];
	else
		return nvidiaMemory[0];
}

int OpenGLES2Interface::getVRAMRemaining()
{
	int nvidiaMemory[4];
	int atiMemory[4];
	
	for (int i=0; i<4; i++)
	{
		nvidiaMemory[i] = -1;
		atiMemory[i] = -1;
	}

	glGetIntegerv(GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, nvidiaMemory);
	glGetIntegerv(TEXTURE_FREE_MEMORY_ATI, atiMemory);

	glGetError(); // clear error state

	if (nvidiaMemory[0] < 1)
		return atiMemory[0];
	else
		return nvidiaMemory[0];
}

void OpenGLES2Interface::onResolutionChange(Vector2 newResolution)
{
	// rebuild viewport
	m_vResolution = newResolution;
	glViewport(0, 0, m_vResolution.x, m_vResolution.y);

	// special case: custom rendertarget resolution rendering, update active projection matrix immediately
	if (m_bInScene)
	{
		m_projectionTransformStack.top() = Camera::buildMatrixOrtho2D(0, m_vResolution.x, m_vResolution.y, 0, -1.0f, 1.0f);
		m_bTransformUpToDate = false;
	}
}

Image *OpenGLES2Interface::createImage(UString filePath, bool mipmapped, bool keepInSystemMemory)
{
	return new OpenGLImage(filePath, mipmapped, keepInSystemMemory);
}

Image *OpenGLES2Interface::createImage(int width, int height, bool mipmapped, bool keepInSystemMemory)
{
	return new OpenGLImage(width, height, mipmapped, keepInSystemMemory);
}

RenderTarget *OpenGLES2Interface::createRenderTarget(int x, int y, int width, int height, Graphics::MULTISAMPLE_TYPE multiSampleType)
{
	return new OpenGLRenderTarget(x, y, width, height, multiSampleType);
}

Shader *OpenGLES2Interface::createShaderFromFile(UString vertexShaderFilePath, UString fragmentShaderFilePath)
{
	return new OpenGLES2Shader(vertexShaderFilePath, fragmentShaderFilePath, false);
}

Shader *OpenGLES2Interface::createShaderFromSource(UString vertexShader, UString fragmentShader)
{
	return new OpenGLES2Shader(vertexShader, fragmentShader, true);
}

Shader *OpenGLES2Interface::createShaderFromFile(UString shaderFilePath)
{
	return new OpenGLES2Shader(shaderFilePath, false);
}

Shader *OpenGLES2Interface::createShaderFromSource(UString shaderSource)
{
	return new OpenGLES2Shader(shaderSource, true);
}

VertexArrayObject *OpenGLES2Interface::createVertexArrayObject(Graphics::PRIMITIVE primitive, Graphics::USAGE_TYPE usage, bool keepInSystemMemory)
{
	return new OpenGLES2VertexArrayObject(primitive, usage, keepInSystemMemory);
}

void OpenGLES2Interface::forceUpdateTransform()
{
	updateTransform();
}

void OpenGLES2Interface::onTransformUpdate(Matrix4 &projectionMatrix, Matrix4 &worldMatrix)
{
	m_projectionMatrix = projectionMatrix;
	m_worldMatrix = worldMatrix;

	m_MP = m_projectionMatrix * m_worldMatrix;

	if (m_shaderTexturedGeneric->isActive())
		m_shaderTexturedGeneric->setUniformMatrix4fv("mvp", m_MP);
}

void OpenGLES2Interface::handleGLErrors()
{
	int error = glGetError();
	if (error != 0)
		debugLog("OpenGL Error: %i on frame %i\n",error,engine->getFrameCount());
}

int OpenGLES2Interface::primitiveToOpenGL(Graphics::PRIMITIVE primitive)
{
	switch (primitive)
	{
	case Graphics::PRIMITIVE::PRIMITIVE_LINES:
		return GL_LINES;
	case Graphics::PRIMITIVE::PRIMITIVE_LINE_STRIP:
		return GL_LINE_STRIP;
	case Graphics::PRIMITIVE::PRIMITIVE_TRIANGLES:
		return GL_TRIANGLES;
	case Graphics::PRIMITIVE::PRIMITIVE_TRIANGLE_FAN:
		return GL_TRIANGLE_FAN;
	case Graphics::PRIMITIVE::PRIMITIVE_TRIANGLE_STRIP:
		return GL_TRIANGLE_STRIP;
	case Graphics::PRIMITIVE::PRIMITIVE_QUADS:
		return 0; // not supported
	}

	return GL_TRIANGLES;
}

int OpenGLES2Interface::compareFuncToOpenGL(Graphics::COMPARE_FUNC compareFunc)
{
	switch (compareFunc)
	{
	case Graphics::COMPARE_FUNC::COMPARE_FUNC_NEVER:
		return GL_NEVER;
	case Graphics::COMPARE_FUNC::COMPARE_FUNC_LESS:
		return GL_LESS;
	case Graphics::COMPARE_FUNC::COMPARE_FUNC_EQUAL:
		return GL_EQUAL;
	case Graphics::COMPARE_FUNC::COMPARE_FUNC_LESSEQUAL:
		return GL_LEQUAL;
	case Graphics::COMPARE_FUNC::COMPARE_FUNC_GREATER:
		return GL_GREATER;
	case Graphics::COMPARE_FUNC::COMPARE_FUNC_NOTEQUAL:
		return GL_NOTEQUAL;
	case Graphics::COMPARE_FUNC::COMPARE_FUNC_GREATEREQUAL:
		return GL_GEQUAL;
	case Graphics::COMPARE_FUNC::COMPARE_FUNC_ALWAYS:
		return GL_ALWAYS;
	}

	return GL_ALWAYS;
}

#endif

//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		openvr wrapper and vr renderer
//
// $NoKeywords: $vr
//===============================================================================//

#include "OpenVRInterface.h"

#include "Engine.h"
#include "ConVar.h"
#include "Environment.h"
#include "ResourceManager.h"
#include "Shader.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Camera.h"
#include "RenderTarget.h"

#include "CBaseUIContainer.h"
#include "ConsoleBox.h"

#include "ModelLoader.h"
#include "OBJModel.h"

ConVar vr_nearz("vr_nearz", 0.1f);
ConVar vr_farz("vr_farz", 100.0f);
ConVar vr_fake_camera_movement("vr_fake_camera_movement", false);
ConVar vr_draw_hmd_to_window("vr_draw_hmd_to_window", true);
ConVar vr_noclip_walk_speed("vr_noclip_speed", 25.0f);
ConVar vr_noclip_sprint_speed("vr_noclip_sprint_speed", 50.0f);
ConVar vr_noclip_crouch_speed("vr_noclip_crouch_speed", 5.0f);
ConVar vr_mousespeed("vr_mousespeed", 0.18f);
ConVar vr_gui_z("vr_gui_z", 0.5f);

OpenVRInterface *openvr = NULL;



#ifdef MCENGINE_FEATURE_OPENVR

std::string OpenVRInterface::getTrackedDeviceString(vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError)
{
	uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, NULL, 0, peError);
	if (unRequiredBufferLen == 0)
		return "";

	char *pchBuffer = new char[unRequiredBufferLen];
	unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
	std::string sResult = pchBuffer;
	delete [] pchBuffer;
	return sResult;
}

Matrix4 OpenVRInterface::convertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose)
{
	Matrix4 matrixObj(
		matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
		);
	return matrixObj;
}

#endif



OpenVRInterface::OpenVRInterface()
{
	openvr = this;
	m_bReady = false;

#ifdef MCENGINE_FEATURE_OPENVR

	// convar callbacks
	vr_nearz.setCallback(fastdelegate::MakeDelegate(this, &OpenVRInterface::onClippingPlaneChange));
	vr_farz.setCallback(fastdelegate::MakeDelegate(this, &OpenVRInterface::onClippingPlaneChange));

	m_bCaptureMouse = false;
	m_fakeCamera = NULL;
	m_testModel = NULL;
	m_testMaterial = NULL;
	m_controllerLeft = NULL;
	m_controllerRight = NULL;
	m_drawCallback = NULL;

	m_bWDown = false;
	m_bADown = false;
	m_bSDown = false;
	m_bDDown = false;
	m_bShiftDown = false;
	m_bCtrlDown = false;

	m_pHMD = NULL;
	m_iTrackedControllerCount = 0;
	m_iTrackedControllerCount_Last = -1;
	m_iValidPoseCount = 0;
	m_iValidPoseCount_Last = -1;
	m_strPoseClasses = "";

	m_pRenderModels = NULL;

	m_unLensVAO = 0;

	m_glControllerVertBuffer = 0;
	m_unControllerVAO = 0;

	m_lensDistortionShader = NULL;
	m_renderModelShader = NULL;
	m_controllerAxisShader = NULL;
	m_genericTexturedShader = NULL;

	m_leftEye = NULL;
	m_rightEye = NULL;

	memset(m_rDevClassChar, 0, sizeof(m_rDevClassChar));

	// check if openvr runtime is installed
	if (!vr::VR_IsRuntimeInstalled())
	{
		engine->showMessageError("OpenVR Error", "OpenVR runtime is not installed!");
		return;
	}

	// load openvr runtime
	debugLog("OpenVR: Loading OpenVR runtime ...\n");
	vr::EVRInitError eError = vr::VRInitError_None;
	m_pHMD = vr::VR_Init(&eError, vr::VRApplication_Scene);
	if (eError != vr::VRInitError_None)
	{
		m_pHMD = NULL;

		if (eError == vr::VRInitError_Init_HmdNotFound || eError == vr::VRInitError_Init_HmdNotFoundPresenceFailed)
		{
			engine->showMessageInfo("OpenVR", "Couldn't find HMD, please connect your headset!");
			return;
		}

		char buf[1024];
		sprintf_s(buf, sizeof(buf), "%s", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
		engine->showMessageError("OpenVR Error", UString::format("Couldn't VR_Init() (%s)", buf));
		return;
	}

	// get render model interface
	debugLog("OpenVR: Getting render model interface ...\n");
	m_pRenderModels = (vr::IVRRenderModels *)vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &eError);
	if (!m_pRenderModels)
	{
		m_pHMD = NULL;
		vr::VR_Shutdown();

		char buf[1024];
		sprintf_s(buf, sizeof(buf), "Unable to get render model interface: %s", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
		engine->showMessageError("OpenVR Error", UString::format("Couldn't VR_GetGenericInterface() (%s)", buf));
		return;
	}

 	// initialize renderer
 	debugLog("OpenVR: Initializing renderer ...\n");
	if (!initRenderer())
	{
		engine->showMessageError("OpenVR Error", "Couldn't initialize renderer!");
		return;
	}

	// initialize compositor
	debugLog("OpenVR: Initializing compositor ...\n");
	if (!initCompositor())
	{
		engine->showMessageError("OpenVR Error", "Couldn't VRCompositor()!");
		return;
	}

	// initialize controllers
	m_controllerLeft = new OpenVRController(m_pHMD, vr::ETrackedControllerRole::TrackedControllerRole_LeftHand);
	m_controllerRight = new OpenVRController(m_pHMD, vr::ETrackedControllerRole::TrackedControllerRole_RightHand);

	// get device strings
	m_strDriver = "No Driver";
	m_strDisplay = "No Display";
	m_strDriver = getTrackedDeviceString(m_pHMD, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
	m_strDisplay = getTrackedDeviceString(m_pHMD, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);
	debugLog("OpenVR: driver = %s, display = %s\n", m_strDriver.c_str(), m_strDisplay.c_str());

	// debugging
	engine->getKeyboard()->addListener(this);
	convar->getConVarByName("debug_shaders")->setValue(1.0f);
	std::string strWindowTitle = "McEngine VR - " + m_strDriver + " " + m_strDisplay;
	engine->getEnvironment()->setWindowTitle(strWindowTitle.c_str());

	MDL mdl = ModelLoader::loadMDL("cake.mdl");
	m_testModel = mdl.model;
	m_testMaterial = mdl.material;

	m_fakeCamera = new Camera();

	if (m_strDriver == "null")
		vr_fake_camera_movement.setValue(1.0f);

	m_bReady = true;

#endif
}

OpenVRInterface::~OpenVRInterface()
{
#ifdef MCENGINE_FEATURE_OPENVR

	// unload openvr runtime
	if (m_pHMD)
	{
		vr::VR_Shutdown();
		m_pHMD = NULL;
	}

	// rendermodels
	for (std::vector<CGLRenderModel*>::iterator i = m_vecRenderModels.begin(); i != m_vecRenderModels.end(); i++)
	{
		delete (*i);
	}
	m_vecRenderModels.clear();

	// shaders
	SAFE_DELETE(m_genericTexturedShader);
	SAFE_DELETE(m_controllerAxisShader);
	SAFE_DELETE(m_renderModelShader);
	SAFE_DELETE(m_lensDistortionShader);

	// index buffers
	glDeleteBuffers(1, &m_glIDVertBuffer);
	glDeleteBuffers(1, &m_glIDIndexBuffer);

	// frame buffers
	SAFE_DELETE(m_leftEye);
	SAFE_DELETE(m_rightEye);

	// vertex buffers
	if (m_unLensVAO != 0)
		glDeleteVertexArrays(1, &m_unLensVAO);
	if (m_unControllerVAO != 0)
		glDeleteVertexArrays(1, &m_unControllerVAO);

#endif

	openvr = NULL;
}

#ifdef MCENGINE_FEATURE_OPENVR

bool OpenVRInterface::initRenderer()
{
	if (!initShaders())
		return false;

	updateStaticMatrices();
	debugLog("OpenVR: Creating rendertargets ...\n");
	initRenderTargets();
	debugLog("OpenVR: Creating distortion ...\n");
	initDistortion();
	debugLog("OpenVR: Creating rendermodels ...\n");
	initRenderModels();

	return true;
}

bool OpenVRInterface::initCompositor()
{
	if (!vr::VRCompositor())
		return false;
	return true;
}

void OpenVRInterface::initDistortion()
{
	if (!m_pHMD)
		return;

	GLushort m_iLensGridSegmentCountH = 43;
	GLushort m_iLensGridSegmentCountV = 43;

	float w = (float)(1.0/float(m_iLensGridSegmentCountH-1));
	float h = (float)(1.0/float(m_iLensGridSegmentCountV-1));

	float u, v = 0;

	std::vector<VertexDataLens> vVerts(0);
	VertexDataLens vert;

	// left eye distortion verts
	float Xoffset = -1;
	for (int y=0; y<m_iLensGridSegmentCountV; y++)
	{
		for (int x=0; x<m_iLensGridSegmentCountH; x++)
		{
			u = x*w; v = 1-y*h;
			vert.position = Vector2(Xoffset+u, -1+2*y*h);

			vr::DistortionCoordinates_t dc0 = m_pHMD->ComputeDistortion(vr::Eye_Left, u, v);

			vert.texCoordRed = Vector2(dc0.rfRed[0], 1 - dc0.rfRed[1]);
			vert.texCoordGreen =  Vector2(dc0.rfGreen[0], 1 - dc0.rfGreen[1]);
			vert.texCoordBlue = Vector2(dc0.rfBlue[0], 1 - dc0.rfBlue[1]);

			vVerts.push_back(vert);
		}
	}

	// right eye distortion verts
	Xoffset = 0;
	for (int y=0; y<m_iLensGridSegmentCountV; y++)
	{
		for (int x=0; x<m_iLensGridSegmentCountH; x++)
		{
			u = x*w; v = 1-y*h;
			vert.position = Vector2(Xoffset+u, -1+2*y*h);

			vr::DistortionCoordinates_t dc0 = m_pHMD->ComputeDistortion(vr::Eye_Right, u, v);

			vert.texCoordRed = Vector2(dc0.rfRed[0], 1 - dc0.rfRed[1]);
			vert.texCoordGreen = Vector2(dc0.rfGreen[0], 1 - dc0.rfGreen[1]);
			vert.texCoordBlue = Vector2(dc0.rfBlue[0], 1 - dc0.rfBlue[1]);

			vVerts.push_back(vert);
		}
	}

	std::vector<GLushort> vIndices;
	GLushort a,b,c,d;

	GLushort offset = 0;
	for (GLushort y=0; y<m_iLensGridSegmentCountV-1; y++)
	{
		for (GLushort x=0; x<m_iLensGridSegmentCountH-1; x++)
		{
			a = m_iLensGridSegmentCountH*y+x +offset;
			b = m_iLensGridSegmentCountH*y+x+1 +offset;
			c = (y+1)*m_iLensGridSegmentCountH+x+1 +offset;
			d = (y+1)*m_iLensGridSegmentCountH+x +offset;

			vIndices.push_back(a);
			vIndices.push_back(b);
			vIndices.push_back(c);

			vIndices.push_back(a);
			vIndices.push_back(c);
			vIndices.push_back(d);
		}
	}

	offset = (m_iLensGridSegmentCountH)*(m_iLensGridSegmentCountV);
	for (GLushort y=0; y<m_iLensGridSegmentCountV-1; y++)
	{
		for (GLushort x=0; x<m_iLensGridSegmentCountH-1; x++)
		{
			a = m_iLensGridSegmentCountH*y+x +offset;
			b = m_iLensGridSegmentCountH*y+x+1 +offset;
			c = (y+1)*m_iLensGridSegmentCountH+x+1 +offset;
			d = (y+1)*m_iLensGridSegmentCountH+x +offset;

			vIndices.push_back(a);
			vIndices.push_back(b);
			vIndices.push_back(c);

			vIndices.push_back(a);
			vIndices.push_back(c);
			vIndices.push_back(d);
		}
	}
	m_uiIndexSize = vIndices.size();

	glGenVertexArrays(1, &m_unLensVAO);
	glBindVertexArray(m_unLensVAO);

	glGenBuffers(1, &m_glIDVertBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_glIDVertBuffer);
	glBufferData(GL_ARRAY_BUFFER, vVerts.size()*sizeof(VertexDataLens), &vVerts[0], GL_STATIC_DRAW);

	glGenBuffers(1, &m_glIDIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glIDIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vIndices.size()*sizeof(GLushort), &vIndices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataLens), (void *)offsetof(VertexDataLens, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataLens), (void *)offsetof(VertexDataLens, texCoordRed));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataLens), (void *)offsetof(VertexDataLens, texCoordGreen));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataLens), (void *)offsetof(VertexDataLens, texCoordBlue));

	glBindVertexArray(0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void OpenVRInterface::initRenderModels()
{
	memset(m_rTrackedDeviceToRenderModel, 0, sizeof(m_rTrackedDeviceToRenderModel));
	if (!m_pHMD) return;

	for (uint32_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
	{
		if (!m_pHMD->IsTrackedDeviceConnected(unTrackedDevice))
			continue;

		updateRenderModelForTrackedDevice(unTrackedDevice);
	}
}

bool OpenVRInterface::initRenderTargets()
{
	if (!m_pHMD) return false;

	uint32_t renderWidth = 1;
	uint32_t renderHeight = 1;
	m_pHMD->GetRecommendedRenderTargetSize(&renderWidth, &renderHeight);

	debugLog("OpenVR: Recommended rendertarget size = (%i, %i)\n", renderWidth, renderHeight);

	Color clearColor = COLORf(0.0f, 0.15f, 0.15f, 0.18f);

	m_leftEye = new RenderTarget(renderWidth, renderHeight, Graphics::MULTISAMPLE_TYPE::MULTISAMPLE_4X);
	m_leftEye->setClearColorOnDraw(true);
	m_leftEye->setClearDepthOnDraw(true);
	m_leftEye->setClearColor(clearColor);

	m_rightEye = new RenderTarget(renderWidth, renderHeight, Graphics::MULTISAMPLE_TYPE::MULTISAMPLE_4X);
	m_rightEye->setClearColorOnDraw(true);
	m_rightEye->setClearDepthOnDraw(true);
	m_rightEye->setClearColor(clearColor);

	m_debugOverlay = new RenderTarget(engine->getScreenWidth(), engine->getScreenHeight());
	m_debugOverlay->setClearColorOnDraw(true);

	return true;
}

bool OpenVRInterface::initShaders()
{
	m_genericTexturedShader = new Shader(

			// vertex Shader
			"#version 110\n"
			"uniform mat4 matrix;\n"
			"varying vec2 texCoords;\n"
			"void main()\n"
			"{\n"
			"	texCoords = gl_MultiTexCoord0.xy;\n"
			"	gl_Position = matrix * gl_Vertex;\n"
			"}\n",

			// fragment Shader
			"#version 110\n"
			"uniform sampler2D mytexture;\n"
			"varying vec2 texCoords;\n"
			"void main()\n"
			"{\n"
			"   gl_FragColor = texture2D(mytexture, texCoords);\n"
			"}\n",

			true
	);

	m_controllerAxisShader = new Shader(

			// vertex shader
			"#version 410\n"
			"uniform mat4 matrix;\n"
			"layout(location = 0) in vec4 position;\n"
			"layout(location = 1) in vec3 v3ColorIn;\n"
			"out vec4 v4Color;\n"
			"void main()\n"
			"{\n"
			"	v4Color.xyz = v3ColorIn; v4Color.a = 1.0;\n"
			"	gl_Position = matrix * position;\n"
			"}\n",

			// fragment shader
			"#version 410\n"
			"in vec4 v4Color;\n"
			"out vec4 outputColor;\n"
			"void main()\n"
			"{\n"
			"   outputColor = v4Color;\n"
			"}\n",

			true
	);

	m_renderModelShader = new Shader(

			// vertex shader
			"#version 410\n"
			"uniform mat4 matrix;\n"
			"layout(location = 0) in vec4 position;\n"
			"layout(location = 1) in vec3 v3NormalIn;\n"
			"layout(location = 2) in vec2 v2TexCoordsIn;\n"
			"out vec2 v2TexCoord;\n"
			"void main()\n"
			"{\n"
			"	v2TexCoord = v2TexCoordsIn;\n"
			"	gl_Position = matrix * vec4(position.xyz, 1);\n"
			"}\n",

			//fragment shader
			"#version 410 core\n"
			"uniform sampler2D diffuse;\n"
			"in vec2 v2TexCoord;\n"
			"out vec4 outputColor;\n"
			"void main()\n"
			"{\n"
			"   outputColor = texture( diffuse, v2TexCoord);\n"
			"}\n",

			true
	);

	m_lensDistortionShader = new Shader(

			// vertex shader
			"#version 410 core\n"
			"layout(location = 0) in vec4 position;\n"
			"layout(location = 1) in vec2 v2UVredIn;\n"
			"layout(location = 2) in vec2 v2UVGreenIn;\n"
			"layout(location = 3) in vec2 v2UVblueIn;\n"
			"noperspective  out vec2 v2UVred;\n"
			"noperspective  out vec2 v2UVgreen;\n"
			"noperspective  out vec2 v2UVblue;\n"
			"void main()\n"
			"{\n"
			"	v2UVred = v2UVredIn;\n"
			"	v2UVgreen = v2UVGreenIn;\n"
			"	v2UVblue = v2UVblueIn;\n"
			"	gl_Position = position;\n"
			"}\n",

			// fragment shader
			"#version 410 core\n"
			"uniform sampler2D mytexture;\n"

			"noperspective  in vec2 v2UVred;\n"
			"noperspective  in vec2 v2UVgreen;\n"
			"noperspective  in vec2 v2UVblue;\n"

			"out vec4 outputColor;\n"

			"void main()\n"
			"{\n"
			"	float fBoundsCheck = ( (dot( vec2( lessThan( v2UVgreen.xy, vec2(0.05, 0.05)) ), vec2(1.0, 1.0))+dot( vec2( greaterThan( v2UVgreen.xy, vec2( 0.95, 0.95)) ), vec2(1.0, 1.0))) );\n"
			"	if( fBoundsCheck > 1.0 )\n"
			"	{ outputColor = vec4( 0, 0, 0, 1.0 ); }\n"
			"	else\n"
			"	{\n"
			"		float red = texture(mytexture, v2UVred).x;\n"
			"		float green = texture(mytexture, v2UVgreen).y;\n"
			"		float blue = texture(mytexture, v2UVblue).z;\n"
			"		outputColor = vec4( red, green, blue, 1.0  ); }\n"
			"}\n",

			true
	);

	return m_genericTexturedShader->isReady() && m_controllerAxisShader->isReady() && m_renderModelShader->isReady() && m_lensDistortionShader->isReady();
}

#endif

void OpenVRInterface::draw(Graphics *g)
{
	if (!m_bReady) return;

#ifdef MCENGINE_FEATURE_OPENVR

	if (m_pHMD)
	{
		updateHMDMatrixPose();
		updateControllerAxes();

		// draw 2d debug overlay
		m_debugOverlay->enable();
			engine->getGUI()->draw(g);
		m_debugOverlay->disable();

		// draw everything
		g->setDepthBuffer(true);
			renderStereoTargets(g);
		g->setDepthBuffer(false);

		// viewer window
		if (vr_draw_hmd_to_window.getBool())
			renderStereoToWindow();

		// push to hmd
		vr::Texture_t leftEyeTexture = {(void*)m_leftEye->getResolveTexture(), vr::API_OpenGL, vr::ColorSpace_Gamma};
		vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
		vr::Texture_t rightEyeTexture = {(void*)m_rightEye->getResolveTexture(), vr::API_OpenGL, vr::ColorSpace_Gamma};
		vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
	}

#endif
}

#ifdef MCENGINE_FEATURE_OPENVR

void OpenVRInterface::renderScene(Graphics *g, vr::Hmd_Eye eye)
{
	bool bIsInputCapturedByAnotherProcess = m_pHMD->IsInputFocusCapturedByAnotherProcess();

	// TODO:
	// render stencil mesh
	g->pushStencil();
	g->setCulling(false);
	{
		vr::HiddenAreaMesh_t hiddenAreaMesh = m_pHMD->GetHiddenAreaMesh(eye);
		if (hiddenAreaMesh.unTriangleCount > 0 && hiddenAreaMesh.pVertexData != NULL)
		{
			VertexArrayObject vao;
			for (int i=0; i<hiddenAreaMesh.unTriangleCount*3; i++)
			{
				vao.addVertex(hiddenAreaMesh.pVertexData[i].v[0], hiddenAreaMesh.pVertexData[i].v[1]);
			}
			g->drawVAO(&vao);
		}
	}
	g->fillStencil(false);
	///g->setCulling(true);

	// draw stuff
	m_genericTexturedShader->enable();
	{
		m_matCurrentMVP = getCurrentViewProjectionMatrix(eye);

		if (vr_fake_camera_movement.getBool())
		{
			Matrix4 translation;
			translation.translate(m_fakeCamera->getPos());
			m_matCurrentMVP = m_matCurrentMVP * m_fakeCamera->getRotation().getMatrix() * translation;
		}

		m_genericTexturedShader->setUniformMatrix4fv("matrix", m_matCurrentMVP);

		m_testModel->draw(engine->getGraphics(), NULL);

		// main draw callback
		if (m_drawCallback != NULL)
			m_drawCallback();

		// draw engine debug gui
		g->setDepthBuffer(false);
		{
			// TODO: finish this
			m_genericTexturedShader->disable();
			m_genericTexturedShader->enable();
			float aspectRatio = m_debugOverlay->getWidth() / m_debugOverlay->getHeight();
			Matrix4 projectionMatrix = Camera::buildMatrixPerspectiveFov(deg2rad(110), aspectRatio, 0.1f, 1000.0f);
			Matrix4 translation;
			translation.translate(-0.3f, 0.20f, -vr_gui_z.getFloat());
			projectionMatrix = getCurrentEyePosMatrix(eye) * projectionMatrix * translation;
			m_genericTexturedShader->setUniformMatrix4fv("matrix", projectionMatrix);

			float x = 0;
			float y = 0;
			float width = aspectRatio;
			float height = 1;

			VertexArrayObject vao;

			vao.addTexcoord(0, 1);
			vao.addVertex(x, y);

			vao.addTexcoord(0, 0);
			vao.addVertex(x, y-height);

			vao.addTexcoord(1, 0);
			vao.addVertex(x+width, y-height);

			vao.addTexcoord(1, 0);
			vao.addVertex(x+width, y-height);

			vao.addTexcoord(1, 1);
			vao.addVertex(x+width, y);

			vao.addTexcoord(0, 1);
			vao.addVertex(x, y);

			m_debugOverlay->bind();
			g->drawVAO(&vao);
			m_debugOverlay->unbind();
		}
		g->setDepthBuffer(true);
	}
	m_genericTexturedShader->disable();

	if (!bIsInputCapturedByAnotherProcess)
	{
		// draw the controller axis lines
		m_controllerAxisShader->enable();
		{
			Matrix4 eyeViewProjectionMatrix = getCurrentViewProjectionMatrix(eye);
			m_controllerAxisShader->setUniformMatrix4fv("matrix", eyeViewProjectionMatrix);
			glBindVertexArray(m_unControllerVAO);
			glDrawArrays(GL_LINES, 0, m_uiControllerVertcount);
			glBindVertexArray(0);
		}
		m_controllerAxisShader->disable();
	}

	// internal rendermodel rendering
	m_renderModelShader->enable();
	{
		for (uint32_t unTrackedDevice = 0; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
		{
			if (!m_rTrackedDeviceToRenderModel[unTrackedDevice])
				continue;

			const vr::TrackedDevicePose_t & pose = m_rTrackedDevicePose[unTrackedDevice];
			if (!pose.bPoseIsValid)
				continue;

			if (bIsInputCapturedByAnotherProcess && m_pHMD->GetTrackedDeviceClass(unTrackedDevice) == vr::TrackedDeviceClass_Controller)
				continue;

			const Matrix4 & matDeviceToTracking = m_rmat4DevicePose[unTrackedDevice];
			Matrix4 matMVP = getCurrentViewProjectionMatrix(eye) * matDeviceToTracking;

			m_renderModelShader->setUniformMatrix4fv("matrix", matMVP);
			m_rTrackedDeviceToRenderModel[unTrackedDevice]->draw();
		}
	}
	m_renderModelShader->disable();

	g->popStencil();
}

void OpenVRInterface::renderStereoTargets(Graphics *g)
{
	// backup resolution
	Vector2 resolutionBackup = g->getResolution();

	// left Eye
	g->setAntialiasing(true);
	{
		g->onResolutionChange(m_leftEye->getSize()); // force engine resolution
		m_leftEye->enable();
			renderScene(g, vr::Eye_Left);
		m_leftEye->disable();
	}
	g->setAntialiasing(false);

	// right Eye
    g->setAntialiasing(true);
	{
		g->onResolutionChange(m_leftEye->getSize()); // force engine resolution
    	m_rightEye->enable();
			renderScene(g, vr::Eye_Right);
		m_rightEye->disable();
	}
	g->setAntialiasing(false);

    // restore engine resolution
    g->onResolutionChange(resolutionBackup);
}

void OpenVRInterface::renderStereoToWindow()
{
	glBindVertexArray(m_unLensVAO);
	m_lensDistortionShader->enable();
	{
		// render left lens (first half of index array)
		m_leftEye->bind();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glDrawElements(GL_TRIANGLES, m_uiIndexSize/2, GL_UNSIGNED_SHORT, 0);
		m_leftEye->unbind();

		// render right lens (second half of index array)
		m_rightEye->bind();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glDrawElements(GL_TRIANGLES, m_uiIndexSize/2, GL_UNSIGNED_SHORT, (const void *)(m_uiIndexSize));
		m_rightEye->unbind();

		glBindVertexArray(0);
	}
	m_lensDistortionShader->disable();
}

void OpenVRInterface::updateControllerAxes()
{
	// don't draw controllers if somebody else has input focus
	if (m_pHMD->IsInputFocusCapturedByAnotherProcess())
		return;

	std::vector<float> vertdataarray;

	m_uiControllerVertcount = 0;
	m_iTrackedControllerCount = 0;

	for (vr::TrackedDeviceIndex_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; ++unTrackedDevice)
	{
		if (!m_pHMD->IsTrackedDeviceConnected(unTrackedDevice))
			continue;

		if (m_pHMD->GetTrackedDeviceClass(unTrackedDevice) != vr::TrackedDeviceClass_Controller)
			continue;

		m_iTrackedControllerCount += 1;

		if (!m_rTrackedDevicePose[unTrackedDevice].bPoseIsValid)
			continue;

		const Matrix4 & mat = m_rmat4DevicePose[unTrackedDevice];

		Vector4 center = mat * Vector4(0, 0, 0, 1);

		for (int i=0; i<3; ++i)
		{
			Vector3 color(0, 0, 0);
			Vector4 point(0, 0, 0, 1);
			point[i] += 0.05f;  // offset in X, Y, Z
			color[i] = 1.0;  // R, G, B
			point = mat * point;

			vertdataarray.push_back(center.x);
			vertdataarray.push_back(center.y);
			vertdataarray.push_back(center.z);

			vertdataarray.push_back(color.x);
			vertdataarray.push_back(color.y);
			vertdataarray.push_back(color.z);

			vertdataarray.push_back(point.x);
			vertdataarray.push_back(point.y);
			vertdataarray.push_back(point.z);

			vertdataarray.push_back(color.x);
			vertdataarray.push_back(color.y);
			vertdataarray.push_back(color.z);

			m_uiControllerVertcount += 2;
		}

		Vector4 start = mat * Vector4(0, 0, -0.02f, 1);
		Vector4 end = mat * Vector4(0, 0, -39.f, 1);
		Vector3 color(.92f, .92f, .71f);

		vertdataarray.push_back(start.x);vertdataarray.push_back(start.y);vertdataarray.push_back(start.z);
		vertdataarray.push_back(color.x);vertdataarray.push_back(color.y);vertdataarray.push_back(color.z);

		vertdataarray.push_back(end.x);vertdataarray.push_back(end.y);vertdataarray.push_back(end.z);
		vertdataarray.push_back(color.x);vertdataarray.push_back(color.y);vertdataarray.push_back(color.z);
		m_uiControllerVertcount += 2;
	}

	// setup the VAO the first time through.
	if (m_unControllerVAO == 0)
	{
		glGenVertexArrays(1, &m_unControllerVAO);
		glBindVertexArray(m_unControllerVAO);

		glGenBuffers(1, &m_glControllerVertBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_glControllerVertBuffer);

		GLuint stride = 2 * 3 * sizeof(float);
		GLuint offset = 0;

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		offset += sizeof(Vector3);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		glBindVertexArray(0);
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_glControllerVertBuffer);

	// set vertex data if we have some
	if (vertdataarray.size() > 0)
	{
		//$ TODO: Use glBufferSubData for this...
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertdataarray.size(), &vertdataarray[0], GL_STREAM_DRAW);
	}
}

#endif

void OpenVRInterface::update()
{
	if (!m_bReady) return;

#ifdef MCENGINE_FEATURE_OPENVR

	// process openvr events
	vr::VREvent_t event;
	while (m_pHMD->PollNextEvent(&event, sizeof(event)))
	{
		switch (event.eventType)
		{
		case vr::VREvent_TrackedDeviceActivated:
			{
				updateRenderModelForTrackedDevice(event.trackedDeviceIndex);
				debugLog("OpenVR: Device %u attached. Setting up render model.\n", event.trackedDeviceIndex);
			}
			break;
		case vr::VREvent_TrackedDeviceDeactivated:
			debugLog("OpenVR: Device %u detached.\n", event.trackedDeviceIndex);
			break;
		case vr::VREvent_TrackedDeviceUpdated:
			debugLog("OpenVR: Device %u updated.\n", event.trackedDeviceIndex);
			break;
		}
	}

	// update controllers
	for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
	{
		vr::VRControllerState_t state;
		if (m_pHMD->GetControllerState(unDevice, &state))
		{
			if (unDevice == m_pHMD->GetTrackedDeviceIndexForControllerRole(m_controllerLeft->getRole()))
				m_controllerLeft->update(state.ulButtonPressed, state.ulButtonTouched, state.rAxis);
			else if (unDevice == m_pHMD->GetTrackedDeviceIndexForControllerRole(m_controllerRight->getRole()))
				m_controllerRight->update(state.ulButtonPressed, state.ulButtonTouched, state.rAxis);
		}
	}

	// debugging
	if (vr_fake_camera_movement.getBool())
	{
		// rotation
		if (m_bCaptureMouse)
		{
			Vector2 rawDelta = engine->getMouse()->getRawDelta();
			if (rawDelta.x != 0.0f || rawDelta.y != 0.0f)
			{
				m_fakeCamera->rotateX(rawDelta.y*vr_mousespeed.getFloat());
				m_fakeCamera->rotateY(-rawDelta.x*vr_mousespeed.getFloat());
			}
			engine->getMouse()->setPos(engine->getScreenSize()/2);
		}

		// movement
		Vector3 velocity = Vector3( ((m_bADown ? 1 : 0) + (m_bDDown ? -1 : 0)),
								0,
								-((m_bSDown ? 1 : 0) + (m_bWDown ?  -1 : 0)));

		if (velocity.x != 0.0f || velocity.z != 0.0f || velocity.z != 0.0f)
		{
			velocity.normalize();
			velocity *= engine->getFrameTime();
			Vector3 camVelocity = m_fakeCamera->getNextPosition(velocity) - m_fakeCamera->getPos();

			float speed = vr_noclip_walk_speed.getFloat();
			if (m_bCtrlDown)
				speed = vr_noclip_crouch_speed.getFloat();
			else if (m_bShiftDown)
				speed = vr_noclip_sprint_speed.getFloat();

			camVelocity *= speed;
			Vector3 nextPos = m_fakeCamera->getPos() + camVelocity;
			m_fakeCamera->setPos(nextPos);
		}
	}

#endif
}

void OpenVRInterface::onKeyDown(KeyboardEvent &e)
{
#ifdef MCENGINE_FEATURE_OPENVR

	if (e == KEY_W)
		m_bWDown = true;
	if (e == KEY_A)
		m_bADown = true;
	if (e == KEY_S)
		m_bSDown = true;
	if (e == KEY_D)
		m_bDDown = true;
	if (e == KEY_SHIFT)
		m_bShiftDown = true;
	if (e == KEY_CONTROL)
		m_bCtrlDown = true;

	if (e == KEY_C && engine->getKeyboard()->isAltDown())
	{
		m_bCaptureMouse = !m_bCaptureMouse;
		if (m_bCaptureMouse)
		{
			engine->getMouse()->setCursorVisible(false);
			engine->getEnvironment()->setCursorClip(true, Rect());
		}
		else
		{
			engine->getEnvironment()->setCursorClip(false, Rect());
			engine->getMouse()->setCursorVisible(true);
		}
	}

#endif
}

void OpenVRInterface::onKeyUp(KeyboardEvent &e)
{
#ifdef MCENGINE_FEATURE_OPENVR

	if (e == KEY_W)
		m_bWDown = false;
	if (e == KEY_A)
		m_bADown = false;
	if (e == KEY_S)
		m_bSDown = false;
	if (e == KEY_D)
		m_bDDown = false;
	if (e == KEY_SHIFT)
		m_bShiftDown = false;
	if (e == KEY_CONTROL)
		m_bCtrlDown = false;

#endif
}

void OpenVRInterface::onResolutionChange(Vector2 newResolution)
{
	if (!m_bReady) return;

#ifdef MCENGINE_FEATURE_OPENVR

	m_debugOverlay->rebuild(newResolution.x, newResolution.y);

#endif
}

#ifdef MCENGINE_FEATURE_OPENVR

void OpenVRInterface::updateStaticMatrices()
{
	debugLog("OpenVR: Updating eye projection and position matrices ...\n");

	m_mat4ProjectionLeft = getHMDMatrixProjectionEye(vr::Eye_Left);
	m_mat4ProjectionRight = getHMDMatrixProjectionEye(vr::Eye_Right);
	m_mat4eyePosLeft = getHMDMatrixPoseEye(vr::Eye_Left);
	m_mat4eyePosRight = getHMDMatrixPoseEye(vr::Eye_Right);
}

void OpenVRInterface::updateHMDMatrixPose()
{
	if (!m_bReady) return;

	vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);

	m_iValidPoseCount = 0;
	m_strPoseClasses = "";
	for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
	{
		if (m_rTrackedDevicePose[nDevice].bPoseIsValid)
		{
			m_iValidPoseCount++;
			m_rmat4DevicePose[nDevice] = convertSteamVRMatrixToMatrix4(m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking);
			if (m_rDevClassChar[nDevice] == 0)
			{
				switch (m_pHMD->GetTrackedDeviceClass(nDevice))
				{
				case vr::TrackedDeviceClass_Controller:        m_rDevClassChar[nDevice] = 'C'; break;
				case vr::TrackedDeviceClass_HMD:               m_rDevClassChar[nDevice] = 'H'; break;
				case vr::TrackedDeviceClass_Invalid:           m_rDevClassChar[nDevice] = 'I'; break;
				case vr::TrackedDeviceClass_Other:             m_rDevClassChar[nDevice] = 'O'; break;
				case vr::TrackedDeviceClass_TrackingReference: m_rDevClassChar[nDevice] = 'T'; break;
				default:                                       m_rDevClassChar[nDevice] = '?'; break;
				}
			}
			m_strPoseClasses += m_rDevClassChar[nDevice];
		}
	}

	if (m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
	{
		m_mat4HMDPose = m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd].invert();
	}
}

void OpenVRInterface::updateRenderModelForTrackedDevice(vr::TrackedDeviceIndex_t unTrackedDeviceIndex)
{
	if (unTrackedDeviceIndex >= vr::k_unMaxTrackedDeviceCount)
		return;

	// try to find a model we've already set up
	std::string sRenderModelName = getTrackedDeviceString(m_pHMD, unTrackedDeviceIndex, vr::Prop_RenderModelName_String);
	if (sRenderModelName.length() > 0)
	{
		CGLRenderModel *pRenderModel = findOrLoadRenderModel(sRenderModelName.c_str());
		if (!pRenderModel)
		{
			std::string sTrackingSystemName = getTrackedDeviceString(m_pHMD, unTrackedDeviceIndex, vr::Prop_TrackingSystemName_String);
			debugLog("OpenVR: Unable to load render model for tracked device %d (%s.%s)", unTrackedDeviceIndex, sTrackingSystemName.c_str(), sRenderModelName.c_str());
		}
		else
		{
			m_rTrackedDeviceToRenderModel[unTrackedDeviceIndex] = pRenderModel;
		}
	}
	else
		debugLog("OpenVR: WARNING! findOrLoadRenderModel( %s ) would have been called!\n", sRenderModelName.c_str());
}

Matrix4 OpenVRInterface::getHMDMatrixProjectionEye(vr::Hmd_Eye eye)
{
	if (!m_pHMD)
		return Matrix4();

	vr::HmdMatrix44_t mat = m_pHMD->GetProjectionMatrix(eye, vr_nearz.getFloat(), vr_farz.getFloat(), vr::API_OpenGL);

	return Matrix4(
		mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
		mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
		mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
		mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
	);
}

CGLRenderModel *OpenVRInterface::findOrLoadRenderModel(const char *pchRenderModelName)
{
	debugLog("OpenVRInterface::findOrLoadRenderModel( %s )\n", pchRenderModelName);

	CGLRenderModel *pRenderModel = NULL;
	for (std::vector<CGLRenderModel*>::iterator i = m_vecRenderModels.begin(); i != m_vecRenderModels.end(); i++)
	{
		if (!stricmp((*i)->getName().c_str(), pchRenderModelName))
		{
			pRenderModel = *i;
			break;
		}
	}

	// load the model if we didn't find one
	if (!pRenderModel)
	{
		vr::RenderModel_t *pModel;
		vr::EVRRenderModelError error;
		while (true)
		{
			error = vr::VRRenderModels()->LoadRenderModel_Async(pchRenderModelName, &pModel);
			if (error != vr::VRRenderModelError_Loading)
				break;
		}

		if (error != vr::VRRenderModelError_None)
		{
			debugLog("OpenVR: Unable to load render model %s - %s\n", pchRenderModelName, vr::VRRenderModels()->GetRenderModelErrorNameFromEnum(error));
			return NULL; // move on to the next tracked device
		}

		vr::RenderModel_TextureMap_t *pTexture;
		while (true)
		{
			error = vr::VRRenderModels()->LoadTexture_Async(pModel->diffuseTextureId, &pTexture);
			if (error != vr::VRRenderModelError_Loading)
				break;
		}

		if (error != vr::VRRenderModelError_None)
		{
			debugLog("OpenVR: Unable to load render texture id:%d for render model %s\n", pModel->diffuseTextureId, pchRenderModelName);
			vr::VRRenderModels()->FreeRenderModel(pModel);
			return NULL; // move on to the next tracked device
		}

		pRenderModel = new CGLRenderModel(pchRenderModelName);
		if (!pRenderModel->init(*pModel, *pTexture))
		{
			debugLog("OpenVR: Unable to create GL model from render model %s\n", pchRenderModelName);
			delete pRenderModel;
			pRenderModel = NULL;
		}
		else
		{
			m_vecRenderModels.push_back(pRenderModel);
		}
		vr::VRRenderModels()->FreeRenderModel(pModel);
		vr::VRRenderModels()->FreeTexture(pTexture);
	}
	return pRenderModel;
}

Matrix4 OpenVRInterface::getHMDMatrixPoseEye(vr::Hmd_Eye eye)
{
	if (!m_pHMD)
		return Matrix4();

	vr::HmdMatrix34_t matEyeRight = m_pHMD->GetEyeToHeadTransform(eye);
	Matrix4 matrixObj(
		matEyeRight.m[0][0], matEyeRight.m[1][0], matEyeRight.m[2][0], 0.0,
		matEyeRight.m[0][1], matEyeRight.m[1][1], matEyeRight.m[2][1], 0.0,
		matEyeRight.m[0][2], matEyeRight.m[1][2], matEyeRight.m[2][2], 0.0,
		matEyeRight.m[0][3], matEyeRight.m[1][3], matEyeRight.m[2][3], 1.0f
		);

	return matrixObj.invert();
}

Matrix4 OpenVRInterface::getCurrentViewProjectionMatrix(vr::Hmd_Eye eye)
{
	Matrix4 matMVP;
	if (eye == vr::Eye_Left)
		matMVP = m_mat4ProjectionLeft * m_mat4eyePosLeft * m_mat4HMDPose;
	else if (eye == vr::Eye_Right)
		matMVP = m_mat4ProjectionRight * m_mat4eyePosRight * m_mat4HMDPose;
	return matMVP;
}

Matrix4 OpenVRInterface::getCurrentEyePosMatrix(vr::Hmd_Eye eye)
{
	Matrix4 matEyePos;
	if (eye == vr::Eye_Left)
		matEyePos = m_mat4eyePosLeft;
	else if (eye == vr::Eye_Right)
		matEyePos = m_mat4eyePosRight;
	return matEyePos;
}

void OpenVRInterface::onClippingPlaneChange(UString oldValue, UString newValue)
{
	if (!m_bReady) return;

	updateStaticMatrices();
}

#endif



CGLRenderModel::CGLRenderModel(const std::string &sRenderModelName)
{
	m_sModelName = sRenderModelName;

	m_glIndexBuffer = 0;
	m_glVertArray = 0;
	m_glVertBuffer = 0;
	m_glTexture = 0;
	m_unVertexCount = 0;
}

CGLRenderModel::~CGLRenderModel()
{
	cleanup();
}

void CGLRenderModel::cleanup()
{
	if (m_glVertBuffer)
	{
		glDeleteBuffers(1, &m_glIndexBuffer);
		glDeleteVertexArrays(1, &m_glVertArray);
		glDeleteBuffers(1, &m_glVertBuffer);

		m_glIndexBuffer = 0;
		m_glVertArray = 0;
		m_glVertBuffer = 0;
	}
}

void CGLRenderModel::draw()
{
	glBindVertexArray(m_glVertArray);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_glTexture);

	glDrawElements(GL_TRIANGLES, m_unVertexCount, GL_UNSIGNED_SHORT, 0);

	glBindVertexArray(0);
}

#ifdef MCENGINE_FEATURE_OPENVR

bool CGLRenderModel::init(const vr::RenderModel_t &vrModel, const vr::RenderModel_TextureMap_t &vrDiffuseTexture)
{
	// create and bind a VAO to hold state for this model
	glGenVertexArrays(1, &m_glVertArray);
	glBindVertexArray(m_glVertArray);

	// populate a vertex buffer
	glGenBuffers(1, &m_glVertBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_glVertBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vr::RenderModel_Vertex_t) * vrModel.unVertexCount, vrModel.rVertexData, GL_STATIC_DRAW);

	// identify the components in the vertex buffer
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (void*)offsetof(vr::RenderModel_Vertex_t, vPosition));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (void*)offsetof(vr::RenderModel_Vertex_t, vNormal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (void*)offsetof(vr::RenderModel_Vertex_t, rfTextureCoord));

	// create and populate the index buffer
	glGenBuffers(1, &m_glIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * vrModel.unTriangleCount * 3, vrModel.rIndexData, GL_STATIC_DRAW);

	glBindVertexArray(0);

	// create and populate the texture
	glGenTextures(1, &m_glTexture);
	glBindTexture(GL_TEXTURE_2D, m_glTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, vrDiffuseTexture.unWidth, vrDiffuseTexture.unHeight,
		0, GL_RGBA, GL_UNSIGNED_BYTE, vrDiffuseTexture.rubTextureMapData);

	// if this renders black ask McJohn what's wrong.
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	GLfloat fLargest;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);

	glBindTexture(GL_TEXTURE_2D, 0);

	m_unVertexCount = vrModel.unTriangleCount * 3;

	return true;
}

#endif



#ifdef MCENGINE_FEATURE_OPENVR

OpenVRController::OpenVRController(vr::IVRSystem *hmd, vr::ETrackedControllerRole role)
{
	m_hmd = hmd;
	m_role = role;

	m_ulButtonPressed = 0;
	m_ulButtonTouched = 0;

	for (int i=0; i<vr::k_unControllerStateAxisCount; i++)
	{
		m_rAxis[i].x = 0.0f;
		m_rAxis[i].y = 0.0f;
	}
}

void OpenVRController::update(uint64_t buttonPressed, uint64_t buttonTouched, vr::VRControllerAxis_t axes[vr::k_unControllerStateAxisCount])
{
	m_ulButtonPressed = buttonPressed;
	m_ulButtonTouched = buttonTouched;

	for (int i=0; i<vr::k_unControllerStateAxisCount; i++)
	{
		m_rAxis[i] = axes[i];
	}
}

void OpenVRController::triggerHapticPulse(unsigned short durationMicroSec, vr::EVRButtonId buttonId)
{
	if (m_hmd == NULL) return;

	m_hmd->TriggerHapticPulse(m_hmd->GetTrackedDeviceIndexForControllerRole(m_role), vr::EVRButtonId::k_EButton_SteamVR_Touchpad, durationMicroSec);
}

bool OpenVRController::isButtonPressed(vr::EVRButtonId button)
{
	return m_ulButtonPressed & ButtonMaskFromId(button);
}

float OpenVRController::getTrigger()
{
	return m_rAxis[vr::EVRButtonId::k_EButton_SteamVR_Trigger].x;
}

Vector2 OpenVRController::getTouchpad()
{
	return Vector2(m_rAxis[vr::EVRButtonId::k_EButton_SteamVR_Touchpad].x, m_rAxis[vr::EVRButtonId::k_EButton_SteamVR_Touchpad].y);
}

#endif

//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		openvr wrapper and vr renderer
//
// $NoKeywords: $vr
//===============================================================================//

#ifndef OPENVRINTERFACE_H
#define OPENVRINTERFACE_H

#include "cbase.h"
#include "KeyboardListener.h"

// HACKHACK:
#include "OpenGLHeaders.h"
#include <stdio.h>
#include <string>
#include <cstdlib>

#ifdef MCENGINE_FEATURE_OPENVR

#include <openvr_mingw.hpp>

#endif

class Shader;
class Camera;
class RenderTarget;

class OpenVRController;
class CGLRenderModel;

//class OBJModel;
//class MDLMaterial;

class OpenVRInterface : public KeyboardListener
{
public:
	OpenVRInterface();
	virtual ~OpenVRInterface();

	void draw(Graphics *g);
	void update();

	virtual void onKeyDown(KeyboardEvent &e);
	virtual void onKeyUp(KeyboardEvent &e);
	virtual void onChar(KeyboardEvent &e) {;}

	void onResolutionChange(Vector2 newResolution);

	typedef fastdelegate::FastDelegate0<> DrawCallback;
	void setDrawCallback(DrawCallback drawCallback) {m_drawCallback = drawCallback;}

	inline Matrix4 getCurrentModelViewProjectionMatrix() {return m_matCurrentMVP;}

	inline OpenVRController *getController() {return m_controller;}
	inline OpenVRController *getLeftController() {return m_controllerLeft;}
	inline OpenVRController *getRightController() {return m_controllerRight;}

	inline bool isReady() const {return m_bReady;}

private:

#ifdef MCENGINE_FEATURE_OPENVR

	bool initRenderer();
	bool initCompositor();
	void initRenderModels();
	bool initRenderTargets();
	bool initShaders();

	void renderStereoTargets(Graphics *g);
	void renderScene(Graphics *g, vr::Hmd_Eye eye);
	void renderStereoToWindow(Graphics *g);
	void updateControllerAxes();

	void updateStaticMatrices(); // eye position offset and projection
	void updateMatrixPoses(); // pose matrices
	void updateRenderModelForTrackedDevice(vr::TrackedDeviceIndex_t unTrackedDeviceIndex);
	CGLRenderModel *findOrLoadRenderModel(const char *pchRenderModelName);

	Matrix4 getHMDMatrixProjectionEye(vr::Hmd_Eye eye);
	Matrix4 getHMDMatrixPoseEye(vr::Hmd_Eye eye);
	Matrix4 getCurrentModelViewProjectionMatrix(vr::Hmd_Eye eye);
	Matrix4 getCurrentViewProjectionMatrix(vr::Hmd_Eye eye);
	Matrix4 getCurrentEyePosMatrix(vr::Hmd_Eye eye);

	void onClippingPlaneChange(UString oldValue, UString newValue);

#endif

private:

#ifdef MCENGINE_FEATURE_OPENVR

	static std::string getTrackedDeviceString(vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL);
	static Matrix4 convertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose);

#endif

	struct VertexDataLens
	{
		Vector2 position;
		Vector2 texCoordRed;
		Vector2 texCoordGreen;
		Vector2 texCoordBlue;
	};

	bool m_bReady;

	OpenVRController *m_controller;
	OpenVRController *m_controllerLeft;
	OpenVRController *m_controllerRight;

	DrawCallback m_drawCallback;

#ifdef MCENGINE_FEATURE_OPENVR

	std::vector<CGLRenderModel*> m_vecRenderModels;
	CGLRenderModel *m_rTrackedDeviceToRenderModel[vr::k_unMaxTrackedDeviceCount];

	// openvr system
	vr::IVRSystem *m_pHMD;
	vr::IVRRenderModels *m_pRenderModels;
	std::string m_strDriver;
	std::string m_strDisplay;
	vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	Matrix4 m_rmat4DevicePose[vr::k_unMaxTrackedDeviceCount];

	// shaders
	Shader *m_renderModelShader;
	Shader *m_controllerAxisShader;
	Shader *m_genericTexturedShader;

	// tracking
	int m_iTrackedControllerCount;
	int m_iTrackedControllerCount_Last;
	int m_iValidPoseCount;
	int m_iValidPoseCount_Last;
	std::string m_strPoseClasses;                            // what classes we saw poses for this frame
	char m_rDevClassChar[ vr::k_unMaxTrackedDeviceCount ];   // for each device, a character representing its class

	// mesh buffers
	GLuint m_glControllerVertBuffer;
	GLuint m_unControllerVAO;
	unsigned int m_uiControllerVertcount;

#endif

	// matrices
	Matrix4 m_mat4HMDPose;
	Matrix4 m_mat4eyePosLeft;
	Matrix4 m_mat4eyePosRight;

	Matrix4 m_mat4ProjectionCenter;
	Matrix4 m_mat4ProjectionLeft;
	Matrix4 m_mat4ProjectionRight;

	Matrix4 m_matCurrentMVP;

	// framebuffers
	RenderTarget *m_leftEye;
	RenderTarget *m_rightEye;
	RenderTarget *m_debugOverlay;

	// debugging
	//OBJModel *m_testModel;
	//MDLMaterial *m_testMaterial;

	Camera *m_fakeCamera;
	bool m_bWDown;
	bool m_bADown;
	bool m_bSDown;
	bool m_bDDown;
	bool m_bCaptureMouse;
	bool m_bShiftDown;
	bool m_bCtrlDown;
};

extern OpenVRInterface *openvr;

class CGLRenderModel
{
public:
	CGLRenderModel(const std::string &sRenderModelName);
	~CGLRenderModel();
	void cleanup();

	void draw();

#ifdef MCENGINE_FEATURE_OPENVR

	bool init(const vr::RenderModel_t &vrModel, const vr::RenderModel_TextureMap_t &vrDiffuseTexture);

#endif

	const std::string &getName() const {return m_sModelName;}

private:
	GLuint m_glVertBuffer;
	GLuint m_glIndexBuffer;
	GLuint m_glVertArray;
	GLuint m_glTexture;
	GLsizei m_unVertexCount;

	std::string m_sModelName;
};

#endif

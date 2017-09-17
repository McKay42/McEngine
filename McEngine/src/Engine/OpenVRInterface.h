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

// HACKHACK: remove this header once a proper model wrapper exists
#include "OpenGLHeaders.h"
#include <string>

#ifdef MCENGINE_FEATURE_OPENVR

#include <openvr_mingw.hpp>

#endif

class Shader;
class Camera;
class RenderTarget;
class OpenVRController;

class CGLRenderModel;

class OpenVRInterface : public KeyboardListener
{
public:
	struct PLAY_AREA_RECT
	{
		Vector3 corners[4];
	};

public:
	OpenVRInterface();
	virtual ~OpenVRInterface();

	void draw(Graphics *g);
	void update();

	virtual void onKeyDown(KeyboardEvent &e);
	virtual void onKeyUp(KeyboardEvent &e);
	virtual void onChar(KeyboardEvent &e);

	void onResolutionChange(Vector2 newResolution);

	void showKeyboard() {showKeyboardEx("", "");}
	void showKeyboardEx(UString description, UString text);
	void hideKeyboard();

	void updatePlayAreaMetrics();
	void resetFakeCameraMovement();
	void resetFakeCameraMouseCapture();

	typedef fastdelegate::FastDelegate1<Graphics*> DrawCallback;
	void setDrawCallback(DrawCallback drawCallback) {m_drawCallback = drawCallback;}

	void setControllerColorOverride(Color controllerColor);

	inline UString getTrackingSystemName() {return m_sTrackingSystemName;}

	inline Matrix4 getCurrentModelMatrix() {return m_matCurrentM;}
	inline Matrix4 getCurrentViewProjectionMatrix() {return m_matCurrentVP;}
	inline Matrix4 getCurrentModelViewProjectionMatrix() {return m_matCurrentMVP;}

	inline OpenVRController *getController() {return m_controller;}
	inline OpenVRController *getLeftController() {return m_controllerLeft;}
	inline OpenVRController *getRightController() {return m_controllerRight;}

	inline Vector2 getPlayAreaSize() {return m_vPlayAreaSize;}
	inline PLAY_AREA_RECT getPlayAreaRect() {return m_playAreaRect;}

	Vector2 getRenderTargetResolution();

	inline bool isReady() const {return m_bReady;}
	inline bool isKeyboardVisible() const {return m_bIsKeyboardVisible;}
	bool hasInputFocus();

private:

#ifdef MCENGINE_FEATURE_OPENVR

	bool initRenderer();
	bool initCompositor();
	void initRenderModels();
	bool initRenderTargets();
	bool initShaders();

	void renderStereoTargets(Graphics *g);
	void renderSpectatorTarget(Graphics *g);
	void renderScene(Graphics *g, vr::Hmd_Eye eye);
	void renderScene(Graphics *g,  Matrix4 &matCurrentEye, Matrix4 &matCurrentM, Matrix4 &matCurrentP, Matrix4 &matCurrentVP, Matrix4 &matCurrentMVP);
	void renderStereoToWindow(Graphics *g);

	void updateControllerAxes(); // for debugging
	void updateStaticMatrices(); // eye position offset and projection
	bool updateMatrixPoses(); // pose matrices
	void updateRenderModelForTrackedDevice(vr::TrackedDeviceIndex_t unTrackedDeviceIndex);
	CGLRenderModel *findOrLoadRenderModel(const char *pchRenderModelName);

	Matrix4 getHMDMatrixProjectionEye(vr::Hmd_Eye eye);
	Matrix4 getHMDMatrixPoseEye(vr::Hmd_Eye eye);
	Matrix4 getCurrentModelViewProjectionMatrix(vr::Hmd_Eye eye);
	Matrix4 getCurrentViewProjectionMatrix(vr::Hmd_Eye eye);
	Matrix4 getCurrentProjectionMatrix(vr::Hmd_Eye eye);
	Matrix4 getCurrentEyePosMatrix(vr::Hmd_Eye eye);

	void onSSChange(UString oldValue, UString newValue);
	void onSSCompositorChange(UString oldValue, UString newValue);
	void onCompositorSubmitDoubleChange(UString oldValue, UString newValue);
	void onAAChange(UString oldValue, UString newValue);
	void onClippingPlaneChange(UString oldValue, UString newValue);
	void onBackgroundBrightnessChange(UString oldValue, UString newValue);
	void onHeadRenderModelChange(UString oldValue, UString newValue);

#endif

private:

#ifdef MCENGINE_FEATURE_OPENVR

	static std::string getTrackedDeviceString(vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL);
	static Matrix4 convertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose);

#endif

	void toggleFakeCameraMouseCapture();
	void saveFakeCamera();
	void loadFakeCamera();

	bool m_bReady;
	UString m_sTrackingSystemName;

	// main draw callback
	DrawCallback m_drawCallback;

	// controllers
	OpenVRController *m_controller;
	OpenVRController *m_controllerLeft;
	OpenVRController *m_controllerRight;
	Color m_controllerColorOverride;

	// matrices
	Matrix4 m_mat4HMDPose;
	Matrix4 m_mat4eyePosLeft;
	Matrix4 m_mat4eyePosRight;

	Matrix4 m_mat4ProjectionCenter;
	Matrix4 m_mat4ProjectionLeft;
	Matrix4 m_mat4ProjectionRight;

	Matrix4 m_matCurrentM;
	Matrix4 m_matCurrentVP;
	Matrix4 m_matCurrentMVP;

	// framebuffers
	RenderTarget *m_leftEye;
	RenderTarget *m_rightEye;
	RenderTarget *m_compositorEye1;
	RenderTarget *m_compositorEye2;
	RenderTarget *m_debugOverlay;

	// play area
	Vector2 m_vPlayAreaSize;
	PLAY_AREA_RECT m_playAreaRect;

	// keyboard
	bool m_bIsKeyboardVisible;

	// debugging
	Camera *m_fakeCamera;
	bool m_bCaptureMouse;
	bool m_bWDown;
	bool m_bADown;
	bool m_bSDown;
	bool m_bDDown;
	bool m_bShiftDown;
	bool m_bCtrlDown;
	bool m_bIsSpectatorDraw;

#ifdef MCENGINE_FEATURE_OPENVR

	std::vector<CGLRenderModel*> m_vecRenderModels;
	CGLRenderModel *m_rTrackedDeviceToRenderModel[vr::k_unMaxTrackedDeviceCount];
	CGLRenderModel *m_headRenderModel;

	// openvr system
	vr::IVRSystem *m_pHMD;
	vr::IVRRenderModels *m_pRenderModels;
	std::string m_strDisplay;
	vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	Matrix4 m_rmat4DevicePose[vr::k_unMaxTrackedDeviceCount];

	// shaders
	Shader *m_renderModelShader;
	Shader *m_controllerAxisShader;
	Shader *m_genericTexturedShader;
	Shader *m_genericUntexturedShader;

	// tracking
	int m_iTrackedControllerCount;
	int m_iTrackedControllerCount_Last;
	int m_iValidPoseCount;
	int m_iValidPoseCount_Last;
	std::string m_strPoseClasses;                            // what classes we saw poses for this frame
	char m_rDevClassChar[ vr::k_unMaxTrackedDeviceCount ];   // for each device, a character representing its class

	// TEMP: mesh buffers
	GLuint m_glControllerVertBuffer;
	GLuint m_unControllerVAO;
	unsigned int m_uiControllerVertcount;

	// custom
	float m_fPrevSSMultiplier;
	float m_fCompositorSSMultiplier;
	float m_fPrevAA;
	bool m_bSteamVRBugWorkaroundCompositorSSChangeAllowed;

#endif
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

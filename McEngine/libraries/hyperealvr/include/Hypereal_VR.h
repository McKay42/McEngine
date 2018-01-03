/**
* File:        Hypereal_VR.h

* Email:       vrsupport@hypereal.com
* Site:        http://www.hypereal.com
*
* Copyright (c) 2017, Shanghai Chai Ming Huang Info&Tech Co, Ltd. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*  * Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*  * Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*  * Neither the name of Chai Ming Huang/Hypereal CORPORATION nor the names of its
*    contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/


#ifndef _HYPEREAL_VR_H_
#define _HYPEREAL_VR_H_

#include <stdint.h>

#define HY_API extern "C"

#ifdef _WIN32
#define HY_CALLTYPE __cdecl
#else
#define HY_CALLTYPE
#endif

#ifndef HY_DEPRECATED_PREFIX
#if defined(_MSC_VER)
#define HY_DEPRECATED_PREFIX __declspec(deprecated)
#define HY_DEPRECATED_SUFFIX
#elif defined(__GNUC__)
#define HY_DEPRECATED_PREFIX
#define HY_DEPRECATED_SUFFIX __attribute__((deprecated))
#else
#define HY_DEPRECATED_PREFIX
#define HY_DEPRECATED_SUFFIX
#endif
#endif

#pragma pack(push, 8)

/**
 * Result codes of SDK.
 */
enum HyResult
{
	hySuccess = 0,							/**< Successful. */
	hySuccess_ShouldQuit	= -1,			/**< Client app should exit immediately, or some functionalities could be compromised */
	hySuccess_FallBack		= -2,			/**< The request isn't/cannot be satisfied & a fall back action is performed instead */
	hySuccess_FocusLost		= -3,			/**< The request is only allowed for app with focus & as this is not, the request is ignored */
	hySuccess_InputNotReady	= -4,			/**< User hasn't confirmed her input yet, only partial or no result is available */

	/**< common errors. */
	hyError								= 100,
	hyError_NeedStartup					= 101,
	hyError_DeviceNotStart				= 102,
	hyError_InvalidHeadsetOrientation	= 103,
	hyError_RenderNotCreated			= 104,
	hyError_TextureNotCreated			= 105,
	hyError_DisplayLost					= 106,
	hyError_NoHmd						= 107,
	hyError_DeviceNotConnected			= 108,
	hyError_ServiceConnection			= 109,
	hyError_ServiceError				= 110,
	hyError_InvalidParameter			= 111,
	hyError_NoCalibration				= 112,
	hyError_NotImplemented				= 113,
	hyError_InvalidClientType			= 114,
	hyError_BufferTooSmall				= 115,
	hyError_InvalidState				= 120,
	hyError_FileNotExist				= 150,
	hyError_AdaptorLuidNotMatched       = 160,
	hyError_RuntimeVersionNotCompatible = 161,
	
	/**<  initialization errors. */
	hyError_Initialize_Texture			= 500,
	hyError_Initialize_TrackingSystem	= 501,

	HyError_InvalidSignature			= 600,

	HyResult_ForceInt = 0x7fffffff
};

#define hySucceeded(hr) (((HyResult)(hr)) <= hySuccess)		// check whether a returned HyResult indicates success

/**
 * Graphics API type.
 */
enum HyGraphicsAPI
{
	HY_GRAPHICS_UNKNOWN	= 0,			/**< Unknown graphics API. */
	HY_GRAPHICS_D3D11	= 1,			/**< Direct3D 11. */
	HY_GRAPHICS_D3D12	= 2,			/**< Direct3D 12. */
	HY_GRAPHICS_OGL		= 3,			/**< OpenGL. */
	HY_GRAPHICS_VULKAN	= 4				/**< Vulkan. */
};

/**
 * Texture pixel format.
 */
enum HyTextureFormat
{
	HY_TEXTURE_UNKNOWN				= 0,	/**< Unknown format. */
	HY_TEXTURE_R8G8B8A8_UNORM		= 1,	/**< Four component, 32-bit unsigned normalized integer format in linear space. */
	HY_TEXTURE_R8G8B8A8_UNORM_SRGB	= 2,	/**< Four component, 32-bit unsigned normalized integer format in SRGB space. */
	HY_TEXTURE_B8G8R8A8_UNORM_SRGB	= 3		/**< Four component, 32-bit unsigned normalized integer format in SRGB space. */
};

/**
 * Eye type.
 */
enum HyEye
{
	HY_EYE_LEFT,						/**< Left eye. */
	HY_EYE_RIGHT,						/**< Right eye. */
	HY_EYE_MAX
};

/**
 * Different Hypereal devices.
 */
enum HySubDevice
{
	HY_SUBDEV_UNKNOWN = -1,									/**< Invalid device type. */
	HY_SUBDEV_HMD = 0,										/**< Head Mounted Display type. */
	HY_SUBDEV_CONTROLLER = 0x100,							/**< Controller type. */
	HY_SUBDEV_CONTROLLER_LEFT = HY_SUBDEV_CONTROLLER,		/**< Left controller. */
	HY_SUBDEV_CONTROLLER_RIGHT = HY_SUBDEV_CONTROLLER + 1,	/**< Right controller. */
	HY_SUBDEV_TRACKER = 0x200,								/**< Tracker type, 0x200 - 0x299. */
	HY_SUBDEV_CUSTOMIZED = 0xf000							/**< Customized Device type */
};
/**
 * Return the device's type of a specific value.
 */
#define  HyGetSubDeviceType(id) ( (id) < 0 ?  HY_SUBDEV_UNKNOWN : (HySubDevice)((id) - (id) % 0x100))
/**
 * Convert a base type and offset to a device type.
 */
#define  HySubDeviceID(type, offset) (HySubDevice)((type) + (offset))
// HY_SUBDEV_CONTROLLER_LEFT  == HySubDeviceID(HY_SUBDEV_CONTROLLER, 0);
// HY_SUBDEV_CONTROLLER_RIGHT == HySubDeviceID(HY_SUBDEV_CONTROLLER, 1);

/**
 * Tracking origin type which specifies the coordinate system of tracked poses.
 */
enum HyTrackingOrigin
{
	HY_TRACKING_ORIGIN_UNKNOWN	= 0,	/**< Used to get current tracking origin type. */
	HY_TRACKING_ORIGIN_EYE		= 1,	/**< Poses are provided relative to calibrated HMD pose. */
	HY_TRACKING_ORIGIN_FLOOR	= 2		/**< Poses are provided relative to calibrated floor pose. */
};

/**
 * Flags of the tracking state.
 */
enum HyTrackingFlag
{
	HY_TRACKING_NONE = 0,					/**< Neither rotation nor position are tracked. */
	HY_TRACKING_ROTATION_TRACKED = 1,		/**< Rotation is tracked. */
	HY_TRACKING_POSITION_TRACKED = 1 << 1	/**< Position is tracked. */
};

/**
 * Flags of the input button state (pressed or not).
 */
enum HyButton
{
	HY_BUTTON_NONE = 0,										/**< None button is pressed. */

	HY_BUTTON_TOUCHPAD_RIGHT = 1,							/**< TouchPad button (right controller) is pressed. */
	HY_BUTTON_RTOUCHPAD = HY_BUTTON_TOUCHPAD_RIGHT,			/**< TouchPad button (right controller) is pressed. */
	HY_BUTTON_RDPAD_UP = 1 << 1,							/**< Emulated D-PAD Up button (right controller) is pressed. */
	HY_BUTTON_RDPAD_DOWN = 1 << 2,							/**< Emulated D-PAD Down button (right controller) is pressed. */
	HY_BUTTON_RDPAD_LEFT = 1 << 3,							/**< Emulated D-PAD Left button (right controller) is pressed. */
	HY_BUTTON_RDPAD_RIGHT = 1 << 4,							/**< Emulated D-PAD Right button (right controller) is pressed. */

	HY_BUTTON_TOUCHPAD_LEFT = 1 << 15,						/**< TouchPad button (left controller) is pressed. */
	HY_BUTTON_LTOUCHPAD = HY_BUTTON_TOUCHPAD_LEFT,			/**< TouchPad button (left controller) is pressed. */
	HY_BUTTON_LDPAD_UP = 1 << 16,							/**< Emulated D-PAD Up button (left controller) is pressed. */
	HY_BUTTON_LDPAD_DOWN = 1 << 17,							/**< Emulated D-PAD Down button (left controller) is pressed. */
	HY_BUTTON_LDPAD_LEFT = 1 << 18,							/**< Emulated D-PAD Left button (left controller) is pressed. */
	HY_BUTTON_LDPAD_RIGHT = 1 << 19,						/**< Emulated D-PAD Right button (left controller) is pressed. */

	HY_BUTTON_RMENU = 1 << 29,								/**< Hypereal button (right controller) is pressed. */
	HY_BUTTON_LMENU = 1 << 30,								/**< Menu button (left controller) is pressed. */
	HY_BUTTON_MENU = HY_BUTTON_LMENU                        /**< deprecated define only for backward compatibility. */
};

enum HyTouch
{
	HY_TOUCH_NONE = 0,										/**< None button is touched. */

	HY_TOUCH_TOUCHPAD_RIGHT = 1,							/**< TouchPad (right controller) is touched. */
	HY_TOUCH_INDEX_TRIGGER_RIGHT = 1 << 1,					/**< Index Trigger (right controller) is touched. */

	HY_TOUCH_RDPAD_UP = 1 << 2,								/**< Emulated D-PAD Up button (right controller) is touched. */
	HY_TOUCH_RDPAD_DOWN = 1 << 3,							/**< Emulated D-PAD Down button (right controller) is touched. */
	HY_TOUCH_RDPAD_LEFT = 1 << 4,							/**< Emulated D-PAD Left button (right controller) is touched. */
	HY_TOUCH_RDPAD_RIGHT = 1 << 5,							/**< Emulated D-PAD Right button (right controller) is touched. */

	HY_TOUCH_SIDE_TRIGGER_RIGHT = 1 << 6,					/**< Side Trigger (right controller) is touched. */

	HY_TOUCH_TOUCHPAD_LEFT = 1 << 15,						/**< TouchPad (left controller) is touched. */
	HY_TOUCH_INDEX_TRIGGER_LEFT = 1 << 16,					/**< Index Trigger (left controller) is touched. */

	HY_TOUCH_LDPAD_UP = 1 << 17,							/**< Emulated D-PAD Up button (left controller) is touched. */
	HY_TOUCH_LDPAD_DOWN = 1 << 18,							/**< Emulated D-PAD Down button (left controller) is touched. */
	HY_TOUCH_LDPAD_LEFT = 1 << 19,							/**< Emulated D-PAD Left button (left controller) is touched. */
	HY_TOUCH_LDPAD_RIGHT = 1 << 20,							/**< Emulated D-PAD Right button (left controller) is touched. */

	HY_TOUCH_SIDE_TRIGGER_LEFT = 1 << 21,					/**< Side Trigger (left controller) is touched. */
};

/**
* Special flags creating device for a certain app by HyCreateInterface().
*/
enum HyFlagBit
{
	HyDeviceCreateFlag_DontSwitchDefaultAudio = 1,  /**< set this flag if the app don't need to switch windows default auido device */
};

/**
* Hardware version
*/
enum HyDeviceVersion {
	HY_DEVICE_UNKNOWN	= 0,	/**< invalid device version */
	HY_DEVICE_HK1		= 1,	/**< HR-HK1-HMD */
	HY_DEVICE_HK2		= 2		/**< HR-HK2-HMD */
};

/**
 * Flags for View Layer
 */
enum HyViewLayerFlags
{
	HY_LAYER_FLAG_LOCK_TO_HELMET = 1,	/**< The layer will move with your helmet */
};

/**
 * Message types that can be sent to apps
 */
enum HyMsgType
{
	HY_MSG_NONE = 0,					/**< no more msg for now; ask again next frame */
	HY_MSG_APP_STATUS_CHANGE	= 1,	/**< HyDevice for the application is started */
	HY_MSG_PENDING_QUIT			= 2,	/**< application should quit now */
	HY_MSG_DEVICE_INVALIDATED	= 3,	/**< this HyDevice instance is no longer valid */
	HY_MSG_VIEW_FOCUS_CHANGED	= 4,	/**< view focus changed; this app get focus when m_id is 0, otherwise focus is lost */
	HY_MSG_INPUT_FOCUS_CHANGED	= 5,	/**< input focus changed; this app get focus when m_id is 0, otherwise focus is lost */
	HY_MSG_SUBDEVICE_STATUS_CHANGED		= 6,	/**< sub device connection status changed; value 0 means disconnected; otherwise connected */
	HY_MSG_SUBDEVICE_BATTERY_CHANGED	= 7,	/**< sub device battery status changed; value is percentage */
	HY_MSG_SUBDEVICE_TRACKING_CHANGED	= 8,	/**< sub device tracking state change; value can be cast to HyTrackingFlag */
	HY_MSG_IPD_CHANGED					= 9,	/**< helmet inter-pupillary distance changed */
	HY_MSG_NOTIFICATION_STATUS_CHANGED	= 100,	/**< notification view layer status change */
	HY_MSG_STANDBY_STATUS_CHANGED		= 101,	/**< standby status change, non-zero for entering standby mode, zero for leaving */

	HY_MSG_USER = 10000,
};

enum HyMsgIdConst
{
	HY_ID_SELF_IN_MSG = 0,
};

enum HyPropertyConst
{
	HY_STRING_VALUE_MAX_LENGHT = 16 * 1024
};

enum HyDeviceProperty
{
	// --------------------------------------------------------------------------
	/**
	 * Common device specific properties (0-199)
	 * Have to specify which sub device is requesting or setting properties for.
	 */
	HY_PROPERTY_DEVICE_CONNECTED_BOOL					= 0,	/**< Get only. Is the device connected or not.*/
	HY_PROPERTY_DEVICE_BATTERY_INT						= 1,	/**< Get only. The battery of the device.*/
	HY_PROPERTY_DEVICE_FWVERSION_INT					= 2,	/**< Get only. The version of firmware of the device.*/
	HY_PROPERTY_DEVICE_COUNT_INT						= 3,	/**< Get only. the count of the devices in the same type.*/

	/**
	 * HMD specific properties (200 -299)
	 * No need to specify the sub device when using, because only have one HMD connected in system.
	 */
	HY_PROPERTY_HMD_IPD_FLOAT							= 200,	/**< Get only. Interpupillary distance.*/
	HY_PROPERTY_HMD_EYE_HEIGHT_FLOAT					= 201,	/**< Get only. Height of eye.*/
	HY_PROPERTY_HMD_BOTH_EYES_FOV_FLOAT8_ARRAY			= 202,	/**< Get only. FOV values for both screens, include inner,outer,top,bottom for each.*/
	HY_PROPERTY_HMD_LEFT_EYE_FOV_FLOAT4_ARRAY			= 203,	/**< Get only. FOV values for left screen, include inner,outer,top,bottom.*/
	HY_PROPERTY_HMD_RIGHT_EYE_FOV_FLOAT4_ARRAY			= 204,	/**< Get only. FOV values for right screen, include inner,outer,top,bottom.*/
	HY_PROPERTY_HMD_RESOLUTION_X_INT					= 205,	/**< Get only. The width of screen resolution.*/
	HY_PROPERTY_HMD_RESOLUTION_Y_INT					= 206,	/**< Get only. The height of screen resolution.*/
	HY_PROPERTY_HMD_REFRESH_RATE_FLOAT					= 207,	/**< Get only. The refresh rate of screen.*/
	HY_PROPERTY_HMD_AUDIO_OUTPUT_GUID_STRING			= 208,  /**< Get only. The GUID of output audio device on HMD*/
	HY_PROPERTY_HMD_AUDIO_INPUT_GUID_STRING				= 209,  /**< Get only. The GUID of input audio device on HMD*/
	HY_PROPERTY_HMD_AUDIO_OUTPUT_ID_STRING				= 210,  /**< Get only. The IMMDevice ID of output audio device on HMD*/
	HY_PROPERTY_HMD_AUDIO_INPUT_ID_STRING				= 211,  /**< Get only. The IMMDevice ID of input audio device on HMD*/

	/**
	 * Controller specific properties (300 - 399)
	 */


	/**
	 * Tracker specific properties (400 - 499)
	 */
	HY_PROPERTY_TRACKER_FRUSTUM_FOV_FLOAT2_ARRARY		= 400,	/**< not implemented yet */
	HY_PROPERTY_TRACKER_FRUSTUM_Z_RANGE_FLOAT2_ARRAY	= 401,	/**< not implemented yet */

	/**
	 * Camera specific properties (500 - 599)
	 */

	// --------------------------------------------------------------------------

	/**
	 * System specific properties (1000 - 1199)
	 */
	HY_PROPERTY_PRODUCT_NAME_STRING						= 1000,	/**< Get only. Product name.*/
	HY_PROPERTY_MANUFACTURER_STRING						= 1001,	/**< Get only. Vendor name*/
	HY_PROPERTY_HAPTIC_ENABLE_BOOL						= 1002,	/**< Get/Set. Enable vibration or not, global setting for all controllers.*/
	HY_PROPERTY_CALIBRATION_REQUIRED_BOOL				= 1003,	/**< Get only. Is a room setting required or not.*/
	HY_PROPERTY_POSITION_TRACKING_ENABLED_BOOL			= 1004,	/**< Get only. Is the position tracking enabled or not.*/

	/**
	 * Chaperon specific properties (1200 - 1299)
	 */
	HY_PROPERTY_CHAPERONE_ENABLE_BOOL					= 1200,	/**< Get/Set. Is safety wall system is enabled or not, global setting for all Apps.*/
	HY_PROPERTY_CHAPERONE_VISIBLE_BOOL					= 1201,	/**< Get/Set. Visibility of safety wall system of current App which is requesting or setting this property. */
	HY_PROPERTY_CHAPERONE_VERTEX_COUNT_INT				= 1202,	/**< Get only. Safety wall data, vertices number.*/
	HY_PROPERTY_CHAPERONE_VERTEX_VEC2_ARRAY				= 1203,	/**< Get only. Safety wall data, vertices array.*/


	/**
	* Layer specific properties (1300 - 1399)
	* Have to specify the layer id when using.
	*/
	HY_PROPERTY_VIEW_LAYER_FLAG_INT						= 1300,	/**< Get only. Flag of the layer.*/
	HY_PROPERTY_VIEW_LAYER_POSE_FLOAT7_ARRAY			= 1301,	/**< Get only. Pose of the layer.*/
	HY_PROPERTY_VIEW_LAYER_SIZE_FLOAT2_ARRAY			= 1302,	/**< Get only. The size of the layer.*/
	HY_PROPERTY_VIEW_LAYER_SIZE_POSE_FLOAT9_ARRAY		= 1303,	/**< Get only. The size and pose of the layer.*/

	/**
	 *  Renderer specific properties (1400 - 1499)
	 */
	HY_PROPERTY_IS_QUEUE_AHEAD_ENABLED_BOOL				= 1400,	/**< Get only. If queue ahead is enabled. */
	HY_PROPERTY_COLOR_MOD_FLOAT_ARRAY					= 1401, /**< Set only. Color factors. */
	HY_PROPERTY_HIDDEN_AREA_POINT_COUNT_INT				= 1402, /**< Get only. Number of hidden mesh points. */
	HY_PROPERTY_HIDDEN_AREA_DATA_FLOAT_ARRAY			= 1403, /**< Get only. Data of hidden mesh points of both eyes by a float array in size of 2 (eyes) * 2 (x,y) * points count. */
	HY_PROPERTY_VISIBLE_AREA_POINT_COUNT_INT			= 1404, /**< Get only. Number of visible mesh points. */
	HY_PROPERTY_VISIBLE_AREA_DATA_FLOAT_ARRAY			= 1405, /**< Get only. Data of visible mesh points of both eyes by a float array in size of 2 (eyes) * 2 (x,y) * points count. */
	HY_PROPERTY_DISPLAY_ADAPTER_LUID_INT				= 1406, /**< Get only. LUID of display adapter. You can compare with Adapter Desc and find out which one to use. */
	// --------------------------------------------------------------------------
	/**
	 * For debugging or other purpose (9900 - 9999)
	 */
	HY_PROPERTY_IS_IN_DEBUGGER_BOOL						= 9900, /**< Get/Set. If debugging in Visual Studio or in Unreal/Unity (preview), it will be true. */
};

/**
 * 2D vector with floating point precision.
 */
struct HyVec2
{
	float x;							/**< Public x component. */
	float y;							/**< Public y component. */
};

/**
 * 3D vector with floating point precision.
 */
struct HyVec3
{
	float x;							/**< Public x component. */
	float y;							/**< Public y component. */
	float z;							/**< Public z component. */
};

/**
 * (Normalized) Quaternion which represents a 3D rotation with floating point precision.
 */
struct HyQuat
{
	float x;							/**< Public x component. */
	float y;							/**< Public y component. */
	float z;							/**< Public z component. */
	float w;							/**< Public w component. */
};

/**
 * 4x4 matrix (column major) with floating point precision.
 */
struct HyMat4
{
	float m[4][4];						/**< Public m component. */
};

/**
 * Rigid euclidean transform.
 */
struct HyPose
{
	HyQuat m_rotation;					/**< Rotation part. */
	HyVec3 m_position;					/**< Translation part. */
};

/**
 * Field of view.
 */
union HyFov
{
	float val[4];
	struct {
		float m_upTan;						/**< Tangent of the angle between viewing direction and top frustum plane. */
		float m_downTan;					/**< Tangent of the angle between viewing direction and bottom frustum plane. */
		float m_leftTan;					/**< Tangent of the angle between viewing direction and left frustum plane. */
		float m_rightTan;					/**< Tangent of the angle between viewing direction and right frustum plane. */
	};
};

/**
 * Viewport/Region of a texture.
 */
struct HyViewport
{
	uint32_t m_leftTopX;				/**< X position of the left top of the Viewport. */
	uint32_t m_leftTopY;				/**< Y position of the left top of the Viewport. */
	uint32_t m_width;					/**< Width of the Viewport. */
	uint32_t m_height;					/**< Height of the Viewport. */
};

/**
 * Texture descriptor.
 */
struct HyTextureDesc
{
	HyVec2 m_uvOffset;					/**< UV offset of the texture. */
	HyVec2 m_uvSize;					/**< UV size of the texture. */
	void *m_texture;					/**< Texture handle. */
	uint32_t m_flags;					/**< reserved */
};

/**
 * Tracking state of sub-device.
 */
struct HyTrackingState
{
	HyPose m_pose;						/**< Transform of the sub-device. */
	HyVec3 m_angularVelocity;			/**< Angular velocity of the sub-device. */
	HyVec3 m_linearVelocity;			/**< Linear velocity of the sub-device. */
	HyVec3 m_angularAcceleration;		/**< Angular acceleration of the sub-device. */
	HyVec3 m_linearAcceleration;		/**< Linear acceleration of the sub-device. */
	uint32_t m_flags;					/**< Flags of the tracking state. @see HyTrackingFlag. */
};

/**
 * Input state of controller.
 */
struct HyInputState
{
	uint32_t m_buttons;					/**< Button states. @see HyButton. */
	uint32_t m_touches;					/**< Touch states.  @see HyTouch. */
	float m_indexTrigger;				/**< Index Trigger state, [0.0f, 1.0f]. */
	float m_sideTrigger;				/**< Side Trigger state, [0.0f, 1.0f]. */
	float m_indexTriggerProximity;		/**< How close the finger is to Index Trigger, [0.0f, 1.0f]. */
	float m_touchpadProximity;			/**< How close the finger is to TouchPad, [0.0f, 1.0f]. */
	HyVec2 m_touchpad;					/**< TouchPad state, [-1.0f, 1.0f]. */
};

/**
 * Header of all messages
 */
struct HyMsgHeader
{
	uint16_t m_size;		/**< size of the msg, in bytes */
	uint16_t m_type;		/**< this value is from HyMsgType */
};

/**
 * Msg for app start
 */
struct HyMsgAppInfo
{
	HyMsgHeader m_header;	/**< type should be HY_MSG_APP_STATUS_CHANGE */
	uint16_t m_id;			/**< global id for this HyDevice */
	uint8_t m_reserved;		/**< should be ignored */
	uint8_t m_action;		/**< should be 0, for starting */
};

/**
 * Msg for pending quit, device invalidated, etc
 */
struct HyMsgWithReason
{
	HyMsgHeader m_header;	/**< type should be HY_MSG_PENDING_QUIT, or HY_MSG_DEVICE_INVALIDATED */
	uint32_t m_reason;		/**< reserved, or depend on msg type */
};

/**
 * Msg for focus change
 */
struct HyMsgFocusChange
{
	HyMsgHeader m_header;	/**< type should be HY_MSG_VIEW_FOCUS_CHANGED, or HY_MSG_INPUT_FOCUS_CHANGED */
	uint16_t m_id;			/**< 0 for changed to this HyDevice, otherwise to others */
};

/**
 * Msg for subdevice changes
 */
struct HyMsgSubdeviceChange
{
	HyMsgHeader m_header;	/**< type should be HY_MSG_SUBDEVICE_***_CHANGED, or HY_MSG_STANDBY_STATUS_CHANGED */
	uint16_t m_subdevice;	/**< Sub device, or HY_SUBDEV_HMD for standby changed */
	uint16_t m_value;		/**< Meaning of this depends on msg type. */
};

/**
 * Msg for IPD change
 */
struct HyMsgIpdChange
{
	HyMsgHeader m_header;	/**< type should be HY_MSG_IPD_CHANGED */
	float m_value;			/**< new inter-pupillary distance */
};

/**
 * Msg for view layer status changes/
 */
struct HyMsgViewLayerChange
{
	HyMsgHeader m_header;	/**< type should be HY_MSG_NOTIFY_STATUS_CHANGE */
	uint16_t	m_id;		/**< view layer id */
	uint16_t	m_value;	/**< non-zero if it is visible; zero if invisible */
};

union HyMsg
{
	HyMsgHeader header;
	HyMsgAppInfo appInfo;
	HyMsgWithReason reason;
	HyMsgFocusChange focusChange;
	HyMsgSubdeviceChange subdeviceChange;
	HyMsgIpdChange ipdChange;
	HyMsgViewLayerChange viewLayerChange;
};

/**
 * Graphics context descriptor.
 */
struct HyGraphicsContextDesc
{
	HyGraphicsAPI m_graphicsAPI;		/**< Graphics API. @see HyGraphicsAPI. */
	void *m_graphicsDevice;				/**< Native graphics device. */
	HyTextureFormat m_pixelFormat;		/**< Pixel format of textures. @see HyTextureFormat. */
	float m_pixelDensity;				/**< Pixel density. */
	uint32_t m_mirrorWidth;				/**< Width of the mirror/distorted texture. */
	uint32_t m_mirrorHeight;			/**< Height of the mirror/distorted texture. */
	uint32_t m_flags;					/**< Reserved. */
};


class HyGraphicsContext;
class HyDevice;

/**
 * Loading Helper helps applications display their loading progress
 *
 * It is helpful when application itself is busy with reading data and not rendering for a long period of time.
 */
class HyLoadingHelper
{
public:
	/**
	 * Initialize the helper with graphics capability.
	 *
	 * @param context	[in]  an instance of HyGraphicsContext, created from HyDevice
	 *
	 * You should call this function before using any of the following functionality.
	 */
	virtual HyResult Init(HyGraphicsContext *context) = 0;

	/**
	 * Release this helper, when application is done with loading. Application should not keep a reference after calling release.
	 */
	virtual void Release() = 0;

	/**
	 * Set progress percentage of loading
	 *
	 * @param percent	[in]  percentage of loading progress
	 */
	virtual HyResult SetProgress(int percent) = 0;

	/**
	 * Set texture for loading screen
	 *
	 * @param desc		[in]  descriptor of texture to be displayed in loading screen
	 *
	 * @see HyTextureDesc
	 */
	virtual HyResult SetTexture(const HyTextureDesc &desc) = 0;

};

static const char* HyLoadingHelper_InterfaceName = "HyLoadingHelper_2";


/**
 * Input Helper helps application get input from virtual keyboard
 *
 * It is helpful when application want a private input which will not display in mirror pass.
 *
 */
class HyInputHelper
{
public:
	/**
	* Initialize the helper with graphics capability.
	*
	* @param device		[in]  an instance of HyDevice
	*
	* @return			An error will occur if a helper is already initialized.
	*
	* You should call this function before using any of the following functionality.
	*/
	virtual HyResult Init(HyDevice *device) = 0;

	/**
	* Release this helper, when application get a succeed return. Application should not keep a reference after calling release.
	*/
	virtual void Release() = 0;

	/**
	* Set default string to virtual keyboard
	*
	* @param str		[in]	default string
	* @param bufSize	[in]	buffer size of str
	*/
	virtual HyResult SetDefaultString(wchar_t* str) = 0;

	/**
	* Get string input form virtual keyboard
	*
	* @param str		[out]	string which user input with the virtual keyboard
	* @param bufSize	[in]	number of wchars
	* @param realSize	[out, optional]	the real buffer size used/required for the string
	* @param pwdMode	[in, optional]	to indicate if it is in password mode
	*
	* @return			After user confirms, the return value will be hySuccess and str is filled.
	*					hyError_BufferTooSmall if there is not enough space to store the result;
	*					and if realSize is not null, it will contain the buffer size needed.
	*					And hySuccess_InputNotReady is returned, if user is still inputing.
	*					
	*/
	virtual HyResult GetString(wchar_t* str, int size, int* realSize = 0, bool pwdMode = false) = 0;
};

static const char* HyInputHelper_InterfaceName = "HyInputHelper_2";


/**
 * View Layer helps to display a 2D image over 3D scene
 */
class HyViewLayer
{
public:
	/**
	 * Initialize the helper with graphics capability.
	*
	* @param context	[in]  an instance of HyGraphicsContext, created from HyDevice
	*
	* You should call this function before using any of the following functionality.
	*/
	virtual HyResult Init(HyGraphicsContext *context) = 0;

	/**
	 * Release this layer. Application should not keep a reference after calling release.
	 */
	virtual void Release() = 0;

	/**
	 * Set position & rotation of the layer
	 *
	 * @param pose		[in]  layer pose
	 *
	 * @see HyPose
	 */
	virtual HyResult SetPose(HyPose& pose) = 0;

	/**
	 * Set size of the layer
	 *
	 * @param size		[in]  layer size
	 *
	 * @see HyVec2
	 */
	virtual HyResult SetSize(HyVec2& size) = 0;

	/**
	 * Set priority of the layer. Layer with higher priority will be displayed above other layers.
	 *
	 * @param priority	[in]  layer priority
	 */
	virtual HyResult SetPriority(int priority) = 0;

	/**
	 * Set flag to indicate special type of the layer
	 *
	 * @param flag		[in]  see HyViewLayerFlags for possible values; 0 (default) for normal layers
	 *
	 * @see HyViewLayerFlags
	 */
	virtual HyResult SetFlags(int flag) = 0;

	/**
	 * Set image for the layer
	 *
	 * @param desc		[in]  descriptor of texture of the layer image
	 *
	 * @see HyTextureDesc
	 */
	virtual HyResult SetTexture(const HyTextureDesc &desc) = 0;

};

static const char* HyViewLayer_InterfaceName = "HyViewLayer_2";


/**
 * The graphics context is the abstract of Hypereal VR rendering solution.
 *
 * A graphics context provides rendering functionality for a device.
 * It's used to query eye poses, render targets' size and projection matrix.
 * It also submits render target textures, and copies mirror/distorted texture back.
 */
class HyGraphicsContext
{
public:
	/**
	 * Release the graphics context. An application should never keep a reference to this graphics context after calling release.
	 */
	virtual void Release() = 0;

	/**
	 * Get eye poses based on head pose and interpupillary distance.
	 *
	 * @param headPose					[in]  head pose.
	 * @param interpupillaryDistance	[in]  interpupillary distance, passing nullptr means using default interpupillary distance.
	 * @param eyePoses					[out] poses of two eyes.
	 *
	 * @see								HyPose.
	 */
	virtual void GetEyePoses(const HyPose &headPose, const float *interpupillaryDistance, HyPose eyePoses[HY_EYE_MAX]) = 0;

	/**
	 * Get projection matrix for left/right hand system based on FOV, near and far clipping planes.
	 *
	 * @param fov						[in]  FOV (field of view).
	 * @param near						[in]  z value of near clipping plane.
	 * @param far						[in]  z value of far clipping plane.
	 * @param rightHandSys				[in]  use right hand system or not.
	 * @param projMatrix				[out] projection matrix.
	 *
	 * @see								HyFov, HyMat4.
	 */
	virtual void GetProjectionMatrix(const HyFov &fov, float near, float far, bool rightHandSys, HyMat4 &projMatrix) = 0;

	/**
	 * Get render target's size of a specified eye.
	 *
	 * @param eye						[in]  target eye.
	 * @param width						[out] width of the render target.
	 * @param height					[out] height of the render target.
	 *
	 * @see								HyEye.
	 */
	virtual void GetRenderTargetSize(HyEye eye, uint32_t &width, uint32_t &height) = 0;

	/**
	 * Copy a region of mirror/distorted texture to a destination texture.
	 *
	 * @param dstTexture				[in]  texture handle of the destination texture.
	 * @param dstWidth					[in]  width of the destination texture.
	 * @param dstHeight					[in]  height of the destination texture.
	 * @param srcViewport				[in]  region of the mirror/distorted texture to copy, passing nullptr means copy the entire region.
	 *
	 * @see								HyViewport.
	 */
	virtual void CopyMirrorTexture(void *dstTexture, uint32_t dstWidth, uint32_t dstHeight, const HyViewport *srcViewport = nullptr) = 0;

	/**
	 * Submit render target textures for distortion.
	 *
	 * @param frame						[in]  frame number of the targeted application, 0 is acceptable.
	 * @param textures					[in]  render target textures.
	 * @param nTextures					[in]  number of render target textures.
	 * @param viewports					[in]  regions of the render target textures, passing nullptr means the entire regions.
	 * @param fov						[in]  FOVs of eyes, passing nullptr means using default FOVs.
	 * @return							check the return value with macro of hySucceeded(), if the macro is true then succeeded, otherwise failed.
	 *
	 * @see								HyTextureDesc, HyViewport, HyFov, HyResult.
	 */
	virtual HyResult Submit(int64_t frame, const HyTextureDesc *textures, uint32_t nTextures, const HyViewport *viewports = nullptr, const HyFov *fov = nullptr) = 0;
};

/**
 * The device is the abstract of Hypereal VR hardwares.
 *
 * A device communicates with Hypereal hardwares.
 * It's used to query hardware information, runtime status, tracking and input state.
 * It also creates graphics context which performs rendering.
 */
class HyDevice
{
public:
	/**
	 * Release the device. An application should never keep a reference to this device after calling release.
	 */
	virtual void Release() = 0;

	/**
	 * Create a graphics context. A device can create only one graphics context.
	 * If an application needs to change graphics context, the existed one must be released first, and then create a new one.
	 *
	 * @param desc						[in]  description of the graphics context.
	 * @param vrGraphicsContext			[out] address of a pointer to the created graphics context.
	 * @return							check the return value with macro of hySucceeded(), if the macro is true then succeeded, otherwise failed.
	 *
	 * @see								HyGraphicsContextDesc, HyGraphicsContext, HyResult.
	 */
	virtual HyResult CreateGraphicsContext(const HyGraphicsContextDesc &desc, HyGraphicsContext **vrGraphicsContext) = 0;

	/**
	 * Retrieve message from system
	 *
	 * @param msg						[out] const pointer to a msg, which can be cast to a more specific type. The pointer is valid until next call to RetrieveMsg or DefaultMsgFunction.
	 * @return							check the return value with macro of hySucceeded(), if the macro is true then succeeded, otherwise failed.
	 *
	 * @see								HyMsgType, HyMsgHeader, HyResult.
	 */
	virtual HyResult RetrieveMsg(const HyMsgHeader** msg) = 0;

	/**
	 * Default message handling function.
	 *
	 * App should call this function when it don't know how to handle a message, or want to use the default behavior.
	 * @param msg						[in] const pointer to the msg.
	 * @return							check the return value with macro of hySucceeded(), if the macro is true then succeeded, otherwise failed.
	 *
	 * @see								HyMsgType, HyMsgHeader, HyResult.
	 */
	virtual HyResult DefaultMsgFunction(const HyMsgHeader* msg) = 0;

	/**
	 * Get the tracking state of a specified sub-device.
	 *
	 * @param subDev					[in]  specify a sub-device, only valid for a device of HY_SUBDEV_HMD or HY_SUBDEV_CONTROLLER.
	 * @param frame						[in]  frame number of the targeted application, 0 is acceptable.
	 * @param state						[out] tracking state of the specified sub-device.
	 * @param predictionTimeByDefault   [in]  specify a default prediction time if can't retrieve from GPU or HyGraphicsContext not in use.
	 * @return							check the return value with macro of hySucceeded(), if the macro is true then succeeded, otherwise failed.
	 *
	 * @see								HySubDevice, HyTrackingState, HyResult.
	 */
	virtual HyResult GetTrackingState(HySubDevice subDev, int64_t frame, HyTrackingState &state, double predictionTimeByDefault = 16.6) = 0;

	/**
	 * Get the input state of a specified controller.
	 *
	 * @param subDev					[in]  specify a controller, only valid for a device of HY_SUBDEV_CONTROLLER.
	 * @param state						[out] input state of the specified controller.
	 * @return							check the return value with macro of hySucceeded(), if the macro is true then succeeded, otherwise failed.
	 *
	 * @see								HySubDevice, HyInputState, HyResult.
	 */
	virtual HyResult GetControllerInputState(HySubDevice subDev, HyInputState &state) = 0;

	/**
	 * Set vibration for a specified controller.
	 *
	 * @param subDev					[in]  specify a controller, only valid for a device of HY_SUBDEV_CONTROLLER.
	 * @param duration					[in]  duration of the vibration, in millisecond.
	 * @param amplitude					[in]  amplitude of the vibration, [0.0f, 1.0f].
	 * @return							check the return value with macro of hySucceeded(), if the macro is true then succeeded, otherwise failed.
	 *
	 * @see								HySubDevice, HyResult.
	 */
	virtual HyResult SetControllerVibration(HySubDevice subDev, float duration, float amplitude) = 0;

	/**
	 * Get the pose of a specified tracker.
	 *
	 * @param subDev					[in]  specify a tracker, only valid for a device of HY_SUBDEV_TRACKER.
	 * @param pose						[out] pose of the specified tracker.
	 * @return							check the return value with macro of hySucceeded(), if the macro is true then succeeded, otherwise failed.
	 *
	 * @see								HySubDevice, HyPose, HyResult.
	 */
	virtual HyResult GetTrackerPose(HySubDevice subDev, HyPose &pose) = 0;

	/**
	 * Set or get the tracking origin type of the device.
	 *
	 * @param origin					[in]  target tracking origin type.
	 * @return							current tracking origin type if passing HY_TRACKING_ORIGIN_UNKNOWN, previous tracking origin type otherwise.
	 *
	 * @see								HyTrackingOrigin.
	 */
	virtual HyTrackingOrigin ConfigureTrackingOrigin(HyTrackingOrigin origin) = 0;

	/**
	* Get bool property from device
	 *
	 * @param property					[in]  id of property to be read.
	 * @param value						[out] value of the property.
	 * @param param						[in]  extra parameter for indicating the usage of this property.
	 *									1. if this property is a sub device specified one, then use the certain Sub Device as parameter.
	 *									2. if this property is a Layer specified one, then use the layer id as parameter.
	 * @return							check the return value with macro of hySucceeded(), if the macro is true then succeeded, otherwise failed.
	*/
	virtual HyResult GetBoolValue(int property, bool &value, unsigned int param = 0xffffffff) = 0;

	/**
	* Set bool property to device
	 *
	 * @param property					[in]  id of property to be set.
	 * @param value						[in]  new value of the property.
	 * @param param						[in]  extra parameter for indicating the usage of this property.
	 *									1. if this property is a sub device specified one, then use the certain Sub Device as parameter.
	 *									2. if this property is a Layer specified one, then use the layer id as parameter.
	 * @return							check the return value with macro of hySucceeded(), if the macro is true then succeeded, otherwise failed.
	*/
	virtual HyResult SetBoolValue(int property, bool value, unsigned int param = 0xffffffff) = 0;

	/**
	* Get float property from device
	 *
	 * @param property					[in]  id of property to be read.
	 * @param value						[out] value of the property.
	 * @param param						[in]  extra parameter for indicating the usage of this property.
	 *									1. if this property is a sub device specified one, then use the certain Sub Device as parameter.
	 *									2. if this property is a Layer specified one, then use the layer id as parameter.
	 * @return							check the return value with macro of hySucceeded(), if the macro is true then succeeded, otherwise failed.
	*/
	virtual HyResult GetFloatValue(int property, float &value, unsigned int param = 0xffffffff) = 0;

	/**
	* Set float property to device
	 *
	 * @param property					[in]  id of property to be set.
	 * @param value						[in]  new value of the property.
	 * @param param						[in]  extra parameter for indicating the usage of this property.
	 *									1. if this property is a sub device specified one, then use the certain Sub Device as parameter.
	 *									2. if this property is a Layer specified one, then use the layer id as parameter.
	 * @return							check the return value with macro of hySucceeded(), if the macro is true then succeeded, otherwise failed.
	*/
	virtual HyResult SetFloatValue(int property, float value, unsigned int param = 0xffffffff) = 0;

	/**
	* Get int property from device
	 *
	 * @param property					[in]  id of property to be read.
	 * @param value						[out] value of the property.
	 * @param param						[in]  extra parameter for indicating the usage of this property.
	 *									1. if this property is a sub device specified one, then use the certain Sub Device as parameter.
	 *									2. if this property is a Layer specified one, then use the layer id as parameter.
	 * @return							check the return value with macro of hySucceeded(), if the macro is true then succeeded, otherwise failed.
	*/
	virtual HyResult GetIntValue(int property, int64_t &value, unsigned int param = 0xffffffff) = 0;

	/**
	* Set int property to device
	 *
	 * @param property					[in]  id of property to be set.
	 * @param value						[in]  new value of the property.
	 * @param param						[in]  extra parameter for indicating the usage of this property.
	 *									1. if this property is a sub device specified one, then use the certain Sub Device as parameter.
	 *									2. if this property is a Layer specified one, then use the layer id as parameter.
	 * @return							check the return value with macro of hySucceeded(), if the macro is true then succeeded, otherwise failed.
	*/
	virtual HyResult SetIntValue(int property, int64_t value, unsigned int param = 0xffffffff) = 0;

	/**
	* Get float array property from device
	 *
	 * @param property					[in]  id of property to be read.
	 * @param array						[out] value array of the property.
	 * @param size						[in]  number elements of the array.
	 * @param param						[in]  extra parameter for indicating the usage of this property.
	 *									1. if this property is a sub device specified one, then use the certain Sub Device as parameter.
	 *									2. if this property is a Layer specified one, then use the layer id as parameter.
	 * @return							check the return value with macro of hySucceeded(), if the macro is true then succeeded, otherwise failed.
	*/
	virtual HyResult GetFloatArray(int property, float* array, int size, unsigned int param = 0xffffffff) = 0;

	/**
	* Set float array property to device
	 *
	 * @param property					[in]  id of property to be set.
	 * @param value						[in]  new value array of the property.
	 * @param size						[in]  number elements of the array
	 * @param param						[in]  extra parameter for indicating the usage of this property.
	 *									1. if this property is a sub device specified one, then use the certain Sub Device as parameter.
	 *									2. if this property is a Layer specified one, then use the layer id as parameter.
	 * @return							check the return value with macro of hySucceeded(), if the macro is true then succeeded, otherwise failed.
	*/
	virtual HyResult SetFloatArray(int property, const float* array, int size, unsigned int param = 0xffffffff) = 0;

	/**
	* Get string property from device
	*
	* @param property					[in]  id of property to be read.
	* @param string						[out] value of the property.
	* @param bufSize					[in]  number chars of the string buffer
	* @param realSize					[out, optional] size of string buffer to hold the result,
	*										including string terminal char
	* @param param						[in]  extra parameter for indicating the usage of this property.
	*									1. if this property is a sub device specified one, then use the certain Sub Device as parameter.
	*									2. if this property is a Layer specified one, then use the layer id as parameter.
	* @return							check the return value with macro of hySucceeded(), if the macro is true then succeeded, otherwise failed.
	*/
	virtual HyResult GetStringValue(int property, char* string, int bufSize, int* realSize=0, unsigned int param = 0xffffffff) = 0;

	/**
	* Set string property to device
	*
	* @param property					[in]  id of property to be set.
	* @param value						[in]  new value of the property.
	* @param param						[in]  extra parameter for indicating the usage of this property.
	*									1. if this property is a sub device specified one, then use the certain Sub Device as parameter.
	*									2. if this property is a Layer specified one, then use the layer id as parameter.
	* @return							check the return value with macro of hySucceeded(), if the macro is true then succeeded, otherwise failed.
	*/
	virtual HyResult SetStringValue(int property, const char* string, unsigned int param = 0xffffffff) = 0;

};

static const char* HyDevice_InterfaceName = "HyDevice_3";


/**
 * Start up Hypereal VR SDK.
 *
 * @param flag			[in] Reserved, It must be 0.
 * @return				check the return value with macro of hySucceeded(), if the macro is true then succeeded, otherwise failed.
 *
 * @see					HyResult.
 */
HY_API HyResult HY_CALLTYPE HyStartup(unsigned int flag = 0);

/**
 * Shut down Hypereal VR SDK.
 *
 * @return				check the return value with macro of hySucceeded(), if the macro is true then succeeded, otherwise failed.
 *
 * @see					HyResult.
 */
HY_API HyResult HY_CALLTYPE HyShutdown();

/**
* Create an instance of specific interface.
* An application must have at least one device, most of the time it only needs one.
*
* @param interface_version		[in]  version of specific interface.
* @param flag					[in]  reserved parameter, must be 0.
* @param ppInterface			[out] address of a pointer to the created instance.
* @return				hySuccess if successful, otherwise failed.
*
* @see					HyDevice, HyResult.
*/
HY_API HyResult HY_CALLTYPE HyCreateInterface(const char* interfaceName, int flag, void** ppInterface);

#pragma pack(pop)

#endif

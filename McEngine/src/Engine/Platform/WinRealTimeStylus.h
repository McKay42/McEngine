#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

#ifndef WINREALTIMESTYLUS_H
#define WINREALTIMESTYLUS_H

#ifdef MCENGINE_WINDOWS_REALTIMESTYLUS_SUPPORT

#include "Engine.h"
#include "ConVar.h"

extern Engine *g_engine;
extern ConVar *win_realtimestylus;

// this is really hacky, I copied and merged some header files from the Microsoft SDK into MinGW to make it work
// MinGW is missing some definitions inside tpcshrd.h, you can get them from the Microsoft SDK includes
// you MUST link to Ole32.lib and Uuid.lib (which is not in the default project configuration!)

// most of this code was taken 1:1 from the windows sdk samples
// the only interesting/useful functions here are StylusDown and StylusUp

#include <rtscom.h> // RTS interface and GUID declarations
#include <rtscom_i.c> // RTS GUID definitions

// yay for missing uuid declarations, had to add this one manually
__CRT_UUID_DECL(IRealTimeStylus, 0xA8BB5D22, 0x3144, 0x4a7b, 0x93, 0xCD, 0xF3, 0x4A, 0x16, 0xBE, 0x51, 0x3A);

IRealTimeStylus* g_pRealTimeStylus = NULL; // RTS object
IStylusSyncPlugin* g_pSyncEventHandlerRTS = NULL; // EventHandler object



///////////////////////////////////////////////////////////////////////////////
// RealTimeStylus utilities

// Creates the RealTimeStylus object, attaches it to the window, and
// enables it for multi-touch.
// in:
//      hWnd        handle to device window
// returns:
//      RTS object through IRealTimeStylus interface, or NULL on failure
IRealTimeStylus* CreateRealTimeStylus(HWND hWnd)
{
    // Check input argument
    if (hWnd == NULL)
    {
        printf("CreateRealTimeStylus: invalid argument hWnd");
        return NULL;
    }

    // Create RTS object
    IRealTimeStylus* pRealTimeStylus = NULL;
    HRESULT hr = CoCreateInstance(CLSID_RealTimeStylus, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pRealTimeStylus));
    if (FAILED(hr))
    {
        printf("CreateRealTimeStylus: failed to CoCreateInstance of RealTimeStylus");
        return NULL;
    }

    // Attach RTS object to a window
    hr = pRealTimeStylus->put_HWND((HANDLE_PTR)hWnd);
    if (FAILED(hr))
    {
        printf("CreateRealTimeStylus: failed to set window handle");
        pRealTimeStylus->Release();
        return NULL;
    }

    return pRealTimeStylus;
}

// Turns on RTS and DynamicRenderer object.
// in:
//      pRealTimeStylus         RTS object
//      pDynamicRenderer        DynamicRenderer object
// returns:
//      flag, is initialization successful
bool EnableRealTimeStylus(IRealTimeStylus* pRealTimeStylus)
{
    // Check input arguments
    if (pRealTimeStylus == NULL)
    {
        printf("EnableRealTimeStylus: invalid argument RealTimeStylus");
        return NULL;
    }

    // Enable RTS object
    HRESULT hr = pRealTimeStylus->put_Enabled(TRUE);
    if (FAILED(hr))
    {
        printf("EnableRealTimeStylus: failed to enable RealTimeStylus");
        return false;
    }

    return true;
}

void DisableRealTimeStylus(IRealTimeStylus* pRealTimeStylus)
{
    // Check input arguments
    if (pRealTimeStylus == NULL)
    {
        printf("DisableRealTimeStylus: invalid argument RealTimeStylus");
        return;
    }

    // Enable RTS object
    HRESULT hr = pRealTimeStylus->put_Enabled(FALSE);
    if (FAILED(hr))
        printf("DisableRealTimeStylus: failed to disable RealTimeStylus");
}



///////////////////////////////////////////////////////////////////////////////
// Real Time Stylus sync event handler

// Synchronous plugin, notitification receiver that changes pen color.
class CSyncEventHandlerRTS : public IStylusSyncPlugin
{
    CSyncEventHandlerRTS();
    virtual ~CSyncEventHandlerRTS();

    public:
    // Factory method
    static IStylusSyncPlugin* Create(IRealTimeStylus* pRealTimeStylus);

    // IStylusSyncPlugin methods

    // Handled IStylusSyncPlugin methods, they require nontrivial implementation
    STDMETHOD(StylusDown)(IRealTimeStylus* piSrcRtp, const StylusInfo* pStylusInfo, ULONG cPropCountPerPkt, LONG* pPacket, LONG** ppInOutPkt);
    STDMETHOD(StylusUp)(IRealTimeStylus* piSrcRtp, const StylusInfo* pStylusInfo, ULONG cPropCountPerPkt, LONG* pPacket, LONG** ppInOutPkt);
    STDMETHOD(Packets)(IRealTimeStylus* piSrcRtp, const StylusInfo* pStylusInfo, ULONG cPktCount, ULONG cPktBuffLength, LONG* pPackets, ULONG* pcInOutPkts, LONG** ppInOutPkts);
    STDMETHOD(DataInterest)(RealTimeStylusDataInterest* pEventInterest);

    // IStylusSyncPlugin methods with trivial inline implementation, they all return S_OK
    STDMETHOD(RealTimeStylusEnabled)(IRealTimeStylus*, ULONG, const TABLET_CONTEXT_ID*) { return S_OK; }
    STDMETHOD(RealTimeStylusDisabled)(IRealTimeStylus*, ULONG, const TABLET_CONTEXT_ID*) { return S_OK; }
    STDMETHOD(StylusInRange)(IRealTimeStylus*, TABLET_CONTEXT_ID, STYLUS_ID) { return S_OK; }
    STDMETHOD(StylusOutOfRange)(IRealTimeStylus*, TABLET_CONTEXT_ID, STYLUS_ID) { return S_OK; }
    STDMETHOD(InAirPackets)(IRealTimeStylus*, const StylusInfo*, ULONG, ULONG, LONG*, ULONG*, LONG**) { return S_OK; }
    STDMETHOD(StylusButtonUp)(IRealTimeStylus*, STYLUS_ID, const GUID*, POINT*) { return S_OK; }
    STDMETHOD(StylusButtonDown)(IRealTimeStylus*, STYLUS_ID, const GUID*, POINT*) { return S_OK; }
    STDMETHOD(SystemEvent)(IRealTimeStylus*, TABLET_CONTEXT_ID, STYLUS_ID, SYSTEM_EVENT, SYSTEM_EVENT_DATA) { return S_OK; }
    STDMETHOD(TabletAdded)(IRealTimeStylus*, IInkTablet*) { return S_OK; }
    STDMETHOD(TabletRemoved)(IRealTimeStylus*, LONG) { return S_OK; }
    STDMETHOD(CustomStylusDataAdded)(IRealTimeStylus*, const GUID*, ULONG, const BYTE*) { return S_OK; }
    STDMETHOD(Error)(IRealTimeStylus*, IStylusPlugin*, RealTimeStylusDataInterest, HRESULT, LONG_PTR*) { return S_OK; }
    STDMETHOD(UpdateMapping)(IRealTimeStylus*) { return S_OK; }

    // IUnknown methods
    STDMETHOD_(ULONG,AddRef)();
    STDMETHOD_(ULONG,Release)();
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppvObj);

private:
    LONG m_cRefCount;                   // COM object reference count
    IUnknown* m_punkFTMarshaller;       // free-threaded marshaller
    int m_nContacts;                    // number of fingers currently in the contact with the touch digitizer
};

// CSyncEventHandlerRTS constructor.
CSyncEventHandlerRTS::CSyncEventHandlerRTS()
:   m_cRefCount(1),
    m_punkFTMarshaller(NULL),
    m_nContacts(0)
{
}

// CSyncEventHandlerRTS destructor.
CSyncEventHandlerRTS::~CSyncEventHandlerRTS()
{
    if (m_punkFTMarshaller != NULL)
    {
        m_punkFTMarshaller->Release();
    }
}

// CSyncEventHandlerRTS factory method: creates new CSyncEventHandlerRTS and adds it to the synchronous
// plugin list of the RTS object.
// in:
//      pRealTimeStylus         RTS object
// returns:
//      CSyncEventHandlerRTS object through IStylusSyncPlugin interface, or NULL on failure
IStylusSyncPlugin* CSyncEventHandlerRTS::Create(IRealTimeStylus* pRealTimeStylus)
{
    // Check input argument
    if (pRealTimeStylus == NULL)
    {
        printf("CSyncEventHandlerRTS::Create: invalid argument RealTimeStylus");
        return NULL;
    }

    // Instantiate CSyncEventHandlerRTS object
    CSyncEventHandlerRTS* pSyncEventHandlerRTS = new CSyncEventHandlerRTS();
    if (pSyncEventHandlerRTS == NULL)
    {
        printf("CSyncEventHandlerRTS::Create: cannot create instance of CSyncEventHandlerRTS");
        return NULL;
    }

    // Create free-threaded marshaller for this object and aggregate it.
    HRESULT hr = CoCreateFreeThreadedMarshaler(pSyncEventHandlerRTS, &pSyncEventHandlerRTS->m_punkFTMarshaller);
    if (FAILED(hr))
    {
        printf("CSyncEventHandlerRTS::Create: cannot create free-threaded marshaller");
        pSyncEventHandlerRTS->Release();
        return NULL;
    }

    // Add CSyncEventHandlerRTS object to the list of synchronous plugins in the RTS object.
    hr = pRealTimeStylus->AddStylusSyncPlugin(
        0,                      // insert plugin at position 0 in the sync plugin list
        pSyncEventHandlerRTS);  // plugin to be inserted - event handler CSyncEventHandlerRTS
    if (FAILED(hr))
    {
        printf("CEventHandlerRTS::Create: failed to add CSyncEventHandlerRTS to the RealTimeStylus plugins");
        pSyncEventHandlerRTS->Release();
        return NULL;
    }

    return pSyncEventHandlerRTS;
}

// Pen-down notification.
// Sets the color for the newly started stroke and increments finger-down counter.
// in:
//      piRtsSrc            RTS object that has sent this event
//      pStylusInfo         StylusInfo struct (context ID, cursor ID, etc)
//      cPropCountPerPkt    number of properties per packet
//      pPacket             packet data (layout depends on packet description set)
// in/out:
//      ppInOutPkt          modified packet data (same layout as pPackets)
// returns:
//      HRESULT error code
HRESULT CSyncEventHandlerRTS::StylusDown(
    IRealTimeStylus* /* piRtsSrc */,
    const StylusInfo* /* pStylusInfo */,
    ULONG /* cPropCountPerPkt */,
    LONG* /* pPacket */,
    LONG** /* ppInOutPkt */)
{
	if (win_realtimestylus->getBool())
		g_engine->onMouseLeftChange(true);

    return S_OK;
}

// Pen-up notification.
// Decrements finger-down counter.
// in:
//      piRtsSrc            RTS object that has sent this event
//      pStylusInfo         StylusInfo struct (context ID, cursor ID, etc)
//      cPropCountPerPkt    number of properties per packet
//      pPacket             packet data (layout depends on packet description set)
// in/out:
//      ppInOutPkt          modified packet data (same layout as pPackets)
// returns:
//      HRESULT error code
HRESULT CSyncEventHandlerRTS::StylusUp(
    IRealTimeStylus* /* piRtsSrc */,
    const StylusInfo* /* pStylusInfo */,
    ULONG /* cPropCountPerPkt */,
    LONG* /* pPacket */,
    LONG** /* ppInOutPkt */)
{
	if (win_realtimestylus->getBool())
		g_engine->onMouseLeftChange(false);

    return S_OK;
}

// Pen-move notification.
// In this case, does nothing, but likely to be used in a more complex application.
// RTS framework does stroke collection and rendering for us.
// in:
//      piRtsRtp            RTS object that has sent this event
//      pStylusInfo         StylusInfo struct (context ID, cursor ID, etc)
//      cPktCount           number of packets
//      cPktBuffLength      pPacket buffer size, in elements, equal to number of packets times number of properties per packet
//      pPackets            packet data (layout depends on packet description set)
// in/out:
//      pcInOutPkts         modified number of packets
//      ppInOutPkts         modified packet data (same layout as pPackets)
// returns:
//      HRESULT error code
HRESULT CSyncEventHandlerRTS::Packets(
    IRealTimeStylus* /* piSrcRtp */,
    const StylusInfo* /* pStylusInfo */,
    ULONG /* cPktCount */,
    ULONG /* cPktBuffLength */,
    LONG* /* pPackets */,
    ULONG* /* pcInOutPkts */,
    LONG** /* ppInOutPkts */)
{
    return S_OK;
}

// Defines which handlers are called by the framework. We set the flags for pen-down, pen-up and pen-move.
// in/out:
//      pDataInterest       flags that enable/disable notification handlers
// returns:
//      HRESULT error code
HRESULT CSyncEventHandlerRTS::DataInterest(RealTimeStylusDataInterest *pDataInterest)
{
    *pDataInterest = (RealTimeStylusDataInterest)(RTSDI_StylusDown | RTSDI_StylusUp);

    return S_OK;
}

// Increments reference count of the COM object.
// returns:
//      reference count
ULONG CSyncEventHandlerRTS::AddRef()
{
    return InterlockedIncrement(&m_cRefCount);
}

// Decrements reference count of the COM object, and deletes it
// if there are no more references left.
// returns:
//      reference count
ULONG CSyncEventHandlerRTS::Release()
{
    ULONG cNewRefCount = InterlockedDecrement(&m_cRefCount);
    if (cNewRefCount == 0)
    {
        delete this;
    }
    return cNewRefCount;
}

// Returns a pointer to any interface supported by this object.
// If IID_IMarshal interface is requested, delegate the call to the aggregated
// free-threaded marshaller.
// If a valid pointer is returned, COM object reference count is increased.
// returns:
//      pointer to the interface requested, or NULL if the interface is not supported by this object
HRESULT CSyncEventHandlerRTS::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    if ((riid == IID_IStylusSyncPlugin) || (riid == IID_IUnknown))
    {
        *ppvObj = this;
        AddRef();
        return S_OK;
    }
    else if ((riid == IID_IMarshal) && (m_punkFTMarshaller != NULL))
    {
        return m_punkFTMarshaller->QueryInterface(riid, ppvObj);
    }

    *ppvObj = NULL;
    return E_NOINTERFACE;
}



// Saves instance handle and creates main window
// This function is generated by Visual Studio app wizard; added RTS object
// construction and initialization.
// In this function, we save the instance handle in a global variable and
// create and display the main program window.
// in:
//      hInstance       handle to the instance of the application
//      nCmdShow        how to show the window
// returns:
//      flag, succeeded or failed to create the window
BOOL InitRealTimeStylus(HINSTANCE hInstance, HWND hWnd)
{
    // Create RTS object
    g_pRealTimeStylus = CreateRealTimeStylus(hWnd);
    if (g_pRealTimeStylus == NULL)
    {
        return FALSE;
    }

    // Create EventHandler object
    g_pSyncEventHandlerRTS = CSyncEventHandlerRTS::Create(g_pRealTimeStylus);
    if (g_pSyncEventHandlerRTS == NULL)
    {
        g_pRealTimeStylus->Release();
        g_pRealTimeStylus = NULL;

        return FALSE;
    }

    // Enable RTS and DynamicRenderer
    if (!EnableRealTimeStylus(g_pRealTimeStylus))
    {
        g_pSyncEventHandlerRTS->Release();
        g_pSyncEventHandlerRTS = NULL;

        g_pRealTimeStylus->Release();
        g_pRealTimeStylus = NULL;

        return FALSE;
    }

    return TRUE;
}

void UninitRealTimeStylus()
{
	// reverse of the init function

	if (g_pRealTimeStylus != NULL)
		DisableRealTimeStylus(g_pRealTimeStylus);

	if (g_pSyncEventHandlerRTS != NULL)
	{
		g_pSyncEventHandlerRTS->Release();
		g_pSyncEventHandlerRTS = NULL;
	}

	if (g_pRealTimeStylus != NULL)
	{
		g_pRealTimeStylus->Release();
		g_pRealTimeStylus = NULL;
	}
}

#endif

#endif

#endif

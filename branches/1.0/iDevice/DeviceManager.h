// DeviceManager.h : Declaration of the CDeviceManager

#pragma once
#include "resource.h"       // main symbols
#include "iDevice_i.h"
#include "AMDevice.h"
#include "Device.h"

using namespace ATL;

struct CAutoLock
{
    CComCriticalSection& m_cs;
    CAutoLock(CComCriticalSection& cs) : m_cs(cs)
    {
        m_cs.Lock();
    }

    ~CAutoLock()
    {
        m_cs.Unlock();
    }
};


// CDeviceManager

class ATL_NO_VTABLE CDeviceManager :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CDeviceManager, &CLSID_DeviceManager>,
    public ISupportErrorInfo,
    public IDispatchImpl<IDeviceManager, &IID_IDeviceManager, &LIBID_iDeviceLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
    CDeviceManager();

DECLARE_REGISTRY_RESOURCEID(IDR_DEVICEMANAGER)
DECLARE_CLASSFACTORY_SINGLETON(CDeviceManager)

BEGIN_COM_MAP(CDeviceManager)
    COM_INTERFACE_ENTRY(IDeviceManager)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

// ISupportsErrorInfo
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);


    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct();
    void FinalRelease();

public:
    STDMETHOD(get_Devices)(IEnumDevices** pVal);

private:
    HRESULT LoadAppleDLLs();
    static void DeviceCallback(am_device_notification_callback_info*, void* callback_data);
    void OnDeviceAdded(AMDeviceRef device);
    void OnDeviceRemoved(AMDeviceRef device);

private:
    HMODULE m_hiTunesDLL;
    HMODULE m_hCFDLL;
    AMDeviceNotificationRef m_notification;
    std::wstring m_appSupportPath;
    CComCriticalSection m_cs;

    typedef std::map<AMDeviceRef, CDevicePtr> Devices;
    Devices m_devices;
    friend class CEnumDevices;
};

OBJECT_ENTRY_AUTO(__uuidof(DeviceManager), CDeviceManager)

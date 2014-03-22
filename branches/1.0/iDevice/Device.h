// Device.h : Declaration of the CDevice

#pragma once
#include "resource.h"       // main symbols
#include "iDevice_i.h"

using namespace ATL;

// CDevice

class ATL_NO_VTABLE CDevice :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CDevice>,
    public ISupportErrorInfo,
    public IDispatchImpl<IDevice, &IID_IDevice, &LIBID_iDeviceLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
    CDevice();

BEGIN_COM_MAP(CDevice)
    COM_INTERFACE_ENTRY(IDevice)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

public:
    HRESULT FinalConstruct()
    {
        return S_OK;
    }

    void FinalRelease()
    {
    }

    HRESULT Init(AMDeviceRef handle);
    void Term();

// ISupportsErrorInfo
public:
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

public:
    STDMETHOD(get_Name)(BSTR* pVal);
    STDMETHOD(get_Id)(GUID* pVal);
    STDMETHOD(get_IsValid)(VARIANT_BOOL* pVal);

    STDMETHOD(ListFiles)(BSTR path, SAFEARRAY * files);
    STDMETHOD(CreateDirectory)(BSTR path);
    STDMETHOD(DeleteDirectory)(BSTR path);
    STDMETHOD(GetFile)(BSTR fromPath, BSTR toPath, IProgressCallback* cb);
    STDMETHOD(PutFile)(BSTR fromPath, BSTR toPath, IProgressCallback* cb);
    STDMETHOD(DeleteFile)(BSTR path);
    STDMETHOD(GetDeviceInfo)(DeviceInfo* info);

private:
    HRESULT Connect();
    void Disconnect();
    HRESULT GetFileSystem(std::wstring bundle, AFCConnectionRef* connection);
    HRESULT GetFileInfo(AFCConnectionRef connection, std::string path, FileInfo& info);
    HRESULT ParsePath(std::wstring& path, AFCConnectionRef* pConn);
    void PurgeConnections();
    HRESULT GetDeviceInfo(AFCConnectionRef connection, DeviceInfo* info);

private:
    AMDeviceRef m_handle;
    std::map<std::wstring, AFCConnectionRef> m_connections;
    std::map<std::wstring, std::wstring> m_applications;
    std::wstring m_name;
    std::wstring m_udid;
    GUID m_guid;
};

typedef CComPtr<CDevice> CDevicePtr;

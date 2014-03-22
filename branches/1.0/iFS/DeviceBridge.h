// DeviceBridge.h : Declaration of the CDeviceBridge

#pragma once

struct DeviceData
{
    CComPtr<IDevice> device;
    GUID id;
    std::wstring name;
};

// CDeviceBridge
class CDeviceBridge
{
public:
    HRESULT Init();
    void Term();

    LPSAFEARRAY CreateFileInfoArray();
    HRESULT GetDeviceList(std::vector<DeviceData>& devices);

private:
    CComPtr<IDeviceManager> m_pDeviceManager;
    CComPtr<IRecordInfo> m_pRecInfo;
};

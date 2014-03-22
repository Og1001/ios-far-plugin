// DeviceBridge.cpp : Implementation of CDeviceBridge

#include "stdafx.h"
#include "DeviceBridge.h"


// {A56709AE-2DE2-4E70-BAB9-5AEC8CFFDE29}
DECLARE_GUID(GUID_FileInfo, 0xA56709AE, 0x2DE2, 0x4E70, 0xBA, 0xb9, 0x5a, 0xec, 0x8c, 0xff, 0xde, 0x29);

// CDeviceBridge

HRESULT CDeviceBridge::Init()
{
    HRESULT hr;
    if (!m_pRecInfo)
    {
        CComPtr<ITypeInfo> pTypeInfo;
        CComPtr<ITypeLib> pTypelib;
        if (FAILED(hr = LoadRegTypeLib(LIBID_iDeviceLib, 1, 0, GetUserDefaultLCID(), &pTypelib)) ||
            FAILED(hr = pTypelib->GetTypeInfoOfGuid(GUID_FileInfo, &pTypeInfo)) ||
            FAILED(hr = GetRecordInfoFromTypeInfo(pTypeInfo, &m_pRecInfo)))
        {
            Term();
            return hr;
        }
    }
    if (!m_pDeviceManager)
    {   
        if (FAILED(hr = m_pDeviceManager.CoCreateInstance(CLSID_DeviceManager)))
        {
            Term();
            return hr;
        }
    }
    return S_OK;
}

void CDeviceBridge::Term()
{
    m_pDeviceManager = NULL;
    m_pRecInfo = NULL;
}

LPSAFEARRAY CDeviceBridge::CreateFileInfoArray()
{
    SAFEARRAYBOUND bounds = { 0, 0 };
    return SafeArrayCreateEx(VT_RECORD, 1, &bounds, m_pRecInfo);
}

HRESULT CDeviceBridge::GetDeviceList(std::vector<DeviceData>& devices)
{
    CComPtr<IEnumDevices> pEnum;
    HRESULT hr;
    if (FAILED(hr = m_pDeviceManager->get_Devices(&pEnum)))
    {
        return hr;
    }

    CComPtr<IDevice> devs[10];
    ULONG count = 0;
    while (SUCCEEDED(hr = pEnum->Next(10, (IDevice**)devs, &count)))
    {
        for (ULONG i = 0;i < count;i++)
        {
            DeviceData deviceInfo;
            deviceInfo.device = devs[i];

            CComBSTR name;
            devs[i]->get_Id(&deviceInfo.id);
            devs[i]->get_Name(&name);
            deviceInfo.name = name.m_str;

            devices.push_back(deviceInfo);
        }
        if (hr == S_FALSE)
            break;
    }
    return hr;
}


// DeviceManager.cpp : Implementation of CDeviceManager

#include "stdafx.h"
#include "DeviceManager.h"
#include "Utils.h"
#include "EnumDevices.h"

// CDeviceManager

CDeviceManager::CDeviceManager()
{
    m_hiTunesDLL = NULL;
    m_hCFDLL = NULL;
    m_notification = NULL;
}

HRESULT CDeviceManager::FinalConstruct()
{
    m_cs.Init();
    HRESULT hr = LoadAppleDLLs();

    OSStatus err = AMDeviceNotificationSubscribe(DeviceCallback, 0, 0, this, &m_notification);
    if (err != ERR_SUCCESS)
    {
        return E_FAIL;
    }

    return hr;
}

void CDeviceManager::FinalRelease()
{
    if (m_hiTunesDLL)
        FreeLibrary(m_hiTunesDLL);
    if (m_hCFDLL)
        FreeLibrary(m_hCFDLL);
    if (m_notification)
        AMDeviceNotificationUnsubscribe(m_notification);
    if (!m_appSupportPath.empty())
    {
        wchar_t* path = _wgetenv(L"PATH");
        std::wstring pathVar = path ? path : L"";
        size_t pos = pathVar.find(m_appSupportPath);
        if (pos != std::wstring::npos)
        {
            pathVar.erase(pos - 1, m_appSupportPath.length() + 1);
            _wputenv((std::wstring(L"PATH=") + pathVar).c_str());
        }
        m_appSupportPath.clear();
    }
    m_cs.Term();
}

STDMETHODIMP CDeviceManager::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* const arr[] = 
	{
		&IID_IDeviceManager
	};

	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}


STDMETHODIMP CDeviceManager::get_Devices(IEnumDevices** pVal)
{
    HRESULT hr;
    if (FAILED(hr = CEnumDevices::_CreatorClass::CreateInstance(NULL, IID_IEnumDevices, (void**)pVal)))
        return hr;
    CEnumDevices* p = (CEnumDevices*)*pVal;
    p->Init(this);
    return S_OK;
}

HRESULT CDeviceManager::LoadAppleDLLs()
{
    std::wstring iTunesDLLPath = get_registry_value(L"SOFTWARE\\Apple Inc.\\Apple Mobile Device Support\\Shared", L"iTunesMobileDeviceDLL");
    m_appSupportPath = get_registry_value(L"SOFTWARE\\Apple Inc.\\Apple Application Support", L"InstallDir");

    wchar_t* path = _wgetenv(L"PATH");
    std::wstring pathVar = path ? path : L"";
    if (pathVar.find(m_appSupportPath) == std::wstring::npos)
    {
        if (!pathVar.empty())
            pathVar += std::wstring(L";") + m_appSupportPath;
        _wputenv((std::wstring(L"PATH=") + pathVar).c_str());
    }

    if (iTunesDLLPath.empty() || m_appSupportPath.empty())
    {
        return E_FAIL;
    }
    m_hiTunesDLL = LoadLibrary(iTunesDLLPath.c_str());
    m_hCFDLL = LoadLibrary((m_appSupportPath + L"CoreFoundation.dll").c_str());
    return S_OK;
}

void CDeviceManager::DeviceCallback(am_device_notification_callback_info* info, void* data)
{
    CDeviceManager* manager = (CDeviceManager*)data;
    if (!manager)
        return;
    switch (info->msg)
    {
        case ADNCI_MSG_CONNECTED:
            manager->OnDeviceAdded(info->device);
            break;
        case ADNCI_MSG_DISCONNECTED:
            manager->OnDeviceRemoved(info->device);
            break;
    }
}

void CDeviceManager::OnDeviceAdded(AMDeviceRef handle)
{
    CAutoLock lock(m_cs);
    if (m_devices.find(handle) != m_devices.end())
        return;

    CComPtr<IDevice> pIDevice;
    CDevice::_CreatorClass::CreateInstance(NULL, IID_IDevice, (void**)&pIDevice);
    CDevice* device = (CDevice*)pIDevice.p;
    HRESULT hr = device->Init(handle);

    if (SUCCEEDED(hr))
        m_devices.insert(Devices::value_type(handle, device));
    else
        delete device;
}

void CDeviceManager::OnDeviceRemoved(AMDeviceRef device)
{
    CAutoLock lock(m_cs);
    auto itd = m_devices.find(device);
    if (itd != m_devices.end())
    {
        CDevice* device = itd->second;
        device->Term();
        m_devices.erase(itd);
    }
}

